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

#ifndef _terimber_db_h_
#define _terimber_db_h_

#include "allinc.h"
#include "threadpool/thread.h"
#include "base/common.h"
#include "base/vector.h"
#include "base/list.h"
#include "base/except.h"
#include "db/dbaccess.h"
#include "dbvalue.h"

BEGIN_TERIMBER_NAMESPACE 
#pragma pack(4)

//! \brief checks if this is input parameter
inline 
bool 
is_param_in(		db_param_type x							//!< in/out parameter property
					) 
{ 
	switch (x)
	{
		case db_param_in:
		case db_param_in_out:
			return true;
		default:
			return false;
	}
}
//! \brief checks if this is output parameter
inline 
bool 
is_param_out(		db_param_type x							//!< in/out parameter property
					) 
{ 
	switch (x)
	{
		case db_param_out:
		case db_param_in_out:
			return true;
		default:
			return false;
	}
}

//! \enum module_state
//! \brief state of module
enum module_state
{	
	STATE_OK,												//!< module is OK
	STATE_WORKING,											//!< module is processing asynchronous request
	STATE_INTERRUPTED										//!< the asynchronous request has been interrupted
};

//! \brief macros to handle exception situation
#define DB_TRY try {

#define DB_CATCH return true; } \
catch( exception& x ) { _code = x.get_code(); _error = x.what(); _set_action(ACTION_NONE); _set_state(STATE_OK); } \
catch(...) { _error = "Unknown Exception"; } return false;

#define DB_CATCH_DB return true; } \
catch( exception& x ) { _code = x.get_code(); _error = x.what(); } \
catch(...) { _error = "Unknown Exception"; } return false;

#define DB_CATCH_EX } catch( exception& x ) { _code = x.get_code(); _error = x.what(); } \
catch(...) { _error = "Unknown Exception"; }

const size_t ex_custom				= 0;					//!< custom exception
const size_t ex_invalid				= 1;					//!< invalid module state
const size_t ex_not_init			= 2;					//!< module is not intialized
const size_t ex_already_init		= 3;					//!< module is already initilaized
const size_t ex_not_connect			= 4;					//!< module is not connected
const size_t ex_already_connect		= 5;					//!< module is already connected
const size_t ex_not_transaction		= 6;					//!< module is not in transaction	
const size_t ex_already_transaction	= 7;					//!< module is alrady in transacion
const size_t ex_not_sql_open		= 8;					//!< module is not in open state
const size_t ex_already_sql_open	= 9;					//!< module is already open sql statement
const size_t ex_sql_in_action		= 10;					//!< module is processing sql statement
const size_t ex_bad_pointer			= 11;					//!< invalid pointer
const size_t ex_invalid_param_count	= 12;					//!< number of parameters in sql statement or stored procedure does not math actually binded parameters
const size_t ex_timeout				= 13;					//!< timeout occurred
const size_t ex_unknown_server_type	= 14;					//!< index out of bounderies
const size_t ex_out_of_bounders		= 15;

//! \brief exception map
extern exception_table dbMsgTable;

