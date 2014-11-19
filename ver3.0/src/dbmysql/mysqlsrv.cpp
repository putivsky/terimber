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

#include "dbmysql/mysqlsrv.h"
#include "base/vector.hpp"
#include "base/list.hpp"
#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/numeric.h"
 
BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

#define check_retcode_db(x) \
	if (!(x)) \
	{ \
		throw exception(mysql_errno(_desc), mysql_error(_desc)); \
	}

#define check_retcode(x) \
	if (!(x)) \
	{ \
		throw exception(mysql_stmt_errno(_stmt), mysql_stmt_error(_stmt)); \
	}

static const char* valid_delimeters = " ,)}";
static const size_t defaultBlobSize = 1024 * 64;

mysql_dbserver::mysql_dbserver(size_t ident) : dbserver_impl(ident),
	_desc(0), _stmt(0)
{
}

mysql_dbserver::~mysql_dbserver()
{
	if (_is_open_sql())
		close_sql(); 

	if (_is_connect()) 
		disconnect();
}

// database specific
void
mysql_dbserver::v_connect(bool trusted_connection, const char* connection_string)
{
	// syntax connection string is:
	// 192.168.0.1  user@password
	// user/password@192.168.0.8:444/dbname
	string_t user, password, host, database;
	unsigned short port = 0;
	parse_connection_string(connection_string, user, password, host, port, database);

//	unsigned int r = mysql_thread_safe();

//	if (!mysql_thread_init())
//		throw exception(0, "can't start mysql  thread");

	if (!(_desc = mysql_init(0)))
		throw exception(0, "can't init mysql");

	// connects
	MYSQL* dummy = 0;
	check_retcode_db((dummy = mysql_real_connect(_desc, host,
					   user,
					   password,
					   0,
					   port,
					   0, // unix_socket
					   CLIENT_MULTI_RESULTS)) != 0)// clientflag

	// selects database
	if (database.length())
		check_retcode_db(mysql_select_db(_desc, database) == 0)

}

void
mysql_dbserver::v_disconnect()
{
	_temp_allocator->clear_extra();

	v_close();

	if (_desc)
	{
		mysql_close(_desc);
		_desc = 0;
//		mysql_thread_end();
	}
}

void 
mysql_dbserver::v_start_transaction()
{
	// turns off autocommit
	check_retcode_db(mysql_autocommit(_desc, false) == 0)
}

void 
mysql_dbserver::v_commit()
{
	// commits
	check_retcode_db(mysql_commit(_desc) == 0)
}

void 
mysql_dbserver::v_rollback()
{
	// rolls back
	check_retcode_db(mysql_rollback(_desc) == 0)
}

bool
mysql_dbserver::v_is_connect_alive()
{
	//  checks connection first
	if (!_is_connect())
		return false;

	return true;
}

///////////////////////////
void
mysql_dbserver::v_before_execute()
{
	check_retcode_db((_stmt = mysql_stmt_init(_desc)) != 0)

	switch (get_action())
	{
		case ACTION_EXEC_SQL:
		case ACTION_EXEC_SQL_ASYNC:
		case ACTION_EXEC_PROC:
		case ACTION_EXEC_PROC_ASYNC:
			break;
		case ACTION_OPEN_SQL:
		case ACTION_OPEN_SQL_ASYNC:
		case ACTION_OPEN_PROC:
		case ACTION_OPEN_PROC_ASYNC:
			{
				const unsigned long type = CURSOR_TYPE_READ_ONLY;
				check_retcode(mysql_stmt_attr_set(_stmt, STMT_ATTR_CURSOR_TYPE, &type) == 0);
			}
			break;
		default:
			break;
	} // switch

	check_retcode(mysql_stmt_prepare(_stmt, (const char*)_sql, (unsigned long)_sql.length()) == 0)
}

