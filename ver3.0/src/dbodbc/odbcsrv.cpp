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
#include "dbodbc/odbcsrv.h"
#include "base/vector.hpp"
#include "base/list.hpp"
#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/numeric.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

#define check_retcode_db(x) \
if (x != SQL_SUCCESS) odbc_dbserver::get_odbc_error(x, _henv, _hdbc, 0);

#define check_retcode(x) \
if (x != SQL_SUCCESS) odbc_dbserver::get_odbc_error(x, _henv, _hdbc, _hstmt);

//#if (OS_TYPE != OS_WIN32) || (defined(_MSC_VER) && (_MSC_VER > 1200))

#ifndef SQLLEN
#define SQLLEN SQLINTEGER
#endif
#ifndef SQLULEN
#define SQLULEN SQLUINTEGER
#endif

static const size_t MAX_CONNECT_LEN = 2048;
static const char* valid_delimeters = " ,)}";
static const size_t MAX_FNAME_LEN = 256;

// static 
void 
odbc_dbserver::get_odbc_error(size_t code, SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt)
{
	SWORD nOutlen;
	SDWORD lNative = 0;	
#if OS_TYPE == OS_WIN32
	SQLWCHAR lpszMsg[SQL_MAX_MESSAGE_LENGTH + 1] = {0};
	SQLWCHAR lpszState[SQL_SQLSTATE_SIZE + 1] = {0};
	// extracts ODBC error as UNICODE string
	RETCODE retCode = ::SQLErrorW(henv, hdbc, hstmt, lpszState, &lNative,
		lpszMsg, SQL_MAX_MESSAGE_LENGTH, &nOutlen);

	// checks state
	if (code != SQL_SUCCESS_WITH_INFO && (!lpszState[0] || wcscmp(lpszState, L"00000") != 0))
		exception::_throw(lpszMsg);
#else
	SQLCHAR lpszMsg[SQL_MAX_MESSAGE_LENGTH + 1] = {0};
	SQLCHAR lpszState[SQL_SQLSTATE_SIZE + 1] = {0};
	// extracts ODBC error as UNICODE string
	RETCODE retCode = ::SQLError(henv, hdbc, hstmt, lpszState, &lNative,
		lpszMsg, SQL_MAX_MESSAGE_LENGTH, &nOutlen);

	// checks state
	if (code != SQL_SUCCESS_WITH_INFO && (!lpszState[0] || strcmp((const char*)lpszState, "00000") != 0))
		exception::_throw((const char*)lpszMsg);
#endif
}

/////////////////////////
odbc_dbserver::odbc_dbserver(size_t ident) : dbserver_impl(ident),
	_henv(SQL_NULL_HENV),
	_hdbc(SQL_NULL_HDBC),
	_hstmt(SQL_NULL_HSTMT),
	_bind_cols(false)
{
}

odbc_dbserver::~odbc_dbserver()
{
	if (_is_open_sql())
		close_sql(); 

	// forces child destruction
	if (_is_connect()) 
		disconnect();
}

// database specific
void
odbc_dbserver::v_connect(bool trusted_connection, const char* connection_string)
{
	// allocates environmental handles
	RETCODE retCode = ::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_henv);
	check_retcode_db(retCode)
	// supports drivers >= 3 version
	retCode = ::SQLSetEnvAttr(_henv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, SQL_IS_INTEGER);
	check_retcode_db(retCode)
	// allocates database handle
	retCode = ::SQLAllocHandle(SQL_HANDLE_DBC, _henv, &_hdbc);
	check_retcode_db(retCode)

	// trusted connection
	retCode = ::SQLSetConnectAttr(_hdbc, 
#if defined(_MSC_VER) && (_MSC_VER > 1200)  // .Net
	SQL_COPT_SS_INTEGRATED_SECURITY,
#else // VC 6.0
    SQL_INTEGRATED_SECURITY,
#endif

					trusted_connection ? (void*)SQL_IS_ON : (void*)SQL_IS_OFF, SQL_IS_INTEGER); 
	check_retcode_db(retCode)
	SWORD nResult;

#if OS_TYPE == OS_WIN32
	SQLWCHAR szConnectOutput[MAX_CONNECT_LEN + 1];
	// tries to establish connection
	retCode = ::SQLDriverConnectW(_hdbc, NULL, (SQLWCHAR*)str_template::multibyte_to_unicode(/*CP_UTF8,*/ *_temp_allocator, connection_string), SQL_NTS,
		szConnectOutput, MAX_CONNECT_LEN, &nResult, SQL_DRIVER_NOPROMPT);
#else
	SQLCHAR szConnectOutput[MAX_CONNECT_LEN + 1];
	// tries to establish connection
	retCode = ::SQLDriverConnect(_hdbc, NULL, (SQLCHAR*)(const char*)connection_string, SQL_NTS,
		szConnectOutput, MAX_CONNECT_LEN, &nResult, SQL_DRIVER_NOPROMPT);
#endif

	check_retcode_db(retCode)
}

void
odbc_dbserver::v_disconnect()
{
	_temp_allocator->clear_extra();
	// releases handles, if any
	v_close();

	if (SQL_NULL_HDBC != _hdbc)
	{
		::SQLDisconnect(_hdbc);
		::SQLFreeHandle(SQL_HANDLE_DBC, _hdbc);
		_hdbc = SQL_NULL_HDBC;
	}

	if (SQL_NULL_HENV != _henv) 
	{
		::SQLFreeHandle(SQL_HANDLE_ENV, _henv);
		_henv = SQL_NULL_HENV;
	}
}

void 
odbc_dbserver::v_start_transaction()
{
	// turns off autocommit
	RETCODE retCode = ::SQLSetConnectAttr(_hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, SQL_IS_INTEGER);
	check_retcode_db(retCode)
}

void 
odbc_dbserver::v_commit()
{
	// ends transaction as commit
	RETCODE retCode = ::SQLEndTran(SQL_HANDLE_DBC, _hdbc, SQL_COMMIT);
	// Turns auto commit back on
	::SQLSetConnectAttr(_hdbc, SQL_AUTOCOMMIT, (void*)SQL_AUTOCOMMIT_ON, SQL_IS_INTEGER);
	check_retcode_db(retCode)
}

void 
odbc_dbserver::v_rollback()
{
	// ends transaction as a rollback
	RETCODE retCode = ::SQLEndTran(SQL_HANDLE_DBC, _hdbc, SQL_ROLLBACK);
	// Turns auto commit back on
	::SQLSetConnectAttr(_hdbc, SQL_AUTOCOMMIT, (void*)SQL_AUTOCOMMIT_ON, SQL_IS_INTEGER);
	check_retcode_db(retCode)
}

