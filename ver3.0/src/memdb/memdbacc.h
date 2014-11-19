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

#ifndef _terimber_memdbacc_h_
#define _terimber_memdbacc_h_

#include "db/dbaccess.h"
#include "dbtypes.h"
#include "log.h"

#pragma pack(4)
//! \enum terimber_db_row_status
//! \brief db row status
enum terimber_db_row_status
{
	status_lookup = 0,										//!< lookup row
	status_original,										//!< original row
	status_new,												//!< new row
	status_updated,											//!< updated row
	status_deleted											//!< deleted row
};

//! \class terimber_index_column_info
//! \brief index column information
class terimber_index_column_info
{
public:
	size_t	_index;											//!< number of column 0- based
	bool	_asc_sort;										//!< how to sort ascend or descend
	bool	_case_insensitive;								//!< for string only
};

//! \class terimber_table_column_desc
//! \brief class helps to create empty table in memory explicitly 
class terimber_table_column_desc
{
public:
	dbtypes			_type;									//!< column type
	const char*		_name;									//!< column name, can be null
	size_t			_scale;									//!< scale makes sense only for numeric/decimal types
	size_t			_precision;								//!< precision makes sense only for numeric/decimal types
	size_t			_max_length;							//!< max length for column, -1 for blobs
	bool			_nullable;								//!< can column has null values
};

//! \class terimber_db_value_vector
// abstract class to set conditions for lookup or update row
class terimber_db_value_vector
{
protected:
	//! \brief destructor
	virtual 
	~terimber_db_value_vector() 
	{
	}

public:
	//! \brief gets vector size
	virtual 
	size_t 
	get_size() const = 0;

	//! sets condition value
	//! \brief sets value as a null
	virtual 
	bool 
	set_value_as_null(size_t index,							//!< index
					dbtypes type							//!< db type
					) = 0;
	//! \brief sets value as a boolean
	virtual 
	bool 
	set_value_as_bool(size_t index,							//!< index
					bool val								//!< value
					) = 0;
	//! \brief sets value as a char
	virtual 
	bool 
	set_value_as_char(size_t index,							//!< index
					sb1_t val								//!< value
					) = 0;
	//! \brief sets value as a byte
	virtual 
	bool 
	set_value_as_byte(size_t index, 						//!< index
					ub1_t val								//!< value
					) = 0;
	//! \brief sets value as a short
	virtual 
	bool 
	set_value_as_short(size_t index, 						//!< index
					sb2_t val								//!< value
					) = 0;
	//! \brief sets value as a unsigned short
	virtual 
	bool 
	set_value_as_word(size_t index, 						//!< index
					ub2_t val								//!< value
					) = 0;
	//! \brief sets value as a long
	virtual 
	bool 
	set_value_as_long(size_t index, 						//!< index
					sb4_t val								//!< value
					) = 0;
	//! \brief sets value as a unsigned long
	virtual 
	bool 
	set_value_as_dword(size_t index, 						//!< index
					ub4_t val								//!< value
					) = 0;
	//! \brief sets value as a float
	virtual 
	bool 
	set_value_as_float(size_t index, 						//!< index
					float val								//!< value
					) = 0;
	//! \brief sets value as a double
	virtual 
	bool 
	set_value_as_double(size_t index, 						//!< index
					double val								//!< value
					) = 0;
	//! \brief sets value as a signed int64
	virtual 
	bool 
	set_value_as_long64(size_t index, 						//!< index
					sb8_t val								//!< value
					) = 0;
	//! \brief sets value as a unsigned int64
	virtual 
	bool 
	set_value_as_dword64(size_t index, 						//!< index
					ub8_t val								//!< value
					) = 0;
	//! \brief sets value as a guid
	virtual 
	bool 
	set_value_as_guid(size_t index, 						//!< index
					const guid_t& val						//!< value
					) = 0;

	// this function depends on format
	// to avoid the confusions of different numeric and date formats