void
mysql_dbserver::v_after_execute()
{
	switch (get_action())
	{
		case ACTION_EXEC_SQL:
		case ACTION_EXEC_SQL_ASYNC:
		case ACTION_EXEC_PROC:
		case ACTION_EXEC_PROC_ASYNC:
			{
				size_t maxRes = 1024;
				while (mysql_more_results(_desc) && maxRes-- > 0);
			}
			break;
		case ACTION_OPEN_SQL:
		case ACTION_OPEN_SQL_ASYNC:
		case ACTION_OPEN_PROC:
		case ACTION_OPEN_PROC_ASYNC:
			mysql_next_result(_desc);
			break;
		default:
			break;
	} // switch
}

// Executes SQL expression
void
mysql_dbserver::v_execute()
{
	switch (get_action())
	{
		case ACTION_EXEC_PROC:
		case ACTION_EXEC_PROC_ASYNC:
		case ACTION_EXEC_SQL:
		case ACTION_EXEC_SQL_ASYNC:
			break;
		default:
			break;
	}

	check_retcode(mysql_stmt_execute(_stmt) == 0)
}

// closes opened query
void
mysql_dbserver::v_close()
{
	if (_stmt)
	{
		mysql_stmt_free_result(_stmt);
		mysql_stmt_close(_stmt);
		_stmt = 0;
	}


	// destroys binders
	_param_binders.clear();
	_column_binders.clear();
}

// Fetches block rows
void
mysql_dbserver::v_fetch()
{
	// sets first row for set
	if (_start_row != 0)
	{
		v_execute();
		size_t skip_rows = 0;
		while (skip_rows++ < _start_row && mysql_stmt_fetch(_stmt));
	}

	// defines the column count
	size_t col_count = _cols.size();
	size_t select_row = 0;

	int x  = 0;

	while (_requested_rows
		&& (x = mysql_stmt_fetch(_stmt)) == 0 
			)
	{
		if (STATE_INTERRUPTED == get_state()) // check interrupt
			exception::_throw("Fetching process is interrupted");

		// adds empty row to recordset
		_vector< terimber_db_value > val;
		recordset_list_t::iterator iter_val = _data.push_back(*_data_allocator, val);
		if (iter_val == _data.end())
			exception::_throw("Not enough memory");

		// resizes row to the column count
		if (!iter_val->resize(*_data_allocator, col_count))
			exception::_throw("Not enough memory");
		// copies/converts data from buffer to row
		for (size_t index = 0; index < col_count; ++index)
			v_convert_one_value(0, index, (*iter_val)[index]);

		++select_row;	
		// increments/decrements _requested_rows
		--_requested_rows;
	}

	check_retcode(_requested_rows == 0 || x == MYSQL_NO_DATA)

	// sets the real fetched row count
	_fetched_rows = select_row;
}

// replaces quote to available sign for native drive
void
mysql_dbserver::v_replace_quote()
{
	if (0 == _quote)
		return;
	
	// saves the current sql
	if (!_sql.length())
		return;

	// allocates buffer for correct string_t
	char* buf = (char*)check_pointer(_temp_allocator->allocate(_sql.length() + 1));

	*buf = 0;

	size_t count_params = 0;

	char* new_sql = buf;
	const char* begin = _sql;
	const char* end = 0;

	// searches param identificator
	while (0 != (end = strchr(begin, _quote)))
	{
	 	++count_params;
		size_t len = end - begin; 
		// copies previous chars
		str_template::strcpy(new_sql, begin, len);
		begin += len + 1;
		new_sql += len;
		// adds parameter mark
		*new_sql++ = '?'; // replace ':' on '?'
		// skips parameter name
		// until valid_delimeters appear
		begin += strcspn(begin, valid_delimeters);
	}

	// checks params count
	if (count_params != _params.size())
		exception::_throw("Number of parameters doesn't match the sql statement syntax");

	// allocates params binders here
	if (_params.size())
	{
		_param_binders.resize(*_columns_allocator, _params.size());
	}

	// copies the rest bytes
	str_template::strcpy(new_sql, begin, os_minus_one);
	// resets sql
	_sql = buf;
}