bool 
odbc_dbserver::v_is_connect_alive()
{
	//  checks connection first
	if (!_is_connect())
		return false;

	SQLINTEGER value = SQL_CD_TRUE;
	::SQLGetConnectAttr(_hdbc, SQL_COPT_SS_CONNECTION_DEAD, &value, SQL_IS_INTEGER, 0);
	return value == SQL_CD_FALSE;
}
/////////////////////
void
odbc_dbserver::v_before_execute()
{
	RETCODE retCode = ::SQLAllocHandle(SQL_HANDLE_STMT, _hdbc, &_hstmt);
	check_retcode(retCode)

	// prepares cursor type
	switch (get_action())
	{
		case ACTION_OPEN_SQL:
		case ACTION_OPEN_SQL_ASYNC:
			retCode = ::SQLSetStmtAttr(_hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, SQL_IS_INTEGER);
			check_retcode(retCode)
			break;
		case ACTION_OPEN_PROC:
		case ACTION_OPEN_PROC_ASYNC:
			retCode = ::SQLSetStmtAttr(_hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, SQL_IS_INTEGER);
			check_retcode(retCode)
			break;
		default:
			break;
	} // switch

	// sets bind type for return recordset
	retCode = ::SQLSetStmtAttr(_hstmt, SQL_ATTR_ROW_BIND_TYPE, (void*)SQL_BIND_BY_COLUMN, 0);
	check_retcode(retCode)

	// sets bind type for in/output parameters
	retCode = ::SQLSetStmtAttr(_hstmt, SQL_ATTR_PARAM_BIND_TYPE, (void*)SQL_BIND_BY_COLUMN, 0);
	check_retcode(retCode)

	// checks bulk params
	SQLUINTEGER bulk = (SQLUINTEGER)_bulk_params.size();
	retCode = ::SQLSetStmtAttr(_hstmt, SQL_ATTR_PARAMSET_SIZE, (void*)(bulk ? bulk : 1), 0);
	check_retcode(retCode)
}

void
odbc_dbserver::v_after_execute()
{
	switch (get_action())
	{
		case ACTION_EXEC_SQL:
		case ACTION_EXEC_SQL_ASYNC:
		case ACTION_EXEC_PROC:
		case ACTION_EXEC_PROC_ASYNC:
			{
				size_t maxRes = 1024;
				while (SQL_SUCCESS == ::SQLMoreResults(_hstmt) && maxRes-- > 0);
			}
			break;
		case ACTION_OPEN_SQL:
		case ACTION_OPEN_SQL_ASYNC:
		case ACTION_OPEN_PROC:
		case ACTION_OPEN_PROC_ASYNC:
			{
				size_t maxRes = 1024;
				SQLLEN infoValuePtr = 0;
				RETCODE retCode = ::SQLRowCount(_hstmt, &infoValuePtr);
				while (!retCode && infoValuePtr > 0 && maxRes-- > 0)
				{
					retCode = ::SQLMoreResults(_hstmt);
					if (!retCode)
					{
						infoValuePtr = 0;
						retCode = ::SQLRowCount(_hstmt, &infoValuePtr);
					}
					else if (retCode == SQL_SUCCESS_WITH_INFO)
					{
						SWORD nOutlen;
						SDWORD lNative = 0;	
#if OS_TYPE == OS_WIN32
						SQLWCHAR lpszMsg[SQL_MAX_MESSAGE_LENGTH + 1] = {0};
						SQLWCHAR lpszState[SQL_SQLSTATE_SIZE + 1] = {0};
						// extracts ODBC error as a UNICODE string
						::SQLErrorW(_henv, _hdbc, _hstmt, lpszState, &lNative,
							lpszMsg, SQL_MAX_MESSAGE_LENGTH, &nOutlen);
#else
						SQLCHAR lpszMsg[SQL_MAX_MESSAGE_LENGTH + 1] = {0};
						SQLCHAR lpszState[SQL_SQLSTATE_SIZE + 1] = {0};
						// extracts ODBC error as a UNICODE string
						::SQLError(_henv, _hdbc, _hstmt, lpszState, &lNative,
							lpszMsg, SQL_MAX_MESSAGE_LENGTH, &nOutlen);
#endif
					}
				}
			} // block
			break;
		default:
			break;
	} // switch
}

// before binding columns sometimes we have db specific
void 
odbc_dbserver::v_before_bind_columns()
{
	// before binding - checks mode
	_bind_cols = true;
	bool find_blob = false;
	size_t cols = _cols.size();
	for (size_t col_ind = 0; _bind_cols && col_ind < cols; ++col_ind)
	{
		switch (_cols[col_ind]._native_type)
		{
			case SQL_LONGVARCHAR:
			case SQL_WLONGVARCHAR:
			case SQL_LONGVARBINARY:
				find_blob = true;
				break;
			default:
				if (find_blob)
					_bind_cols = false;
				break;
		} // switch
	} /// for


	if (!_bind_cols)
		_bulk_rows = 1;
}

// Executes SQL expression
void
odbc_dbserver::v_execute()
{
#if OS_TYPE == OS_WIN32
	// executes unicode sql statment
	RETCODE retCode = ::SQLExecDirectW(_hstmt, (SQLWCHAR*)str_template::multibyte_to_unicode(/*CP_UTF8,*/ *_temp_allocator, _sql), SQL_NTS);
#else
	// executes unicode sql statment
	RETCODE retCode = ::SQLExecDirect(_hstmt, (SQLCHAR*)(const char*)_sql, SQL_NTS);
#endif
	check_retcode(retCode)
}

// closes opened query
void
odbc_dbserver::v_close()
{
	if (_hstmt != SQL_NULL_HSTMT)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, _hstmt);
		_hstmt = SQL_NULL_HSTMT;
	}
}

// Fetches block rows
void
odbc_dbserver::v_fetch()
{
	RETCODE retCode;
	size_t select_row = 0;
	// defines the column count
	size_t col_count = _cols.size();
	SQLUINTEGER row_count = 0;
	SQLUINTEGER row_fetch = (SQLUINTEGER)__min(_bulk_rows, _requested_rows);
	
	// sets the attributes
	retCode = ::SQLSetStmtAttr(_hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (void*)row_fetch, 0);
	check_retcode(retCode)
	retCode = ::SQLSetStmtAttr(_hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &row_count, 0);
	check_retcode(retCode)

	SQLUSMALLINT orientation = _start_row ? SQL_FETCH_ABSOLUTE : (_forward ? SQL_FETCH_NEXT : SQL_FETCH_PRIOR);
	// defines the first row for fetching
	SQLINTEGER offset = (SQLINTEGER)_start_row;
	offset *= _forward ? 1 : -1;
	while (_requested_rows
			&& (retCode = ::SQLFetchScroll(_hstmt, orientation, offset)) != SQL_NO_DATA_FOUND)
	{
		check_retcode(retCode)

		if (STATE_INTERRUPTED == get_state()) // check interrupt
			exception::_throw("Fetching process is interrupted");

		// loop for all rows
		for (size_t rows = 0; rows < row_count; ++rows)
		{
			// adds empty row to recordset
			_vector< terimber_db_value > val;
			recordset_list_t::iterator iter_val = _data.push_back(*_data_allocator, val);
			if (iter_val == _data.end())
				exception::_throw("Not enough memory");

			// resizes row to the column count
			if (!iter_val->resize(*_data_allocator, col_count))
				exception::_throw("Not enough memory");
			// copies/converts data from the buffer to the row
			for (size_t index = 0; index < col_count; ++index)
				v_convert_one_value(rows, index, (*iter_val)[index]);

			// increment row counter
			++select_row;
			// increment/decrement offset for FETCH_ABSOLUTE mode
			_forward ? ++offset : --offset;
			// increment/decrement _requested_rows
			--_requested_rows;
		}

		// adjusts rows, if any
		if (_requested_rows > 0 && _requested_rows < row_fetch)
		{
			row_fetch = (SQLUINTEGER)_requested_rows;
			retCode = ::SQLSetStmtAttr(_hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (void*)row_fetch, 0);
			check_retcode(retCode)
		}
	}

	// sets the real fetched row count
	_fetched_rows = select_row;
}