	// -1223456.7894 - delimeter here is "."
	// -1223456,7894 - delimeter here is ","
	//! \brief sets value as a numeric
	virtual 
	bool 
	set_value_as_numeric(size_t index,  					//!< index
					const char* val, 						//!< value
					char delimeter							//!< delimeter
					) = 0;
	//! \brief sets value as a decimal
	virtual 
	bool 
	set_value_as_decimal(size_t index,  					//!< index
					const char* val, 						//!< value
					char delimeter							//!< delimeter
					) = 0;
	// date from A.C. year > 0
	// valid input
	// year [1000000]
	// 
	// 
	// 
	// 
	// 
	// millisec [0 - 999]
	//! \brief sets value as a date
	virtual 
	bool 
	set_value_as_date(size_t index,   						//!< index
					ub4_t year,								//!< years [0-1000000]
					ub1_t month,							//!< months [1 - 12]
					ub1_t day,								//!< days [1 - 31]
					ub1_t hour,								//!< hours [0 - 23]
					ub1_t minute,							//!< minutes [0 - 59]
					ub1_t second,							//!< seconds [0 - 59]
					ub2_t millisec							//!< milliseconds [0 - 999]
					) = 0;

	//! \brief sets value as a string
	//! for 0 terminated string len can be -1
	virtual 
	bool 
	set_value_as_string(size_t index,						//!< index
					const char* val,						//!< string pointer
					size_t len								//!< string length
					) = 0;
	
	//! \brief sets value as a string pointer
	//! to avoid copying already allocated memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statement has been executed.
	//! it is supposed to work only for input parameters, not for output or input/output ones
	virtual 
	bool 
	set_value_as_string_ptr(size_t index,					//!< index
					const char* val							//!< string pointer
					) = 0;
	
	//! \brief sets value as a wide string
	//! for 0 terminated string len can be -1
	virtual 
	bool 
	set_value_as_wstring(size_t index,						//!< index
					const wchar_t* val,						//!< string pointer
					size_t len								//!< string length
					) = 0;
	//! \brief sets value as a wide string pointer
	//! to avoid copying already allocated memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statement has been executed.
	virtual 
	bool 
	set_value_as_wstring_ptr(size_t index,					//!< index
					const wchar_t* val						//!< string pointer
					) = 0;
	//! \brief sets value as a binary
	//! val points to the valid buffer of raw byte of "len" length
	virtual 
	bool 
	set_value_as_binary(size_t index,						//!< index
					const ub1_t* val,						//!< value pointer
					size_t len								//!< length of bytes
					) = 0;
	//! \brief sets value as a binary pointer
	//! to avoid copying already allocated memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statement has been executed.
	//! the format of buffer must be as follows
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
	//! it is supposed to work only for input parameters, not for output or input/output ones
	virtual 
	bool 
	set_value_as_binary_ptr(size_t index,					//!< index
					const ub1_t* val						//!< value pointer
					) = 0;


	//! get values
	//! \brief check if value is null
	virtual 
	bool 
	get_value_is_null(size_t index							//!< index
					) const = 0;

	//! \brief get value as boolean
	virtual 
	bool 
	get_value_as_bool(size_t index							//!< index
					) const = 0;
	//! \brief get value as char
	virtual 
	sb1_t 
	get_value_as_char(size_t index							//!< index
					) const = 0;
	//! \brief get value as byte
	virtual 
	ub1_t 
	get_value_as_byte(size_t index							//!< index
					) const = 0;
	//! \brief get value as short
	virtual 
	sb2_t 
	get_value_as_short(size_t index							//!< index
					) const = 0;
	//! \brief get value as unsigned short
	virtual 
	ub2_t 
	get_value_as_word(size_t index							//!< index
					) const = 0;
	//! \brief get value as long
	virtual 
	sb4_t 
	get_value_as_long(size_t index							//!< index
					) const = 0;
	//! \brief get value as unsigned long
	virtual 
	ub4_t 
	get_value_as_dword(size_t index							//!< index
					) const = 0;
	//! \brief get value as float
	virtual 
	float 
	get_value_as_float(size_t index							//!< index
					) const = 0;
	//! \brief get value as double
	virtual 
	double 
	get_value_as_double(size_t index						//!< index
					) const = 0;
	//! \brief get value as int32
	virtual 
	sb8_t 
	get_value_as_long64(size_t index						//!< index
					) const = 0;
	//! \brief get value as unsigned int32
	virtual 
	ub8_t 
	get_value_as_dword64(size_t index						//!< index
					) const = 0;
	//! \brief get value as guid
	//! for high performance we are using reference
	virtual 
	bool 
	get_value_as_guid(size_t index, 						//!< index
					guid_t& val								//!< [out] value
					) const = 0;
	// delimeter defines the desirable delimeter sign
	//! \brief get value as numeric
	virtual 
	const char* 
	get_value_as_numeric(size_t index, 						//!< index
					char delimeter							//!< delimeter
					) const = 0;
	// delimeter defines the desirable delimeter sign
	//! \brief get value as deciaml
	virtual 
	const char* 
	get_value_as_decimal(size_t index, 						//!< index
					char delimeter							//!< delimeter
					) const = 0;
	// to avoid confusions with different date formats, functions return parsed date
	//! \brief get value as date
	virtual 
	bool 
	get_value_as_date(size_t index, 						//!< index
					ub4_t& year,							//!< [out] year [0-1000000]
					ub1_t& month,							//!< [out] month [1-12]
					ub1_t& day,								//!< [out] day [1-31]
					ub1_t& hour,							//!< [out] hour [0-23]
					ub1_t& minute,							//!< [out] minute [0-59]
					ub1_t& second,							//!< [out] second [0-59]
					ub2_t& millisec,						//!< [out] millisecond [0-999]
					ub1_t& wday,							//!< [out] day of week [1-7]
					ub2_t& yday								//!< [out] day of year [1-366]
					) const = 0;