// Binds one param with the number index 
void
mysql_dbserver::v_bind_one_param(size_t index)
{
	binder& cur = _params[index];

	MYSQL_BIND& mysql_binder = _param_binders[index];
	mysql_binder.is_unsigned = 1;

	mysql_binder.is_null = (char*)&cur._value.nullVal;
	mysql_binder.length = (unsigned long*)&cur._real_length;

	switch (cur._type)
	{
		case db_bool: // bool
			cur._max_length = sizeof(bool);
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal ? 0 : cur._max_length);	
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.boolVal); // set value
			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_TINY;
			break;	
		case db_sb1: // char
			mysql_binder.is_unsigned = 0;
		case db_ub1: // byte
			cur._max_length = sizeof(ub1_t);
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal ? 0 : cur._max_length);	
			mysql_binder.buffer = cur._bind_buffer = cur._type == db_sb1 ? (void*)&(cur._value.val.cVal) : (void*)&(cur._value.val.bVal); // set value
			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_TINY;
			break;	
		case db_sb2:
			mysql_binder.is_unsigned = 0;
		case db_ub2:
			cur._max_length = sizeof(ub2_t);
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal ? 0 : cur._max_length);	
			cur._bind_buffer = cur._type == db_sb2 ? (void*)&(cur._value.val.iVal) : (void*)&(cur._value.val.uiVal); // set value
			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_SHORT;
			break;	
		case db_sb4: 
			mysql_binder.is_unsigned = 0;
		case db_ub4: 
			cur._max_length = sizeof(ub4_t);
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal ? 0 : cur._max_length);	
			mysql_binder.buffer = cur._bind_buffer = cur._type == db_ub4 ? (void*)&(cur._value.val.lVal) : (void*)&(cur._value.val.ulVal); // set value
			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_LONG;
			break;	
		case db_float: 
			cur._max_length = sizeof(float);
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal ? 0 : cur._max_length);	
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.fltVal); // sets value
			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_FLOAT;
			break;	
		case db_double: 
			cur._max_length = sizeof(double);
#ifdef OS_64BIT
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal ? 0 : cur._max_length);
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.dblVal); // sets value
#else
			mysql_binder.buffer_length = cur._real_length = cur._value.nullVal || !cur._value.val.dblVal ? 0 : cur._max_length;
			
			if (is_param_out(cur._in_out))
			{
				double* buf = (double*)check_pointer(_temp_allocator->allocate(sizeof(double)));
				*buf = cur._value.val.dblVal ? *cur._value.val.dblVal : 0.0;
				mysql_binder.buffer = cur._bind_buffer = buf; // set value
			}
			else
				mysql_binder.buffer = cur._bind_buffer = (void*)cur._value.val.dblVal;
#endif
			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_DOUBLE;
			break;	
		case db_sb8:
			mysql_binder.is_unsigned = 0;
		case db_ub8:
			// let's use numeric type
			cur._max_length = sizeof(ub8_t);
#ifdef OS_64BIT
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal ? 0 : cur._max_length);
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.uintVal); // set value
#else
			mysql_binder.buffer_length = cur._real_length = cur._value.nullVal || !cur._value.val.uintVal ? 0 : cur._max_length;	

			if (is_param_out(cur._in_out))
			{
				ub8_t* buf = (ub8_t*)check_pointer(_temp_allocator->allocate(sizeof(ub8_t)));
				*buf = cur._value.val.uintVal ? *cur._value.val.uintVal : 0;
				mysql_binder.buffer = cur._bind_buffer = buf; // set value
			}
			else
				mysql_binder.buffer = cur._bind_buffer = (void*)cur._value.val.intVal;
#endif

			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_LONGLONG;
			break;	
		case db_date: 
#ifdef OS_64BIT
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal ? 0 : sizeof(MYSQL_TIME));
#else
			mysql_binder.buffer_length = cur._real_length = cur._value.nullVal || !cur._value.val.intVal ? 0 : sizeof(MYSQL_TIME);