// Formed SQL expression depends on the type of the select and native driver
void
odbc_dbserver::v_form_sql_string()
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
				val = "{call ";
				val += _sql;

				for (size_t index = 0; index < count_params; index++)
				{
					if (is_param_out(_params[index]._in_out))
					{
						if (0 == index)
							val += "(:";
						else
							val += ", :";
					}
					else
					{
						if (0 == index)
							val += "(:";
						else
							val += ", :";	
					}
				}
				
				if (count_params != 0)
					val += ")";

				val += "}";
				
				_sql = val;
			}
			break;
		default:
			assert(false);
	}
}

// replace quote to available sign for native drive
void
odbc_dbserver::v_replace_quote()
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
		*new_sql++ = '?'; // replaces ':' on '?'
		// skips parameter name
		// until valid_delimeters appears
		begin += strcspn(begin, valid_delimeters);
	}

	// checks the params count
	if (count_params != _params.size())
		exception::_throw("Number of parameters doesn't match the sql statement syntax");

	// copies the rest of the bytes
	str_template::strcpy(new_sql, begin, os_minus_one);
	// resets sql
	_sql = buf;
}

// Binds one param with the number index 
void
odbc_dbserver::v_bind_one_param(size_t index)
{
	binder& cur = _params[index];
	size_t num = _bulk_params.size(); // number bulk params

	// allocates length/status
	cur._real_length = (size_t)check_pointer(_temp_allocator->allocate((num ? num : 1) * sizeof(SQLINTEGER)));

	size_t i;
	bulk_params_t::const_iterator iter(_bulk_params.end());

	switch (cur._type)
	{
		case db_bool: // bool
			cur._max_length = sizeof(bool);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.boolVal, cur._max_length); // set value
				}
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.boolVal, cur._max_length); // set value
			}

			cur._native_type = (size_t)SQL_BIT;
			cur._bind_type = (size_t)SQL_C_BIT;
			cur._precision = 1;
			cur._scale = 0;
			break;	
		case db_sb1: // char
			cur._max_length = sizeof(sb1_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.cVal, cur._max_length); // set value
				}
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.cVal, cur._max_length); // set value
			}
			cur._native_type = (size_t)SQL_TINYINT;
			cur._bind_type = (size_t)SQL_C_TINYINT;
			cur._precision = 3;
			cur._scale = 0;
			break;	
		case db_ub1: // byte
			cur._max_length = sizeof(ub1_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.bVal, cur._max_length); // set value
				}
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.bVal, cur._max_length); // set value
			}
			cur._native_type = (size_t)SQL_TINYINT;
			cur._bind_type = (size_t)SQL_C_TINYINT;
			cur._precision = 3;
			cur._scale = 0;
			break;	
		case db_sb2:
			cur._max_length = sizeof(ub2_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.iVal, cur._max_length); // set value
				}
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.iVal, cur._max_length); // set value
			}
			cur._native_type = SQL_SMALLINT;
			cur._bind_type = SQL_C_SSHORT;
			cur._precision = 5;
			cur._scale = 0;
			break;	
		case db_ub2:
			cur._max_length = sizeof(ub2_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.uiVal, cur._max_length); // set value
				}
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.uiVal, cur._max_length); // set value
			}
			cur._native_type = SQL_SMALLINT;
			cur._bind_type = SQL_C_SSHORT;
			cur._precision = 5;
			cur._scale = 0;
			break;	
		case db_sb4: 
			cur._max_length = sizeof(sb4_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.lVal, cur._max_length); // set value
				}
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.lVal, cur._max_length); // set value
			}
			cur._native_type = SQL_INTEGER;
			cur._bind_type = SQL_C_LONG;
			cur._precision = 10;
			cur._scale = 0;
			break;	
		case db_ub4: 
			cur._max_length = sizeof(ub4_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.ulVal, cur._max_length); // set value
				}
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.ulVal, cur._max_length); // set value
			}
			cur._native_type = SQL_INTEGER;
			cur._bind_type = SQL_C_LONG;
			cur._precision = 10;
			cur._scale = 0;
			break;	
		case db_float: 
			cur._max_length = sizeof(float);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.fltVal, cur._max_length); // set value
				}
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.fltVal, cur._max_length); // set value
			}
			cur._native_type = SQL_REAL;
			cur._bind_type = SQL_C_FLOAT;
			cur._precision = 7;
			cur._scale = 0;
			break;	
		case db_double: 
			cur._max_length = sizeof(double);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
#ifdef OS_64BIT
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.dblVal, cur._max_length); // set value
#else
					*((SQLINTEGER*)cur._real_length + i) = ((*iter)[index]._value.nullVal || !(*iter)[index]._value.val.dblVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;
					if ((*iter)[index]._value.val.dblVal)
						memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, (*iter)[index]._value.val.dblVal, cur._max_length); // set value
					else
						memset((ub1_t*)cur._bind_buffer + i * cur._max_length, 0, cur._max_length); // set value
#endif
				}
			}
			else
			{
#ifdef OS_64BIT
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.dblVal, cur._max_length); // set value
#else
				*(SQLINTEGER*)cur._real_length = (cur._value.nullVal || !cur._value.val.dblVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				if (cur._value.val.dblVal)
					memcpy(cur._bind_buffer, cur._value.val.dblVal, cur._max_length); // set value
				else
					memset(cur._bind_buffer, 0, cur._max_length); // set value

#endif
			}
			cur._native_type = SQL_DOUBLE;
			cur._bind_type = SQL_C_DOUBLE;
			cur._precision = 15;
			cur._scale = 0;
			break;	
		case db_sb8:
			cur._max_length = sizeof(sb8_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
#ifdef OS_64BIT
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.intVal, cur._max_length); // set value
#else
					*((SQLINTEGER*)cur._real_length + i) = ((*iter)[index]._value.nullVal || !(*iter)[index]._value.val.intVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;
					if ((*iter)[index]._value.val.intVal)
						memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, (*iter)[index]._value.val.intVal, cur._max_length); // set value
					else
						memset((ub1_t*)cur._bind_buffer + i * cur._max_length, 0, cur._max_length); // set value
#endif
				}
			}
			else
			{
#ifdef OS_64BIT
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.intVal, cur._max_length); // set value
#else
				*(SQLINTEGER*)cur._real_length = (cur._value.nullVal || !cur._value.val.intVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				if (cur._value.val.intVal)
					memcpy(cur._bind_buffer, cur._value.val.intVal, cur._max_length); // set value
				else
					memset(cur._bind_buffer, 0, cur._max_length); // set value

#endif
			}
			cur._native_type = (size_t)SQL_BIGINT;
			cur._bind_type = (size_t)SQL_C_SBIGINT;
			cur._precision = 10;
			cur._scale = 0;
			break;	
		case db_ub8:
			cur._max_length = sizeof(ub8_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
#ifdef OS_64BIT
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
					memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, &(*iter)[index]._value.val.uintVal, cur._max_length); // set value
#else
					*((SQLINTEGER*)cur._real_length + i) = ((*iter)[index]._value.nullVal || !(*iter)[index]._value.val.uintVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;
					if ((*iter)[index]._value.val.uintVal)
						memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, (*iter)[index]._value.val.uintVal, cur._max_length); // set value
					else
						memset((ub1_t*)cur._bind_buffer + i * cur._max_length, 0, cur._max_length); // set value
