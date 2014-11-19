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

#include "dborcl/orclsrv.h"
#include "base/vector.hpp"
#include "base/list.hpp"
#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/numeric.h"
 
BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

#define check_retcode_db(x, ehandle, type) \
	if (x != OCI_SUCCESS) \
	{ \
	ub1_t err[256] = {0}; \
		sword code; \
		OCIErrorGet(ehandle, 1, 0, &code, err, sizeof(err), type); \
		throw exception(code, (const char*)err); \
	}

#define check_retcode(x, ehandle, type) \
	if (x != OCI_SUCCESS && x != OCI_NO_DATA) \
	{ \
	ub1_t err[256] = {0}; \
		sword code = 0; \
		OCIErrorGet(ehandle, 1, 0, &code, err, sizeof(err), type); \
		throw exception(code, (const char*)err); \
	}

static const char* valid_delimeters = " ,)}";

orcl_dbserver::orcl_dbserver(size_t ident) : dbserver_impl(ident),
	_envhp(0), 
	_svchp(0),
	_errhp(0),
	_stmthp(0)
{
}

orcl_dbserver::~orcl_dbserver()
{
	if (_is_open_sql())
		close_sql(); 

	if (_is_connect()) 
		disconnect();
}

// database specific
void
orcl_dbserver::v_connect(bool trusted_connection, const char* connection_string)
{
	// initializes OCI library
	int err = 0;
	if (0 != (err = OCIInitialize(OCI_OBJECT | OCI_THREADED, 
		0, 0, 0, 0)))
		exception::_throw("Can't initialize OCI library");
	if (0 != (err = OCIEnvInit(&_envhp, OCI_DEFAULT, 0, 0)))
		exception::_throw("Can't initialize OCI environment");

	check_retcode_db(OCIHandleAlloc(_envhp, (void**)&_errhp, OCI_HTYPE_ERROR, 0, 0),  _envhp, OCI_HTYPE_ENV)

	// login string
	// uid/pwd@sid 
	// extracts SID
	const char* sid = connection_string ? strchr(connection_string, '@') : 0;
	// extracts uid and pwd
	const char* uid = sid != connection_string ? connection_string : 0;
	const char* pwd = connection_string ? strchr(connection_string, '/') : 0;
	if (pwd) ++pwd;

	if (sid) ++sid;

	size_t uid_len = uid ? (pwd ? pwd - uid - 1 : (sid ? sid - uid - 1 : strlen(uid)) ) : 0;
	size_t pwd_len = pwd ? (sid ? sid - pwd - 1 : strlen(pwd)) : 0;
	size_t sid_len = sid ? strlen(sid) : 0;

	// sets login
	check_retcode_db(OCILogon(_envhp, _errhp, &_svchp, (const text*)uid, (ub4)uid_len, (const text*)pwd, (ub4)pwd_len, (const text*)sid, (ub4)sid_len), _errhp, OCI_HTYPE_ERROR)
 
}

void
orcl_dbserver::v_disconnect()
{
	_temp_allocator->clear_extra();

	v_close();

	if (_svchp)
	{
		//sword x = OCIHandleFree(_svchp, OCI_HTYPE_SERVER);//OCI_HTYPE_SVCCTX);
		check_retcode_db(OCILogoff(_svchp, _errhp), _errhp, OCI_HTYPE_ERROR) 
		_svchp = 0;
	}

	if (_errhp)
	{
		sword x = OCIHandleFree(_errhp, OCI_HTYPE_ERROR);
		_errhp = 0;
	}

	if (_envhp)
	{
		sword x = OCIHandleFree(_envhp, OCI_HTYPE_ENV);
		_envhp = 0;
	}

}

void
orcl_dbserver::v_start_transaction()
{
	// turns off autocommit
	check_retcode_db(OCITransCommit(_svchp, _errhp, OCI_DEFAULT), _errhp, OCI_HTYPE_ERROR)
}

void 
orcl_dbserver::v_commit()
{
	// commits
	check_retcode_db(OCITransCommit(_svchp, _errhp, OCI_DEFAULT), _errhp, OCI_HTYPE_ERROR)
}

void 
orcl_dbserver::v_rollback()
{
	// rolls back
	check_retcode_db(OCITransRollback(_svchp, _errhp, OCI_DEFAULT), _errhp, OCI_HTYPE_ERROR)
}

