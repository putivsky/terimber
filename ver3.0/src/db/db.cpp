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

#include "db/db.hpp"
#include "base/list.hpp"
#include "base/vector.hpp"
#include "base/memory.hpp"
#include "base/template.hpp"
#include "base/common.hpp"
#include "base/string.hpp"
#include "base/numeric.h"
#include "alg/algorith.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// exception map
exception_item dbMsgs[] =
{
{ex_invalid, "Module is invalid"},
{ex_not_init, "Server is not initialized"},
{ex_already_init, "Server is already initialized"},
{ex_not_connect, "Database is not connected"},
{ex_already_connect, "Database is already connected"},
{ex_not_transaction, "Database is not in transaction"},
{ex_already_transaction, "Database is already in transaction"},
{ex_not_sql_open, "Query is not open"},
{ex_already_sql_open, "Query is already open"},
{ex_sql_in_action, "Query is working"},
{ex_bad_pointer, "NULL input parameter pointer"},
{ex_invalid_param_count, "Incorrect number of parameters"},
{ex_timeout, "Timeout occured"},
{ex_unknown_server_type, "Unknown server type"},
{ex_out_of_bounders, "Index is out of bounders"},
{0, 0},
};

exception_table dbMsgTable(dbMsgs);


//////////////////////////////////////////////////////
binder::~binder()
{
	deallocate_value();
}

binder::binder() : 
	_in_out(db_param_unknown),
	_type(db_unknown),
	_name(0),
	_scale(0),
	_precision(0),
	_max_length(0),
	_bind_type(0),
	_native_type(0),
	_real_length(0),
	_bind_buffer(0),
	_user_code(0),
	_responsible(false)
{
	_value.nullVal = true;
	memset(&_value.val, 0, sizeof(terimber_xml_value));
}

//
// full copy with memory allocation
//
binder::binder(const binder& x) : 
	_in_out(db_param_unknown),
	_type(db_unknown),
	_name(0),
	_scale(0),
	_precision(0),
	_max_length(0),
	_bind_type(0),
	_native_type(0),
	_real_length(0),
	_bind_buffer(0),
	_user_code(0),
	_responsible(false)
{
	x.clone(*this);
}

binder& 
binder::operator=(const binder& x)
{
	deallocate_value();
	x.clone(*this);
	return *this;
}

void
binder::set_name(byte_allocator* data_allocator, const char* x)
{
	_name = copy_string(x, *data_allocator, os_minus_one);
}


void* 
binder::allocate_value(size_t n)
{
	_responsible = true;
	switch (_type)
	{
		case db_unknown:
		case db_bool:
		case db_sb1:
		case db_ub1:
		case db_sb2:
		case db_ub2:
		case db_sb4:
		case db_ub4:
		case db_float:
			assert(false);
			return 0;
#ifdef OS_64BIT
		case db_double:
		case db_sb8:
		case db_ub8:
		case db_date:
			assert(false);
			return 0;
#else
		case db_double:
			return (double*)check_pointer((void*)(_value.val.dblVal = new double));
		case db_date:
		case db_sb8:
			return (sb8_t*)check_pointer((void*)(_value.val.intVal = new sb8_t));
		case db_ub8:
			return (ub8_t*)check_pointer((void*)(_value.val.uintVal = new ub8_t));
#endif
		case db_string:
			{
				char* retVal = (char*)check_pointer((void*)(_value.val.strVal = new char[n + 1]));
				retVal[n] = 0;
				return retVal;
			}
		case db_wstring:
			{
				wchar_t* retVal = (wchar_t*)check_pointer((void*)(_value.val.wstrVal = new wchar_t[n + 1]));
				retVal[n] = 0;
				return retVal;
			}
		case db_decimal:
		case db_numeric:
			return (ub1_t*)check_pointer((void*)(_value.val.bufVal = new ub1_t[n]));
		case db_binary:
			{
				ub1_t* retVal = (ub1_t*)check_pointer((void*)(_value.val.bufVal = new ub1_t[n + sizeof(size_t)]));
				*(size_t*)retVal = n;
				return retVal + sizeof(size_t);
			}
		case db_guid:
			return (guid_t*)check_pointer((void*)(_value.val.guidVal = new guid_t));
		default:
			assert(false);
	} // switch

	return 0;
}

void 
binder::deallocate_value()
{
	if (!_responsible)
		return;

	switch (_type)
	{
		case db_unknown:
			return;
		case db_bool:
		case db_sb1:
		case db_ub1:
		case db_sb2:
		case db_ub2:
		case db_sb4:
		case db_ub4:
		case db_float:
			//asserts(false);
			break;
#ifdef OS_64BIT
		case db_double:
		case db_sb8:
		case db_ub8:
		case db_date:
			//asserts(false);
			break;
#else
		case db_double:
			if (_value.val.dblVal)
			{
				delete (double*)_value.val.dblVal;
				_value.val.dblVal = 0;
			}
			break;
		case db_date:
		case db_sb8:
			if (_value.val.intVal)
			{
				delete (sb8_t*)_value.val.intVal;
				_value.val.intVal = 0;
			}
			break;
		case db_ub8:
			if (_value.val.uintVal)
			{
				delete (ub8_t*)_value.val.uintVal;
				_value.val.uintVal = 0;
			}
			break;
#endif
		case db_string:
			if (_value.val.strVal)
			{
				delete [] (char*)_value.val.strVal;
				_value.val.strVal = 0;
			}
			break;
		case db_wstring:
			if (_value.val.wstrVal)
			{
				delete [] (wchar_t*)_value.val.wstrVal;
				_value.val.wstrVal = 0;
			}
			break;
		case db_decimal:
		case db_numeric:
		case db_binary:
			if (_value.val.bufVal)
			{
				delete [] (ub1_t*)_value.val.bufVal;
				_value.val.bufVal = 0;
			}
			break;
		case db_guid:
			if (_value.val.guidVal)
			{
				delete (guid_t*)_value.val.guidVal;
				_value.val.guidVal = 0;
			}
			break;
		default:
			assert(false);
	} // switch

	_responsible = false;
}