#endif
			{
				MYSQL_TIME* ptds = (MYSQL_TIME*)check_pointer(_temp_allocator->allocate(sizeof(MYSQL_TIME)));
				mysql_binder.buffer = cur._bind_buffer = ptds;

				if (cur._value.val.intVal)
				{
					ub4_t year32;
					ub1_t month8, day8, hour8, minute8, second8, wday8;
					ub2_t millisec16, yday16;
					date::convert_from(
#ifdef OS_64BIT
						cur._value.val.intVal,
#else
						*cur._value.val.intVal, 
#endif
										year32,
										month8,
										day8,
										hour8,
										minute8,
										second8,
										millisec16,
										wday8,
										yday16);

					ptds->year = year32;
					ptds->month = month8;
					ptds->day = day8;
					ptds->hour = hour8;
					ptds->minute = minute8;
					ptds->second = second8;
					ptds->second_part = millisec16;
					ptds->neg = false;
					ptds->time_type = MYSQL_TIMESTAMP_DATETIME;
				}
			}
		
			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_TIMESTAMP;
			break;	
		case db_string:
			cur._real_length = cur._value.val.strVal ? strlen(cur._value.val.strVal) : 0;
			cur._max_length = __max(cur._max_length, cur._real_length);
			mysql_binder.buffer_length = (unsigned long)cur._max_length;	

			if (is_param_out(cur._in_out))
			{
				mysql_binder.buffer = cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
				if (cur._value.val.strVal)
					str_template::strcpy((char*)cur._bind_buffer, cur._value.val.strVal, os_minus_one);
				else
					memset(cur._bind_buffer, 0, cur._max_length);
			}
			else
				mysql_binder.buffer = cur._bind_buffer = (void*)cur._value.val.strVal;
			

			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_VAR_STRING;
			break;	
		case db_wstring:
			cur._real_length = cur._value.val.wstrVal ? wcslen(cur._value.val.wstrVal) : 0;
			cur._max_length = 6 * __max(cur._max_length, cur._real_length);
			mysql_binder.buffer_length = (unsigned long)cur._max_length;	

			mysql_binder.buffer = cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			if (cur._value.val.wstrVal)
			{
				string_t conv(_temp_allocator);
				str_template::unicode_to_multibyte(conv, cur._value.val.wstrVal, os_minus_one);
				// converts to utf8
				memcpy(cur._bind_buffer, (const char*)conv, conv.length() + 1);

				cur._real_length = conv.length();
			}
			else
				memset(cur._bind_buffer, 0, cur._max_length);
	

			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_VAR_STRING;
			break;	
		case db_decimal: // decimal
		case db_numeric:
			cur._max_length = 40;
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal || !cur._value.val.bufVal ? 0 : cur._max_length);

			{
				char* buf = (char*)check_pointer(_temp_allocator->allocate(cur._max_length));
				mysql_binder.buffer = cur._bind_buffer = buf;

				if (cur._value.val.bufVal)
				{
					numeric num(_temp_allocator);
					if (!num.parse_orcl(cur._value.val.bufVal))
						exception::_throw("Out of range");


					if (!num.format(buf, '.'))
						exception::_throw("Out of range");

					mysql_binder.buffer_length = (unsigned long)(cur._real_length = strlen(buf));
					cur._precision = num.precision();
					cur._scale = num.scale();
				}
			}

			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_DECIMAL;
			break;	
		case db_binary: // binary
			cur._max_length = __max(cur._max_length, cur._value.val.bufVal ? *(size_t*)cur._value.val.bufVal : 0);
			mysql_binder.buffer_length = (unsigned long)(cur._real_length = cur._value.nullVal || !cur._value.val.bufVal ? 0 : cur._max_length);	

			mysql_binder.buffer = cur._bind_buffer = cur._value.val.bufVal ? (ub1_t*)cur._value.val.bufVal + sizeof(size_t) : 0;

			cur._native_type = mysql_binder.buffer_type = MYSQL_TYPE_LONG_BLOB;

			// mysql_stmt_send_long_data ???
			break;
		default:
			exception::_throw("Unsupported parameter type");
			assert(false);
	}

	// is it the last one?
	if (_params.size() == index + 1)
	{
		check_retcode(mysql_stmt_bind_param(_stmt, &_param_binders[0]) == 0)
	}
}

