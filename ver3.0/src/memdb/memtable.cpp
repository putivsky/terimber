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
#include "base/memory.hpp"
#include "base/vector.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

////////////////////////////////////////////////////////////////

memtable::memtable()
{
}

//
// destructor
// user is responsible for destoying memtable object
//
memtable::~memtable()
{
	uninit();
}

void 
memtable::uninit()
{
	mutexKeeper guard(_mtx);
	for (list_indexes_t::iterator iiter = _indexes.begin(); iiter != _indexes.end(); ++iiter)
		delete *iiter;

	_indexes.clear();

	for (list_values_t::iterator viter = _values.begin(); viter != _values.end(); ++viter)
		delete *viter;

	_values.clear();

	_cols.clear();
	_rowset.clear();
	_allocator.clear_all();
}

//
// returns the last occured error
//
const char*
memtable::get_last_error()
{
	return _error;
}

// 
// creates an internal table structure from @server object
// db_server has to be after executing query or stored procedure
// and ready to fetch data
// memtable automatically detects the list of columns in recordset
// and creates table in memory with the same columns' order and types.
// @max_rows sets a restriction on max rows to be retrived, -1 - all of them
// @start_row means absolute number of row to start with
// unless you specify 0, then the memtable starts with the current row

bool
memtable::populate(dbserver* server, size_t start_row, size_t max_rows)
{
	uninit();

	// fetches here rows
	if (!server->fetch_data(false, start_row, max_rows, true))
	{
		_error = server->get_error();
		return false;
	}

	// extracts columns here
	size_t col_count = server->get_column_count();
	if (!col_count)
	{
		_error = "no columns found";
		return false;
	}

	binder dummy;
	_cols.resize(_allocator, col_count, dummy);
	if (_cols.size() != col_count)
		return false;

	for (size_t icol = 0; icol < col_count; ++icol)
	{
		_cols[icol]._type = server->get_column_type(icol);
		_cols[icol]._max_length = server->get_column_max_length(icol);
		_cols[icol]._value.nullVal = server->get_column_nullable(icol);
		_cols[icol]._precision = server->get_column_precision(icol);
		_cols[icol]._scale = server->get_column_scale(icol);
		_cols[icol].set_name(&_allocator, server->get_column_name(icol));
	}

	while (server->next())
	{
		// inserts new row
		memdb_row dummy_row;
		_rowset.push_back(dummy_row);
		memdb_row& row = _rowset.back();
		// resizes row, room for columns
		row._row.resize(_allocator, col_count);
		row._status = status_original;

		if (!copy_db_row(server, row, _cols, _cols.size(), _allocator, _error))
			return false;

	} // while

	return true;
}

bool 
memtable::create(size_t columns, const terimber_table_column_desc desc[])
{
	uninit();
	
	binder dummy;
	_cols.resize(_allocator, columns, dummy);
	if (_cols.size() != columns)
		return false;

	for (size_t icol = 0; icol < columns; ++icol)
	{
		_cols[icol]._type = desc[icol]._type;
		_cols[icol]._max_length = desc[icol]._max_length;
		_cols[icol]._value.nullVal = desc[icol]._nullable;
		_cols[icol]._precision = desc[icol]._precision;
		_cols[icol]._scale = desc[icol]._scale;
		_cols[icol].set_name(&_allocator, desc[icol]._name);
	}

	return true;
}

// resets all new, updates rows status to original, removes deleted rows
// updates all indexes
void 
memtable::refresh()
{
	// updates status
	for (memdb_rowset_t::iterator it = _rowset.begin(); it != _rowset.end(); ++it)
	{
		it->_status = status_original;
	}
}

//
// after populating and any time after that you can request the number of rows in the table
//
size_t 
memtable::get_row_count()
{
	return _rowset.size();
}

//
// column's description is pretty much the same as db_server provides
//

//
// gets column count
//
size_t 
memtable::get_column_count() const
{
	return _cols.size();
}

//
// returns the virtual terimber dbtype like db_bool, ...
//
dbtypes
memtable::get_column_type(size_t index) const
{
	return index >= _cols.size() ? db_unknown : _cols[index]._type;
}

//
// returns the name of the column
//
const char*
memtable::get_column_name(size_t index) const
{
	return index >= _cols.size() ? 0 : _cols[index]._name;
}

//
// returns the nullable - database specific
//
bool 
memtable::get_column_nullable(size_t index) const
{
	return index >= _cols.size() ? false : _cols[index]._value.nullVal;
}

//
// returns the scale
//
size_t 
memtable::get_column_scale(size_t index) const
{
	return index >= _cols.size() ? 0 : _cols[index]._scale;
}
//
// returns the precision
//
size_t 
memtable::get_column_precision(size_t index) const
{
	return index >= _cols.size() ? 0 : _cols[index]._precision;
}

//
// returns the length of the column (only for fixed length columns)
//
size_t 
memtable::get_column_max_length(size_t index) const
{
	return index >= _cols.size() ? 0 : _cols[index]._max_length;
}

//
// index management
// there are no connections between the indexes in the memory and the indexes in the database
//

