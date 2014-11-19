/*
 * The Software License
 * =================================================================================
 * Copyright (c) 2003-2010 The Terimber Corporation. All rights reserved.
 * =================================================================================
 * Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution.
 * The end-user documentation included with the redistribution, if any, 
 * must include the following acknowledgment:
 * "This product includes software developed by the Terimber Corporation."
 * =================================================================================
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL THE TERIMBER CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ================================================================================
*/

#include "memdb/memdb.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/vector.hpp"
#include "base/memory.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

///////////////////////////////////
memindex::memindex(memtable& parent, const memdb_rowset_less& pred) : 
	_parent(parent),
	_index(pred)
{
}

memindex::~memindex()
{
	mutexKeeper guard(_mtx);
	for (list_lookups_t::iterator iter = _lookups.begin(); iter != _lookups.end(); ++iter)
		delete *iter;

	_lookups.clear();
}

bool 
memindex::construct()
{
	// looks up thought the rowset and constructs index
	for (memdb_rowset_citerator_t iter = _parent.get_rowset().begin(); iter != _parent.get_rowset().end(); ++iter)
		_index.insert(iter, iter);

	return true;
}

terimber_memlookup*
memindex::add_lookup(const terimber_db_value_vector* info)
{
	memlookup* obj = new memlookup(*this);

	if (obj)
	{
		if (!obj->construct(static_cast< const terimber_db_value_vector_impl* >(info)))
		{
			delete obj;
			return 0;
		}

		// adds to the list of indexes
		mutexKeeper guard(_mtx);
		_lookups.push_back(obj);
	}

	obj->log_on(this);
	return obj;
}

//
// destroys lookup after using
//
bool
memindex::remove_lookup(terimber_memlookup* obj)
{
	mutexKeeper guard(_mtx);
	for (list_lookups_t::iterator iter = _lookups.begin(); iter != _lookups.end(); ++iter)
		if (*iter == obj)
		{
			(*iter)->log_on(0);
			delete *iter;
			_lookups.erase(iter);
			return true;
		}

	return false;
}

//
// parent table send notification
//
void 
memindex::notify(memdb_rowset_citerator_t iter, bool insert_or_delete)
{
	if (insert_or_delete)
	{
		// notifies all lookups
		memdb_index_citer_t index_iter = _index.insert(iter, iter).first;
		mutexKeeper guard(_mtx);
		for (list_lookups_t::iterator liter = _lookups.begin(); liter != _lookups.end(); ++liter)
			(*liter)->notify(index_iter, true);
	}
	else
	{
		// notify all lookups
		memdb_index_t::pairii_t range = _index.equal_range(iter);
		if (range.first != _index.end())
		{
			while (range.first != range.second)
			{
				if (range.first.key() == iter)
				{
					mutexKeeper guard(_mtx);
					for (list_lookups_t::iterator liter = _lookups.begin(); liter != _lookups.end(); ++liter)
						(*liter)->notify(range.first, false);

					// erases
					_index.erase(range.first);
					break;
				}
				else
					++range.first;
			}
		}
	}
}

#pragma pack()
END_TERIMBER_NAMESPACE