void
mysql_dbserver::v_before_bind_columns()
{
	if (_cols.size())
		_column_binders.resize(*_columns_allocator, _cols.size());
}

// Binds one column with the number index
void
mysql_dbserver::v_bind_one_column(size_t index)
{
	binder& cur = _cols[index];
	
	MYSQL_BIND& mysql_binder = _column_binders[index];
	cur._value.nullVal = false;
	mysql_binder.is_null = (char*)&cur._value.nullVal;
	mysql_binder.buffer_type = (enum_field_types)cur._native_type;
	mysql_binder.length = (unsigned long*)&cur._real_length;
	mysql_binder.is_unsigned = (cur._bind_type == UNSIGNED_FLAG);

	switch (cur._native_type)
	{
		case MYSQL_TYPE_TINY:
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = sizeof(sb1_t));
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.cVal);
			break;
		case MYSQL_TYPE_SHORT:
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = sizeof(sb2_t));
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.iVal);
			break;
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONG:
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = sizeof(sb4_t));
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.lVal);
			break;
		case MYSQL_TYPE_FLOAT: 
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = sizeof(float));
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.fltVal);
			break;
		case MYSQL_TYPE_DOUBLE: 
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = sizeof(double));
#ifdef OS_64BIT
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.dblVal);
#else
			mysql_binder.buffer = cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
#endif
			break;
		case MYSQL_TYPE_TIMESTAMP: 
		case MYSQL_TYPE_DATE: 
		case MYSQL_TYPE_DATETIME: 
		case MYSQL_TYPE_TIME: 
		case MYSQL_TYPE_YEAR: 
		case MYSQL_TYPE_NEWDATE:
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = sizeof(MYSQL_TIME));
			mysql_binder.buffer = cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case MYSQL_TYPE_DECIMAL:
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = 40);
			mysql_binder.buffer = cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case MYSQL_TYPE_LONGLONG:
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = sizeof(sb8_t));
#ifdef OS_64BIT
			mysql_binder.buffer = cur._bind_buffer = &(cur._value.val.uintVal);
#else
			mysql_binder.buffer = cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
#endif
			break;
		case MYSQL_TYPE_ENUM:
		case MYSQL_TYPE_SET:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_GEOMETRY:
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = 4000);
			mysql_binder.buffer = cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
			// allocated LOB
			mysql_binder.buffer_length = (unsigned long)(cur._max_length = cur._real_length = defaultBlobSize);
			mysql_binder.buffer = cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._real_length));
			break;
		default:
			exception::_throw("Unsupported native sql type");
			assert(false);
	}

	// uses bind_type for nullability
	if (_cols.size() == index + 1)
	{
		check_retcode(mysql_stmt_bind_result(_stmt, &_column_binders[0]) == 0)
	}
}

// Get number columns in query
size_t
mysql_dbserver::v_get_number_columns()
{
	return mysql_stmt_field_count(_stmt);
}