void 
binder::clone(binder& out) const
{
	out._in_out = _in_out;
	out._type = _type;
	out._name = _name;
	out._scale = _scale;
	out._precision = _precision;
	out._max_length = _max_length;
	out._bind_type = _bind_type;
	out._native_type = _native_type;
	out._real_length = _real_length;
	out._bind_buffer = _bind_buffer;
	out._user_code = _user_code;
	out._responsible = _responsible;
	out._value.nullVal = _value.nullVal;
	memset(&out._value.val, 0, sizeof(out._value.val));

	if (_responsible && !_value.nullVal) // makes a copy
	{
		switch (_type)
		{
			case db_unknown:
				return;
			case db_bool:
			case db_sb1:
			case db_ub1:
			case db_sb2:
			case db_ub2:
			case db_sb4:
			case db_ub4:
			case db_float:
				out._value.val = _value.val;
				break;
	#ifdef OS_64BIT
			case db_double:
			case db_sb8:
			case db_ub8:
			case db_date:
				out._value.val = _value.val;
				break;
	#else
			case db_double:
				if (_value.val.dblVal)
					*(double*)check_pointer((void*)(out._value.val.dblVal = new double)) = *_value.val.dblVal;
				break;
			case db_date:
			case db_sb8:
				if (_value.val.intVal)
					*(sb8_t*)check_pointer((void*)(out._value.val.intVal = new sb8_t)) = *_value.val.intVal;
				break;
			case db_ub8:
				if (_value.val.uintVal)
					*(ub8_t*)check_pointer((void*)(out._value.val.uintVal = new ub8_t)) = *_value.val.uintVal;
				break;
	#endif
			case db_string:
				if (_value.val.strVal)
				{
					size_t n = str_template::strlen(_value.val.strVal);
					char* ret = (char*)check_pointer((void*)(out._value.val.strVal = new char[n + 1]));
					memcpy(ret, _value.val.strVal, n + 1);
				}
				break;
			case db_wstring:
				{
					size_t n = str_template::strlen(_value.val.wstrVal);
					wchar_t* ret = (wchar_t*)check_pointer((void*)(out._value.val.wstrVal = new wchar_t[n + 1]));
					memcpy(ret, _value.val.wstrVal, sizeof(wchar_t) * (n + 1));
				}
				break;
			case db_decimal:
			case db_numeric:
				if (_value.val.bufVal)
				{
					// detect size
					numeric conv;
					conv.parse_orcl(_value.val.bufVal);
					size_t n = conv.orcl_len();
					ub1_t* ret = (ub1_t*)check_pointer((void*)(out._value.val.bufVal = new ub1_t[n]));
					memcpy(ret, _value.val.bufVal, n);
				}
				break;
			case db_binary:
				if (_value.val.bufVal)
				{
					size_t n = *(size_t*)_value.val.bufVal;
					ub1_t* ret = (ub1_t*)check_pointer((void*)(out._value.val.bufVal = new ub1_t[n + sizeof(size_t)]));
					*(size_t*)ret = n;
					memcpy(ret + sizeof(size_t), _value.val.bufVal + sizeof(size_t), n);
				}
				break;
			case db_guid:
				if (_value.val.guidVal)
				{
					guid_t* ret = (guid_t*)check_pointer((void*)(out._value.val.guidVal = new guid_t));
					memcpy(ret, _value.val.guidVal, sizeof(guid_t));
				}
				break;
			default:
				assert(false);
		} // switch
	}
	else
	{
		out._value.nullVal = _value.nullVal;
		out._value.val = _value.val;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
dbserver_impl::callback_finder::callback_finder(const async_db_notify* obj) :
	_obj(obj)
{
}

dbserver_impl::callback_fire::callback_fire(bool noerrors, size_t ident) :
	_noerrors(noerrors), _ident(ident)
{
}
/////////////////////////////////////////////////////////////////////////////////////
dbserver_impl::dbserver_impl(size_t ident) : 
	// server
	_ident(ident),
	_code(0), 
	// connection
	_connection(false),
	_transaction(false),
	_trusted(false),
	_columns_allocator(0),
	_data_allocator(0),
	_bulk_allocator(0),
	_sql(&_sql_allocator),
	_quote(':'),
	_start_row(0),
	_requested_rows(0),
	_fetched_rows(0),
	_forward(true),
	_bulk_rows(100),
	// query
	_state(STATE_OK),
	_action(ACTION_NONE),
	_is_open(false),
	_list_callback(16),
	_iter(_data.end())
{
	_temp_allocator = _manager.loan_object();
	_columns_allocator = _manager.loan_object();
	_data_allocator = _manager.loan_object();
	_bulk_allocator = _manager.loan_object(); 
}

// virtual
dbserver_impl::~dbserver_impl()
{
	// stops thread
	_thread.cancel_job();
	_thread.stop();
	_manager.return_object(_temp_allocator);
	_manager.return_object(_columns_allocator);
	_manager.return_object(_data_allocator);
	_manager.return_object(_bulk_allocator);
}

size_t 
dbserver_impl::get_ident() const
{
	return _ident;
}

bool 
dbserver_impl::is_ready() const
{
	return get_state() == STATE_OK;
}

size_t
dbserver_impl::get_code() const
{
	return _code;
}

const char*
dbserver_impl::get_error() const
{
	return _error;
}



// connects to db, uses parameter set that is specific for the database type
// user name, password, database name & so on.
bool
dbserver_impl::connect(bool trusted_connection, const char* connection_string)
{
	DB_TRY // try block
	_check_disconnect(); // must be disconnected
	v_connect(trusted_connection, connection_string); // connects
	_trusted = trusted_connection;
	_connect_string = connection_string;
	_set_connect(); // sets connect

	format_logging(0, __FILE__, __LINE__, en_log_info, "connect succeeded, login: %s", connection_string);

	DB_CATCH_DB // error processing
}

// disconnects from db
bool
dbserver_impl::disconnect()
{
	DB_TRY // try block
	_check_connect(); // must be connected
	v_disconnect(); // disconnects
	_trusted = false;
	_connect_string = 0;
	_set_disconnect(); // sets disconnect
	DB_CATCH_DB // error processing
}

// starts transaction
bool
dbserver_impl::start_transaction()
{
	DB_TRY // try block
	_check_out_transaction(); // must be out of transaction
	v_start_transaction(); // starts transaction
	_set_in_transaction(); // sets in transaction
	DB_CATCH_DB // error processing
}

// commits transaction
bool
dbserver_impl::commit()
{
	DB_TRY // try block
	_check_in_transaction(); // must be out of transaction	
	v_commit(); // commit
	_set_out_transaction(); // sets out transaction
	DB_CATCH_DB // error processing
}

// rolls back transaction
bool
dbserver_impl::rollback()
{
	DB_TRY // try block
	_check_in_transaction(); // must be out of transactiont
	v_rollback(); // rollback
	_set_out_transaction(); // sets out transaction
	DB_CATCH_DB // error processing
}

// returns connection state
bool
dbserver_impl::is_connect() const
{ 
	return _is_connect();
}

// returns trasaction state
bool
dbserver_impl::is_in_transaction() const
{ 
	return _is_in_transaction();
}

// returns trasaction state
bool
dbserver_impl::is_connect_alive()
{ 
	return v_is_connect_alive();
}


//
// prepares sql methods
//
//
// allocates params
//
bool 
dbserver_impl::resize_params(size_t size)
{
	try
	{
		// checks if number params are changed
		size_t num = _params.size();
		
		if (num != size && !_bulk_params.empty())
			_bulk_params.clear();


		_params.resize(size);
	}
	catch (exception& x)
	{
		_error = x.what();
		_code = x.get_code();
		return false;
	}

	return true;
}

//
// gets param count
//
size_t
dbserver_impl::get_param_count() const
{
	return _params.size();
}

//
// returns the virtual terimber type like vt_int32, ...
//
dbtypes
dbserver_impl::get_param_type(size_t index) const
{
	return index >= _params.size() ? db_unknown : _params[index]._type;
}

//
// returns the in/out
//
db_param_type
dbserver_impl::get_param_in_out(size_t index) const
{
	return index >= _params.size() ? db_param_unknown : _params[index]._in_out;
}

//
// returns the nullable - database specific
//
bool 
dbserver_impl::get_param_is_null(size_t index) const
{
	return index >= _params.size() || _params[index]._value.nullVal;
}
 
//
// returns param value
// if index out of range then zero will be return
// the best practice to check type before call gets value functions
bool 
dbserver_impl::get_param_as_bool(size_t index) const
{
	return get_param_as_value(index, vt_bool).boolVal;
}

sb1_t
dbserver_impl::get_param_as_char(size_t index) const
{
	return get_param_as_value(index, vt_sb1).cVal;
}

ub1_t
dbserver_impl::get_param_as_byte(size_t index) const
{
	return get_param_as_value(index, vt_ub1).bVal;
}

sb2_t
dbserver_impl::get_param_as_short(size_t index) const
{
	return get_param_as_value(index, vt_sb2).iVal;
}

ub2_t
dbserver_impl::get_param_as_word(size_t index) const
{
	return get_param_as_value(index, vt_ub2).uiVal;
}

sb4_t
dbserver_impl::get_param_as_long(size_t index) const
{
	return get_param_as_value(index, vt_sb4).lVal;
}

ub4_t
dbserver_impl::get_param_as_dword(size_t index) const
{
	return get_param_as_value(index, vt_ub4).ulVal;
}

float
dbserver_impl::get_param_as_float(size_t index) const
{
	return get_param_as_value(index, vt_float).fltVal;
}

double
dbserver_impl::get_param_as_double(size_t index) const
{
#ifdef OS_64BIT
	return get_param_as_value(index, vt_double).dblVal;
#else
	const double* ret = get_param_as_value(index, vt_double).dblVal;
	return ret ? *ret : 0.0;
#endif
}

sb8_t
dbserver_impl::get_param_as_long64(size_t index) const
{
#ifdef OS_64BIT
	return get_param_as_value(index, vt_sb8).intVal;
#else
	const sb8_t* ret = get_param_as_value(index, vt_sb8).intVal;
	return ret ? *ret : 0;
#endif
}

ub8_t
dbserver_impl::get_param_as_dword64(size_t index) const
{
#ifdef OS_64BIT
	return get_param_as_value(index, vt_ub8).uintVal;
#else
	const ub8_t* ret = get_param_as_value(index, vt_ub8).uintVal;
	return ret ? *ret : 0;
#endif
}

// for high performance we are using reference
bool 
dbserver_impl::get_param_as_guid(size_t index, guid_t& val) const
{
	const guid_t* ret = get_param_as_value(index, vt_guid).guidVal;
	
	if (!ret)
		return false;

	val = *ret;
	return true;
}

// if val pointer is null then the function returns the required buffer size
// delimeter defines the desirable delimeter sign
const char*
dbserver_impl::get_param_as_numeric(size_t index, char delimeter) const
{
	const ub1_t* retVal = get_param_as_value(index, vt_numeric).bufVal;

	if(!retVal)
		return 0;

	numeric num;
	if (!num.parse_orcl(retVal))
		return 0;

	size_t len = num.is_zero() ? 2 : num.precision() + (num.sign() ? 1 : 0) + (num.scale() ? 1 : 0) + (num.precision() == num.scale()) + 1;
	char* retStr = (char*)_temp_allocator->allocate(len);
	num.format(retStr, delimeter);
	return retStr;
}

// if the val pointer is null then the function returns the required buffer size
// delimeter defines the desirable delimeter sign
const char*
dbserver_impl::get_param_as_decimal(size_t index, char delimeter) const
{
	return get_param_as_numeric(index, delimeter);
}

// to avoid confusions with different date format the function returns parsed date
bool 
dbserver_impl::get_param_as_date(size_t index, ub4_t& year, ub1_t& month, ub1_t& day, ub1_t& hour, ub1_t& minute, ub1_t& second, ub2_t& millisec, ub1_t& wday, ub2_t& yday) const
{
#ifdef OS_64BIT
	date dt(get_param_as_value(index, vt_date).intVal);
#else
	const sb8_t* val = get_param_as_value(index, vt_date).intVal;
	if (!val)
		return false;
	date dt(*val);
#endif
	return date::convert_from(dt, year, month, day, hour, minute, second, millisec, wday, yday);
}

// if val pointer is null then the function returns the required buffer size
const char*
dbserver_impl::get_param_as_string(size_t index) const
{
	return get_param_as_value(index, vt_string).strVal;
}

// if val pointer is null then the function returns the required buffer size
const wchar_t*
dbserver_impl::get_param_as_wstring(size_t index) const
{
	return get_param_as_value(index, vt_wstring).wstrVal;
}

// if val pointer is null then the function returns the required buffer size
const ub1_t*
dbserver_impl::get_param_as_binary(size_t index, size_t& len) const
{
	const ub1_t* buf = get_param_as_value(index, vt_binary).bufVal;
	len = buf ? *(size_t*)buf : 0;
	return buf ? buf + sizeof(size_t) : 0;
}

// the format of buffer is as follows
// first size_t bytes the length of the rest of buffer,
// which contains the raw byte data
const ub1_t*
dbserver_impl::get_param_as_binary_ptr(size_t index) const
{
	return get_param_as_value(index, vt_binary).bufVal;
}


////////////////////////////////////////////////////////
bool 
dbserver_impl::set_param_as_null(size_t index, db_param_type in_out, dbtypes type)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && type != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_null(in_out, type);
	}
}