	//! \brief gets value as a string
	//! function returns the pointer to string 
	virtual 
	const char* 
	get_value_as_string(size_t index						//!< index
					) const = 0;
	//! \brief gets value as a wide string
	//! function returns the pointer to wide string 
	virtual 
	const wchar_t* 
	get_value_as_wstring(size_t index						//!< index
					) const = 0;

	//! \brief gets value as a binary
	//! returns pointer to the raw bytes
	//! size of bytes is put to len
	virtual 
	const ub1_t* 
	get_value_as_binary(size_t index, 						//!< index
					size_t& len								//!< [out] length
					) const = 0;
	//! \brief gets value as a binary pointer
	//! for reusing only
	//! the format of buffer is as follows
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
 	virtual 
	const ub1_t* 
	get_value_as_binary_ptr(size_t index					//!< index
					) const = 0;

};

// forward declaration
class terimber_memindex;
class terimber_memlookup;

//! \class terimber_memtable
//! \brief abstract interface to memory table
class terimber_memtable : public terimber_log_helper
{
public:
	//! \brief destructor
	//! user is responsible for destoying the memtable object
	virtual 
	~terimber_memtable() 
	{
	}

	//! \brief returns the last occured error
	virtual 
	const char* 
	get_last_error() = 0;

	//! \brief creates internal table structure from server object
	//! db_server has to be after executing query or stored procedure
	//! and ready to fetch data
	//! memtable automatically detects the list of columns in recordset
	//! and creates a table in the memory with the same columns' order and types.
	//! max_rows sets a restriction on max rows to be retrived, -1 - all of them
	//! start_row means absolute number of row to start with
	//! unless you specify 0, then memtable starts with the current row
	//! to create an empty table, the fake select can be used
	//! for instanse, 
	//! select CAST(0 as NUMBER) as col1, CAST('' as VARCHAR2(255)) as col2 from dual;
	//! or you can specifiy the max_rows = 0

	virtual 
	bool 
	populate(		dbserver* server,						//!< dbserver instance, connected to database and opened the recordset
					size_t start_row,						//!< row to fetch from, 0 - current row
					size_t max_rows							//!< max row to fetch
					) = 0;

	//! \brief the other alternative is to create an empty table in the memory is specification 
	//! the list of columns descriptions explicitly
	virtual 
	bool 
	create(			size_t columns,							//!< number of columns
					const terimber_table_column_desc desc[]	//!< array of columns' descriptors
					) = 0;


	//! \brief resets all new, updates rows status to original, removes deleted rows
	//! updates all indexes
	virtual 
	void 
	refresh() = 0;
	//! \brief after populating and at any time after that you can request the number of rows in table
	virtual 
	size_t
	get_row_count() = 0;

	//! columns description is pretty much the same as db_server provides

	//! gets column count
	virtual 
	size_t 
	get_column_count() const = 0;
	//! \brief returns the virtual terimber dbtype like db_bool, ...
	virtual 
	dbtypes 
	get_column_type(size_t index							//!< index
					) const = 0;
	//! \brief returns the name of the column
	virtual 
	const char* 
	get_column_name(size_t index							//!< index
					) const = 0;
	//! \brief returns the nullable - database specific
 	virtual 
	bool 
	get_column_nullable(size_t index						//!< index
					) const = 0;
	//! \brief returns the scale
	virtual 
	size_t 
	get_column_scale(size_t index						//!< index
					) const = 0;
	//! \brief returns the precision
	virtual 
	size_t 
	get_column_precision(size_t index					//!< index
					) const = 0;
	//! \brief returns the length of the column (only for fixed length columns)
	virtual 
	size_t 
	get_column_max_length(size_t index					//!< index
					) const = 0;