#endif
				}
			}
			else
			{
#ifdef OS_64BIT
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				memcpy(cur._bind_buffer, &cur._value.val.uintVal, cur._max_length); // set value
#else
				*(SQLINTEGER*)cur._real_length = (cur._value.nullVal || !cur._value.val.uintVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				if (cur._value.val.uintVal)
					memcpy(cur._bind_buffer, cur._value.val.uintVal, cur._max_length); // set value
				else
					memset(cur._bind_buffer, 0, cur._max_length); // set value

#endif
			}
			cur._native_type = (size_t)SQL_BIGINT;
			cur._bind_type = (size_t)SQL_C_SBIGINT;
			cur._precision = 10;
			cur._scale = 0;
			break;	
		case db_date: 
			cur._max_length = sizeof(TIMESTAMP_STRUCT);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
#ifdef OS_64BIT
					*((SQLINTEGER*)cur._real_length + i) = (*iter)[index]._value.nullVal ? SQL_NULL_DATA : 0;	
#else
					*((SQLINTEGER*)cur._real_length + i) = ((*iter)[index]._value.nullVal || !(*iter)[index]._value.val.intVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
#endif
					TIMESTAMP_STRUCT* ptds = (TIMESTAMP_STRUCT*)cur._bind_buffer + i;
					ub4_t year32;
					ub1_t month8, day8, hour8, minute8, second8, wday8;
					ub2_t millisec16, yday16;
#ifdef OS_64BIT
					if (!(*iter)[index]._value.nullVal)
#else
					if (!(*iter)[index]._value.nullVal && (*iter)[index]._value.val.intVal)
#endif
					{
						date::convert_from(
#ifdef OS_64BIT
							(*iter)[index]._value.val.intVal,
#else
							*(*iter)[index]._value.val.intVal, 
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
						ptds->fraction = millisec16 * 1000000;
					}
					else
					{
						memset((ub1_t*)cur._bind_buffer + i * cur._max_length, 0, cur._max_length); // set value
					}
				}
			}
			else
			{
#ifdef OS_64BIT
				*(SQLINTEGER*)cur._real_length = (cur._value.nullVal || !cur._value.val.intVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				if (!cur._value.nullVal)
#else
				*(SQLINTEGER*)cur._real_length = cur._value.nullVal ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				if (!cur._value.nullVal && cur._value.val.intVal)
#endif			
				{
					TIMESTAMP_STRUCT* ptds = (TIMESTAMP_STRUCT*)cur._bind_buffer;
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
					ptds->fraction = millisec16 * 1000000;
				}
				else
				{
					memset((ub1_t*)cur._bind_buffer, 0, cur._max_length); // set value
				}
			}

			cur._native_type = SQL_TIMESTAMP;
			cur._bind_type = SQL_C_TIMESTAMP;
			cur._precision = 23;
			cur._scale = 3;
			break;	
		case db_string:
			// gets max length first
			if (num)
			{
				size_t max_len = 0;
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					size_t len = ((*iter)[index]._value.nullVal || !(*iter)[index]._value.val.strVal) ? 0 : str_template::strlen((*iter)[index]._value.val.strVal);
					*((SQLINTEGER*)cur._real_length + i) = !len ? SQL_NULL_DATA : SQL_NTS;	

					if (len > max_len)
						max_len = len;
				}

				if (!is_param_out(cur._in_out))
					cur._max_length = max_len;
				else
					cur._max_length = __max(cur._max_length, max_len);
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = (cur._value.nullVal || !cur._value.val.strVal) ? SQL_NULL_DATA : SQL_NTS;
				// does not override the output length
				if (!is_param_out(cur._in_out))
					cur._max_length = (!cur._value.nullVal && cur._value.val.strVal) ? str_template::strlen(cur._value.val.strVal) : 0;
				else
					cur._max_length = __max(cur._max_length, (!cur._value.nullVal && cur._value.val.strVal) ? str_template::strlen(cur._value.val.strVal) : 0);
			}

			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * (cur._max_length + 1)));

			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					memset((char*)cur._bind_buffer + i * (cur._max_length + 1), 0, cur._max_length + 1); // set value
					if ((*iter)[index]._value.val.strVal)
						str_template::strcpy((char*)cur._bind_buffer + i * (cur._max_length + 1), (*iter)[index]._value.val.strVal, os_minus_one); // set value
				}
			}
			else
			{
				if (!cur._value.nullVal && cur._value.val.strVal)
					str_template::strcpy((char*)cur._bind_buffer, cur._value.val.strVal, os_minus_one);
				else
					memset(cur._bind_buffer, 0, cur._max_length + 1);
			}
			cur._bind_type = SQL_C_CHAR;
			cur._precision = cur._max_length ? cur._max_length : 1;
			cur._native_type = cur._precision > 4000 ? SQL_LONGVARCHAR : SQL_VARCHAR;
			cur._scale = 0;
			cur._max_length += 1;
			break;	
		case db_wstring:
			// NB! multiplies length by sizeof(wchar_t)
			if (num)
			{
				size_t max_len = 0;
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					size_t len = ((*iter)[index]._value.nullVal || !(*iter)[index]._value.val.wstrVal) ? 0 : str_template::strlen((*iter)[index]._value.val.wstrVal);
					*((SQLINTEGER*)cur._real_length + i) = !len ? SQL_NULL_DATA : SQL_NTS;	

					if (len > max_len)
						max_len = len;
				}

				if (!is_param_out(cur._in_out))
					cur._max_length = max_len;
				else
					cur._max_length = __max(cur._max_length, max_len);
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = (cur._value.nullVal || !cur._value.val.wstrVal) ? SQL_NULL_DATA : SQL_NTS;	
				// does not override the output length
				if (!is_param_out(cur._in_out))
					cur._max_length = (!cur._value.nullVal && cur._value.val.wstrVal) ? str_template::strlen(cur._value.val.wstrVal) : 0;
				else
					cur._max_length = __max(cur._max_length, (!cur._value.nullVal && cur._value.val.wstrVal) ? str_template::strlen(cur._value.val.wstrVal) : 0);
			}

			// allocates buffer - room for tailing zero byte
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * (cur._max_length + 1) * sizeof(wchar_t)));

			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					memset((wchar_t*)cur._bind_buffer + i * (cur._max_length + 1), 0, cur._max_length + 1); // set value
					if ((*iter)[index]._value.val.wstrVal)
						str_template::strcpy((wchar_t*)cur._bind_buffer + i * (cur._max_length + 1), (*iter)[index]._value.val.wstrVal, os_minus_one); // set value
				}
			}
			else
			{
				if (!cur._value.nullVal && cur._value.val.wstrVal)
					str_template::strcpy((wchar_t*)cur._bind_buffer, cur._value.val.wstrVal, os_minus_one);
				else
					memset(cur._bind_buffer, 0, sizeof(wchar_t) * (cur._max_length + 1));
			}
			
			cur._bind_type = (size_t)SQL_C_WCHAR;
			cur._precision = cur._max_length ? cur._max_length : 1;
			cur._native_type = cur._precision > 4000 ? SQL_WLONGVARCHAR : SQL_WVARCHAR;
			cur._scale = 0;
			cur._max_length += 1;
			cur._max_length *= sizeof(wchar_t);
			break;	
		case db_binary: // binary
			if (num)
			{
				size_t max_len = 0;
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					size_t len = ((*iter)[index]._value.nullVal || !(*iter)[index]._value.val.bufVal) ? 0 : *(size_t*)(*iter)[index]._value.val.bufVal;
					*((SQLINTEGER*)cur._real_length + i) = !len ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	

					if (len > max_len)
						max_len = len;
				}

				if (!is_param_out(cur._in_out))
					cur._max_length = max_len;
				else
					cur._max_length = __max(cur._max_length, max_len);
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = (cur._value.nullVal || !cur._value.val.bufVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;	
				// does not override the output length
				if (!is_param_out(cur._in_out))
					cur._max_length = (!cur._value.nullVal && cur._value.val.bufVal) ? *(size_t*)cur._value.val.bufVal : 0;
				else
					cur._max_length = __max(cur._max_length, (!cur._value.nullVal && cur._value.val.bufVal) ? *(size_t*)cur._value.val.bufVal : 0);
			}

			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));

			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					if (cur._value.val.bufVal)
						memcpy((ub1_t*)cur._bind_buffer + i * cur._max_length, (*iter)[index]._value.val.bufVal + sizeof(size_t),  *(size_t*)(*iter)[index]._value.val.bufVal); 
					else
						memset((ub1_t*)cur._bind_buffer + i * cur._max_length, 0, cur._max_length);
				}
			}
			else
			{
				if (!cur._value.nullVal && cur._value.val.bufVal)
					memcpy(cur._bind_buffer, cur._value.val.bufVal + sizeof(size_t),  *(size_t*)cur._value.val.bufVal); 
				else
					memset(cur._bind_buffer, 0, cur._max_length);
			}

			
			cur._scale = 0;
			cur._precision = cur._max_length ? cur._max_length : 1;
			cur._native_type = cur._precision > 255 ? (size_t)SQL_LONGVARBINARY : (size_t)SQL_VARBINARY;
			cur._bind_type = (size_t)SQL_C_BINARY;
			break;
		case db_decimal: // decimal
		case db_numeric:
			cur._max_length = 40;
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = ((*iter)[index]._value.nullVal || !(*iter)[index]._value.val.bufVal) ? SQL_NULL_DATA : SQL_NTS;
					if ((*iter)[index]._value.val.bufVal)
					{
						numeric num(_temp_allocator);
						if (!num.parse_orcl((*iter)[index]._value.val.bufVal))
							exception::_throw("Out of range");


						if (!num.format((char*)cur._bind_buffer + i * cur._max_length, '.'))
							exception::_throw("Out of range");

						*((SQLINTEGER*)cur._real_length + i) = (SQLINTEGER)str_template::strlen((const char*)cur._bind_buffer + i * cur._max_length);
					}
					else
						memset((ub1_t*)cur._bind_buffer + i * cur._max_length, 0, cur._max_length); // sets value
				}

				cur._precision = cur._max_length;
				cur._scale = 0;
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = (cur._value.nullVal || !cur._value.val.bufVal) ? SQL_NULL_DATA : SQL_NTS;
				if (cur._value.val.bufVal)
				{
					numeric num(_temp_allocator);
					if (!num.parse_orcl(cur._value.val.bufVal))
						exception::_throw("Out of range");


					if (!num.format((char*)cur._bind_buffer, '.'))
						exception::_throw("Out of range");

					*(SQLINTEGER*)cur._real_length = (SQLINTEGER)str_template::strlen((const char*)cur._bind_buffer);
					cur._precision = num.precision();
					cur._scale = num.scale();
				}
			}

			cur._native_type = SQL_NUMERIC;
			cur._bind_type = SQL_C_CHAR;
			break;	
		case db_guid:
			cur._max_length = sizeof(guid_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate((num ? num : 1) * cur._max_length));
			if (num)
			{
				for (i = 0, iter = _bulk_params.begin(); iter != _bulk_params.end(); ++iter, ++i)
				{
					*((SQLINTEGER*)cur._real_length + i) = ((*iter)[index]._value.nullVal || !(*iter)[index]._value.val.guidVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;
					*((guid_t*)cur._bind_buffer + i) = (*iter)[index]._value.val.guidVal ? *(*iter)[index]._value.val.guidVal : null_uuid;
				}

				cur._precision = cur._max_length;
				cur._scale = 0;
			}
			else
			{
				*(SQLINTEGER*)cur._real_length = (cur._value.nullVal || !cur._value.val.bufVal) ? SQL_NULL_DATA : (SQLINTEGER)cur._max_length;
				*((guid_t*)cur._bind_buffer) = (!cur._value.nullVal && cur._value.val.bufVal) ? *cur._value.val.guidVal : null_uuid;

			}

			cur._native_type = (size_t)SQL_GUID;
			cur._bind_type = (size_t)SQL_C_GUID;
			cur._precision = 36;
			cur._scale = 0;
			break;	
		default:
			exception::_throw("Unsupported parameter type");
			assert(false);
	}

	RETCODE retCode = ::SQLBindParameter(_hstmt, (SQLUSMALLINT)(index + 1), 
		(SQLSMALLINT)(is_param_in(cur._in_out) ? (is_param_out(cur._in_out) ? SQL_PARAM_INPUT_OUTPUT : SQL_PARAM_INPUT) : SQL_PARAM_OUTPUT),
		(SQLSMALLINT)cur._bind_type, (SQLSMALLINT)cur._native_type,
		(SQLUINTEGER)cur._precision, (SQLSMALLINT)cur._scale,
		cur._bind_buffer, (SQLINTEGER)cur._max_length, 
		(SQLLEN*)cur._real_length);
	
	check_retcode(retCode)
}