///////////////////////////////////////////////////////////////
//! \class binder
//! \brief orvide functionality for binding sql parameters
class binder
{
public:
	//! \brief destructor
	~binder();
	//! \brief constructor
	binder();
	//! \brief copy constructor
	//! full copy with memory allocation
	binder(const binder& x);
	//! \brief assign operator
	//! full copy with memory allocation
	binder& operator=(const binder&x);
	//! \brief sets name, makes a copy or assigns only pointers - depends on responsibility
	void 
	set_name(		byte_allocator* data_allocator,			//!< external allocator, if any
					const char* x							//!< name
					);
	//! \brief sets function
	inline 
	bool 
	set_as_null(	db_param_type in_out,					//!< in/out binder property
					dbtypes type							//!< binder type
					);
	//! \brief sets param as boolean
	inline 
	bool 
	set_as_bool(	db_param_type in_out,					//!< in/out binder property
					bool val								//!< value
					);
	//! \brief sets param as char
	inline 
	bool 
	set_as_char(	db_param_type in_out,					//!< in/out binder property
					sb1_t val								//!< value
					);
	//! \brief sets param as byte
	inline 
	bool 
	set_as_byte(	db_param_type in_out,					//!< in/out binder property
					ub1_t val								//!< value
					);
	//! \brief sets param as short
	inline 
	bool 
	set_as_short(	db_param_type in_out,					//!< in/out binder property
					sb2_t val								//!< value
					);
	//! \brief sets param as word
	inline 
	bool
	set_as_word(	db_param_type in_out,					//!< in/out binder property
					ub2_t val								//!< value
					);
	//! \brief sets param as long
	inline 
	bool 
	set_as_long(	db_param_type in_out,					//!< in/out binder property
					sb4_t val								//!< value
					);
	//! \brief sets param as dword
	inline 
	bool 
	set_as_dword(	db_param_type in_out,					//!< in/out binder property
					ub4_t val								//!< value
					);
	//! \brief sets param as float
	inline 
	bool 
	set_as_float(	db_param_type in_out,					//!< in/out binder property
					float val								//!< value
					);
	//! \brief sets param as double
	inline 
	bool 
	set_as_double(	db_param_type in_out,					//!< in/out binder property
					double val								//!< value
					);
	//! \brief sets param as long64
	inline 
	bool 
	set_as_long64(	db_param_type in_out,					//!< in/out binder property
					sb8_t val								//!< value
					);
	//! \brief sets param as dword64
	inline 
	bool 
	set_as_dword64(	db_param_type in_out,					//!< in/out binder property
					ub8_t val								//!< value
					);
	//! \brief sets param as guid
	inline 
	bool 
	set_as_guid(	db_param_type in_out,					//!< in/out binder property
					const guid_t& val						//!< value
					);
	//! \brief sets param as numeric
	inline 
	bool 
	set_as_numeric(	db_param_type in_out,					//!< in/out binder property
					const char* val,						//!< value
					char delimeter							//!< delimeter
					);
	//! \brief sets param as decimal
	inline 
	bool 
	set_as_decimal(	db_param_type in_out,					//!< in/out binder property
					const char* val,						//!< value
					char delimeter							//!< delimeter
					);
	//! \brief sets param as date
	inline 
	bool 
	set_as_date(	db_param_type in_out,					//!< in/out binder property
					ub4_t year,								//!< year [0-1000000]
					ub1_t month,							//!< month [1-12]
					ub1_t day,								//!< day [1-31]
					ub1_t hour,								//!< hour [0-23]
					ub1_t minute,							//!< minute [0-59]
					ub1_t second,							//!< second [0-59]
					ub2_t millisec							//!< millisec [0-999]
					);
	//! \brief sets param as string
	inline 
	bool 
	set_as_string(	db_param_type in_out,					//!< in/out binder property
					const char* val,						//!< value
					size_t len								//!< string length, -1 for zero terminated
					);
	//! \brief sets param as string pointer
	inline 
	bool 
	set_as_string_ptr(const char* val						//!< value, only for input
					);
	//! \brief sets param as wide string
	inline 
	bool 
	set_as_wstring(	db_param_type in_out,					//!< in/out binder property
					const wchar_t* val,						//!< value
					size_t len								//!< string length, -1 for zero terminated
					);
	//! \brief sets param as wide string pointer
	inline 
	bool 
	set_as_wstring_ptr(const wchar_t* val					//!< value, only for input
					);
	//! \brief sets param as binary
	inline 
	bool 
	set_as_binary(	db_param_type in_out,					//!< in/out binder property
					const ub1_t* val,						//!< value
					size_t len								//!< binary length
					);
	//! \brief sets param as binary pointer
	inline 
	bool 
	set_as_binary_ptr(const ub1_t* val						//!< value, only for input, first size_t bytes are length 
					);

public:
	//! \brief allocates value accoring to the assigned type
	void* 
	allocate_value(	size_t n								//!< length of value for variable byte length
					);
	//! \brief deallocates value if required
	void 
	deallocate_value();
	//! \brief makes a full copy of value
	void 
	clone(			binder& out
					) const;
public:
	db_param_type			_in_out;						//!< input/output parameter
	dbtypes					_type;							//!< db predifined types
	const char*				_name;							//!< name of binder
	size_t					_scale;							//!< size of binder, usually for numeric types	
	size_t					_precision;						//!< precision of binder, usually for numeric types
	size_t					_max_length;					//!< length of binder max
	size_t					_bind_type;						//!< binds type while binding column or output parameter
	size_t					_native_type;					//!< native type, specific for database server
	size_t					_real_length;					//!< actual length of column value or output parameter
	mutable void*			_bind_buffer;					//!< memory for binding
	void*					_user_code;						//!< additional memory for some types of database server
	terimber_db_value		_value;							//!< parameter value
	bool					_responsible;					//!< memory cleans up responsibility
};

//! \typedef params_t
//! \brief vector of binders
typedef vector< binder > params_t;
//! \typedef binders_t
//! \brief vector of binders with external allocator
typedef _vector< binder > binders_t;
//! \typedef bulk_params_t
//! \brief list of binders
typedef list< binders_t > bulk_params_t;
//! \typedef recordset_list_t
//! \brief list of rows with external allocator
//! where row is a vector of db values
typedef _list< _vector< terimber_db_value > > recordset_list_t;