bool 
orcl_dbserver::v_is_connect_alive()
{
	//  checks connection first
	if (!_is_connect())
		return false;

// can't find the analog in orcle
//	SQLINTEGER value = SQL_CD_TRUE;
//	::SQLGetConnectAttr(_hdbc, SQL_COPT_SS_CONNECTION_DEAD, &value, SQL_IS_INTEGER, 0);
	return true;
}

///////////////////////////
void
orcl_dbserver::v_before_execute()
{
	check_retcode(OCIHandleAlloc(_envhp, (void**)&_stmthp, OCI_HTYPE_STMT, 0, 0),  _envhp, OCI_HTYPE_ENV)
	check_retcode(OCIStmtPrepare(_stmthp, _errhp, (const text*)(const char*)_sql, (ub4)_sql.length(), OCI_NTV_SYNTAX, OCI_DEFAULT), _errhp, OCI_HTYPE_ERROR)
}

void
orcl_dbserver::v_after_execute()
{
	// nothing to do
}

// Executes the SQL expression
void
orcl_dbserver::v_execute()
{
	ub4_t iters = 0;
	
	switch (get_action())
	{
		case ACTION_EXEC_PROC:
		case ACTION_EXEC_PROC_ASYNC:
		case ACTION_EXEC_SQL:
		case ACTION_EXEC_SQL_ASYNC:
			iters = 1;
			break;
		default:
			break;
	}

	// can return OCI_NO_DATA
	sword status = OCIStmtExecute(_svchp, _stmthp, _errhp, iters, 0, 0, 0, OCI_DEFAULT);
	check_retcode(status,  _envhp, OCI_HTYPE_ERROR)
}

// closes opened query
void
orcl_dbserver::v_close()
{
	if (_stmthp)
	{
		OCIHandleFree(_stmthp, OCI_HTYPE_STMT);
		_stmthp = 0;
	}

	// deallocates LOB blocks
	size_t index;
	for (index = 0; index < _params.size(); ++index)
		if (_params[index]._user_code)
		{
			OCIDescriptorFree(_params[index]._user_code, OCI_DTYPE_LOB);
			_params[index]._user_code = 0;
		}

	for (index = 0; index < _cols.size(); ++index)
		if (_cols[index]._user_code)
		{
			OCIDescriptorFree(_cols[index]._user_code, OCI_DTYPE_LOB);
			_cols[index]._user_code = 0;
		}
}

// Fetches block rows
void
orcl_dbserver::v_fetch()
{
	// sets the first row for set
	if (_start_row != 0)
	{
		// re-executes
		v_execute();
		size_t skip_rows = 0;
		while (skip_rows++ < _start_row
			&& !OCIStmtFetch(_stmthp, _errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT));
	}

	// defines the column count
	size_t col_count = _cols.size();
	sword x = OCI_SUCCESS;
	size_t select_row = 0;

	while (_requested_rows
		&& ((x = OCIStmtFetch(_stmthp, _errhp, 1, OCI_FETCH_NEXT, OCI_DEFAULT)) == OCI_SUCCESS 
				|| x == OCI_SUCCESS_WITH_INFO)
			)
	{
		if (STATE_INTERRUPTED == get_state()) // check interrupt
			exception::_throw("Fetching process is interrupted");

		check_retcode(x, _errhp, OCI_HTYPE_ERROR)

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
			v_convert_one_value(0, index, (*iter_val)[index]);

		++select_row;	
		// increment/decrement _requested_rows
		--_requested_rows;
	}

	if (x != OCI_NO_DATA)
		check_retcode(x, _errhp, OCI_HTYPE_ERROR)

	// sets the real fetched row count
	_fetched_rows = select_row;
}

// replaces the quote to the available sign for native drive
void
orcl_dbserver::v_replace_quote()
{
	if (0 == _quote || _quote == ':')
		return;
	
	// saves the current sql
	if (!_sql.length())
		return;

	// allocates the buffer for correct string_t
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
		*new_sql++ = ':'; // replace '?' on ':'
		// skips parameter name
		// until valid_delimeters appears
		begin += strcspn(begin, valid_delimeters);
	}

	// copies the rest bytes
	str_template::strcpy(new_sql, begin, os_minus_one);
	// resets sql
	_sql = buf;
}