// Binds one column with the number index
void
odbc_dbserver::v_bind_one_column(size_t index)
{
	binder& cur = _cols[index];
	switch (cur._native_type)
	{
		case SQL_BIT: 
			cur._bind_type = (size_t)SQL_C_BIT;
			cur._max_length = sizeof(bool);
			//cur._bind_buffer = &(cur._value.val.boolVal);
			break;
		case SQL_TINYINT: 
			cur._bind_type = (size_t)SQL_C_TINYINT;
			cur._max_length = sizeof(ub1_t);
			//cur._bind_buffer = &(cur._value.val.bVal);
			break;
		case SQL_SMALLINT: 
			cur._bind_type = (size_t)SQL_C_SSHORT;
			cur._max_length = sizeof(ub2_t);
			//cur._bind_buffer = &(cur._value.val.uiVal);
			break;
		case SQL_INTEGER: 
			cur._bind_type = (size_t)SQL_C_LONG;
			cur._max_length = sizeof(ub4_t);
			//cur._bind_buffer = &(cur._value.val.ulVal);
			break;
		case SQL_REAL: 
			cur._bind_type = (size_t)SQL_C_FLOAT;
			cur._max_length = sizeof(float);
			//cur._bind_buffer = &(cur._value.val.fltVal);
			break;
		case SQL_FLOAT:
		case SQL_DOUBLE:
			cur._max_length = sizeof(double);
			cur._bind_type = (size_t)SQL_C_DOUBLE;		
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQL_BIGINT:
			cur._max_length = sizeof(sb8_t);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			//cur._bind_type = (size_t)SQL_C_UBIGINT;
			break;	
		case SQL_TIMESTAMP:
		case SQL_TYPE_TIMESTAMP:
			cur._max_length = sizeof(TIMESTAMP_STRUCT);
			cur._bind_type = (size_t)SQL_C_TIMESTAMP;
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQL_TIME: 
		case SQL_TYPE_TIME:
			cur._max_length = sizeof(TIME_STRUCT);
			cur._bind_type = (size_t)SQL_C_TIME;
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQL_DATE: 
		case SQL_TYPE_DATE:
			cur._max_length = sizeof(DATE_STRUCT);
			cur._bind_type = (size_t)SQL_C_DATE;
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQL_CHAR:
		case SQL_VARCHAR:
			cur._max_length = cur._precision + 1;
			cur._bind_type = (size_t)SQL_C_CHAR;
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQL_LONGVARCHAR:
			cur._max_length = 0;
			cur._bind_type = (size_t)SQL_C_CHAR;
			//cur._bind_buffer = 0;
			break;
		case SQL_WCHAR:
			cur._max_length = 2 * (cur._precision + 1);
			cur._bind_type = (size_t)SQL_C_WCHAR;
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQL_WVARCHAR:
			cur._max_length = 2 * (cur._precision + 1);
			cur._bind_type = (size_t)SQL_C_WCHAR;
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQL_WLONGVARCHAR:
			cur._max_length = 0;
			cur._bind_type = (size_t)SQL_C_WCHAR;
			//cur._bind_buffer = 0;
			break;
		case SQL_DECIMAL:
		case SQL_NUMERIC:
			cur._max_length = 40;
			cur._bind_type = (size_t)SQL_C_CHAR;
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQL_BINARY: 
		case SQL_VARBINARY: 
			cur._max_length = cur._precision;
			cur._bind_type = (size_t)SQL_C_BINARY;
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQL_LONGVARBINARY:
			cur._max_length = 0;
			cur._bind_type = (size_t)SQL_C_BINARY;
			//cur._bind_buffer = 0;
			break;
		case SQL_GUID:
			cur._max_length = sizeof(guid_t);
			cur._bind_type = (size_t)SQL_C_GUID;
			//cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		default:
			exception::_throw("Unsupported native sql type");
			assert(false);
	}

	cur._bind_buffer = (cur._max_length ? check_pointer(_temp_allocator->allocate(cur._max_length * _bulk_rows)) : 0);
	cur._real_length = (size_t)check_pointer(_temp_allocator->allocate(sizeof(SQLINTEGER) * _bulk_rows));

	check_retcode(::SQLBindCol(_hstmt, (SQLUSMALLINT)index + 1, (SQLSMALLINT)cur._bind_type,
		_bind_cols ? cur._bind_buffer : 0, cur._max_length, (SQLLEN*)cur._real_length))

}