//! \class dbserver_impl
//! \brief implements most of the methods of the dbserver interface
class dbserver_impl : public dbserver, 
						public terimber_thread_employer
{
	//! \typedef async_db_notify_list_t
	//! \brief list of callback pointers
	typedef list< async_db_notify* > async_db_notify_list_t;
	//! \class callback_finder
	//! \brief class help to find callback
	class callback_finder
	{
	public:
		//! \brief constructor
		callback_finder(const async_db_notify* obj			//!< callback pointer
					);
		//! \brief compare operator
		inline 
		bool 
		operator()(	const async_db_notify* obj
					) const 
		{ 
			return obj == _obj; 
		}
	private:
		const async_db_notify* _obj;						//!< calback pointer
	};

	//! \class callback_fire
	//! \brief class help the stl algorithm for_each to fire events
	class callback_fire
	{
	public:
		//! \brief constructor
		callback_fire(bool noerrors,						//!< error flag
					size_t ident							//!< server ident
					);
		//! \brief operator()
		inline 
		void 
		operator()(	async_db_notify* obj					//!< callback pointer
					) 
		{ 
			if (obj) 
				obj->notify_async(_noerrors, _ident); 
		}
	private:
		bool	_noerrors;									//!< error flag
		size_t	_ident;										//!< server ident
	};

protected:
	//! \enum action
	//! \brief predifined actions
	enum action
	{
		ACTION_NONE = 0,									//!< nothing to do
		ACTION_OPEN_SQL,									//!< open sql like --select * from table--
		ACTION_OPEN_PROC,									//!< open rowset through stored procedure
		ACTION_EXEC_SQL,									//!< execute sql statment like -- insert into t_names(name) values('John');
		ACTION_EXEC_PROC,									//!< execute sql statment like -- exec sp_find_id(:id);
		ACTION_FETCH,										//!< fetch data
		ACTION_OPEN_SQL_ASYNC,								//!< open sql asynchronousally
		ACTION_OPEN_PROC_ASYNC,								//!< open proc asynchronousally
		ACTION_EXEC_SQL_ASYNC,								//!< execute sql asynchronousally
		ACTION_EXEC_PROC_ASYNC,								//!< execute proc asynchronousally
		ACTION_FETCH_ASYNC									//!< fetch data asynchronousally
	};

	//! \brief constructor
	dbserver_impl(	size_t ident							//!< server ident
					);
	//! \brief destructor
	virtual 
	~dbserver_impl();
public:
	//! base methods
	//! \brief returns current ident
	virtual 
	size_t 
	get_ident() const;
	//! \brief returns current state for asynchronous actions
	virtual 
	bool 
	is_ready() const;
	//! \brief returns error code
	virtual 
	size_t 
	get_code() const;
	//! \brief returns error description, NULL will be returned if there are no errors
	virtual 
	const char* 
	get_error() const;
	//! db connection methods
	//! \brief establishes db connection
	//! the syntax of the connection string depends on db server type
	virtual 
	bool 
	connect(		bool trusted_connection,				//!< trusted connection flag
					const char* connect_string				//!< connection string
					);
	//! \brief closes connection
	virtual 
	bool 
	disconnect();
	//! \brief starts transaction
	virtual 
	bool 
	start_transaction();
	//! \brief commits transaction
	virtual 
	bool 
	commit();
	//! \brief rolls back transaction
	virtual 
	bool 
	rollback();
	//! \brief returns the connection status
	virtual 
	bool 
	is_connect() const;
	//! \brief returns the transaction status
	virtual 
	bool 
	is_in_transaction() const;
	//! \brief checks if ythe connection is alive or not
	virtual 
	bool 
	is_connect_alive();
	//! sql methods
	//! \brief resizes the parameters
	virtual 
	bool 
	resize_params(	size_t size								//!< number of parameters
					);
	//! \brief gets parameter count
	virtual 
	size_t 
	get_param_count() const;
	//! \brief returns the virtual db terimber type
	virtual 
	dbtypes 
	get_param_type(	size_t index							//!< parameter index
					) const;
	//! \brief returns the in/out
    virtual 
	db_param_type 
	get_param_in_out(size_t index							//!< parameter index
					) const ;
	//! \brief returns the nullable - database specific
    virtual 
	bool 
	get_param_is_null(size_t index							//!< parameter index
					) const;

	//! \brief returns param value as boolean
    //! if index out of range then zero will be return
	//! the best practice to check type before call gets value functions
	virtual 
	bool 
	get_param_as_bool(size_t index							//!< parameter index
					) const;
	//! \brief returns param value as char
	virtual 
	sb1_t 
	get_param_as_char(size_t index							//!< parameter index
					) const;
	//! \brief returns param value as byte
	virtual 
	ub1_t 
	get_param_as_byte(size_t index							//!< parameter index
					) const;
	//! \brief returns param value as short
	virtual 
	sb2_t 
	get_param_as_short(size_t index							//!< parameter index
					) const;
	//! \brief returns param value as word
	virtual 
	ub2_t 
	get_param_as_word(size_t index							//!< parameter index
					) const;
	//! \brief returns param value as long
	virtual 
	sb4_t 
	get_param_as_long(size_t index							//!< parameter index
					) const;
	//! \brief returns param value as dword
	virtual 
	ub4_t 
	get_param_as_dword(size_t index							//!< parameter index
					) const;
	//! \brief returns param value as float
	virtual 
	float 
	get_param_as_float(size_t index							//!< parameter index
					) const;
	//! \brief returns param value as double
	virtual 
	double 
	get_param_as_double(size_t index						//!< parameter index
					) const;
	//! \brief returns param value as long64
	virtual 
	sb8_t 
	get_param_as_long64(size_t index						//!< parameter index
					) const;
	//! \brief returns param value as dword64
	virtual 
	ub8_t 
	get_param_as_dword64(size_t index						//!< parameter index
					) const;
	//! \brief returns param value as guid
	//! for high performance we are using reference
	virtual 
	bool 
	get_param_as_guid(size_t index, 						//!< parameter index
					guid_t& val								//!< [out] value
					) const;
	//! \brief returns param value as numeric
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_param_as_numeric(size_t index,  					//!< parameter index
					char delimeter							//!< delimeter
					) const;
	//! \brief returns param value as decimal
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_param_as_decimal(size_t index,   					//!< parameter index
					char delimeter							//!< delimeter
					) const;
	//! \brief returns param value as date
	//! to avoid confusions with different date format functions returns parsed date
	virtual 
	bool 
	get_param_as_date(size_t index,    						//!< parameter index
					ub4_t& year,							//!< [out] year [0-1000000] from 1 A.D.
					ub1_t& month,							//!< [out] month [1-12]
					ub1_t& day,								//!< [out] day [1-31]
					ub1_t& hour,							//!< [out] hour [0-23]
					ub1_t& minute,							//!< [out] minute [0-59]
					ub1_t& second,							//!< [out] second [0-59]
					ub2_t& millisec,						//!< [out] millisecond [0-999]
					ub1_t& wday,							//!< [out] day of week [1-7]
					ub2_t& yday								//!< [out] day of year [1-366]
					) const;
	//! \brief returns param value as string
	//! NB!!! return value can be allocated on a temporary allocator
	virtual 
	const char* 
	get_param_as_string(size_t index   						//!< parameter index
					) const;
	//! \brief returns param value as wide string
	//! NB!!! return value can be allocated on a temporary allocator
	virtual 
	const wchar_t* 
	get_param_as_wstring(size_t index   					//!< parameter index
					) const;
	//! \brief returns param value as binary
	//! NB!!! return value can be allocated on a temporary allocator
	virtual 
	const ub1_t* 
	get_param_as_binary(size_t index,   					//!< parameter index
					size_t& len								//!< [out] the binary length
					) const;
	//! \brief returns param value as binary pointer
	//! the format of buffer is as following
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
 	//! NB!!! return value can be allocated on a temporary allocator
	virtual 
	const ub1_t* 
	get_param_as_binary_ptr(size_t index  					//!< parameter index
					) const;

	//! sets parameter values
	//! \brief sets parameter as null
	virtual 
	bool 
	set_param_as_null(size_t index,  						//!< parameter index
					db_param_type in_out,					//!< in/out type
					dbtypes type							//!< db type
					);
	//! \brief sets parameter as boolean
	virtual 
	bool 
	set_param_as_bool(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out type
					bool val								//!< value
					);
	//! \brief sets parameter as char
	virtual 
	bool 
	set_param_as_char(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out type
					sb1_t val								//!< value
					);
	//! \brief sets parameter as byte
	virtual 
	bool 
	set_param_as_byte(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out type
					ub1_t val								//!< value
					);
	//! \brief sets parameter as short
	virtual 
	bool 
	set_param_as_short(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out type
					sb2_t val								//!< value
					);
	//! \brief sets parameter as word
	virtual 
	bool 
	set_param_as_word(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out type
					ub2_t val								//!< value
					);
	//! \brief sets parameter as long
	virtual 
	bool 
	set_param_as_long(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out type
					sb4_t val								//!< value
					);
	//! \brief sets parameter as dword
	virtual 
	bool 
	set_param_as_dword(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out type
					ub4_t val								//!< value
					);
	//! \brief sets parameter as float
	virtual 
	bool 
	set_param_as_float(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out type
					float val								//!< value
					);
	//! \brief sets parameter as double
	virtual 
	bool 
	set_param_as_double(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out type
					double val								//!< value
					);
	//! \brief sets parameter as long64
	virtual 
	bool 
	set_param_as_long64(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out type
					sb8_t val								//!< value
					);
	//! \brief sets parameter as dword64
	virtual 
	bool 
	set_param_as_dword64(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out type
					ub8_t val								//!< value
					);
	//! \brief sets parameter as guid
	virtual 
	bool 
	set_param_as_guid(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out type
					const guid_t& val						//!< value
					);
	//! \brief sets parameter as numeric
	//! -1223456.7894 - delimeter here "."
	//! -1223456,7894 - delimeter here ","
	virtual 
	bool 
	set_param_as_numeric(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out type
					const char* val,						//!< value
					char delimeter							//!< delimeter
					);
	//! \brief sets parameter as decimal
	virtual 
	bool 
	set_param_as_decimal(size_t index, 						//!< parameter index
					db_param_type in_out,					//!< in/out type
					const char* val,						//!< value
					char delimeter							//!< delimeter
					);
	//! \brief sets parameter as date
	//! date from A.C. year > 0
	virtual 
	bool 
	set_param_as_date(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out type
					ub4_t year,								//!< year [0-1000000]
					ub1_t month,							//!< month [1-12]
					ub1_t day,								//!< day [1-31]
					ub1_t hour,								//!< hour [0-23]
					ub1_t minute,							//!< minute [0-59]
					ub1_t second,							//!< second [0-59]
					ub2_t millisec							//!< millisec [0-999]
					);
	//! \brief sets parameter as string
	virtual 
	bool 
	set_param_as_string(size_t index, 						//!< parameter index
					db_param_type in_out,					//!< in/out type
					const char* val, 						//!< value				
					size_t len,								//!< string length, -1 for zero terminated string
					size_t max_len = 0						//!< optional max length, only for output parameter
					);
	
	//! \brief sets parameter as string pointer
	//! to avoid copying already allocated memory, the only pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statment is executed.
	virtual 
	bool 
	set_param_as_string_ptr(size_t index,					//!< parameter index
					const char* val							//!< value	
					);
	
	//! \brief sets parameter as wide string
	virtual 
	bool 
	set_param_as_wstring(size_t index,  					//!< parameter index
					db_param_type in_out,					//!< in/out type
					const wchar_t* val,						//!< value	
					size_t len,								//!< string length, -1 for zero terminated string
					size_t max_len = 0						//!< optional max length, only for output parameter
					);
	//! \brief sets parameter as wide string pointer
	//! to avoid copying already allocated memory, the only pointer can be used
	//! be careful! the caller is responsible for the life time of tee provided pointer, 
	//! which has to be valid until the sql statment is executed.
	virtual 
	bool 
	set_param_as_wstring_ptr(size_t index,					//!< parameter index
					const wchar_t* val						//!< value	
					);
	//! \brief sets parameter as binary
	//! val points to the valid buffer of raw byte of "len" length
	virtual 
	bool 
	set_param_as_binary(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out type
					const ub1_t* val,						//!< value
					size_t len,								//!< binary length
					size_t max_len = 0						//!< optional max length, only for output parameter
					);
	//! \brief sets parameter as binary pointer
	//! to avoid copying already allocated huge memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statment is executed.
	//! it is supposed to workonly for input parameters, not for output or input/output ones
	//! the format of the buffer must be as follows
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
	virtual 
	bool 
	set_param_as_binary_ptr(size_t index,					//!< parameter index
					const ub1_t* val						//!< value
					);

	//! internal columns access
	//! \brief gets column count
	virtual 
	size_t 
	get_column_count() const;
	//! \brief returns the terimber db type
	virtual 
	dbtypes 
	get_column_type(size_t index
					) const;
	//! \brief returns the name
	virtual 
	const char* 
	get_column_name(size_t index
					) const;
	//! \brief returns the nullable - database specific
	virtual 
	bool 
	get_column_nullable(size_t index
					) const;
	//! \brief returns the scale
	virtual 
	size_t 
	get_column_scale(size_t index
					) const;
	//! \brief returns the precision
	virtual 
	size_t 
	get_column_precision(size_t index
					) const;
	//! \brief returns the length of parameter (only for output binders)
	virtual 
	size_t 
	get_column_max_length(size_t index
					) const;

	
	//! rowset navigation methods
	//! \brief gets row count
	virtual 
	size_t 
	get_row_count() const;
	//! \brief resets internal cursor next to the last element
	virtual 
	void 
	reset() const;
	//! \brief tries to find the next row, after reset it goes to the first row
	virtual 
	bool 
	next() const;
	//! \brief tries to find the previous row
	virtual 
	bool 
	prev() const;
	//! const version of current row
	//! \brief checks if value is null
	//! returns param value
    //! if index is out of range then zero will be returned
	//! the best practice is to check type before call gets value functions
	virtual 
	bool 
	get_value_is_null(size_t index							//!< column index
					) const;

	//! \brief gets value as boolean
	virtual 
	bool 
	get_value_as_bool(size_t index							//!< column index
					) const;
	//! \brief gets value as char
	virtual 
	sb1_t 
	get_value_as_char(size_t index							//!< column index
					) const;
	//! \brief gets value as byte
	virtual 
	ub1_t 
	get_value_as_byte(size_t index							//!< column index
					) const;
	//! \brief gets value as short
	virtual 
	sb2_t 
	get_value_as_short(size_t index							//!< column index
					) const;
	//! \brief gets value as word
	virtual 
	ub2_t 
	get_value_as_word(size_t index							//!< column index
					) const;
	//! \brief gets value as long
	virtual 
	sb4_t 
	get_value_as_long(size_t index							//!< column index
					) const;
	//! \brief gets value as dword
	virtual 
	ub4_t 
	get_value_as_dword(size_t index							//!< column index
					) const;
	//! \brief gets value as float
	virtual 
	float
	get_value_as_float(size_t index							//!< column index
					) const;
	//! \brief gets value as double
	virtual 
	double 
	get_value_as_double(size_t index						//!< column index
					) const;
	//! \brief gets value as long64
	virtual 
	sb8_t 
	get_value_as_long64(size_t index						//!< column index
					) const;
	//! \brief gets value as dword64
	virtual 
	ub8_t 
	get_value_as_dword64(size_t index						//!< column index
					) const;
	//! \brief gets value as guid
	//! for high performance we are using reference
	virtual 
	bool 
	get_value_as_guid(size_t index, 						//!< column index
					guid_t& val								//!< [out] value
					) const;
	//! \brief gets value as numeric
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_value_as_numeric(size_t index, 						//!< column index
					char delimeter							//!< delimeter
					) const;
	//! \brief gets value as decimal
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_value_as_decimal(size_t index,						//!< column index
					char delimeter							//!< delimeter
					) const;
	//! \brief gets value as date
	//! to avoid confusions with different date format functions returns parsed date
	virtual 
	bool 
	get_value_as_date(size_t index,							//!< column index
					ub4_t& year,							//!< [out] year [0-1000000] from 1 A.D.
					ub1_t& month,							//!< [out] month [1-12]
					ub1_t& day,								//!< [out] day [1-31]
					ub1_t& hour,							//!< [out] hour [0-23]
					ub1_t& minute,							//!< [out] minute [0-59]
					ub1_t& second,							//!< [out] second [0-59]
					ub2_t& millisec,						//!< [out] millisecond [0-999]
					ub1_t& wday,							//!< [out] day of week [1-7]
					ub2_t& yday								//!< [out] day of year [1-366]
					) const;
	//! \brief gets value as string
	//! NB!!! return value can be allocated on a temporary allocator
	virtual 
	const char* 
	get_value_as_string(size_t index						//!< column index
					) const;
	//! \brief gets value as wide string
	//! NB!!! return value can be allocated on a temporary allocator
	virtual 
	const wchar_t* 
	get_value_as_wstring(size_t index						//!< column index
					) const;
	//! \brief gets value as binary
	//! returns pointer to the raw bytes
	//! size of bytes is put to len
	virtual 
	const ub1_t* 
	get_value_as_binary(size_t index,						//!< column index
					size_t& len								//!< [out] binary length
					) const;
	//! \brief gets value as binary pointer
	//! for reusing only
	//! the format of buffer is as following
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
 	virtual 
	const ub1_t* 
	get_value_as_binary_ptr(size_t index					//!< column index
					) const;


	//! checks the state of sql request
	//! \brief checks if sql is open
	virtual 
	bool 
	is_open_sql() const;
	//! \brief executes parameterized sql statement with recordset result
	virtual 
	bool 
	open_sql(		bool async,								//!< asychronous executing flag
					const char* sql							//!< SQL statement (select)
					);
	//! \brief executes parameterized sql statement without result
	virtual 
	bool 
	exec_sql(		bool async,								//!< asychronous executing flag
					const char* sql							//!< SQL statement (DDL, DML except select)
					);
	//! \brief executes stored procedure with recordset result
	virtual 
	bool 
	open_proc(		bool async,								//!< asychronous executing flag
					const char* name						//!< stored procedure name
					);
	//! \brief executes stored procedure without result
	virtual 
	bool 
	exec_proc(		bool async,								//!< asychronous executing flag
					const char* name						//!< stored procedure name
					);
	
	//! fetches data and returns recordset

	//! \brief/ defines the start row, number of rows and fetch direction
	virtual 
	bool 
	fetch_data(		bool async,								//!< asychronous executing flag
					size_t start_row,						//!< start row, 0 means current
					size_t num_rows,						//!< rows to fetch
					bool forward							//!< direction
					);
	//! \brief close sql - free allocated resources
	virtual 
	bool 
	close_sql();
	//! \brief interrupts asychronous request
	virtual 
	bool 
	interrupt_request();
	//! \brief the syntax of sql statement with binders must be understandable
	//! for internal sql processor
	//! therefore it should look like
	//! select * from table where pk = :pk
	//! here ':' char is an identificator of the parameter with the name "pk"
	//! function set_quote allows to change this identificator to the preferable one
	virtual 
	bool 
	set_quote(		char quote								//!< quote sign
					);
	//! \brief sets the callback function for asynchronous actions
	virtual 
	void 
	set_callback(	async_db_notify* target					//!< callback pointer
					);
	//! \brief removes the callback function for asynchronous actions
	virtual 
	void 
	remove_callback(async_db_notify* target					//!< callback pointer
					);

	//! bulk parameter operations - must be supported by SQL server
	//! \brief current set of parameters previously set by set_param_* functions
	//! will be moved to the bulk parameters list
	//! the current parameters set will have the same values, but will not participate in bulk operations
	//! moreover if the next call set_param_* function will change the type - all bulk parameters will be deleted.
	virtual 
	bool 
	param_bulk_store();
	//! \brief removes the last stored parameter
	virtual 
	bool 
	param_bulk_remove();
	//! \brief removes all
	virtual 
	bool 
	param_bulk_remove_all();
	//! \brief converts db terimber type into virtual terimber type
	static 
	inline 
	vt_types 
	convert_types(	dbtypes type							//!< terimber db type
					);

// common part
protected:
	mem_pool_t				_manager;						//!< memory allocator pool
	byte_allocator*			_temp_allocator;				//!< temporary memory allocator
	mutex					_mtx;							//!< multithreaded mutex
	size_t					_ident;							//!< server ident
	size_t					_code;							//!< code of the last error
	string_t				_error;							//!< last error description

// connection part
protected:
	//! must be overriden
	//! database specific
	
	//! \brief establishes connection to db
	virtual 
	void 
	v_connect(		bool trusted_connection,				//!< trusted connection flag
					const char* connection_string			//!< connection string
					) = 0;
	//! \brief closes connection
	virtual 
	void 
	v_disconnect() = 0;
	//! \brief starts transaction
	virtual 
	void 
	v_start_transaction() = 0;
	//! \brief commits transaction
	virtual 
	void
	v_commit() = 0;
	//! \brief rolls back transaction
	virtual 
	void
	v_rollback() = 0;
	//! \brief checks if the connection is alive
	virtual 
	bool 
	v_is_connect_alive() = 0;

protected:
	//! inline functions
	
	//! \brief checks the connection state
	inline 
	bool 
	_is_connect() const 
	{ 
		mutexKeeper keeper(_mtx); 
		return _connection; 
	}
	//! \brief checks the transaction state
	inline 
	bool 
	_is_in_transaction() const 
	{ 
		mutexKeeper keeper(_mtx); 
		return _transaction; 
	}
	//! \brief checks open status
	inline 
	bool 
	_is_open_sql() const 
	{ 
		mutexKeeper keeper(_mtx); 
		return _is_open; 
	}
private:
	//! \brief sets the connect state 
	inline 
	void 
	_set_connect() 
	{ 
		mutexKeeper keeper(_mtx); 
		_connection = true; 
	}
	//! \brief sets the disconnect state
	inline 
	void 
	_set_disconnect() 
	{ 
		mutexKeeper keeper(_mtx); 
		_connection = false; 
	}
	//! \brief sets the transaction state on
	inline 
	void 
	_set_in_transaction() 
	{
		mutexKeeper keeper(_mtx); 
		_transaction = true; 
	}
	//! \brief sets the transaction state off
	inline 
	void 
	_set_out_transaction() 
	{ 
		mutexKeeper keeper(_mtx); 
		_transaction = false; 
	}
	//! \brief sets the open state
	inline 
	void 
	_set_open() 
	{ 
		mutexKeeper keeper(_mtx); 
		_is_open = true; 
	}
	//! \brief sets the close state
	inline 
	void 
	_set_close() 
	{ 
		mutexKeeper keeper(_mtx); 
		_is_open = false; 
	}
	//! \brief sets action 
	inline 
	void 
	_set_action(	action action_							//!< new action
					) 
	{ 
		mutexKeeper keeper(_mtx); 
		_action = action_; 
	}
	//! \brief sets a new state
	inline 
	void 
	_set_state(		module_state state						//!< new state
					)
	{ 
		mutexKeeper keeper(_mtx); 
		_state = state; 
	}
private:
	bool			_connection;							//!< connection state
	bool			_transaction;							//!< transaction state
	bool			_trusted;								//!< trusted connecton flag
	string_t		_connect_string;						//!< connection string

// query part
protected:
	//! \brief gets number params in query
	void 
	_get_number_params();
	//! \brief rebinds all params
	void 
	_rebind_params();
	//! \brief binds all params
	void 
	_bind_params();
	//! \brief binds all columns
	void 
	_bind_columns();
	//! \brief gets columns info
	void 
	_get_columns_info();

protected:
	//! pure virtual functions
	
	//! \brief allocates resources before executing sql statement
	virtual 
	void 
	v_before_execute() = 0;
	//! \brief calls additional function after executing the sql statement
	virtual 
	void 
	v_after_execute() = 0;
	//! \brief executes SQL expression
	virtual 
	void 
	v_execute() = 0;
	//! \brief closes opened query
	virtual 
	void 
	v_close() = 0;
	//! \brief fetches block of rows
	virtual 
	void 
	v_fetch() = 0;
	//! \brief replaces quote to the available sign for native drive
	virtual 
	void 
	v_replace_quote() = 0;
	//! \brief binds one param by index 
	virtual 
	void 
	v_bind_one_param(size_t index							//!< parameter index
					) = 0;
	//! \brief before bind columns sometimes we got db specific
	virtual 
	void 
	v_before_bind_columns() = 0;
	//! \brief binds one column by index
	virtual 
	void 
	v_bind_one_column(size_t index							//!< parameter index
					) = 0;
	//! \brief gets number columns in query
	virtual 
	size_t 
	v_get_number_columns() = 0;
	//! \brief converts one field value to variant buffer
	virtual 
	void 
	v_convert_one_value(size_t row,							//!< row index
					size_t col,								//!< column index
					terimber_db_value& val					//!< [out] value
					) = 0;
	//! \brief gets one column info from native driver
	virtual 
	void 
	v_get_one_column_info(size_t index						//!< column index
					) = 0;
	//! \brief forms SQL expression depending on type selected and native driver
	virtual 
	void 
	v_form_sql_string() = 0;
	//! \brief rebinds one param for input/output or output params
	virtual 
	void 
	v_rebind_one_param(size_t index							//!< parameter index
					) = 0;
	//! \brief terminates asynchronous action
	virtual 
	void 
	v_interrupt_async() = 0;
	//! \brief converts types
	virtual 
	dbtypes 
	v_native_type_to_client_type(size_t native_type			//!< SQL native type
					) = 0;

protected:
	//! overrides function for employer class
	//! \brief checks for job
	bool 
	v_has_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);
	//! \brief does the real job
	void 
	v_do_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);
	//! \brief notifies subscribers
	virtual 
	void 
	_notify_async(	bool noerrors,							//!< error flag
					size_t ident							//!< server ident
					);