// converts one field value to variant buffer
void
mysql_dbserver::v_convert_one_value(size_t row,  size_t index, terimber_db_value& val)
{
	const binder& cur = _cols[index];
	MYSQL_BIND& mysql_binder = _column_binders[index];

	if (cur._value.nullVal || !cur._real_length)
	{
		memset(&val, 0, sizeof(terimber_db_value));
		val.nullVal = true;
		return;
	}
	else
		val.nullVal = false;


	switch (cur._native_type)
	{
		case MYSQL_TYPE_TINY:
			val.val.cVal = cur._value.val.cVal;
			break;
		case MYSQL_TYPE_SHORT:
			val.val.iVal = cur._value.val.iVal;
			break;
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONG:
			val.val.lVal = cur._value.val.lVal;
			break;
		case MYSQL_TYPE_FLOAT: 
			val.val.fltVal = cur._value.val.fltVal;
			break;
		case MYSQL_TYPE_DOUBLE: 
#ifdef OS_64BIT
			val.val.dblVal = cur._value.val.dblVal;
#else
			val.val.dblVal = (double*)check_pointer(_data_allocator->allocate(cur._max_length));
			memcpy((void*)val.val.dblVal, cur._bind_buffer, cur._max_length);
#endif
			break;
		case MYSQL_TYPE_TIMESTAMP: 
		case MYSQL_TYPE_DATE: 
		case MYSQL_TYPE_DATETIME: 
		case MYSQL_TYPE_TIME: 
		case MYSQL_TYPE_YEAR: 
		case MYSQL_TYPE_NEWDATE:
			{
				MYSQL_TIME* ptds = (MYSQL_TIME*)cur._bind_buffer;
				sb8_t dummy64;
				date::convert_to(ptds->year, (ub1_t)ptds->month, (ub1_t)ptds->day, (ub1_t)ptds->hour, (ub1_t)ptds->minute, (ub1_t)ptds->second, (ub2_t)ptds->second_part, dummy64);
#ifdef OS_64BIT
			    val.val.intVal = dummy64;
#else
				sb8_t* dummy = (sb8_t*)check_pointer(_data_allocator->allocate(sizeof(sb8_t)));
				*dummy = dummy64;
				val.val.intVal = dummy;
#endif
			}
			break;
		case MYSQL_TYPE_DECIMAL:
			{
				const char* buf = (const char*)cur._bind_buffer;
				size_t len = (buf) ? strlen(buf) : 0;
				numeric num(buf, len, '.', _temp_allocator);
				ub1_t* bval = (ub1_t*)check_pointer(_data_allocator->allocate(num.orcl_len()));
				if (!num.persist_orcl(bval))
					exception::_throw("Out of range");
				val.val.bufVal = bval;
			}
			break;
		case MYSQL_TYPE_LONGLONG:
#ifdef OS_64BIT
			val.val.intVal = cur._value.val.intVal;
#else
			val.val.intVal = (sb8_t*)check_pointer(_data_allocator->allocate(cur._max_length));
			memcpy((void*)val.val.intVal, cur._bind_buffer, cur._max_length);
#endif
			break;
		case MYSQL_TYPE_ENUM:
		case MYSQL_TYPE_SET:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_GEOMETRY:
			{
				char* sz = (char*)check_pointer(_data_allocator->allocate(cur._real_length + 1));
				sz[cur._real_length] = 0;
				memcpy(sz, cur._bind_buffer, cur._real_length);
				val.val.strVal = sz;
			}
			break;
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
      if (cur._real_length > defaultBlobSize)
			{
				ub1_t* bval = (ub1_t*)check_pointer(_data_allocator->allocate(cur._real_length + sizeof(size_t)));
				*(ub4_t*)bval = (ub4_t)cur._real_length;
        // fetch again
        MYSQL_BIND blobBinder = mysql_binder;
        blobBinder.buffer = (ub1_t*)bval + sizeof(size_t);
        blobBinder.buffer_length = cur._real_length;
        check_retcode(mysql_stmt_fetch_column(_stmt, &blobBinder, index, 0) == 0)
				val.val.bufVal = bval;
			} 
      else
      {
				ub1_t* bval = (ub1_t*)check_pointer(_data_allocator->allocate(cur._real_length + sizeof(size_t)));
				*(ub4_t*)bval = (ub4_t)cur._real_length;
				memcpy((ub1_t*)bval + sizeof(size_t), cur._bind_buffer, cur._real_length);
				val.val.bufVal = bval;
      }
			break;
		default:
			exception::_throw("Unsupported native sql type");
			assert(false);
	}
}

// Gets column info from native driver
void
mysql_dbserver::v_get_one_column_info(size_t index)
{
	binder& cur = _cols[index];

	cur._real_length = _stmt->fields[index].length;
	cur.set_name(_columns_allocator, _stmt->fields[index].name);
	cur._native_type = _stmt->fields[index].type;
	cur._scale = _stmt->fields[index].decimals;
	cur._precision = cur._real_length - cur._scale;
	cur._value.nullVal = IS_NOT_NULL(_stmt->fields[index].flags);
	cur._max_length = __max(_stmt->fields[index].max_length, _stmt->fields[index].length);
	// place holder for unsigned 
	cur._bind_type = (_stmt->fields[index].flags & UNSIGNED_FLAG);
}

