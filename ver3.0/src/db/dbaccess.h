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

#ifndef _terimber_dbaccess_h_
#define _terimber_dbaccess_h_

#include "ostypes.h"
#include "dbtypes.h"
#include "log.h"

//! \class async_db_notify
//! \brief abstract class for asynchronous db notifications
class async_db_notify
{
public:
	//! \brief destructor
	virtual
	~async_db_notify() {}
	
	//! \brief sends the notification message
	virtual 
	void 
	notify_async(	bool noerrors,							//!< flag if error occurred
					size_t ident							//!< dbserver ident
					) = 0;
};

//! \class dbserver
//! \brief class implements the access to database in the general way
class dbserver : public terimber_log_helper
{
public:
	//! \brief destructor
	virtual 
	~dbserver() 
	{
	}

	//! base methods
	//! \brief returns current ident
	//! to distingush different instances of dbserver
	virtual 
	size_t 
	get_ident() const = 0;
	//! \brief returns current state
	//! to check if the sql asynchronous action completed.
	virtual 
	bool 
	is_ready() const = 0;
	//! \brief returns error code
	virtual 
	size_t 
	get_code() const = 0;
	//! \brief returns error description, NULL will be returned if there are no errors
	virtual 
	const char* 
	get_error() const = 0;

	//
	//! db connection methods
	// 

	//! \brief establish db connection
	//! the syntax of the connection string depends on db server type
	//! ODBC syntax: DSN=alias;UID=user;PWD=password
	// ORACLE syntax: user/password@SID
	//! MYSQL syntax: UID=user;PWD=password;HOST=hostname;PORT=port;DB=database;
	virtual 
	bool 
	connect(		bool trusted_connection,				//!< flag trusted connection, if BD sever supports trusted connections
					const char* connect_string				//!< connection string
					) = 0;
	//! \brief closes connection
	virtual 
	bool 
	disconnect() = 0;
	//! \brief starts transaction
	virtual 
	bool 
	start_transaction() = 0;
	//! \brief commits transaction
	virtual 
	bool 
	commit() = 0;
	//! \brief rolls back transaction
	virtual 
	bool 
	rollback() = 0;
	//! \brief returns the connection status
	virtual 
	bool 
	is_connect() const = 0;
	//! \brief returns the transaction status
	virtual 
	bool 
	is_in_transaction() const = 0;
	//! \brief checks if the connection is alive or not
	virtual 
	bool 
	is_connect_alive() = 0;

	//
	//! sql methods
	//
	
	//! \brief prepares the sql parameter methods
	//! allocates params - the param count can reduced or incresed and 
	//! the parameters, which shouldn't be affected will be still valid 
	//! to clear parameters use size == 0
	virtual 
	bool 
	resize_params(	size_t size								//!< number of parameters
					) = 0;
	//! \brief gets param count
	virtual 
	size_t 
	get_param_count() const = 0;
	//! \brief returns the virtual terimber type like db_bool, ...
	virtual 
	dbtypes 
	get_param_type(	size_t index							//!< parameter index
					) const = 0;
	//! \brief returns the in/out
	virtual 
	db_param_type 
	get_param_in_out(size_t index							//!< parameter index
					) const = 0;
	//! \brief returns the nullable - database specific
    virtual 
	bool 
	get_param_is_null(size_t index							//!< parameter index
					) const = 0;