// Gets the number columns in query
size_t
odbc_dbserver::v_get_number_columns()
{
	size_t count_column = 0;
	// Get column's info from support class and fill
	RETCODE retCode = ::SQLNumResultCols(_hstmt, (SQLSMALLINT*)&count_column);
	check_retcode(retCode)
	return count_column;
}

// converts one field value to the variant buffer
void
odbc_dbserver::v_convert_one_value(size_t row, size_t col, terimber_db_value& val)
{
	SQLRETURN retCode;
	binder& cur = _cols[col];
	// adjusts buffer
	void* buffer = (cur._bind_buffer ? ((unsigned char*)cur._bind_buffer + row * cur._max_length) : 0);
	size_t rlen = *((SQLINTEGER*)cur._real_length + row);

	// here we have the real length for all columns for current row placed into cur.m_real_length
	if (!_bind_cols)
	{
		retCode = ::SQLGetData(_hstmt, (SQLUSMALLINT)col + 1, (SQLUSMALLINT)cur._bind_type, buffer ? buffer : &cur._user_code, cur._max_length, (SQLLEN*)&rlen);
		if (retCode != SQL_SUCCESS && retCode != SQL_SUCCESS_WITH_INFO)
			check_retcode(retCode)
	}

	if (rlen == SQL_NULL_DATA)
	{
		memset(&val, 0, sizeof(terimber_db_value));
		val.nullVal = true;
		return;
	}
	else
		val.nullVal = false;


	switch (cur._native_type)
	{
		case SQL_BIT:
			val.val.boolVal = *(bool*)buffer;
			break;
		case SQL_TINYINT:
			val.val.bVal = *(ub1_t*)buffer;
			break;
		case SQL_SMALLINT:
			val.val.uiVal = *(ub2_t*)buffer;
			break;
		case SQL_INTEGER:
			val.val.ulVal = *(ub4_t*)buffer;
			break;
		case SQL_REAL:
			val.val.fltVal = *(float*)buffer;
			break;
		case SQL_FLOAT:
		case SQL_DOUBLE:
#ifdef OS_64BIT
			val.val.dblVal = *(double*)buffer;
#else
			val.val.dblVal = (double*)check_pointer(_data_allocator->allocate(cur._max_length));
			memcpy((void*)val.val.dblVal,  buffer, cur._max_length);
#endif
			break;
		case SQL_BIGINT:
#ifdef OS_64BIT
			val.val.intVal = *(sb8_t*)buffer;
#else
			val.val.intVal = (sb8_t*)check_pointer(_data_allocator->allocate(cur._max_length));
			memcpy((void*)val.val.intVal, buffer, cur._max_length);
#endif
			break;	
		case SQL_TIMESTAMP:
		case SQL_TYPE_TIMESTAMP:
			{
				TIMESTAMP_STRUCT* ptds = (TIMESTAMP_STRUCT*)buffer;
				sb8_t dummy64;
				ptds->fraction /= 1000000;
				date::convert_to(ptds->year, (ub1_t)ptds->month, (ub1_t)ptds->day, (ub1_t)ptds->hour, (ub1_t)ptds->minute, (ub1_t)ptds->second, (ub2_t)ptds->fraction, dummy64);
#ifdef OS_64BIT
			    val.val.intVal = dummy64;
#else
				sb8_t* dummy = (sb8_t*)check_pointer(_data_allocator->allocate(sizeof(sb8_t)));
				*dummy = dummy64;
				val.val.intVal = dummy;
#endif
			}
			break;
		case SQL_TIME:
		case SQL_TYPE_TIME:
			{
				TIME_STRUCT* ptds = (TIME_STRUCT*)buffer;
				sb8_t dummy64;
				date::convert_to(0, 0, 0, (ub1_t)ptds->hour, (ub1_t)ptds->minute, (ub1_t)ptds->second, 0, dummy64);
#ifdef OS_64BIT
			    val.val.intVal = dummy64;
#else
				sb8_t* dummy = (sb8_t*)check_pointer(_data_allocator->allocate(sizeof(sb8_t)));
				*dummy = dummy64;
				val.val.intVal = dummy;
#endif
			}
			break;
		case SQL_DATE:
		case SQL_TYPE_DATE:
			{
				DATE_STRUCT* ptds = (DATE_STRUCT*)buffer;
				sb8_t dummy64;
				date::convert_to(ptds->year, (ub1_t)ptds->month, (ub1_t)ptds->day, 0, 0, 0, 0, dummy64);
#ifdef OS_64BIT
			    val.val.intVal = dummy64;
#else
				sb8_t* dummy = (sb8_t*)check_pointer(_data_allocator->allocate(sizeof(sb8_t)));
				*dummy = dummy64;
				val.val.intVal = dummy;
#endif
			}
			break;
		case SQL_DECIMAL:
		case SQL_NUMERIC:
			{
				const char* buf = (const char*)buffer;
				size_t len = (buf) ? strlen(buf) : 0;
				numeric num(buf, len, '.', _temp_allocator);
				ub1_t* bval = (ub1_t*)check_pointer(_data_allocator->allocate(num.orcl_len()));
				if (!num.persist_orcl(bval))
					exception::_throw("Out of range");
				val.val.bufVal = bval;
			}
			break;
		case SQL_CHAR:
		case SQL_VARCHAR:
			{
				char* sz = (char*)check_pointer(_data_allocator->allocate(rlen + 1));
				sz[rlen] = 0;
				memcpy(sz, buffer, rlen);
				val.val.strVal = sz;
			}
			break;
		case SQL_LONGVARCHAR:
			{
				if (_bind_cols)
				{
					retCode = ::SQLGetData(_hstmt, (SQLUSMALLINT)col + 1, (SQLUSMALLINT)cur._bind_type, &cur._user_code, 0, (SQLLEN*)&rlen);
					switch (retCode)
					{
						case SQL_SUCCESS:
							// check for null
							if (rlen == SQL_NULL_DATA)
							{
								memset(&val, 0, sizeof(terimber_db_value));
								val.nullVal = true;
								return;
							}
							break;
						case SQL_SUCCESS_WITH_INFO:
							break;
						default:
							check_retcode(retCode);
					}
				}

				// requests additional data
				char* sz = (char*)check_pointer(_data_allocator->allocate(rlen + 1));
				sz[rlen] = 0;
				size_t done = 0;
				size_t shift = 0;
				while (done < rlen)
				{
					SQLLEN available = 0;
					check_retcode(::SQLGetData(_hstmt, (SQLUSMALLINT)col + 1, (SQLUSMALLINT)cur._bind_type, sz + shift, rlen + 1 - shift, &available));
					shift += available;
					done += available;
				}

				val.val.strVal = sz;
			}
			break;
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			{
				wchar_t* wsz = (wchar_t*)check_pointer(_data_allocator->allocate((rlen + 1) * sizeof(ub2_t)));
				wsz[rlen] = 0;
				memcpy(wsz, buffer, rlen * sizeof(ub2_t));
				val.val.wstrVal = wsz;
			}
			break;
		case SQL_WLONGVARCHAR:
			{
				if (_bind_cols)
				{
					retCode = ::SQLGetData(_hstmt, (SQLUSMALLINT)col + 1, (SQLUSMALLINT)cur._bind_type, &cur._user_code, 0, (SQLLEN*)&rlen);
					switch (retCode)
					{
						case SQL_SUCCESS:
							// checks for null
							if (rlen == SQL_NULL_DATA)
							{
								memset(&val, 0, sizeof(terimber_db_value));
								val.nullVal = true;
								return;
							}
							break;
						case SQL_SUCCESS_WITH_INFO:
							break;
						default:
							check_retcode(retCode);
					}
				}

				// requests additional data
				wchar_t* wsz = (wchar_t*)check_pointer(_data_allocator->allocate(rlen + 2));
				wsz[rlen / 2] = 0; // length in bytes

				size_t done = 0;
				size_t shift = 0;
				while (done < rlen)
				{
					SQLLEN available = 0;
					check_retcode(::SQLGetData(_hstmt, (SQLUSMALLINT)col + 1, (SQLUSMALLINT)cur._bind_type, wsz + shift, rlen + 2 - shift, &available));
					shift += available;
					done += available;
				}

				val.val.wstrVal = wsz;
			}
			break;
		case SQL_BINARY:
		case SQL_VARBINARY:
			{
				ub1_t* bval = (ub1_t*)check_pointer(_data_allocator->allocate(rlen + sizeof(size_t)));
				*(ub4_t*)bval = (ub4_t)rlen;
				memcpy((ub1_t*)bval + sizeof(size_t), buffer, rlen);
				val.val.bufVal = bval;
			}
			break;
		case SQL_LONGVARBINARY:
			{
				if (_bind_cols)
				{
					retCode = ::SQLGetData(_hstmt, (SQLUSMALLINT)col + 1, (SQLUSMALLINT)cur._bind_type, &cur._user_code, 0, (SQLLEN*)&rlen);
					switch (retCode)
					{
						case SQL_SUCCESS:
							// checks for null
							if (rlen == SQL_NULL_DATA)
							{
								memset(&val, 0, sizeof(terimber_db_value));
								val.nullVal = true;
								return;
							}
							break;
						case SQL_SUCCESS_WITH_INFO:
							break;
						default:
							check_retcode(retCode);
					}
				}

				// requests additional data
				ub1_t* buf = (ub1_t*)check_pointer(_data_allocator->allocate(rlen + sizeof(size_t)));
				*(size_t*)buf = rlen; // length in bytes

				size_t done = 0;
				size_t shift = 0;
				while (done < rlen)
				{
					SQLLEN available = 0;
					check_retcode(::SQLGetData(_hstmt, (SQLUSMALLINT)col + 1, (SQLUSMALLINT)cur._bind_type, buf + sizeof(size_t) + shift, rlen - shift, &available));
					shift += available;
					done += available;
				}

				val.val.bufVal = buf;
			}
			break;
		case SQL_GUID:
			val.val.guidVal = (guid_t*)check_pointer(_data_allocator->allocate(cur._max_length));
			memcpy((void*)val.val.guidVal, buffer, cur._max_length);
			break;
		default: 
			exception::_throw("Unsupported native sql type");
			assert(false);
	}
}