// Forms SQL expression depends on type select and native driver
void
mysql_dbserver::v_form_sql_string()
{
	size_t count_params = _params.size();
	switch (get_action())
	{
		case ACTION_NONE:
		case ACTION_EXEC_SQL:
		case ACTION_OPEN_SQL: // nothing to do
		case ACTION_EXEC_SQL_ASYNC:
		case ACTION_OPEN_SQL_ASYNC: // nothing to do
		case ACTION_FETCH:
		case ACTION_FETCH_ASYNC:
			break;
		case ACTION_OPEN_PROC:
		case ACTION_EXEC_PROC:
		case ACTION_OPEN_PROC_ASYNC:
		case ACTION_EXEC_PROC_ASYNC:
			{
				string_t val(_temp_allocator);
				char q[2] = {_quote, 0};
				val = "call ";
				val += _sql;
				val += "("; // compatibility with old versions

				for (size_t index = 0; index < count_params; index++)
				{
					if (0 != index)
						val += ", ";	

					val += q;
				}
				
				val += ")";
		
				_sql = val;
			}
			break;
		default:
			assert(false);
	}
}

// Rebinds one param for input/output or output params
void
mysql_dbserver::v_rebind_one_param(size_t index)
{
	binder& cur = _params[index];
	if (!is_param_out(cur._in_out)) // nothing rebind
		return;

	cur._value.nullVal = false;

	if (cur._real_length == os_minus_one)
	{
		cur._value.nullVal = true;
		memset(&cur._value.val, 0, sizeof(terimber_db_value));
		return;
	}

	switch (cur._native_type)
	{
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_FLOAT: 
			break;
		case MYSQL_TYPE_DOUBLE:
#ifdef OS_64BIT
			cur._value.val.dblVal = cur._value.val.dblVal;
#else
			cur._value.val.dblVal = (double*)check_pointer(cur.allocate_value(0));
			memcpy((void*)cur._value.val.dblVal, cur._bind_buffer, cur._max_length);
#endif
			break;
		case MYSQL_TYPE_TIMESTAMP: 
		case MYSQL_TYPE_DATE: 
		case MYSQL_TYPE_DATETIME: 
		case MYSQL_TYPE_TIME: 
		case MYSQL_TYPE_YEAR: 
		case MYSQL_TYPE_NEWDATE:
			{
				const MYSQL_TIME* ptds = (const MYSQL_TIME*)cur._bind_buffer;
				sb8_t dummy64;
				date::convert_to(ptds->year, (ub1_t)ptds->month, (ub1_t)ptds->day, (ub1_t)ptds->hour, (ub1_t)ptds->minute, (ub1_t)ptds->second, (ub2_t)ptds->second_part, dummy64);
#ifdef OS_64BIT
			    cur._value.val.intVal = dummy64;
#else
				sb8_t* dummy = (sb8_t*)check_pointer(cur.allocate_value(0));
				*dummy = dummy64;
				cur._value.val.intVal = dummy;
#endif
			}
			break;
		case MYSQL_TYPE_LONGLONG:
#ifdef OS_64BIT
			cur._value.val.intVal = cur._value.val.intVal;
#else
			cur._value.val.intVal = (sb8_t*)check_pointer(cur.allocate_value(0));
			memcpy((void*)cur._value.val.intVal, cur._bind_buffer, cur._max_length);
#endif
			break;	
		case MYSQL_TYPE_DECIMAL:
			{
				const char* buf = (const char*)cur._bind_buffer;
				size_t len = (buf) ? strlen(buf) : 0;
				numeric num(buf, len, '.', _temp_allocator);
				ub1_t* bval = (ub1_t*)check_pointer(cur.allocate_value(num.orcl_len()));
				if (!num.persist_orcl(bval))
					exception::_throw("Out of range");
				cur._value.val.bufVal = bval;
			}
			break;
		case MYSQL_TYPE_ENUM:
		case MYSQL_TYPE_SET:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_GEOMETRY:
			{
				char* sz = (char*)check_pointer(cur.allocate_value(cur._real_length));
				sz[cur._real_length] = 0;
				memcpy(sz, cur._bind_buffer, cur._real_length);
			}
			break;
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
			{
				ub1_t* bval = (ub1_t*)check_pointer(cur.allocate_value(cur._real_length));
				memcpy(bval, cur._bind_buffer, cur._real_length);				
			}
			break;
		default: 
			exception::_throw("Unsupported native sql type");
			assert(false);
	}
}

