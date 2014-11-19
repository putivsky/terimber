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
#include "base/string.hpp"
#include "base/common.hpp"
#include "base/vector.hpp"
#include "base/map.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)
/////////////////////////////////////////////////

// 
// constructor
// 
terimber_db_value_vector_impl::terimber_db_value_vector_impl(size_t count)
{
	_value.resize(count);
}

//
// destructor
//
// virtual 
terimber_db_value_vector_impl::~terimber_db_value_vector_impl()
{
}

size_t 
terimber_db_value_vector_impl::get_size() const
{
	return _value.size();
}

// sets condition value
bool
terimber_db_value_vector_impl::set_value_as_null(size_t index, dbtypes type)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_null(db_param_in, type);
}

bool 
terimber_db_value_vector_impl::set_value_as_bool(size_t index, bool val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_bool(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_char(size_t index, sb1_t val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_char(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_byte(size_t index, ub1_t val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_byte(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_short(size_t index, sb2_t val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_short(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_word(size_t index, ub2_t val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_word(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_long(size_t index, sb4_t val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_long(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_dword(size_t index, ub4_t val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_dword(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_float(size_t index, float val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_float(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_double(size_t index, double val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_double(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_long64(size_t index, sb8_t val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_long64(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_dword64(size_t index, ub8_t val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_dword64(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_guid(size_t index, const guid_t& val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_guid(db_param_in, val);
}

bool 
terimber_db_value_vector_impl::set_value_as_numeric(size_t index, const char* val, char delimeter)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_numeric(db_param_in, val, delimeter);
}

bool 
terimber_db_value_vector_impl::set_value_as_decimal(size_t index, const char* val, char delimeter)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_decimal(db_param_in, val, delimeter);
}

bool 
terimber_db_value_vector_impl::set_value_as_date(size_t index, ub4_t year, ub1_t month, ub1_t day, ub1_t hour, ub1_t minute, ub1_t second, ub2_t millisec)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_date(db_param_in, year, month, day, hour, minute, second, millisec);
}

bool 
terimber_db_value_vector_impl::set_value_as_string(size_t index, const char* val, size_t len)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_string(db_param_in, val, len);
}

bool 
terimber_db_value_vector_impl::set_value_as_string_ptr(size_t index, const char* val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_string_ptr(val);
}

bool 
terimber_db_value_vector_impl::set_value_as_wstring(size_t index, const wchar_t* val, size_t len)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_wstring(db_param_in, val, len);
}

bool 
terimber_db_value_vector_impl::set_value_as_wstring_ptr(size_t index, const wchar_t* val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_wstring_ptr(val);
}

// val points to the valid buffer of raw byte of "len" length
bool 
terimber_db_value_vector_impl::set_value_as_binary(size_t index, const ub1_t* val, size_t len)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_binary(db_param_in, val, len);
}

bool 
terimber_db_value_vector_impl::set_value_as_binary_ptr(size_t index, const ub1_t* val)
{
	if (index >= _value.size())
		return false;

	return _value[index].set_as_binary_ptr(val);
}

	// gets values
bool 
terimber_db_value_vector_impl::get_value_is_null(size_t index) const
{
	if (index >= _value.size())
		return false;

	return _value[index]._value.nullVal;
}

bool 
terimber_db_value_vector_impl::get_value_as_bool(size_t index) const
{
	return get_value_as_value(index, vt_bool).boolVal;
}

sb1_t
terimber_db_value_vector_impl::get_value_as_char(size_t index) const
{
	return get_value_as_value(index, vt_sb1).cVal;
}

ub1_t
terimber_db_value_vector_impl::get_value_as_byte(size_t index) const
{
	return get_value_as_value(index, vt_ub1).bVal;
}

sb2_t
terimber_db_value_vector_impl::get_value_as_short(size_t index) const
{
	return get_value_as_value(index, vt_sb2).iVal;
}

ub2_t
terimber_db_value_vector_impl::get_value_as_word(size_t index) const
{
	return get_value_as_value(index, vt_ub2).uiVal;
}

sb4_t
terimber_db_value_vector_impl::get_value_as_long(size_t index) const
{
	return get_value_as_value(index, vt_sb4).lVal;
}

ub4_t
terimber_db_value_vector_impl::get_value_as_dword(size_t index) const
{
	return get_value_as_value(index, vt_ub4).ulVal;
}

float
terimber_db_value_vector_impl::get_value_as_float(size_t index) const
{
	return get_value_as_value(index, vt_float).fltVal;
}

double
terimber_db_value_vector_impl::get_value_as_double(size_t index) const
{
#ifdef OS_64BIT
	return get_value_as_value(index, vt_double).dblVal;
#else
	const double* ret = get_value_as_value(index, vt_double).dblVal;
	return ret ? *ret : 0.0;
#endif
}

sb8_t
terimber_db_value_vector_impl::get_value_as_long64(size_t index) const
{
#ifdef OS_64BIT
	return get_value_as_value(index, vt_sb8).intVal;
#else
	const sb8_t* ret = get_value_as_value(index, vt_sb8).intVal;
	return ret ? *ret : 0;
#endif
}

ub8_t
terimber_db_value_vector_impl::get_value_as_dword64(size_t index) const
{
#ifdef OS_64BIT
	return get_value_as_value(index, vt_ub8).uintVal;
#else
	const ub8_t* ret = get_value_as_value(index, vt_ub8).uintVal;
	return ret ? *ret : 0;
#endif
}

// for high performance we are using reference
bool 
terimber_db_value_vector_impl::get_value_as_guid(size_t index, guid_t& val) const
{
	const guid_t* ret = get_value_as_value(index, vt_guid).guidVal;
	if (!ret)
		return false;

	val = *ret;
	return true;
}

// delimeter defines the desirable delimeter sign
const char*
terimber_db_value_vector_impl::get_value_as_numeric(size_t index, char delimeter) const
{
	const ub1_t* retVal = get_value_as_value(index, vt_numeric).bufVal;

	if(!retVal)
	{
		assert(false);
		return 0;
	}

	numeric num;
	if (!num.parse_orcl(retVal))
	{
		assert(false);
		return 0;
	}

	size_t len = num.is_zero() ? 2 : num.precision() + (num.sign() ? 1 : 0) + (num.scale() ? 1 : 0) + (num.precision() == num.scale()) + 1;
	char* retStr = (char*)_tmp_allocator.allocate(len);
	num.format(retStr, delimeter);
	return retStr;
}

// delimeter defines the desirable delimeter sign
const char*
terimber_db_value_vector_impl::get_value_as_decimal(size_t index, char delimeter) const
{
	return get_value_as_numeric(index, delimeter);
}

// to avoid confusions with different date formats, functions return parsed date
bool 
terimber_db_value_vector_impl::get_value_as_date(size_t index, ub4_t& year, ub1_t& month, ub1_t& day, ub1_t& hour, ub1_t& minute, ub1_t& second, ub2_t& millisec, ub1_t& wday, ub2_t& yday) const
{
#ifdef OS_64BIT
	date dt(get_value_as_value(index, vt_date).intVal);
#else
	const sb8_t* val = get_value_as_value(index, vt_date).intVal;
	if (!val)
		return false;
	date dt(*val);
#endif

	return date::convert_from(dt, year, month, day, hour, minute, second, millisec, wday, yday);
}

// function returns the pointer to string 
const char*
terimber_db_value_vector_impl::get_value_as_string(size_t index) const
{
	return get_value_as_value(index, vt_string).strVal;
}

// function returns the pointer to wide string 
const wchar_t*
terimber_db_value_vector_impl::get_value_as_wstring(size_t index) const
{
	return get_value_as_value(index, vt_wstring).wstrVal;
}


// returns pointer to the raw bytes
// size of bytes is put to len
const ub1_t*
terimber_db_value_vector_impl::get_value_as_binary(size_t index, size_t& len) const
{
	const ub1_t* buf = get_value_as_value(index, vt_binary).bufVal;
	len = buf ? *(size_t*)buf : 0;
	return buf ? buf + sizeof(size_t) : 0;
}

// for reusing only
// the format of buffer is as follows
// first size_t bytes the length of the rest of buffer,
// which contains the raw byte data
const ub1_t*
terimber_db_value_vector_impl::get_value_as_binary_ptr(size_t index) const
{
	return get_value_as_value(index, vt_binary).bufVal;
}


terimber_xml_value
terimber_db_value_vector_impl::get_value_as_value(size_t index, vt_types type) const
{
	try
	{
		if (index >= _value.size()
			|| _value[index]._value.nullVal)
			exception::_throw("Out of range");

		vt_types type_ = dbserver_impl::convert_types(_value[index]._type);
		_tmp_allocator.reset();
		return type_ != type ? parse_value(type, 
										persist_value(type_, _value[index]._value.val, &_tmp_allocator), 
										0xffffffff, 
										&_tmp_allocator) : _value[index]._value.val;
	}
	catch (...)
	{
		terimber_xml_value val;
		memset(&val, 0, sizeof(terimber_xml_value));
		return val;
	}
}

//////////////////////////////////////////////////////////////
memlookup::memlookup(memindex& parent) :
	_parent(parent),
	_low_bounder(parent.get_index().end()),
	_upper_bounder(parent.get_index().end()),
	_current_iter(parent.get_index().end())
{
	memdb_row dummy_row;
	_condition_rowset.push_back(dummy_row);
	memdb_rowset_t::iterator iter = _condition_rowset.begin();
	// resizes according to the index length
	const memdb_rowset_less& pred = _parent.get_index().comp();
	size_t length = pred.get_info().size();
	iter->_row.resize(_condition_allocator, length);
	// resets null flag
	for (size_t i = 0; i < length; ++i)
		iter->_row[i].nullVal = true;

	iter->_status = status_lookup;
}

memlookup::~memlookup()
{
}

bool 
memlookup::construct(const terimber_db_value_vector_impl* info)
{
	// checks sizes
	const memdb_rowset_less& pred = _parent.get_index().comp();
	size_t size = info ? info->get_size() : 0;

	if (size > pred.get_info().size())
		return false;

	if (size)
	{
		// creates temporary map
		_tmp_allocator.reset();
		
		memdb_rowset_t::iterator iter = _condition_rowset.begin();
		
		string_t error;

		if (!copy_db_row(info, *iter, pred.get_info(), size, _tmp_allocator, error))
			return false;

		memdb_index_t::paircc_t bounders = _parent.get_index().equal_range(iter);
		_low_bounder = bounders.first;
		_current_iter = _upper_bounder = bounders.second;
	}
	else
	{
		// no conditions
		_low_bounder = _parent.get_index().begin();
		_current_iter = _upper_bounder = _parent.get_index().end();
	}

	return true;
}

// copy method
// lookups must be created for the same index
//
bool 
memlookup::assign(const terimber_memlookup& x)
{
	// checks parents
	const memlookup& that = static_cast< const memlookup& >(x);

	if (&_parent != &that._parent)
		return false;

	_low_bounder = that._low_bounder;
	_upper_bounder = that._upper_bounder;
	_current_iter = that._current_iter;
	return true;
}

//
// sets lookup outside the row sequence
// and initializes the selection criteria
//
bool 
memlookup::reset(const terimber_db_value_vector* info)
{
	return construct(static_cast< const terimber_db_value_vector_impl* >(info));
}

// 
// tries to find the next row
// according to lookup values
//
bool 
memlookup::next() const
{
	// tries to find next
	// first search
	if (_current_iter == _upper_bounder)
		_current_iter = _low_bounder; // resets to the beginning
	else
		++_current_iter; // moves to the next position

	// returns result
	return _current_iter != _upper_bounder;
}

// 
// tries to find the previous row
// according to lookup values
//
bool 
memlookup::prev() const
{
		// tries to find next
	// first search
	if (_current_iter == _low_bounder)
		_current_iter = _upper_bounder; // resets to the beginning
	else
		--_current_iter; // moves to the next position

	// returns result
	return _current_iter != _upper_bounder;
}

//
// access to the row
// this is pretty much the same as db_server provides.
//

//
// 
//

// returns the value of the column at the current row position
// if the index is out of range then zero will be returned
// if type is mismatched then attempt to convert will be made
// the convertion of the internal value is made on the temporary allocator
// the returned pointer is valid until the next call
// the best practice is to check the type before call gets value functions
bool 
memlookup::get_value_is_null(size_t index) const
{
	if (_current_iter == _upper_bounder
		|| index >= _parent.get_table().get_column_count())
		return false;

	return (*_current_iter)->_row[index].nullVal;
}

bool 
memlookup::get_value_as_bool(size_t index) const
{
	return get_value_as_value(index, vt_bool).boolVal;
}

sb1_t
memlookup::get_value_as_char(size_t index) const
{
	return get_value_as_value(index, vt_sb1).cVal;
}

ub1_t 
memlookup::get_value_as_byte(size_t index) const
{
	return get_value_as_value(index, vt_ub1).bVal;
}

sb2_t
memlookup::get_value_as_short(size_t index) const
{
	return get_value_as_value(index, vt_sb2).iVal;
}

ub2_t
memlookup::get_value_as_word(size_t index) const
{
	return get_value_as_value(index, vt_ub2).uiVal;
}

sb4_t
memlookup::get_value_as_long(size_t index) const
{
	return get_value_as_value(index, vt_sb4).lVal;
}

ub4_t
memlookup::get_value_as_dword(size_t index) const
{
	return get_value_as_value(index, vt_ub4).ulVal;
}

float
memlookup::get_value_as_float(size_t index) const
{
	return get_value_as_value(index, vt_float).fltVal;
}

double
memlookup::get_value_as_double(size_t index) const
{
#ifdef OS_64BIT
	return get_value_as_value(index, vt_double).dblVal;
#else
	const double* ret = get_value_as_value(index, vt_double).dblVal;
	return ret ? *ret : 0.0;
#endif
}

sb8_t
memlookup::get_value_as_long64(size_t index) const
{
#ifdef OS_64BIT
	return get_value_as_value(index, vt_sb8).intVal;
#else
	const sb8_t* ret = get_value_as_value(index, vt_sb8).intVal;
	return ret ? *ret : 0;
#endif
}

ub8_t
memlookup::get_value_as_dword64(size_t index) const
{
#ifdef OS_64BIT
	return get_value_as_value(index, vt_ub8).uintVal;
#else
	const ub8_t* ret = get_value_as_value(index, vt_ub8).uintVal;
	return ret ? *ret : 0;
#endif
}

// for high performance we are using reference
bool 
memlookup::get_value_as_guid(size_t index, guid_t& val) const
{
	const guid_t* ret = get_value_as_value(index, vt_guid).guidVal;
	if (!ret)
		return false;

	val = *ret;
	return true;
}

// delimeter defines the desirable delimeter sign
const char*
memlookup::get_value_as_numeric(size_t index, char delimeter) const
{
	const ub1_t* retVal = get_value_as_value(index, vt_numeric).bufVal;

	if(!retVal)
	{
		assert(false);
		return 0;
	}

	numeric num;
	if (!num.parse_orcl(retVal))
	{
		assert(false);
		return 0;
	}

	size_t len = num.is_zero() ? 2 : num.precision() + (num.sign() ? 1 : 0) + (num.scale() ? 1 : 0) + (num.precision() == num.scale()) + 1;
	char* retStr = (char*)_tmp_allocator.allocate(len);
	num.format(retStr, delimeter);
	return retStr;
}

// delimeter defines the desirable delimeter sign
const char*
memlookup::get_value_as_decimal(size_t index, char delimeter) const
{
	return get_value_as_numeric(index, delimeter);
}

// to avoid confusions with different date formats, functions return parsed date
bool 
memlookup::get_value_as_date(size_t index, ub4_t& year, ub1_t& month, ub1_t& day, ub1_t& hour, ub1_t& minute, ub1_t& second, ub2_t& millisec, ub1_t& wday, ub2_t& yday) const
{
#ifdef OS_64BIT
	date dt(get_value_as_value(index, vt_date).intVal);
#else
	const sb8_t* val = get_value_as_value(index, vt_date).intVal;
	if (!val)
		return false;
	date dt(*val);
#endif

	return date::convert_from(dt, year, month, day, hour, minute, second, millisec, wday, yday);
}

// function returns the pointer to string 
const char*
memlookup::get_value_as_string(size_t index) const
{
	return get_value_as_value(index, vt_string).strVal;
}

// function returns the pointer to wide string 
const wchar_t*
memlookup::get_value_as_wstring(size_t index) const
{
	return get_value_as_value(index, vt_wstring).wstrVal;
}


// returns pointer to the raw bytes
// size of bytes is put to len
const ub1_t*
memlookup::get_value_as_binary(size_t index, size_t& len) const
{
	const ub1_t* buf = get_value_as_value(index, vt_binary).bufVal;
	len = buf ? *(size_t*)buf : 0;
	return buf ? buf + sizeof(size_t) : 0;
}

// for reusing only
// the format of buffer is as follows
// first size_t bytes the length of the rest of buffer,
// which contains the raw byte data
const ub1_t*
memlookup::get_value_as_binary_ptr(size_t index) const
{
	return get_value_as_value(index, vt_binary).bufVal;
}

terimber_xml_value
memlookup::get_value_as_value(size_t index, vt_types type) const
{
	try
	{
		if (_current_iter == _upper_bounder
			|| index >= _parent.get_table().get_column_count())
			exception::_throw("Out of range");

		vt_types type_ = dbserver_impl::convert_types(_parent.get_table().get_column_type(index));
		_tmp_allocator.reset();
		return type_ != type ? parse_value(type, 
										persist_value(type_,(*_current_iter)->_row[index].val, &_tmp_allocator), 
										0xffffffff, 
										&_tmp_allocator) : (*_current_iter)->_row[index].val;
	}
	catch (...) 
	{
		terimber_xml_value val;
		memset(&val, 0, sizeof(terimber_xml_value));
		return val;
	}
}

//
// row modificators
// the current row can be changed
// there are possible scenarios
// 1. inserts the new row - it will become the current one
// 2. updates the current row - it will be the same current row, even index columns are affected
// 3. deletes the current row - the current row will the next to the current one or current will point to the out of row sequence
//

//
// inserts new row
// if the row is modified it will still have a status = new
// if the row is removed it is just deleted from recordset
// the inserted row has all columns' value as null
//
bool 
memlookup::insert_row(const terimber_db_value_vector* info)
{
	// 1. inserts new row
	// 2. updates indexes
	// 3. updates lookups
	return _parent.get_table().insert_row(info);
}


// 
// deletes existing row
// if the row was in the original recordset (modified or not), it will have a status = delete, and will not be accessible anymore
// deleting of new row will just remove row from recordset
//
bool 
memlookup::delete_row()
{
	// here is algorithm
	// 1. marks row as deleted
	// 2. notifies all indexes
	// 3. notifies all lookups
	if (_current_iter == _upper_bounder)
		return false;

	return _parent.get_table().delete_row(*_current_iter);
}

//
// updates the column value
// updates of new row will preserve status = new
// updates of original row will change status from original to update.
//
bool 
memlookup::update_row(const terimber_db_value_vector* info)
{
	// here is the algorithm
	// 1. marks row as deleted
	// 2. notifies all indexes
	// 3. notifies all lookups
	if (_current_iter == _upper_bounder)
		return false;

	return _parent.get_table().update_row(*_current_iter, info);
}


//
// returns the current status
// user can't get the status if row is already deleted
// however, if the lookup is created from an empty index, then all rows are available to see
// including deleted ones
// 
terimber_db_row_status
memlookup::get_row_status()
{
	if (_current_iter == _upper_bounder)
		return status_deleted;

	return (*_current_iter)->_status;
}

void 
memlookup::notify(memdb_index_citer_t iter, bool insert_or_delete)
{
	memdb_rowset_t::const_iterator conditer = _condition_rowset.begin();
	memdb_index_t::paircc_t bounders = _parent.get_index().equal_range(conditer);
	memdb_index_citer_t curr = _low_bounder = bounders.first;
	_upper_bounder = bounders.second;


	// tries to find the old current
	if (curr != _parent.get_index().end())
	{
		while (curr != _upper_bounder)
			if (_current_iter == curr)
				return;
			else
				++curr;
	}

	_current_iter = _upper_bounder;
}


#pragma pack()
END_TERIMBER_NAMESPACE