// Binds one param with the number index 
void
orcl_dbserver::v_bind_one_param(size_t index)
{
	binder& cur = _params[index];

	cur._bind_type = cur._value.nullVal ? os_minus_one : 0;

	switch (cur._type)
	{
		case db_bool: // bool
			cur._max_length = sizeof(bool);
			cur._real_length = cur._value.nullVal ? 0 : cur._max_length;	
			cur._bind_buffer = &(cur._value.val.boolVal); // set value
			cur._native_type = SQLT_INT;
			break;	
		case db_sb1: // char
		case db_ub1: // byte
			cur._max_length = sizeof(ub1_t);
			cur._real_length = cur._value.nullVal ? 0 : cur._max_length;	
			cur._bind_buffer = cur._type == db_sb1 ? (void*)&(cur._value.val.cVal) : (void*)&(cur._value.val.bVal); // set value
			cur._native_type = SQLT_INT;
			break;	
		case db_sb2:
		case db_ub2:
			cur._max_length = sizeof(ub2_t);
			cur._real_length = cur._value.nullVal ? 0 : cur._max_length;	
			cur._bind_buffer = cur._type == db_sb2 ? (void*)&(cur._value.val.iVal) : (void*)&(cur._value.val.uiVal); // set value
			cur._real_length = cur._value.nullVal ? os_minus_one : 0;	
			cur._native_type = SQLT_INT;
			break;	
		case db_sb4: 
		case db_ub4: 
			cur._max_length = sizeof(ub4_t);
			cur._real_length = cur._value.nullVal ? 0 : cur._max_length;	
			cur._bind_buffer = cur._type == db_ub4 ? (void*)&(cur._value.val.lVal) : (void*)&(cur._value.val.ulVal); // set value
			cur._native_type = SQLT_INT;
			break;	
		case db_float: 
			cur._max_length = sizeof(float);
			cur._real_length = cur._value.nullVal ? 0 : cur._max_length;	
			cur._bind_buffer = &(cur._value.val.fltVal); // set value
			cur._native_type = SQLT_FLT;
			break;	
		case db_double: 
			cur._max_length = sizeof(double);
#ifdef OS_64BIT
			cur._real_length = cur._value.nullVal ? 0 : cur._max_length;
			cur._bind_buffer = &(cur._value.val.dblVal); // set value
#else

			cur._real_length = cur._value.nullVal || !cur._value.val.dblVal ? 0 : cur._max_length;
			
			if (is_param_out(cur._in_out))
			{
				double* buf = (double*)check_pointer(_temp_allocator->allocate(sizeof(double)));
				*buf = cur._value.val.dblVal ? *cur._value.val.dblVal : 0.0;
				cur._bind_buffer = buf; // set value
			}
			else
				cur._bind_buffer = (void*)cur._value.val.dblVal;
#endif
			cur._native_type = SQLT_FLT;
			break;	
		case db_sb8:
		case db_ub8:
			// lets use the numeric type
			cur._max_length = 22;
			{
#ifdef OS_64BIT
				numeric conv(cur._value.val.intVal, _temp_allocator);
#else
				numeric conv(cur._value.val.intVal ? *cur._value.val.intVal : (sb8_t)0, _temp_allocator);
#endif
				ub1_t* bval = (ub1_t*)check_pointer(_data_allocator->allocate(is_param_out(cur._in_out) ? cur._max_length : conv.orcl_len()));
				if (!conv.persist_orcl(bval))
					exception::_throw("Out of range");
				cur._value.val.bufVal = bval;

				cur._real_length = cur._value.nullVal || !cur._value.val.intVal ? 0 : cur._max_length;
			}
			cur._native_type = SQLT_NUM;
			break;	
		case db_date: 
			cur._max_length = 7;
			cur._real_length = cur._value.nullVal || !cur._value.val.dblVal ? 0 : cur._max_length;
			{
				ub1_t* ptds = (ub1_t*)check_pointer(_temp_allocator->allocate(7));
				cur._bind_buffer = ptds;

				if (cur._value.val.dblVal)
				{
					ub4_t year32;
					ub1_t month8, day8, hour8, minute8, second8, wday8;
					ub2_t millisec16, yday16;
					date::convert_from(
#ifdef OS_64BIT
						cur._value.val.intVal,
#else
						cur._value.val.intVal ? *cur._value.val.intVal : 0, 
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


					ptds[0] = year32 / 100 + 100;
					ptds[1] = year32 % 100 + 100;
					ptds[2] = month8;
					ptds[3] = day8;
					ptds[4] = hour8 + 1;
					ptds[5] = minute8 + 1;
					ptds[6] = second8 + 1;
				}
				else
					memset(ptds, 0, 7);
			}
			
			cur._native_type = SQLT_DAT;
			break;	
		case db_string:
			cur._max_length = __max(cur._max_length, cur._value.val.strVal ? strlen(cur._value.val.strVal) + 1 : 0);
			cur._real_length = cur._max_length;	

			if (is_param_out(cur._in_out))
			{
				cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
				if (cur._value.val.strVal)
					str_template::strcpy((char*)cur._bind_buffer, cur._value.val.strVal, os_minus_one);
				else
					memset(cur._bind_buffer, 0, cur._max_length);
			}
			else
				cur._bind_buffer = (void*)cur._value.val.strVal;
			

			// checks long string
			if (cur._real_length > 4000)
				cur._native_type = SQLT_LNG;
			else
				cur._native_type = SQLT_STR;
			break;	
		case db_wstring:
			cur._max_length = 2 * __max(cur._max_length, cur._value.val.wstrVal ? wcslen(cur._value.val.wstrVal) + 1 : 0);
			cur._real_length = cur._max_length;	

			if (is_param_out(cur._in_out))
			{
				cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
				if (cur._value.val.strVal)
					str_template::strcpy((wchar_t*)cur._bind_buffer, cur._value.val.wstrVal, os_minus_one);
				else
					memset(cur._bind_buffer, 0, cur._max_length);
			}
			else
				cur._bind_buffer = (void*)cur._value.val.wstrVal;
			

			// checks long string
			if (cur._real_length > 4000)
				cur._native_type = SQLT_LNG;
			else
				cur._native_type = SQLT_STR;
			break;	
		case db_decimal: // decimal
		case db_numeric:
			cur._max_length = 22;

			if (is_param_out(cur._in_out))
			{
				numeric conv(_temp_allocator);
				if (!conv.parse_orcl(cur._value.val.bufVal))
					exception::_throw("Out of range");

				ub1_t* buf = (ub1_t*)check_pointer(_data_allocator->allocate(cur._max_length));
				conv.persist_orcl(buf);
				cur._bind_buffer = buf;
			}
			else
				cur._bind_buffer = (void*)cur._value.val.bufVal;

			cur._real_length = cur._value.nullVal || !cur._value.val.bufVal ? 0 : cur._max_length;
			cur._native_type = SQLT_NUM;
			break;	
		case db_binary: // binary
			cur._max_length = __max(cur._max_length, cur._value.val.bufVal ? *(size_t*)cur._value.val.bufVal : 0);
			cur._real_length = cur._value.nullVal || !cur._value.val.bufVal ? 0 : cur._max_length;	

			cur._bind_buffer = cur._value.val.bufVal ? (ub1_t*)cur._value.val.bufVal + sizeof(size_t) : 0;

			if (cur._real_length > 4000)
				cur._native_type = SQLT_LBI;
 			else
				cur._native_type = SQLT_BIN;
			
			break;
		default:
			exception::_throw("Unsupported parameter type");
			assert(false);
	}

	OCIBind* bindp = 0;
	check_retcode(OCIBindByPos(_stmthp, &bindp, _errhp,
		   (ub4)index + 1, cur._bind_buffer, (ub4)cur._max_length,
		   (ub2)cur._native_type, (ub2*)&cur._bind_type,
		   (ub2*)&cur._real_length, 0, 0, 0,
		   OCI_DEFAULT), _errhp, OCI_HTYPE_ERROR)

}

void 
orcl_dbserver::v_before_bind_columns()
{
}

// Binds one column with the number index
void
orcl_dbserver::v_bind_one_column(size_t index)
{
	binder& cur = _cols[index];
	switch (cur._native_type)
	{
		case SQLT_INT:
			//cur._bind_type = SQL_C_LONG;
			cur._max_length = sizeof(sb4_t);
			cur._bind_buffer = &(cur._value.val.lVal);
			break;
		case SQLT_UIN:
			cur._max_length = sizeof(ub4_t);
			cur._bind_buffer = &(cur._value.val.ulVal);
			break;
		case SQLT_FLT:
			cur._max_length = sizeof(double);
			cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQLT_DAT:
			cur._max_length = 7;
			cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQLT_VNU:
			cur._max_length = 23;
			cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQLT_NUM:
			cur._max_length = 22;
			cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQLT_VCS:
		case SQLT_CHR:
		case SQLT_STR:
		case SQLT_RID:
		case SQLT_VBI:
		case SQLT_BIN:
		case SQLT_AFC:
		case SQLT_AVC:
			cur._max_length = 4000;
			cur._bind_buffer = check_pointer(_temp_allocator->allocate(cur._max_length));
			break;
		case SQLT_LNG:
		case SQLT_LBI:
		case SQLT_LVC:
		case SQLT_LVB:
			// allocated LOB
			check_retcode(OCIDescriptorAlloc(_envhp, (void **)&cur._user_code, OCI_DTYPE_LOB, 0, 0),  _envhp, OCI_HTYPE_ENV)
			cur._max_length = 0;
			cur._bind_buffer = &cur._user_code;
			break;
		case SQLT_BLOB:
		case SQLT_CLOB:
			// allocated LOB
			check_retcode(OCIDescriptorAlloc(_envhp, (void **)&cur._user_code, OCI_DTYPE_LOB, 0, 0),  _envhp, OCI_HTYPE_ENV)
			cur._max_length = 0;
			cur._bind_buffer = &cur._user_code;
			break;
		default:
			exception::_throw("Unsupported native sql type");
			assert(false);
	}

	// uses bind_type for nullability
	OCIDefine* defnp = 0;

	check_retcode(OCIDefineByPos(_stmthp, &defnp, _errhp, (ub4)index + 1,
              cur._bind_buffer, (ub4)cur._max_length,
              (ub2)cur._native_type, (ub2*)&cur._bind_type,
              (ub2*)&cur._real_length,
			  0,
			  OCI_DEFAULT), _errhp, OCI_HTYPE_ERROR)
}

// Gets number columns in query
size_t
orcl_dbserver::v_get_number_columns()
{
	ub4 count_column = 0;
	// gets the number of columns in the select list *
	check_retcode(OCIAttrGet(_stmthp, OCI_HTYPE_STMT, (void*)&count_column, 0, OCI_ATTR_PARAM_COUNT, _errhp), _errhp, OCI_HTYPE_ERROR)

	return count_column;
}

// converts one field value to the variant buffer
void
orcl_dbserver::v_convert_one_value(size_t row, size_t index, terimber_db_value& val)
{
	binder& cur = _cols[index];

	if ((ub4)cur._bind_type == 0xffffffff || cur._real_length == 0)
	{
		memset(&val, 0, sizeof(terimber_db_value));
		val.nullVal = true;
		return;
	}
	else
		val.nullVal = false;


	switch (cur._native_type)
	{
		case SQLT_INT:
			val.val.boolVal = cur._value.val.boolVal;
			break;
		case SQLT_FLT:
#ifdef OS_64BIT
			val.val.dblVal = cur._value.val.dblVal;
#else
			val.val.dblVal = (double*)check_pointer(_data_allocator->allocate(cur._max_length));
			memcpy((void*)val.val.dblVal, cur._bind_buffer, cur._max_length);
#endif
			break;
		case SQLT_DAT:
			{
				const ub1_t* ptds = (const ub1_t*)cur._bind_buffer;
				sb8_t dummy64;
				date::convert_to((ptds[0] - 100) * 100 + (ptds[1] - 100), ptds[2], ptds[3], ptds[4] - 1, ptds[5] - 1, ptds[6] - 1, 0, dummy64);
#ifdef OS_64BIT
			    val.val.intVal = dummy64;
#else
				sb8_t* dummy = (sb8_t*)check_pointer(_data_allocator->allocate(sizeof(sb8_t)));
				*dummy = dummy64;
				val.val.intVal = dummy;
#endif
			}
			break;
		case SQLT_LNG:
		case SQLT_LVC:
			if (*(unsigned short*)&cur._user_code == 1406) // truncate
			{
				char* sz = (char*)check_pointer(_data_allocator->allocate(cur._real_length + 1));
				sz[cur._real_length] = 0;

				size_t shift = 0;
				ub4 gotLen = 0;
				while (shift < cur._real_length)
				{
					check_retcode(OCILobRead(_svchp, _errhp, (OCILobLocator*)cur._user_code,
						&gotLen, (ub4)shift + 1, (sz + shift), (ub4)(cur._real_length - shift), 
						OCI_ONE_PIECE, 0, 0, SQLCS_IMPLICIT), _errhp, OCI_HTYPE_ERROR)

					shift += gotLen;
				}
				val.val.strVal = sz;
			}
			else
			{
				char* sz = (char*)check_pointer(_data_allocator->allocate(cur._real_length + 1));
				sz[cur._real_length] = 0;
				memcpy(sz, cur._bind_buffer, cur._real_length);
				val.val.strVal = sz;
			}
			break;			
		case SQLT_VNU:
			{
				((ub1_t*)cur._bind_buffer)[cur._real_length] = 0;
				numeric conv(_temp_allocator);
				if (!conv.parse_orcl((const ub1_t*)cur._bind_buffer + 1))
					exception::_throw("Out of range");

				ub1_t* buf = (ub1_t*)check_pointer(_data_allocator->allocate(conv.orcl_len()));
				conv.persist_orcl(buf);
				val.val.bufVal = buf;
			}
			break;
		case SQLT_NUM:
			{
				((ub1_t*)cur._bind_buffer)[cur._real_length] = 0;
				numeric conv(_temp_allocator);
				if (!conv.parse_orcl((const ub1_t*)cur._bind_buffer))
					exception::_throw("Out of range");

				ub1_t* buf = (ub1_t*)check_pointer(_data_allocator->allocate(conv.orcl_len()));
				conv.persist_orcl(buf);
				val.val.bufVal = buf;
			}
			break;
		case SQLT_VCS:
		case SQLT_CHR:
		case SQLT_STR:
		case SQLT_RID:
		case SQLT_AFC:
		case SQLT_AVC:
			{
				char* sz = (char*)check_pointer(_data_allocator->allocate(cur._real_length + 1));
				sz[cur._real_length] = 0;
				memcpy(sz, cur._bind_buffer, cur._real_length);
				val.val.strVal = sz;
			}
			break;
		case SQLT_LBI:
		case SQLT_LVB:
			if (*(unsigned short*)&cur._user_code == 1406)
			{
				ub1_t* buf = (ub1_t*)check_pointer(_data_allocator->allocate(cur._real_length + sizeof(size_t)));
				*(size_t*)buf = cur._real_length;

				size_t shift = 0;
				ub1_t* start_buf = buf + sizeof(size_t);
				ub4 gotLen = 0;
				while (shift < cur._real_length)
				{
					check_retcode(OCILobRead(_svchp, _errhp, (OCILobLocator*)cur._user_code,
						&gotLen, (ub4)shift + 1, (start_buf + shift), (ub4)(cur._real_length - shift), 
						OCI_ONE_PIECE, 0, 0, SQLCS_IMPLICIT), _errhp, OCI_HTYPE_ERROR)

					shift += gotLen;
				}
				val.val.bufVal = buf;
			}
			else
			{
				val.val.bufVal = (ub1_t*)check_pointer(_data_allocator->allocate(cur._real_length + sizeof(size_t)));
				*(size_t*)val.val.bufVal = cur._real_length;
				memcpy((char*)val.val.bufVal + sizeof(size_t), cur._bind_buffer, cur._real_length);				
			}
			break;
		case SQLT_VBI:
		case SQLT_BIN:
			{
				ub1_t* buf = (ub1_t*)check_pointer(_data_allocator->allocate(cur._real_length + sizeof(ub4_t)));
				*(ub4_t*)buf = (ub4_t)cur._real_length;

				size_t shift = 0;
				ub1_t* start_buf = buf + sizeof(ub4_t);
				ub4 gotLen = 0;
				while (shift < cur._real_length)
				{
					check_retcode(OCILobRead(_svchp, _errhp, (OCILobLocator*)cur._user_code,
						&gotLen, (ub4)shift + 1, (start_buf + shift), (ub4)(cur._real_length - shift), 
						OCI_ONE_PIECE, 0, 0, SQLCS_IMPLICIT), _errhp, OCI_HTYPE_ERROR)

					shift += gotLen;
				}
				val.val.bufVal = buf;
			}
			break;
		default: 
			exception::_throw("Unsupported native sql type");
			assert(false);
	}
}

// Gets column info from native driver
void
orcl_dbserver::v_get_one_column_info(size_t index)
{
	binder& cur = _cols[index];
	ub2 real_length = 0;
	char* name = 0;
	ub4 name_len = 0;
	ub4 display_len;
	ub2 native_type = 0;
	ub1 precision = 0;
	b1 scale = 0;
	ub1 nullable = 0;

	OCIParam* parmdp = 0;
	check_retcode(OCIParamGet(_stmthp, OCI_HTYPE_STMT, _errhp, (void**)&parmdp, (ub4)index + 1), _errhp, OCI_HTYPE_ERROR) 

	// gets the data name
	check_retcode(OCIAttrGet(parmdp, OCI_DTYPE_PARAM, (void*)&name, &name_len, OCI_ATTR_NAME, _errhp), _errhp, OCI_HTYPE_ERROR)

	// gets the data type
	check_retcode(OCIAttrGet(parmdp, OCI_DTYPE_PARAM, (void*)&native_type, 0, OCI_ATTR_DATA_TYPE, _errhp), _errhp, OCI_HTYPE_ERROR)

	// gets the data size
	check_retcode(OCIAttrGet(parmdp, OCI_DTYPE_PARAM, (void*)&real_length, 0, OCI_ATTR_DATA_SIZE, _errhp), _errhp, OCI_HTYPE_ERROR)

	// gets the precision
	check_retcode(OCIAttrGet(parmdp, OCI_DTYPE_PARAM, (void*)&precision, 0, OCI_ATTR_PRECISION, _errhp), _errhp, OCI_HTYPE_ERROR)

	// gets the scale
	check_retcode(OCIAttrGet(parmdp, OCI_DTYPE_PARAM, (void*)&scale, 0, OCI_ATTR_SCALE, _errhp), _errhp, OCI_HTYPE_ERROR)

 	// gets the display size
	check_retcode(OCIAttrGet(parmdp, OCI_DTYPE_PARAM, (void*)&display_len, 0, OCI_ATTR_DISP_SIZE, _errhp), _errhp, OCI_HTYPE_ERROR)

	// is null
	check_retcode(OCIAttrGet(parmdp, OCI_DTYPE_PARAM, (void*)&nullable, 0, OCI_ATTR_IS_NULL, _errhp), _errhp, OCI_HTYPE_ERROR)


    check_retcode(OCIAttrGet(parmdp, OCI_DTYPE_PARAM, &native_type, 0, OCI_ATTR_DATA_TYPE, _errhp), _errhp, OCI_HTYPE_ERROR)

	cur._max_length = real_length;
	cur.set_name(_columns_allocator, name);
	cur._native_type = native_type;
	cur._scale = scale;
	cur._precision = precision;
	cur._value.nullVal = nullable != 0;
	cur._max_length = cur._precision;
}

// Forms SQL expression depending on the types selected and the native driver
void
orcl_dbserver::v_form_sql_string()
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
				// Looks like "BEGIN PROCNAME(:PARAM1, :PARAM2); END;" 
				string_t val(_temp_allocator);
				char q[2] = {_quote, 0};
				val = "BEGIN ";
				val += _sql;
				val += "(";
				for (size_t index = 0; index < count_params; ++index)
				{
					if (index)
						val += ", ";

					val += q;

					if (_params[index]._name)
						val += _params[index]._name;
					else
					{
						char buf[32];
						str_template::strprint(buf, 32, "%d", index + 1);
						val += buf;
					}
				}

				val += "); END;";		
				_sql = val;
			}
			break;
		default:
			assert(false);
	}
}