protected:
	//! \brief returns current action
	inline 
	action 
	get_action() const 
	{ 
		mutexKeeper keeper(_mtx); 
		return _action; 
	}
	//! \brief returns the current state
	inline 
	module_state 
	get_state() const
	{ 
		mutexKeeper keeper(_mtx); 
		return _state; 
	}

private:
	//! \brief makes sure the connection is established
	inline 
	void 
	_check_connect() 
	{ 
		if (!_is_connect()) 
			exception::_throw(ex_not_connect, &dbMsgTable); 
	}
	//! \brief makes sure the connection is closed
	inline 
	void 
	_check_disconnect() 
	{ 
		if (_is_connect()) 
			exception::_throw(ex_already_connect, &dbMsgTable); 
	}
	//! \brief makes sure the transaction is on
	inline 
	void 
	_check_in_transaction() 
	{ 
		if (!_is_in_transaction()) 
			exception::_throw(ex_not_transaction, &dbMsgTable); 
	}
	//! \brief makes sure the transaction is off
	inline 
	void 
	_check_out_transaction() 
	{ 
		if (_is_in_transaction()) 
			exception::_throw(ex_already_transaction, &dbMsgTable); 
	}
	//! \brief checks if the state is OK
	inline 
	void 
	_check_state()
	{ 
		if (get_state() != STATE_OK) 
			exception::_throw(ex_invalid, &dbMsgTable); 
	}
	//! \brief checks open state; must be open or exception will be thrown
	inline 
	void 
	_check_open() const 
	{ 
		if (!_is_open_sql()) 
			exception::_throw(ex_not_sql_open, &dbMsgTable); 
	}
	//! \brief checks close state; must be closed or exception will be thrown
	inline 
	void 
	_check_close() const 
	{ 
		if (_is_open_sql()) 
			exception::_throw(ex_already_sql_open, &dbMsgTable); 
	}
	//! \brief checks action state; must be none or exception will be thrown
	inline 
	void 
	_check_action() const 
	{ 
		if (ACTION_NONE != get_action()) 
			exception::_throw(ex_sql_in_action, &dbMsgTable); 
	}
	//! \brief checks asynchronous action
	bool 
	_is_asynchronous_action() const;
	//! \brief processes sql statement
	void 
	_process_sql(	const char* sql							//!< SQL statement
					);
	//! \brief starts thread for asynchronous actions
	void 
	_start_thread();
	//! \brief converts parameter value (if required) 
	inline 
	terimber_xml_value 
	get_param_as_value(size_t index,						//!< parameter index
					vt_types type							//!< new type
					) const;
	//! \brief converts column value (if required) 
	inline 
	terimber_xml_value 
	get_value_as_value(size_t index,						//!< column index
					vt_types type							//!< new type
					) const;