	//! index management
	//! there are no connections between indexes in the memory and indexes in database

	//! \brief create index to perform the quick search
	//! columns the size of index
	//! info - array of index column descriptions
	//! returns the new created memindex object
	//! user must not to use object after returning it back
	//! or after memtable will be destoyed
	//! this is an expensive operation in order to consume memory and CPU time
	virtual 
	terimber_memindex* 
	add_index(		size_t columns,							//!< columns in index
					const terimber_index_column_info info[]	//!< array of index columns' descriptions
					) = 0;

	//! \brief destroys index
	//! memtable data is still valid
	//! all memlookups on the current index will be destroyed! 
	virtual 
	bool 
	remove_index(	terimber_memindex* obj					//!< index pointer
					) = 0;

		
	//! \brief requests value array
	virtual 
	terimber_db_value_vector* 
	allocate_db_values(size_t count							//!< array size
					) = 0;

	//! \brief destroys value array
	virtual 
	void 
	destroy_db_values(terimber_db_value_vector* obj			//!< value array pointer
					) = 0;
};

//! \class terimber_memindex
//! \brief abstract interface to memory index
class terimber_memindex
{
protected:
	//! \brief destructor
	//! users can't call destructor, memtable will take case about cleanup
	virtual 
	~terimber_memindex() 
	{
	}
public:

	//! \brief for navigation user has to create index lookup first
	//! the list of values, should be the same length & type as the index columns or less
	//! for example, let us assume we have an index with two columns (id, integer) & (name, string)
	//! if the user provides values for id & name then the lookup will search for rows with specified id & name
	//! however, if only id is specified, lookup will ignore the second column values.
	//! it's impossible to set search lookup on the second column without setting the first column
	//! to do such a thing the user needs to create an index with a different order of index columns, for instance 
	//! name and id, and set search criteria for the name column.
	//! user is allowed to ignore search criteria competely, (info == null)
	//! then lookup will move row by row in order they came from database, including the row with status = deleted
	virtual 
	terimber_memlookup* 
	add_lookup(		const terimber_db_value_vector* info	//!< value vector
					) = 0;

	//! \brief destroys lookup after using
	virtual 
	bool 
	remove_lookup(	terimber_memlookup* obj					//!< pointer to the lookup object
					) = 0;
};

//! \class terimber_memlookup
//! \brief abstract interface to memory lookup
class terimber_memlookup
{
protected:
	//! \brief destructor
	//! users can't call destructor, memtable will take case about cleanup
	virtual 
	~terimber_memlookup() 
	{
	}
public:

	//! \brief copy method
	//! lookups must be created for the same index
	virtual 
	bool 
	assign(			const terimber_memlookup& x				//!< input lookup
					) = 0;

	//! navigation methods

	//! \brief sets lookup outside the row sequence
	//! and initializes the selection criteria
	virtual 
	bool 
	reset(			const terimber_db_value_vector* info	//!< value vector
					) = 0;

	//! \brief tries to find the next row
	//! according to lookup values
	virtual 
	bool 
	next() const = 0;

	//! \brief tries to find the previous row
	//! according to lookup values
	virtual 
	bool 
	prev() const = 0;

	//! access to the row
	//! this is pretty much the same as db_server provides.
	//! returns the value of the column at the current row position
    //! if the index is out of range, then zero will be returned
	//! if the type is mismatched then an attempt to convert will be made
	//! the convertion of internal values is made on a temporary allocator
	//! the returned pointer is valid until next call
	//! the best practice to check type before call get value functions

	//! \brief checks if value is null
	virtual 
	bool 
	get_value_is_null(size_t index							//!< index
					) const = 0;
	//! \brief gets value as a boolean
	virtual 
	bool 
	get_value_as_bool(size_t index							//!< index
					) const = 0;
	//! \brief gets value as a char	
	virtual 
	sb1_t 
	get_value_as_char(size_t index							//!< index
					) const = 0;
	//! \brief gets value as a byte
	virtual 
	ub1_t 
	get_value_as_byte(size_t index							//!< index
					) const = 0;
	//! \brief gets value as a short	
	virtual 
	sb2_t 
	get_value_as_short(size_t index							//!< index
					) const = 0;
	//! \brief gets value as an unsigned short
	virtual 
	ub2_t 
	get_value_as_word(size_t index							//!< index
					) const = 0;
	//! \brief gets value as a long
	virtual 
	sb4_t 
	get_value_as_long(size_t index							//!< index
					) const = 0;
	//! \brief gets value as an unsigned long
	virtual 
	ub4_t 
	get_value_as_dword(size_t index							//!< index
					) const = 0;
	//! \brief gets value as a float
	virtual 
	float 
	get_value_as_float(size_t index							//!< index
					) const = 0;
	//! \brief gets value as a double
	virtual 
	double 
	get_value_as_double(size_t index						//!< index
					) const = 0;
	//! \brief gets value as a int64
	virtual 
	sb8_t 
	get_value_as_long64(size_t index						//!< index
					) const = 0;
	//! \brief gets value as an unsigned int64
	virtual 
	ub8_t 
	get_value_as_dword64(size_t index						//!< index
					) const = 0;