// Rebinds one param for input/output or output params
void
orcl_dbserver::v_rebind_one_param(size_t index)
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
		case SQLT_INT:
			break;
		case SQLT_FLT:
			memcpy((void*)cur._value.val.bufVal, cur._bind_buffer, cur._max_length);
			break;
		case SQLT_DAT:
			{
				const ub1_t* ptds = (const ub1_t*)cur._bind_buffer;
				sb8_t dummy64;
				date::convert_to((ptds[0] - 100) * 100 + (ptds[1] - 100), ptds[2], ptds[3], ptds[4] - 1, ptds[5] - 1, ptds[6] - 1, 0, dummy64);
#ifdef OS_64BIT
			    cur._value.val.intVal = dummy64;
#else
				sb8_t* dummy = (sb8_t*)check_pointer(cur.allocate_value(0));
				*dummy = dummy64;
#endif
			}
			break;
		case SQLT_LNG:
		case SQLT_LVC:
			if (*(unsigned short*)&cur._user_code == 1406) // truncate
			{
				char* sz = (char*)check_pointer(cur.allocate_value(cur._real_length));
				sz[cur._real_length] = 0;

				size_t shift = 0;
				ub4 gotLen = 0;
				while (shift < cur._real_length)
				{
					check_retcode(OCILobRead(_svchp, _errhp, (OCILobLocator*)cur._user_code,
						&gotLen, (ub4)shift, (sz + shift), (ub4)(cur._real_length - shift), 
						0, 0,
						0, SQLCS_IMPLICIT), _errhp, OCI_HTYPE_ERROR)


					shift += gotLen;
				}
			}
			else
			{
				char* sz = (char*)check_pointer(cur.allocate_value(cur._real_length));
				sz[cur._real_length] = 0;
				memcpy(sz, cur._bind_buffer, cur._real_length);
			}
			break;
		case SQLT_VNU:
			{
				((ub1_t*)cur._bind_buffer)[cur._real_length] = 0;
				numeric conv(_temp_allocator);
				if (!conv.parse_orcl((const ub1_t*)cur._bind_buffer + 1))
					exception::_throw("Out of range");

				ub1_t* buf = (ub1_t*)check_pointer(cur.allocate_value(conv.orcl_len()));
				conv.persist_orcl(buf);
			}
			break;
		case SQLT_NUM:
			{
				((ub1_t*)cur._bind_buffer)[cur._real_length] = 0;
				numeric conv(_temp_allocator);
				if (!conv.parse_orcl((const ub1_t*)cur._bind_buffer))
					exception::_throw("Out of range");

				ub1_t* buf = (ub1_t*)check_pointer(cur.allocate_value(conv.orcl_len()));
				conv.persist_orcl(buf);
			}
			break;	
		case SQLT_VCS:
		case SQLT_CHR:
		case SQLT_STR:
		case SQLT_RID:
		case SQLT_AFC:
		case SQLT_AVC:
			{
				char* sz = (char*)check_pointer(cur.allocate_value(cur._real_length));
				sz[cur._real_length] = 0;
				memcpy(sz, cur._bind_buffer, cur._real_length);
			}
			break;
		case SQLT_LBI:
		case SQLT_LVB:
			if (*(unsigned short*)&cur._user_code == 1406)
			{
				ub1_t* buf = (ub1_t*)check_pointer(cur.allocate_value(cur._real_length));

				size_t shift = 0;
				ub4 gotLen = 0;
				while (shift < cur._real_length)
				{
					check_retcode(OCILobRead(_svchp, _errhp, (OCILobLocator*)cur._user_code,
						&gotLen, (ub4)shift, (buf + shift), (ub4)(cur._real_length - shift), 
						0, 0,
						0, SQLCS_IMPLICIT), _errhp, OCI_HTYPE_ERROR)


					shift += gotLen;
				}
			}
			else
			{
				ub1_t* bval = (ub1_t*)check_pointer(cur.allocate_value(cur._real_length));
				memcpy(bval, cur._bind_buffer, cur._real_length);				
			}
			break;
		case SQLT_VBI:
		case SQLT_BIN:
			{
				ub1_t* buf = (ub1_t*)check_pointer(cur.allocate_value(cur._real_length));

				size_t shift = 0;
				ub4 gotLen = 0;
				while (shift < cur._real_length)
				{
					check_retcode(OCILobRead(_svchp, _errhp, (OCILobLocator*)cur._user_code,
						&gotLen, (ub4)shift, (buf + shift), (ub4)(cur._real_length - shift), 
						0, 0,
						0, SQLCS_IMPLICIT), _errhp, OCI_HTYPE_ERROR)

					shift += gotLen;
				}
			}
			break;
		default: 
			exception::_throw("Unsupported native sql type");
			assert(false);
	}
}