// Terminates executing
void
mysql_dbserver::v_interrupt_async()
{
	// there is no cancel function
	v_close();
}

// converts types
dbtypes
mysql_dbserver::v_native_type_to_client_type(size_t native_type)
{
	switch (native_type)
	{
		case MYSQL_TYPE_NULL:
			return db_unknown;
		case MYSQL_TYPE_TINY:
			return db_sb1;
		case MYSQL_TYPE_SHORT:
			return db_sb2;
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONG:
			return db_sb4;
		case MYSQL_TYPE_FLOAT: 
			return db_float; 
		case MYSQL_TYPE_DOUBLE: 
			return db_double; 
		case MYSQL_TYPE_TIMESTAMP: 
		case MYSQL_TYPE_DATE: 
		case MYSQL_TYPE_DATETIME: 
		case MYSQL_TYPE_TIME: 
		case MYSQL_TYPE_YEAR: 
		case MYSQL_TYPE_NEWDATE:
			return db_date; 
		case MYSQL_TYPE_DECIMAL:
			return db_decimal;
		case MYSQL_TYPE_LONGLONG:
			return db_sb8;
		case MYSQL_TYPE_ENUM:
		case MYSQL_TYPE_SET:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_GEOMETRY:
			return db_string; 
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_BLOB:
			return db_binary; 
		default:
			assert(false);
			return db_unknown;
	}
}

void 
mysql_dbserver::parse_connection_string(const char* connection_string, string_t& user, string_t& password, string_t& host, unsigned short& port, string_t& database)
{
	// UID=;PWD=;HOST=;PORT=;DB=;

	if (!connection_string)
		return;

	const char* semicolumn = 0;

	const char* find = strstr(connection_string, "UID=");
	size_t len = strlen("UID=");
	if (find)
	{
		semicolumn = strchr(find + len, ';');
		user.assign(find + len, semicolumn ? semicolumn - find -len : os_minus_one);
	}
	

	find = strstr(connection_string, "PWD=");
	len = strlen("PWD=");
	if (find)
	{
		semicolumn = strchr(find + len, ';');
		password.assign(find + len, semicolumn ? semicolumn - find -len : os_minus_one);
	}
	
	find = strstr(connection_string, "HOST=");
	len = strlen("HOST=");
	if (find)
	{
		semicolumn = strchr(find + len, ';');
		host.assign(find + len, semicolumn ? semicolumn - find -len : os_minus_one);
	}
	
	find = strstr(connection_string, "DB=");
	len = strlen("DB=");
	if (find)
	{
		semicolumn = strchr(find + len, ';');
		database.assign(find + len, semicolumn ? semicolumn - find -len : os_minus_one);
	}

	find = strstr(connection_string, "PORT=");
	len = strlen("PORT=");
	if (find)
	{
		semicolumn = strchr(find + len, ';');
		string_t strport;
		strport.assign(find + len, semicolumn ? semicolumn - find -len : os_minus_one);
		if (strport.length())
			str_template::strscan((const char*)strport, 32, "%hu", &port);
	}
}

/////////////////////////////////////////////////////////////////////////////////
// pool support
//! \brief creates object
//static
db_entry* 
mysql_db_creator::create(			const db_arg& arg						
					)
{
	db_entry* obj = 0;
	if (0 != (obj = new db_entry(arg._ident, arg._trusted, arg._login_string)))
	{
		if (!(obj->_obj = new mysql_dbserver(arg._ident)))
			delete obj, obj = 0;
	}

	return obj;
}

#pragma pack()
END_TERIMBER_NAMESPACE