//
// creates an index to perform the quick search
// @columns the size of index, can be 0 for empty index
// @info - array of index column descriptions
// returns the new created memindex object
// user mustn't to use object after returning it back
// or after memtable will be destoyed
// this is an expensive operation in order to consume memory and CPU time
//
terimber_memindex* 
memtable::add_index(size_t columns, const terimber_index_column_info info[])
{
	terimber_index_column_array_t vec_info;
	vec_info.resize(columns);

	// checks column's count
	if (columns > _cols.size())
	{
		_error = "info size exceeds the number of available columns";
		return 0;
	}

	for (size_t icol = 0; icol < columns; ++icol)
	{
		vec_info[icol]._asc_sort = info[icol]._asc_sort;
		vec_info[icol]._case_insensitive = info[icol]._case_insensitive;
		vec_info[icol]._index = info[icol]._index;

		if (vec_info[icol]._index >= _cols.size())
		{
			char buf[128];
			str_template::strprint(buf, 128, "specified index %d is out of range", vec_info[icol]._index);
			_error = buf;
			return 0;
		}

		vec_info[icol]._type = _cols[vec_info[icol]._index]._type;
	}

	memdb_rowset_less pred(vec_info);

	memindex* obj = new memindex(*this, pred);
	if (obj)
	{
		// constructs index
		if (!obj->construct())
		{
			delete obj;
			return 0;
		}

		// adds to the list of indexes
		mutexKeeper guard(_mtx);
		_indexes.push_back(obj);
	}

	obj->log_on(this);

	return obj;
}


//
// destroys index
// memtable data is still valid
// all memlookups on the current index will be destroyed! 
//
bool 
memtable::remove_index(terimber_memindex* obj)
{
	mutexKeeper guard(_mtx);
	for (list_indexes_t::iterator iter = _indexes.begin(); iter != _indexes.end(); ++iter)
		if (*iter == obj)
		{
			(*iter)->log_on(0);
			delete *iter;
			_indexes.erase(iter);
			return true;
		}

	return false;
}

terimber_db_value_vector*
memtable::allocate_db_values(size_t count)
{
	// adds to the list of values
	terimber_db_value_vector_impl* obj = new terimber_db_value_vector_impl(count);
	if (obj)
	{
		mutexKeeper guard(_mtx);
		_values.push_back(obj);
	}

	return obj;
}

void
memtable::destroy_db_values(terimber_db_value_vector* obj)
{
	mutexKeeper guard(_mtx);
	for (list_values_t::iterator iter = _values.begin(); iter != _values.end(); ++iter)
		if (*iter == obj)
		{
			delete *iter;
			_values.erase(iter);
			return;
		}
}

////////////////////////////////////////
bool 
memtable::insert_row(const terimber_db_value_vector* info)
{
	size_t col_count = _cols.size();

	if (info->get_size() != col_count)
	{
		_error = "Out of range";
		return false;
	}

	// inserts new row
	memdb_row dummy_row;
	_rowset.push_back(dummy_row);
	memdb_row& row = _rowset.back();
	// resizes row, room for columns
	row._row.resize(_allocator, col_count);
	row._status = status_new;

	if (!copy_db_row(info, row, _cols,  _cols.size(), _allocator, _error))
		return false;

	memdb_rowset_citerator_t new_iter = --_rowset.end();

	// notify all indexes
	mutexKeeper guard(_mtx);
	for (list_indexes_t::iterator iter = _indexes.begin(); iter != _indexes.end(); ++iter)
		(*iter)->notify(new_iter, true);

	return true;
}

bool 
memtable::update_row(memdb_rowset_citerator_t iter, const terimber_db_value_vector* info)
{
	// notifies all indexes
	mutexKeeper guard(_mtx);
	
	for (list_indexes_t::iterator diter = _indexes.begin(); diter != _indexes.end(); ++diter)
		(*diter)->notify(iter, false);

	// updates row
	memdb_rowset_iterator_t uiter(iter.node());

	uiter->_status = status_new;

	if (!copy_db_row(info, *uiter, _cols,  _cols.size(), _allocator, _error))
		return false;

	switch (uiter->_status)
	{
		case status_new:
			break;
		case status_updated:
			break;
		case status_original:
			uiter->_status = status_updated;
			break;
		case status_deleted:
			assert(false);
	}

	// notifies all indexes
	for (list_indexes_t::iterator niter = _indexes.begin(); niter != _indexes.end(); ++niter)
		(*niter)->notify(uiter, true);

	return true;
}

bool 
memtable::delete_row(memdb_rowset_citerator_t iter)
{
	// notifies all indexes
	mutexKeeper guard(_mtx);
	for (list_indexes_t::iterator diter = _indexes.begin(); diter != _indexes.end(); ++diter)
		(*diter)->notify(iter, false);

	// remove
	memdb_rowset_iterator_t uiter(iter.node());

	switch (uiter->_status)
	{
		case status_new:
			_rowset.erase(uiter);
			break;
		case status_updated:
			break;
		case status_original:
			uiter->_status = status_deleted;
			break;
		case status_deleted:
			assert(false);
	}

	return true;
}


#pragma pack()
END_TERIMBER_NAMESPACE