	//! \brief returns param value
    //! if index out of range then zero will be return
	//! if type is mismatched then attempt to convert will be taken
	//! the convertion of internal value is made on temporary allocator
	//! the returned pointer is valid until next call
	//! the best practice to check type before call gets value functions
	virtual 
	bool 
	get_param_as_bool(size_t index							//!< parameter index
					) const = 0;
	//! \brief get parameter as char
	//! conversion can be made
	virtual 
	sb1_t 
	get_param_as_char(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the parameter as a byte
	//! conversion can be made
	virtual 
	ub1_t 
	get_param_as_byte(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the parameter as a short
	//! conversion can be made
	virtual 
	sb2_t 
	get_param_as_short(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the parameter as an unsigned short
	//! conversion can be made
	virtual 
	ub2_t 
	get_param_as_word(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the parameter as a long int (4 bytes)
	//! conversion can be made
	virtual 
	sb4_t 
	get_param_as_long(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the parameter as an unsigned long int (4 bytes)
	//! conversion can be made
	virtual 
	ub4_t 
	get_param_as_dword(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the parameter as a float (4 bytes)
	//! conversion can be made
	virtual 
	float 
	get_param_as_float(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the parameter as a double (8 bytes)
	//! conversion can be made
	virtual 
	double 
	get_param_as_double(size_t index						//!< parameter index
					) const = 0;
	//! \brief gets the parameter as a signed long long (8 bytes)
	//! conversion can be made
	virtual 
	sb8_t 
	get_param_as_long64(size_t index						//!< parameter index
					) const = 0;
	//! \brief gets the parameter as an unsigned long long (8 bytes)
	//! conversion can be made
	virtual 
	ub8_t 
	get_param_as_dword64(size_t index						//!< parameter index
					) const = 0;
	//! \brief gets the parameter as a guid
	//! conversion can be made
	//! for high performance we are using reference
	virtual 
	bool 
	get_param_as_guid(size_t index,							//!< parameter index
					guid_t& val								//!< [out] value
					) const = 0;
	//! \brief gets the parameter as a word
	//! conversion can be made
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_param_as_numeric(size_t index,						//!< parameter index
					char delimeter							//!< delimdeter like '.' or ','
					) const = 0;	
	//! \brief gets the parameter as a word
	//! conversion can be made
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_param_as_decimal(size_t index,						//!< parameter index
					char delimeter							//!< delimdeter like '.' or ','
					) const = 0;
	//! \brief gets the parameter as a word
	//! conversion can be made
	//! to avoid confusions with different date format functions returns parsed date
	virtual 
	bool 
	get_param_as_date(size_t index,							//!< parameter index
					ub4_t& year,							//!< [out] year [0 : 1000000]
					ub1_t& month,							//!< [out] month [1-12]
					ub1_t& day,								//!< [out] day [1-31]
					ub1_t& hour,							//!< [out] hour [0-23]
					ub1_t& minute,							//!< [out] minute [0-59]
					ub1_t& second,							//!< [out] second [0-59]
					ub2_t& millisec,						//!< [out] millisec [0-999]
					ub1_t& wday,							//!< [out] wday [1-7]
					ub2_t& yday								//!< [out] yday [1-366]
					) const = 0;
	//! \brief gets the parameter as a word
	//! conversion can be made
	//! function returns the pointer to the string
	virtual 
	const char* 
	get_param_as_string(size_t index						//!< parameter index
					) const = 0;
	//! \brief gets the parameter as a word
	//! conversion can be made
	//! function returns the pointer to the wide string
	virtual 
	const wchar_t* 
	get_param_as_wstring(size_t index						//!< parameter index
					) const = 0;
	//! \brief function returns the pointer to byte array
	//! len will contain the length of byte array
	virtual 
	const ub1_t* 
	get_param_as_binary(size_t index,						//!< parameter index
					size_t& len								//!< [out] the length of binaries
					) const = 0;
	//! \brief the format of buffer is as follows
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
 	virtual 
	const ub1_t* 
	get_param_as_binary_ptr(size_t index					//!< parameter index
					) const = 0;
	//! \brief sets the parameter as null
	virtual 
	bool 
	set_param_as_null(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					dbtypes type							//!< parameter type
					) = 0;
	//! \brief sets the parameter as a bool
	virtual 
	bool 
	set_param_as_bool(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					bool val								//!< value
					) = 0;
	//! \brief sets the parameter as a char	 
	virtual 
	bool 
	set_param_as_char(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					sb1_t val								//!< value
					) = 0;
	//! \brief sets the parameter as a byte	 
	virtual 
	bool 
	set_param_as_byte(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					ub1_t val								//!< value
					) = 0;
	//! \brief sets the parameter as a signed short (2 bytes)	 
	virtual 
	bool 
	set_param_as_short(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					sb2_t val								//!< value
					) = 0;
	//! \brief sets the parameter as an unsigned short (2 bytes)	 
	virtual 
	bool 
	set_param_as_word(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					ub2_t val								//!< value
					) = 0;
	//! \brief sets the parameter as a long int (4 bytes)
	virtual 
	bool 
	set_param_as_long(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					sb4_t val								//!< value
					) = 0;
	//! \brief sets the parameter as an unsigned long int (4 bytes)
	virtual 
	bool 
	set_param_as_dword(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					ub4_t val								//!< value
					) = 0;
	//! \brief sets the parameter as a float (4 bytes)
	virtual 
	bool 
	set_param_as_float(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					float val								//!< value
					) = 0;
	//! \brief sets the parameter as a double (8 bytes)
	virtual 
	bool 
	set_param_as_double(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					double val								//!< value
					) = 0;
	//! \brief sets the parameter as a long long int (8 bytes)	 
	virtual 
	bool 
	set_param_as_long64(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					sb8_t val								//!< value
					) = 0;
	//! \brief sets the parameter as an unsigned long long int (8 bytes)	 
	virtual 
	bool 
	set_param_as_dword64(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					ub8_t val								//!< value
					) = 0;
	//! \brief sets the parameter as a guid
	virtual 
	bool 
	set_param_as_guid(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					const guid_t& val						//!< value
					) = 0;

	//! this function depends on the format
	//! to avoid the confusions of different numeric and date formats
	//! -1223456.7894 - delimeter here is "."
	//! -1223456,7894 - delimeter here is ","
	//! \brief sets the parameter as numeric
	virtual 
	bool 
	set_param_as_numeric(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					const char* val,						//!< value
					char delimeter							//!< delimeter
					) = 0;
	//! \brief sets the parameter as a decimal
	virtual 
	bool 
	set_param_as_decimal(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					const char* val,						//!< value
					char delimeter							//!< delimeter
					) = 0;
	//! \brief sets the parameter as a date
	virtual 
	bool
	set_param_as_date(size_t index,							//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					ub4_t year,								//!< year [0-1000000]
					ub1_t month,							//!< month [1-12]
					ub1_t day,								//!< day [1-31]
					ub1_t hour,								//!< hour [0-23]
					ub1_t minute,							//!< minute [0-59]
					ub1_t second,							//!< second [0-59]
					ub2_t millisec							//!< millisec [0-999]
					) = 0;
	//! \brief sets the parameter as a string
	//! for 0 terminated string len can be -1
	//! max_len for output parameter only
	virtual 
	bool 
	set_param_as_string(size_t index,						//!< parameter index	
					db_param_type in_out,					//!< in/out parameter property
					const char* val,						//!< value
					size_t len,								//!< string length
					size_t max_len = 0						//!< optional expected max length for output parameter
					) = 0;
	//! \brief sets the parameter as a string pointer
	//! to avoid copying already allocated memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statement has been executed.
	//! it supposed that it will work only for the input parameters, not for output or input/output ones
	virtual 
	bool 
	set_param_as_string_ptr(size_t index,					//!< parameter index
					const char* val							//!< value
					) = 0;
	
	//! \brief sets the parameter as a wide string
	//! for 0 terminated string len can be -1
	//! max_len for output parameter only
	virtual 
	bool 
	set_param_as_wstring(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					const wchar_t* val,						//!< value
					size_t len,								//!< string length
					size_t max_len = 0						//!< optional expected max length for output parameter
					) = 0;
	//! \brief sets the parameter as a wide string pointer	
	//! to avoid copying the already allocated huge memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of provided pointer, 
	//! which has to be valid until the sql statement is been executing.
	//! it is supposed to work only for input parameters, not for output or input/output ones
	virtual 
	bool 
	set_param_as_wstring_ptr(size_t index,					//!< parameter index
					const wchar_t* val						//!< value
					) = 0;
	//! \brief sets the parameter as a wide string pointer	
	//! val points to the valid buffer of raw byte of "len" length
	//! max_len for output parameter only
	virtual 
	bool 
	set_param_as_binary(size_t index,						//!< parameter index
					db_param_type in_out,					//!< in/out parameter property
					const ub1_t* val,						//!< value
					size_t len,								//!< byte stream length
					size_t max_len = 0						//!< optional expected max length for output parameter
					) = 0;
	//! \brief sets the parameter as a wide string pointer	
	//! to avoid copying the already allocated huge memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statement is been executing.
	//! the format of buffer must be as follow
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
	//! it is supposed to only work for input parameters, not for output or input/output ones
	virtual 
	bool 
	set_param_as_binary_ptr(size_t index,					//!< parameter index
					const ub1_t* val						//!< value
					) = 0;

	//! internal columns access

	//! \brief gets the column count
	virtual 
	size_t 
	get_column_count() const = 0;
	//! \brief returns the virtual terimber dbtype like db_bool, ...
	virtual 
	dbtypes 
	get_column_type(size_t index							//!< parameter index
					) const = 0;
	//! \brief returns the name of the column
	virtual 
	const char* 
	get_column_name(size_t index							//!< parameter index
					) const = 0;
	//! \brief returns the nullable - database specific
	virtual 
	bool 
	get_column_nullable(size_t index						//!< parameter index
					) const = 0;
	//! \brief returns the scale
	virtual 
	size_t 
	get_column_scale(size_t index							//!< parameter index
					) const = 0;
	//! \brief returns the precision
	virtual 
	size_t 
	get_column_precision(size_t index						//!< parameter index
					) const = 0;
	//! \brief returns the length of column (only for fixed length columns)
	virtual 
	size_t 
	get_column_max_length(size_t index						//!< parameter index
					) const = 0;

	//
	//! rowset navigation methods
	// 

	//! \brief gets the row count
	virtual 
	size_t 
	get_row_count() const = 0;
	//! \brief resets the internal cursor to the out of end
	virtual 
	void 
	reset() const = 0;
	//! \brief tries to find the next row, after reset it goes to the first row
	virtual 
	bool 
	next() const = 0;
	//! \brief tries to find the previous row, after reset it goes to the last row
	virtual 
	bool 
	prev() const = 0;

	//! \brief returns the value of the column at the current row position
    //! if index out of range then zero will be return
	//! if type is mismatched then attempt to convert will be taken
	//! the convertion of internal value is made on temporary allocator
	//! the returned pointer is valid until next call
	//! the best practice to check type before call gets value functions
	virtual 
	bool 
	get_value_is_null(size_t index							//!< parameter index			
					) const = 0;
	//! \brief gets the value as a boolean
	virtual 
	bool 
	get_value_as_bool(size_t index							//!< parameter index
					) const = 0;
	virtual 
	//! \brief gets the value as a char
	sb1_t 
	get_value_as_char(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the value as a byte
	virtual 
	ub1_t 
	get_value_as_byte(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the value as a short int (2 bytes)
	virtual 
	sb2_t 
	get_value_as_short(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the value as an unsigned short int (2 bytes)
	virtual 
	ub2_t 
	get_value_as_word(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the value as a long int (4 bytes)
	virtual
	sb4_t 
	get_value_as_long(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the value as an unsigned long int (4 bytes)
	virtual 
	ub4_t 
	get_value_as_dword(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the value as a float (4 bytes)
	virtual 
	float 
	get_value_as_float(size_t index							//!< parameter index
					) const = 0;
	//! \brief gets the value as a double (8 bytes)
	virtual 
	double 
	get_value_as_double(size_t index						//!< parameter index
					) const = 0;
	//! \brief gets the value as a long long int (8 bytes)
	virtual 
	sb8_t 
	get_value_as_long64(size_t index						//!< parameter index
					) const = 0;
	//! \brief gets the value as an unsigned long long int (8 bytes)
	virtual 
	ub8_t 
	get_value_as_dword64(size_t index						//!< parameter index
					) const = 0;

	//! \brief gets the value as a guid
	virtual 
	bool 
	get_value_as_guid(size_t index,							//!< parameter index
					guid_t& val								//!< [out] value
					) const = 0;
	//! \brief gets the value as a guid
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_value_as_numeric(size_t index,						//!< parameter index
					char delimeter							//!< delimeter
					) const = 0;
	//! \brief gets the value as a guid
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_value_as_decimal(size_t index,						//!< parameter index
					char delimeter							//!< delimeter
					) const = 0;
	//! \brief gets the value as a date
	//! to avoid confusions with different date format the functions return parsed date
	virtual 
	bool 
	get_value_as_date(size_t index,							//!< parameter index
					ub4_t& year,							//!< [out] year [0-1000000] from 1 A.D.
					ub1_t& month,							//!< [out] month [1-12]
					ub1_t& day,								//!< [out] day [1-31]
					ub1_t& hour,							//!< [out] hour [0-23]
					ub1_t& minute,							//!< [out] minute [0-59]
					ub1_t& second,							//!< [out] second [0-59]
					ub2_t& millisec,						//!< [out] millisecond [0-999]
					ub1_t& wday,							//!< [out] day of week [1-7]
					ub2_t& yday								//!< [out] day of year [1-366]
					) const = 0;
	//! \brief gets the value as a date
	//! function returns the pointer to string 
	virtual 
	const char* 
	get_value_as_string(size_t index						//!< parameter index
					) const = 0;
	//! \brief function returns the pointer to wide string 
	virtual 
	const wchar_t* 
	get_value_as_wstring(size_t index						//!< parameter index
					) const = 0;

	//! \brief returns pointer to the raw bytes
	//! size of bytes is put to len
	virtual 
	const ub1_t* 
	get_value_as_binary(size_t index,						//!< parameter index
					size_t& len								//!< [out] length of byte array
					) const = 0;
	//! \brief returns pointer to the raw bytes
	//! for reusing only
	//! the format of buffer is as follow
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
 	virtual 
	const ub1_t* 
	get_value_as_binary_ptr(size_t index					//!< parameter index
					) const = 0;
	//! \brief checks the state of the sql request
	virtual 
	bool 
	is_open_sql() const = 0;
	//! \brief executes the parameterized sql statement with recordset result
	virtual 
	bool 
	open_sql(		bool async,								//!< asynchronous flag
					const char* sql							//!< sql statement
					) = 0;
	//! \brief executes the parameterized sql statement without recordset result
	virtual 
	bool 
	exec_sql(		bool async,								//!< asynchronous flag
					const char* sql							//!< sql statement
					) = 0;
	//! \brief executes the stored procedure with recordset result
	virtual 
	bool 
	open_proc(		bool async,								//!< asynchronous flag
					const char* name						//!< stored procedure name
					) = 0;
	//! \brief executes the stored procedure without recordset result
	virtual 
	bool 
	exec_proc(		bool async,								//!< asynchronous flag
					const char* name						//!< stored procedure name
					) = 0;
	//! \brief fetches the data and returns the recordset
	//! defines the start row, numbers rows and fetches direction
	//! if start_row is 0 then the fetch starts from the position after the last position fetched before
	//! if start_row > 0 then start_row is considered as absolute row number in recordset
	virtual
	bool 
	fetch_data(		bool async,								//!< asynchronous flag
					size_t start_row,						//!< start row
					size_t num_rows,						//!< number of rows to fetch
					bool forward							//!< direction
					) = 0;
	//! \brief closes sql - free allocated resources
	virtual 
	bool 
	close_sql() = 0;
	//! \brief interrupts asynchronous request
	virtual 
	bool 
	interrupt_request() = 0;
	//! \brief sets quote
	//! the syntax of sql statement with parameters must be understandable
	//! for internal sql processor
	//! therefore it should look like
	//! selects * from the table where pk = :pk
	//! here ':' char is an identificator of parameter with name "pk"
	//! function set_quote allows to change this identificator to the preferable one
	//! ':' is a default identificator
	virtual 
	bool 
	set_quote(		char quote								//!< quote
					) = 0;
	//! \brief sets the callback function for asynchronous actions
	virtual 
	void 
	set_callback(	async_db_notify* target					//!< notification callback
					) = 0;
	//! \brief remove the callback function for asynchronous actions
	virtual 
	void 
	remove_callback(async_db_notify* target					//!< notification callback
					) = 0;

	//! \brief bulk operation support
	//! caller should be very careful with this functionality
	//! current set of parameters previously set by set_param_* functions
	//! will be moved to the bulk parameters list
	//! the current parameters set will have the same values, but will not participate in bulk operations
	//! moreover if the next call set_param_* function will change the type - all bulk parameters will be deleted.
	virtual 
	bool 
	param_bulk_store() = 0;
	//! \brief removes the last stored bulk parameter
	virtual 
	bool 
	param_bulk_remove() = 0;
	//! \brief removes all stored bulk parameters
	virtual 
	bool 
	param_bulk_remove_all() = 0;
};

#endif // _terimber_dbaccess_h_