// Terminates execution
void
orcl_dbserver::v_interrupt_async()
{
	// there are no cancel functions
	v_close();
}

// converts types
dbtypes
orcl_dbserver::v_native_type_to_client_type(size_t native_type)
{
	switch (native_type)
	{
		case SQLT_NON:
			return db_unknown;
		case SQLT_INT: 
			return db_sb4;
		case SQLT_UIN: 
			return db_ub4; 
		case SQLT_FLT: 
			return db_double; 
		case SQLT_DAT: 
			return db_date; 
		case SQLT_NUM:
		case SQLT_VNU: 
		case SQLT_PDN:
			return db_numeric;
		case SQLT_VCS:
		case SQLT_CHR:
		case SQLT_STR:
		case SQLT_RID:
		case SQLT_VBI:
		case SQLT_BIN:
		case SQLT_AFC:
		case SQLT_AVC: 
		case SQLT_CLOB:
			return db_string; 
		case SQLT_LNG:
		case SQLT_LBI:
		case SQLT_LVC:
		case SQLT_LVB: 
		case SQLT_BLOB:
			return db_binary; 
		case SQLT_CUR: 
		case SQLT_SLS:
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
orcl_db_creator::create(			const db_arg& arg						
					)
{
	db_entry* obj = 0;
	if (0 != (obj = new db_entry(arg._ident, arg._trusted, arg._login_string)))
	{
		if (!(obj->_obj = new orcl_dbserver(arg._ident)))
			delete obj, obj = 0;
	}

	return obj;
}
#pragma pack()
END_TERIMBER_NAMESPACE