	//! \brief gets value as a guid
	//! for high performance we are using reference
	virtual 
	bool 
	get_value_as_guid(size_t index, 						//!< index
					guid_t& val
					) const = 0;
	//! \brief gets value as a numeric
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_value_as_numeric(size_t index,  					//!< index
					char delimeter
					) const = 0;
	//! \brief gets value as a decimal
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_value_as_decimal(size_t index,  					//!< index
					char delimeter
					) const = 0;
	//! \brief gets value as a date
	//! to avoid confusions with different date formats the functions returns parsed date
	virtual 
	bool 
	get_value_as_date(size_t index,  						//!< index
					ub4_t& year,							//!< [out] year [0-1000000]
					ub1_t& month,							//!< [out] month [1-12]
					ub1_t& day,								//!< [out] day [1-31]
					ub1_t& hour,							//!< [out] hour [0-23]
					ub1_t& minute,							//!< [out] minute [0-59]
					ub1_t& second,							//!< [out] second [0-59]
					ub2_t& millisec,						//!< [out] millisecond [0-999]
					ub1_t& wday,							//!< [out] day of week [1-7]
					ub2_t& yday								//!< [out] day of year [1-366]
					) const = 0;
	//! \brief gets value as a string
	//! function returns the pointer to string 
	virtual 
	const char* 
	get_value_as_string(size_t index  						//!< index
					) const = 0;
	//! \brief gets value as a wide string
	//! function returns the pointer to wide string 
	virtual 
	const wchar_t* 
	get_value_as_wstring(size_t index 						//!< index
					) const = 0;

	//! \brief gets value as a binary
	//! returns pointer to the raw bytes
	//! size of bytes is put to len
	virtual 
	const ub1_t* 
	get_value_as_binary(size_t index,  						//!< index
					size_t& len
					) const = 0;
	//! \brief gets value as a binary pointer
	//! for reusing only
	//! the format of buffer is as follows
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
 	virtual 
	const ub1_t* 
	get_value_as_binary_ptr(size_t index  					//!< index
					) const = 0;


	//! row modificators
	//! the current row can be changed
	//! there are possible scenarios
	//! 1. insert the new row - it will become the current one
	//! 2. update the current row - it will be the same current row, even index columns are affected
	//! 3. delete the current row - the current row will the next to the current one or current will point to the out of row sequence

	//! \brief insert new row
	//! if the row is modified it will still have a status = new
	//! if the row is removed it's just deleted from recordset
	//! inserted row has all columns' value as null
	virtual 
	bool 
	insert_row(	const terimber_db_value_vector* info		//!< value vector
				) = 0;


	//! \brief deletes existing row
	//! if row was in original recordset (modified or not), it will have a status = delete, and will not be accessible anymore
	//! deleting of new row will just remove row from recordset
	virtual 
	bool 
	delete_row() = 0;

	//! \brief updates the column value
	//! update of new row will preserve status = new
	//! update of original row will change status from original to update.
	virtual 
	bool 
	update_row(	const terimber_db_value_vector* info		//!< value vector
				) = 0;

	//! so far the possible status changes are
	//! "original -> "update"
	//! "original -> "delete"
	//! "original -> "update" -> "delete"
	//! "new" will be always "new" unless row will be deleted, there will be no status so.

	//! \brief returns the current status
	//! user can't get the status of row already deleted
	//! however if lookup is created from empty index, then all rows are available to see
	//! including deleted ones
	virtual 
	terimber_db_row_status 
	get_row_status() = 0;
};

//! \class terimber_memtable_factory
// creator
//
class terimber_memtable_factory
{
public:
	//! \brief creates table in memory object
	//! caller is responsible for destroying it
	terimber_memtable* 
	get_memtable();
};

#pragma pack()

#endif // _terimber_memdbacc_h_