bool 
dbserver_impl::set_param_as_bool(size_t index, db_param_type in_out, bool val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_bool != _params[index]._type)
			_bulk_params.clear();
	
		return _params[index].set_as_bool(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_char(size_t index, db_param_type in_out, sb1_t val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_sb1 != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_char(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_byte(size_t index, db_param_type in_out, ub1_t val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_ub1 != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_byte(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_short(size_t index, db_param_type in_out, sb2_t val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_sb2 != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_short(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_word(size_t index, db_param_type in_out, ub2_t val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_ub2 != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_word(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_long(size_t index, db_param_type in_out, sb4_t val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_sb4 != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_long(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_dword(size_t index, db_param_type in_out, ub4_t val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_sb4 != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_dword(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_float(size_t index, db_param_type in_out, float val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_float != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_float(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_double(size_t index, db_param_type in_out, double val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_double != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_double(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_long64(size_t index, db_param_type in_out, sb8_t val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_sb8 != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_long64(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_dword64(size_t index, db_param_type in_out, ub8_t val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_ub8 != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_dword64(in_out, val);
	}
}

bool 
dbserver_impl::set_param_as_guid(size_t index, db_param_type in_out, const guid_t& val)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_guid != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_guid(in_out, val);
	}
}


// this function depends on format
// to avoid the confusions of different numeric and date formats

// -1223456.7894 - delimeter here "."
// -1223456,7894 - delimeter here ","
bool 
dbserver_impl::set_param_as_numeric(size_t index, db_param_type in_out, const char* val, char delimeter)
{
	if (index >= _params.size() || !val)
		return false;
	else
	{
		if (!_bulk_params.empty() && db_decimal != _params[index]._type && db_numeric != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_numeric(in_out, val, delimeter);
	}
}

bool 
dbserver_impl::set_param_as_decimal(size_t index, db_param_type in_out, const char* val, char delimeter)
{
	if (index >= _params.size() || !val)
		return false;
	else
	{
		if (!_bulk_params.empty() && db_decimal != _params[index]._type && db_numeric != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_decimal(in_out, val, delimeter);
	}
}

// date from A.C. year > 0
bool 
dbserver_impl::set_param_as_date(size_t index, db_param_type in_out, ub4_t year, ub1_t month, ub1_t day, ub1_t hour, ub1_t minute, ub1_t second, ub2_t millisec)
{
	if (index >= _params.size())
		return false;
	else
	{
		if (!_bulk_params.empty() && db_date != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_date(in_out, year, month, day, hour, minute, second, millisec);
	}
}


// for 0 terminated string len can be -1
bool 
dbserver_impl::set_param_as_string(size_t index, db_param_type in_out, const char* val, size_t len, size_t max_len)
{
	if (index >= _params.size() || !val)
		return false;
	else
	{
		if (!_bulk_params.empty() && db_string != _params[index]._type)
			_bulk_params.clear();

		_params[index]._max_length = max_len;
		return _params[index].set_as_string(in_out, val, len);
	}
}


// for 0 terminated string len can be -1
// to avoid copying the already allocated memory, only the pointer can be used
// be careful! the caller is responsible for the life time of provided pointer, 
// which has to be valid until the sql statment has been executed.
bool 
dbserver_impl::set_param_as_string_ptr(size_t index, const char* val)
{
	if (index >= _params.size() || !val)
		return false;
	else
	{
		if (!_bulk_params.empty() && db_string != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_string_ptr(val);
	}
}


// for 0 terminated string len can be -1
bool 
dbserver_impl::set_param_as_wstring(size_t index, db_param_type in_out, const wchar_t* val, size_t len, size_t max_len)
{
	if (index >= _params.size() || !val)
		return false;
	else
	{
		if (!_bulk_params.empty() && db_wstring != _params[index]._type)
			_bulk_params.clear();

		_params[index]._max_length = max_len;
		return _params[index].set_as_wstring(in_out, val, len);
	}
}


// for 0 terminated string len can be -1
// to avoid copying the already allocated huge memory, only the pointer can be used
// be careful! the caller is responsible for the life time of provided pointer, 
// which has to be valid until the sql statment has been executed.
// v
bool 
dbserver_impl::set_param_as_wstring_ptr(size_t index, const wchar_t* val)
{
	if (index >= _params.size() || !val)
		return false;
	else
	{
		if (!_bulk_params.empty() && db_wstring != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_wstring_ptr(val);
	}
}


// val points to the valid buffer of raw byte of "len" length
bool 
dbserver_impl::set_param_as_binary(size_t index, db_param_type in_out, const ub1_t* val, size_t len, size_t max_len)
{
	if (index >= _params.size() || !val || !len)
		return false;
	else
	{
		if (!_bulk_params.empty() && db_binary != _params[index]._type)
			_bulk_params.clear();

		_params[index]._max_length = max_len;
		return _params[index].set_as_binary(in_out, val, len);
	}
}


// to avoid coping already allocated huge memory, the pointer can be used only
// be careful! the caller is responsible for the life time of provided pointer, 
// which has to be valid until the sql statment has been executed.
// it is supposed to only work for input parameters, not for output or input/output ones
// the format of the buffer must be as follows
// first size_t bytes the length of the rest of buffer,
// which contains the raw byte data
bool 
dbserver_impl::set_param_as_binary_ptr(size_t index, const ub1_t* val)
{
	if (index >= _params.size() || !val)
		return false;
	else
	{
		if (!_bulk_params.empty() && db_binary != _params[index]._type)
			_bulk_params.clear();

		return _params[index].set_as_binary_ptr(val);
	}
}

///////////////////////////////////////////////////////
//
// internal columns access
//

//
// gets column count
//
size_t
dbserver_impl::get_column_count() const
{
	return _cols.size();
}
//
// returns the virtual terimber type like vt_int32, ...
//
dbtypes
dbserver_impl::get_column_type(size_t index) const
{
	//return index >= _cols.size() ? db_unknown : _cols[index]._type;
	return index >= _cols.size() ? db_unknown : (_cols.begin() + index)->_type;
}
//
// returns the name
//
const char*
dbserver_impl::get_column_name(size_t index) const
{
	//returns index >= _cols.size() ? 0 : _cols[index]._name;
	return index >= _cols.size() ? 0 : (_cols.begin() + index)->_name;
}

//
// returns the nullable - database specific
//
bool 
dbserver_impl::get_column_nullable(size_t index) const
{
	//returns index >= _cols.size() ? false : _cols[index]._value.nullVal;
	return index >= _cols.size() ? false : (_cols.begin() + index)->_value.nullVal;
}

//
// returns the scale
//
size_t
dbserver_impl::get_column_scale(size_t index) const
{
	//return index >= _cols.size() ? 0 : _cols[index]._scale;
	return index >= _cols.size() ? 0 : (_cols.begin() + index)->_scale;
}

//
// returns the precision
//
size_t
dbserver_impl::get_column_precision(size_t index) const
{
	//return index >= _cols.size() ? 0 : _cols[index]._precision;
	return index >= _cols.size() ? 0 : (_cols.begin() + index)->_precision;
}

//
// returns the length of the parameter (only for output binders)
//
size_t
dbserver_impl::get_column_max_length(size_t index) const
{
	//returns index >= _cols.size() ? 0 : _cols[index]._max_length;
	return index >= _cols.size() ? 0 : (_cols.begin() + index)->_max_length;
}

//
// rowset navigation methods
// 

//
// gets row count
//
size_t
dbserver_impl::get_row_count() const
{
	return _fetched_rows;
}

//
// resets internal cursor next to the last element
//
void
dbserver_impl::reset() const
{
	_iter = get_iter_end();
}

//
// tries to find the next row, after reset it goes to the first row
//
bool
dbserver_impl::next() const
{
	if (_iter != get_iter_end())
		++_iter;
	else
		_iter = get_iter_begin();

	return _iter != get_iter_end();
}

//
// tries to find the previous row
//
bool
dbserver_impl::prev() const
{
	if (_iter != get_iter_begin())
		--_iter;
	else
		_iter = get_iter_end();

	return _iter != get_iter_end();
}

//////////////////////////////////////////////////////////////////////////////
bool 
dbserver_impl::get_value_is_null(size_t index) const
{
	return _iter == get_iter_end() || index >= _cols.size() || (*_iter)[index].nullVal;
}

bool 
dbserver_impl::get_value_as_bool(size_t index) const
{
	return get_value_as_value(index, vt_bool).boolVal;
}

sb1_t
dbserver_impl::get_value_as_char(size_t index) const
{
	return get_value_as_value(index, vt_sb1).cVal;
}

ub1_t
dbserver_impl::get_value_as_byte(size_t index) const
{
	return get_value_as_value(index, vt_ub1).bVal;
}

sb2_t
dbserver_impl::get_value_as_short(size_t index) const
{
	return get_value_as_value(index, vt_sb2).iVal;
}

ub2_t
dbserver_impl::get_value_as_word(size_t index) const
{
	return get_value_as_value(index, vt_ub2).uiVal;
}

sb4_t
dbserver_impl::get_value_as_long(size_t index) const
{
	return get_value_as_value(index, vt_sb4).lVal;
}

ub4_t
dbserver_impl::get_value_as_dword(size_t index) const
{
	return get_value_as_value(index, vt_ub4).ulVal;
}

float
dbserver_impl::get_value_as_float(size_t index) const
{
	return get_value_as_value(index, vt_float).fltVal;
}

double
dbserver_impl::get_value_as_double(size_t index) const
{
#ifdef OS_64BIT
	return get_value_as_value(index, vt_double).dblVal;
#else
	const double* ret = get_value_as_value(index, vt_double).dblVal;
	return ret ? *ret : 0.0;
#endif
}

sb8_t
dbserver_impl::get_value_as_long64(size_t index) const
{
#ifdef OS_64BIT
	return get_value_as_value(index, vt_sb8).intVal;
#else
	const sb8_t* ret = get_value_as_value(index, vt_sb8).intVal;
	return ret ? *ret : 0;
#endif
}

ub8_t
dbserver_impl::get_value_as_dword64(size_t index) const
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
dbserver_impl::get_value_as_guid(size_t index, guid_t& val) const
{
	const guid_t* ret = get_value_as_value(index, vt_guid).guidVal;
	if (!ret)
		return false;

	val = *ret;
	return true;
}

// if the val pointer is null then the function returns the required buffer size
// delimeter defines the desirable delimeter sign
const char* 
dbserver_impl::get_value_as_numeric(size_t index, char delimeter) const
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
	char* retStr = (char*)_temp_allocator->allocate(len);
	num.format(retStr, delimeter);
	return retStr;
}

// if the val pointer is null then the function returns the required buffer size
// delimeter defines the desirable delimeter sign
const char*
dbserver_impl::get_value_as_decimal(size_t index, char delimeter) const
{
	return get_value_as_numeric(index, delimeter);
}

// to avoid confusions with different date formats function returns parsed date
bool 
dbserver_impl::get_value_as_date(size_t index, ub4_t& year, ub1_t& month, ub1_t& day, ub1_t& hour, ub1_t& minute, ub1_t& second, ub2_t& millisec, ub1_t& wday, ub2_t& yday) const
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

// if the val pointer is null then the function returns the required buffer size
const char*
dbserver_impl::get_value_as_string(size_t index) const
{
	return get_value_as_value(index, vt_string).strVal;
}

// if the val pointer is null then the function returns the required buffer size
const wchar_t*
dbserver_impl::get_value_as_wstring(size_t index) const
{
	return get_value_as_value(index, vt_wstring).wstrVal;
}

// returns pointer to the raw bytes
// size of bytes is put to len
const ub1_t*
dbserver_impl::get_value_as_binary(size_t index, size_t& len) const
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
dbserver_impl::get_value_as_binary_ptr(size_t index) const
{	
	return get_value_as_value(index, vt_binary).bufVal;
}


//
// checks the state of the sql request
//
bool 
dbserver_impl::is_open_sql() const
{
	return _is_open_sql();
}
//
// executes the parameterized sql statement with recordset result
//
bool 
dbserver_impl::open_sql(bool async, const char* sql)
{
	DB_TRY // try block
	_check_state(); // state must be OK
	_check_close(); // if open already - throws exception
	_check_action(); // action must be NONE
	_set_action(async ? ACTION_OPEN_SQL_ASYNC : ACTION_OPEN_SQL); // set action
	_process_sql(sql); // processes input sql
	_get_number_params(); // gets number binders
	_bind_params(); // binds binders

	if (async)
		_start_thread();
	else
	{
		v_execute(); // opens SQL
		_set_open(); // sets open
		_get_columns_info(); // gets columns' information
		_set_action(ACTION_NONE); // completes
	}
	DB_CATCH // error processing
}
//
// executes the parameterized sql statement without result
//
bool 
dbserver_impl::exec_sql(bool async, const char* sql)
{
	DB_TRY // try block
	_check_state(); // state must be OK
	_check_close(); // if open already - throws exception
	_check_action(); // action must be NONE
	_set_action(async ? ACTION_EXEC_SQL_ASYNC : ACTION_EXEC_SQL); // set action
	_process_sql(sql); // processes input sql
	_get_number_params(); // gets number binders
	_bind_params(); // binds binders

	if (async)
		_start_thread();
	else
	{
		v_execute(); // opens SQL
		_set_open(); // sets open
		_rebind_params();
		_set_action(ACTION_NONE); // completes
	}
	DB_CATCH // error processing
}
//
// executes stored procedure with recordset result
//
bool 
dbserver_impl::open_proc(bool async, const char* name)
{
	DB_TRY // try block
	_check_state(); // state must be OK
	_check_close(); // if open already - throws exception
	_check_action(); // action must be NONE
	_set_action(async ? ACTION_OPEN_PROC_ASYNC : ACTION_OPEN_PROC); // set action
	_process_sql(name); // processes input sql
	_get_number_params(); // gets number binders
	_bind_params(); // binds binders

	if (async)
		_start_thread();
	else
	{
		v_execute(); // opens SQL
		_set_open(); // sets open
		_get_columns_info(); // gets columns' information
		_rebind_params();
		_set_action(ACTION_NONE); // completes
	}
	DB_CATCH // error processing
}
//
// executes stored procedure without result
//
bool 
dbserver_impl::exec_proc(bool async, const char* name)
{
	DB_TRY // try block
	_check_state(); // state must be OK
	_check_close(); // if open already - throws exception
	_check_action(); // action must be NONE
	_set_action(async ? ACTION_EXEC_PROC_ASYNC : ACTION_EXEC_PROC); // set action
	_process_sql(name); // processes input sql
	_get_number_params(); // gets number binders
	_bind_params(); // binds binders

	if (async)
		_start_thread();
	else
	{
		v_execute(); // opens SQL
		_set_open(); // sets open
		_rebind_params();
		_set_action(ACTION_NONE); // completes
	}
	DB_CATCH // error processing
}

//
// fetches data and returns recordset
//
bool 
dbserver_impl::fetch_data(bool async, size_t start_row, size_t num_rows, bool forward)
{
	DB_TRY // try block
	_check_state(); 	// state must be OK
	_check_open(); 	// if closed already - throws exception

	// cleans up dataset
	_data.clear();
	_temp_allocator->reset();
	_data_allocator->reset();

	_set_action(async ? ACTION_FETCH_ASYNC : ACTION_FETCH); // set action
	_bind_columns(); 	// binds columns
	_start_row = start_row; // stores starting row
	_fetched_rows = 0;
	_requested_rows = num_rows;
	_forward = forward;
	_iter = get_iter_end();

	if (async) 	// fetches row
		_start_thread();
	else
	{
		v_fetch();
	}
	DB_CATCH // error processing
}

//
// closes sql - frees allocated resources
//
bool 
dbserver_impl::close_sql()
{
	DB_TRY // try block
	_check_state(); // state must be OK
	_check_open(); 	// if closed already - throws exception
	_thread.cancel_job(); // stops working
	v_close(); // closes SQL
	_set_close();
	_set_action(ACTION_NONE);

	_sql = 0;
	_data.clear();
	_start_row = 0;
	_fetched_rows = 0;
	_forward = true;
	_bulk_rows = 100;
	_iter = get_iter_end();
	
	_sql_allocator.reset();
	_temp_allocator->reset();
	_data_allocator->reset();
	_columns_allocator->reset();

	_set_state(STATE_OK);
	DB_CATCH
}

bool 
dbserver_impl::interrupt_request()
{
	// state must be OK
	if (get_state() == STATE_WORKING)
	{
		// Interrupts asyncronic executing
		_set_state(STATE_INTERRUPTED);
		v_interrupt_async();
		_thread.cancel_job();
		_set_state(STATE_OK);
		v_close(); // closes SQL
		// sets action
		_set_action(ACTION_NONE);
	}

	return true;
}

//
// the syntax of the sql statement with binders must be understandable
// for the internal sql processor
// therefore it should look like
// select * from table where pk = :pk
// here ':' char is an identificator of the parameter with name "pk"
// function set_quote allows to change this identificator to the preferable one
//
bool 
dbserver_impl::set_quote(char quote)
{
	mutexKeeper keeper(_mtx);
	_quote = quote;
	return true;
}

void
dbserver_impl::set_callback(async_db_notify* target)
{
	callback_finder finder(target);
	algorithm::push_back_if(_list_callback, finder, target, _mtx);
}

void
dbserver_impl::remove_callback(async_db_notify* target)
{
	callback_finder finder(target);
	algorithm::remove_if(_list_callback, finder, _mtx);
}


void 
dbserver_impl::_get_number_params()
{
	// Form SQL string
	v_form_sql_string();
	// Replaces quter (for SQL with parameters)
	v_replace_quote();
}

void
dbserver_impl::_bind_params()
{
	// allocates resiurces
	v_before_execute();
	// gets number binders
	size_t count_params = _params.size();
	// Gets the field info each field
	for (size_t cur_param = 0; cur_param < count_params; ++cur_param)
		// calls virtual function
		v_bind_one_param(cur_param);
}

void
dbserver_impl::_rebind_params()
{
	// completes executing
	v_after_execute();
	// gets number binders
	size_t count_params = _params.size();
	// Gets the field info each field
	for (size_t cur_param = 0; cur_param < count_params; ++cur_param)
	{
		// calls virtual function
		v_rebind_one_param(cur_param);
	}
}

void
dbserver_impl::_bind_columns()
{
	// gets column count
	size_t count_columns = _cols.size();

	// estimates the max length of all columns
	size_t cur_column;
	size_t total_memory_per_row = 1;

	for (cur_column = 0; cur_column < count_columns; ++cur_column)
	{
		// gets column length
		if (_cols[cur_column]._max_length < 1024*64) // max allowed
			total_memory_per_row += _cols[cur_column]._max_length;
	}


	// estimates how many rows we can afford
	// max memory ~ 1M, but < 1K rows
	_bulk_rows = __max((size_t)1, __min((size_t)1024*1024 / total_memory_per_row, (size_t)1024));


	// does some db specific before bind columns
	v_before_bind_columns();


	// gets the field info for each field
	for (cur_column = 0; cur_column < count_columns; ++cur_column)
	{
		// calls virtual function
		v_bind_one_column(cur_column);
	}
}

void
dbserver_impl::_get_columns_info()
{
	_cols.clear();

	// Finds field count
	size_t count_columns = v_get_number_columns();

	_cols.resize(*_columns_allocator, count_columns);

	// Gets the field info for each field
	for (size_t cur_column = 0; cur_column < count_columns; ++cur_column)
	{
		// calls virtual function
		v_get_one_column_info(cur_column);
		_cols[cur_column]._type = v_native_type_to_client_type(_cols[cur_column]._native_type);
	}
}

void 
dbserver_impl::_process_sql(const char* sql)
{
	_sql = sql;
}

void 
dbserver_impl::_start_thread()
{
	if (THREAD_CLOSE == _thread.get_state())
		_thread.start();

	job_task task(this, 0, INFINITE, 0);
	_set_state(STATE_WORKING);
	_thread.assign_job(task);
	_thread.wakeup();
}

bool 
dbserver_impl::v_has_job(size_t ident, void* user_data)
{
	module_state	state = get_state();
	action			action_ = get_action();

	if (state == STATE_WORKING
		&&
		(action_ == ACTION_OPEN_SQL_ASYNC
		|| action_ == ACTION_OPEN_PROC_ASYNC
		|| action_ == ACTION_EXEC_SQL_ASYNC
		|| action_ == ACTION_EXEC_PROC_ASYNC
		|| action_ == ACTION_FETCH_ASYNC)
		)
		return true;
	else
		return false;
}

void 
dbserver_impl::v_do_job(size_t ident, void* user_data)
{
	module_state	state = get_state();
	action			action_ = get_action();
	if (state != STATE_WORKING)
		return;

	try
	{
		switch(action_)
		{
			case ACTION_OPEN_SQL_ASYNC:
				v_execute(); // opens SQL
				_set_open(); // sets open
				_get_columns_info(); // gets columns information
				break;
			case ACTION_OPEN_PROC_ASYNC:
				v_execute(); // open SQL
				_set_open(); // set open
				_get_columns_info(); // gets columns information
				_rebind_params();
				break;
			case ACTION_EXEC_SQL_ASYNC:
				v_execute(); // opens SQL
				_set_open(); // sets open
				_rebind_params();
				break;
			case ACTION_EXEC_PROC_ASYNC:
				v_execute(); // opens SQL
				_set_open(); // sets open
				_rebind_params();
				break;
			case ACTION_FETCH_ASYNC:
				v_fetch();
				break;
			default:
				break;
		}

		// set action
		_set_action(ACTION_NONE);
		_set_state(STATE_OK);
		_notify_async(true, _ident);
	}
	catch( exception& ex )
	{
		_thread.cancel_job();
		_code = ex.get_code();
		_error = ex.what();
		_set_state(STATE_OK);
		_notify_async(false, _ident);
	}
	catch(...)
	{
		_thread.cancel_job();
		_set_state(STATE_OK);
		_notify_async(false, _ident);
	}
}

void 
dbserver_impl::_notify_async(bool noerrors, size_t ident)
{
	callback_fire fire(noerrors, ident);
	algorithm::for_each(_list_callback, fire, _mtx);
}

bool 
dbserver_impl::_is_asynchronous_action() const
{
	switch (get_action())
	{
		case ACTION_OPEN_SQL_ASYNC:
		case ACTION_OPEN_PROC_ASYNC:
		case ACTION_EXEC_SQL_ASYNC:
		case ACTION_EXEC_PROC_ASYNC:
		case ACTION_FETCH_ASYNC:
			return true;
		default:
			return false;
	}
}

// current set of parameters previously set by set_param_* functions
// will be moved to the bulk parameters list
// the current parameters set will have the same values, but will not participate in bulk operations
// moreover, if the next call set_param_* function will change the type - all bulk parameters will be deleted.
bool 
dbserver_impl::param_bulk_store()
{
	// resizes vector of binders
	size_t num = _params.size();
	if (!num)
		return false; // nothing to add

	size_t capacity = _bulk_params.size();

	if (capacity >= 1024)
		return false;

	// dummy vector
	_vector< binder > item;
	// pushes back 
	bulk_params_t::iterator iter_vec = _bulk_params.push_back(item);
	if (iter_vec == _bulk_params.end())
		return false;

	if (!iter_vec->resize(*_bulk_allocator, num))
		return false;

	// copies current data from _params to r_vec
	for (size_t index = 0; index < num; ++index)
	{
		// makes a clone
		_params[index].clone((*iter_vec)[index]);
	}

	return true;
}

// removes the last stored parameter
bool 
dbserver_impl::param_bulk_remove()
{
	if (_bulk_params.empty())
		return false;

	try
	{
		_vector< binder >& r_vec = _bulk_params.back();

		// cleans up current params
		size_t num = r_vec.size();
		
		for (size_t index = 0; index < num; ++index)
		{
			// frees param memory
			_params[index].deallocate_value();
			// make a clone
			r_vec[index].clone(_params[index]);
			// releases memory
			r_vec[index].deallocate_value();
		}
		
		// removes back bulk param
		_bulk_params.pop_back();

		if (_bulk_params.empty())
			_bulk_allocator->reset();

	}
	catch (...) // memory and other issues
	{
		return false;
	}

	return true;
}

bool 
dbserver_impl::param_bulk_remove_all()
{
	if (_bulk_params.empty())
		return false;

	try
	{
		// removes back bulk param
		for (bulk_params_t::iterator it_list = _bulk_params.begin(); it_list != _bulk_params.end(); ++it_list)
		{
			for (_vector< binder >::iterator it_vec = it_list->begin(); it_vec != it_list->end(); ++it_vec)
			{
				it_vec->deallocate_value();
			}
		}

		_bulk_params.clear();
		_bulk_allocator->reset();
	}
	catch (...) // memory and other issues
	{
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////
// pool support
//! \brief login constructor
db_arg::db_arg(	size_t ident,	
			   bool trusted,								//!< trusted connection flag
				const char* login_string					//!< login string
			) :
	_ident(ident),
	_trusted(trusted),
	_login_string(login_string)
{
}

//! \brief login constructor
db_entry::db_entry(size_t ident,	
				   bool trusted,							//!< trusted connection flag
					const char* login_string				//!< login string
			) : 
	db_arg(ident, trusted, login_string),
	_obj(0)
{
}


#pragma pack()
END_TERIMBER_NAMESPACE