protected:
	byte_allocator*						_columns_allocator; //!< columns allocator
	byte_allocator*						_data_allocator;	//!< data allocator
	byte_allocator*						_bulk_allocator;	//!< bulk params allocator
	byte_allocator						_sql_allocator;		//!< sql allocator
	string_t							_sql;				//!< SQL statement
	char								_quote;				//!< quote
	params_t							_params;			//!< parameters
	bulk_params_t						_bulk_params;		//!< bulk parameters
	binders_t							_cols;				//!< column
	recordset_list_t					_data;				//!< output recodset
	size_t								_start_row;			//!< start row for fetching
	size_t								_requested_rows;	//!< rows to fetch
	size_t								_fetched_rows;		//!< actually fetched rows
	bool								_forward;			//!< fetching direction
	size_t								_bulk_rows;			//!< bulk rows for select
private:
	volatile module_state				_state;				//!< server state
	volatile action						_action;			//!< server action
	volatile bool						_is_open;			//!< open flag
	thread								_thread;			//!< thread
	async_db_notify_list_t				_list_callback;		//!< list of callbacks

//! recordset part
private:
	mutable recordset_list_t::iterator	_iter;				//!< current iterator for recordset
	//! \brief returns end iterator
	inline 
	recordset_list_t::iterator 
	get_iter_end() const
	{ 
		return const_cast< recordset_list_t& >(_data).end(); 
	}
	//! \brief returns begin iterator
	inline 
	recordset_list_t::iterator 
	get_iter_begin() const
	{
		return const_cast< recordset_list_t& >(_data).begin(); 
	}
};