// Gets column info from native driver
void
odbc_dbserver::v_get_one_column_info(size_t index)
{
	binder& cur = _cols[index];
	SQLSMALLINT native_type = 0, scale = 0, nullable = 0, nActualLen = 0;
	SQLULEN precision = 0;

#if OS_TYPE == OS_WIN32
	SQLWCHAR* szColumnName = (SQLWCHAR*)check_pointer(_temp_allocator->allocate((MAX_FNAME_LEN + 1)*sizeof(USHORT)));
	memset(szColumnName, 0, (MAX_FNAME_LEN + 1) * sizeof(USHORT));

	RETCODE	retCode = ::SQLDescribeColW(_hstmt, (SQLUSMALLINT)index + 1,
			szColumnName, MAX_FNAME_LEN, &nActualLen,
			&native_type,
			&precision,
			&scale,
			&nullable);
	check_retcode(retCode)

	cur.set_name(_columns_allocator, str_template::unicode_to_multibyte(/*CP_UTF8,*/ *_temp_allocator, (wchar_t*)szColumnName));
#else
	SQLCHAR* szColumnName = (SQLCHAR*)check_pointer(_temp_allocator->allocate((MAX_FNAME_LEN + 1)));
	memset(szColumnName, 0, (MAX_FNAME_LEN + 1));

	RETCODE	retCode = ::SQLDescribeCol(_hstmt, index + 1,
			szColumnName, MAX_FNAME_LEN, &nActualLen,
			&native_type,
			&precision,
			&scale,
			&nullable);
	check_retcode(retCode)

	cur.set_name(_columns_allocator, (const char*)szColumnName);
#endif

	cur._responsible = true;
	cur._native_type = native_type;
	cur._precision = precision;
	cur._scale = scale;
	cur._value.nullVal = nullable != 0;
	cur._max_length = cur._precision;
}

