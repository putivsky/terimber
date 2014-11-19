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

#include "fuzzy/fuzzyimpl.h"
#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/common.hpp"
#include "base/map.hpp"
#include "base/vector.hpp"
#include "base/stack.hpp"
#include "base/string.hpp"
#include "base/date.h"
#include "base/keymaker.h"
#include "smart/byterep.hpp"

#include "fuzzyphonetic.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

class fuzzy_wrapper_impl : public fuzzy_wrapper
{
public:
	fuzzy_wrapper_impl(size_t memory_usage);
	~fuzzy_wrapper_impl();

	virtual bool add(const char* phrase);
	
	// removes previously added ngram
	// caller can remove the same n-gram many times unless the reference count goes to zero
	virtual bool remove(const char* phrase);

	// does the fuzzy match
	virtual bool match(	ngram_quality nq,
						phonetic_quality fq,
						const char* phrase, 
						linked_result& suggestions) const;
private:
	keylocker						_key;
	mutex							_mtx;
	mutable list< byte_allocator* >	_all_container;
	fuzzy_matcher_impl				_matcher;
};

#pragma pack()
END_TERIMBER_NAMESPACE


fuzzy_wrapper*
fuzzy_wrapper_factory::get_fuzzy_wrapper(size_t memory_usage)
{
	return new TERIMBER::fuzzy_wrapper_impl(memory_usage);
}

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

const size_t all_default_size = 1024*64;

fuzzy_wrapper_impl::fuzzy_wrapper_impl(size_t memory_usage) : _matcher(memory_usage)
{
}

fuzzy_wrapper_impl::~fuzzy_wrapper_impl()
{
	while (!_all_container.empty())
	{
		delete _all_container.front();
		_all_container.pop_front();
	}
}

// virtual 
bool 
fuzzy_wrapper_impl::add(const char* phrase)
{
	keylockerWriter locker(_key);
	byte_allocator* tmp = 0;

	mutexKeeper keeper(_mtx);
	if (_all_container.empty())
		tmp = new byte_allocator(all_default_size);
	else
	{
		tmp = _all_container.front();
		_all_container.pop_front();
	}

	if (!tmp)
		return false;

	keeper.unlock();

	size_t ident = _matcher.add(phrase, *tmp);

	keeper.lock();
	_all_container.push_front(tmp);

	return ident != 0;
}
	
// removes previously added ngram
// caller can remove the same n-gram many times unless the reference count goes to zero
// virtual 
bool 
fuzzy_wrapper_impl::remove(const char* phrase)
{
	keylockerWriter locker(_key);
	byte_allocator* tmp = 0;

	mutexKeeper keeper(_mtx);
	if (_all_container.empty())
		tmp = new byte_allocator(all_default_size);
	else
	{
		tmp = _all_container.front();
		_all_container.pop_front();
	}

	if (!tmp)
		return false;

	keeper.unlock();

	bool res = _matcher.remove(phrase, *tmp);

	keeper.lock();
	_all_container.push_front(tmp);

	return res;

}

// does the fuzzy match
// virtual 
bool 
fuzzy_wrapper_impl::match(	ngram_quality nq,
						phonetic_quality pq,
						const char* phrase, 
						linked_result& suggestions) const
{
	keylockerReader locker(_key);

	byte_allocator* tmp = 0;
	byte_allocator* all = 0;

	mutexKeeper keeper(_mtx);
	if (_all_container.empty())
		tmp = new byte_allocator(all_default_size);
	else
	{
		tmp = _all_container.front();
		_all_container.pop_front();
	}

	if (_all_container.empty())
		all = new byte_allocator(all_default_size);
	else
	{
		all = _all_container.front();
		_all_container.pop_front();
	}

	if (!tmp || !all)
		return false;

	keeper.unlock();

	_list< const char* > candidates; 
	bool res = _matcher.match(nq, pq, phrase, *all, *tmp, candidates);

	linked_result* curr = 0;

	if (res)
	{
		for (_list< const char* >::const_iterator it = candidates.begin(); it != candidates.end(); ++it)
		{
			if (!curr)
			{
				curr = &suggestions;
			}
			else
			{
                linked_result* next = new linked_result();
				if (!next)
					break;

				curr->_next = next;
				curr = next;
			}

			size_t len = (*it) ? strlen(*it) : 0;
			if (len)
			{
				// makes a copy
				char* str = new char[len + 1];
				if (!str)
					break;

				memcpy(str, *it, len + 1);
				curr->_str = str;
			}
		}
	}

	keeper.lock();

	_all_container.push_front(tmp);
	_all_container.push_front(all);


	return res;
}

#pragma pack()
END_TERIMBER_NAMESPACE