/////////////////////////////////////////////////////////////////
// pool support
//! \class db_arg
//! \brief contain login information
class db_arg
{
public:
	//! \brief login constructor
	db_arg(	size_t ident = 0,								//!< server ident 
			bool trusted = false,							//!< trusted connection flag
			const char* login_string = 0					//!< login string
			); 


	//! \brief less operator
	inline 
	bool
	operator<(const db_arg& x) const
	{
		return _trusted != x._trusted ? _trusted < x._trusted : _login_string < x._login_string;
	}

	//! \brief equal operator
	inline 
	bool
	operator==(const db_arg& x) const
	{
		return _trusted == x._trusted && _login_string == x._login_string;
	}
public:
	size_t				_ident;								//!< server ident
	bool				_trusted;							//!< trusted connection
	string_t			_login_string;						//!< login string
};

//! \class db_entry
//! \brief contain login information
class db_entry : public db_arg
{
public:
	//! \brief login constructor
	db_entry(	size_t ident = 0,							//!< server ident
				bool trusted = false,						//!< trusted connection flag
				const char* login_string = 0				//!< login string
			);

public:
	dbserver*			_obj;								//!< server object
};

//! \class db_creator
//! \brief db creator
template < class T >
class db_creator : public proto_creator< T, db_entry, db_arg > 
{
public:
	//! \brief activates object
	//! nothing to do
	static 
	void 
	activate(		db_entry* obj,							//!< object pointer 
					const db_arg& arg
					);

	//! \brief finds object
	static 
	bool 
	find(			db_entry* obj,							//!< object pointer 
					const db_arg& arg
					);
	//! \brief backing object
	static 
	void 
	back(			db_entry* obj,							//!< object pointer 
					const db_arg& arg
					);
	//! \brief destroys object
	static 
	void 
	destroy(		db_entry* obj,							//!< object pointer
					const db_arg& arg
					);

	//! \brief deactivates object - nothing to do
	static 
	void 
	deactivate(		db_entry* obj,							//!< object pointer 
					const db_arg& arg
					);
};


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_db_h_