// Rebinds one param for input/output or output params
void
odbc_dbserver::v_rebind_one_param(size_t index)
{
	binder& cur = _params[index];
	if (!is_param_out(cur._in_out)) // nothing rebind
		return;

	cur._value.nullVal = false;

	size_t rlen = *(SQLINTEGER*)cur._real_length;
	if (rlen == SQL_NULL_DATA)
	{
		cur._value.nullVal = true;
		return;
	}

	switch (cur._native_type)
	{
		case SQL_BIT:
			cur._value.val.boolVal = *(bool*)cur._bind_buffer;
			break;
		case SQL_TINYINT:
			cur._value.val.cVal = *(char*)cur._bind_buffer;
			break;
		case SQL_SMALLINT:
			cur._value.val.iVal = *(sb2_t*)cur._bind_buffer;
			break;
		case SQL_INTEGER:
			cur._value.val.lVal= *(sb4_t*)cur._bind_buffer;
			break;
		case SQL_REAL:
			cur._value.val.fltVal = *(float*)cur._bind_buffer;
			break;
		case SQL_FLOAT:
		case SQL_DOUBLE:
#ifdef OS_64BIT
			cur._value.val.dblVal = *(double*)cur._bind_buffer;
#else
			memcpy(check_pointer(cur.allocate_value(0)), cur._bind_buffer, cur._max_length);
#endif
			break;
		case SQL_BIGINT:
#ifdef OS_64BIT
			cur._value.val.intVal = *(sb8_t*)cur._bind_buffer;
#else
			memcpy(check_pointer(cur.allocate_value(0)), cur._bind_buffer, cur._max_length);
#endif
			break;	
		case SQL_TIMESTAMP:
		case SQL_TYPE_TIMESTAMP:
			{
				TIMESTAMP_STRUCT* ptds = (TIMESTAMP_STRUCT*)cur._bind_buffer;
				sb8_t dummy64;
				ptds->fraction /= 1000000;
				date::convert_to(ptds->year, (ub1_t)ptds->month, (ub1_t)ptds->day, (ub1_t)ptds->hour, (ub1_t)ptds->minute, (ub1_t)ptds->second, (ub2_t)ptds->fraction, dummy64);

#ifdef OS_64BIT
			    cur._value.val.intVal = dummy64;
#else
				memcpy(check_pointer(cur.allocate_value(0)), &dummy64, sizeof(sb8_t));
#endif

			}
			break;
		case SQL_TIME:
		case SQL_TYPE_TIME:
			{
				TIME_STRUCT* ptds = (TIME_STRUCT*)cur._bind_buffer;
				sb8_t dummy64;
				date::convert_to(0, 0, 0, (ub1_t)ptds->hour, (ub1_t)ptds->minute, (ub1_t)ptds->second, 0, dummy64);

#ifdef OS_64BIT
			    cur._value.val.intVal = dummy64;
#else
				memcpy(check_pointer(cur.allocate_value(0)), &dummy64, sizeof(sb8_t));
#endif
			}
			break;
		case SQL_DATE:
		case SQL_TYPE_DATE:
			{
				DATE_STRUCT* ptds = (DATE_STRUCT*)cur._bind_buffer;
				sb8_t dummy64;
				date::convert_to(ptds->year, (ub1_t)ptds->month, (ub1_t)ptds->day, 0, 0, 0, 0, dummy64);
#ifdef OS_64BIT
			    cur._value.val.intVal = dummy64;
#else
				memcpy(check_pointer(cur.allocate_value(0)), &dummy64, sizeof(sb8_t));
#endif
			}
			break;
		case SQL_DECIMAL:
		case SQL_NUMERIC:
			{
				const char* buf = (const char*)cur._bind_buffer;
				size_t len = (buf) ? strlen(buf) : 0;
				numeric num(buf, len, '.', _temp_allocator);
				ub1_t* bval = (ub1_t*)check_pointer(cur.allocate_value(num.orcl_len()));
				if (!num.persist_orcl(bval))
					exception::_throw("Out of range");
			}
			break;
		case SQL_CHAR:
		case SQL_VARCHAR:
		case SQL_LONGVARCHAR:
			{
				char* sz = (char*)check_pointer(cur.allocate_value(rlen));
				sz[rlen] = 0;
				memcpy(sz, cur._bind_buffer, rlen);
			}
			break;
		case SQL_WCHAR:
		case SQL_WVARCHAR:
		case SQL_WLONGVARCHAR:
			{
				wchar_t* wsz = (wchar_t*)check_pointer(cur.allocate_value(rlen));
				wsz[rlen] = 0;
				memcpy(wsz, cur._bind_buffer, rlen * sizeof(ub2_t));
			}
			break;
		case SQL_BINARY:
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY:
			{
				ub1_t* bval = (ub1_t*)check_pointer(cur.allocate_value(rlen));
				memcpy(bval, cur._bind_buffer, rlen);
			}
			break;
		case SQL_GUID:
			{
				guid_t* gval = (guid_t*)check_pointer(cur.allocate_value(0));
				memcpy(gval, cur._bind_buffer, cur._max_length);
			}
			break;
		default: 
			exception::_throw("Unsupported native sql type");
			assert(false);
	}
}

// Terminates executing
void
odbc_dbserver::v_interrupt_async()
{
	::SQLCancel(_hstmt); 
}

// converts types
dbtypes
odbc_dbserver::v_native_type_to_client_type(size_t native_type)
{
	switch (native_type)
	{
		case SQL_BIT:
			return db_bool;
		case SQL_TINYINT:
			return db_ub1;
		case SQL_SMALLINT:
			return db_sb2;
		case SQL_INTEGER:
			return db_sb4;
		case SQL_REAL:
			return db_float;
		case SQL_FLOAT:
		case SQL_DOUBLE:
			return db_double;
		case SQL_BIGINT:
			return db_sb8;
		case SQL_TIMESTAMP:
		case SQL_TYPE_TIMESTAMP:
		case SQL_TIME:
		case SQL_TYPE_TIME:
		case SQL_DATE:
		case SQL_TYPE_DATE:
			return db_date;
		case SQL_DECIMAL:
			return db_decimal;
		case SQL_NUMERIC:
			return db_numeric;
		case SQL_CHAR:
		case SQL_VARCHAR:
		case SQL_LONGVARCHAR:
			return db_string;
		case SQL_WCHAR:
		case SQL_WVARCHAR:
		case SQL_WLONGVARCHAR:
			return db_wstring;
		case SQL_BINARY:
		case SQL_VARBINARY:
		case SQL_LONGVARBINARY:
			return db_binary;
		case SQL_GUID:
			return db_guid;
		default:
			assert(false);
			return db_unknown;
	}
}

/////////////////////////////////////////////////////////////////////////////////
// pool support
//! \brief creates object
//static
db_entry* 
odbc_db_creator::create(			const db_arg& arg						
					)
{
	db_entry* obj = 0;
	if (0 != (obj = new db_entry(arg._ident, arg._trusted, arg._login_string)))
	{
		if (!(obj->_obj = new odbc_dbserver(arg._ident)))
			delete obj, obj = 0;
	}

	return obj;
}
#pragma pack()
END_TERIMBER_NAMESPACE
