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

#ifndef _terimber_memdb_h_
#define _terimber_memdb_h_

#include "db/db.h"
#include "memdb/memdbacc.h"

#include "base/vector.h"
#include "base/list.h"
#include "base/map.h"
#include "base/common.h"


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class terimber_index_column_info_ex
//! \brief types for internal data structure
class terimber_index_column_info_ex : public terimber_index_column_info
{
public:
	dbtypes	_type;
};

//! \class terimber_db_value_vector_impl
//! \brief abstract class to set conditions for lookup
class terimber_db_value_vector_impl : public terimber_db_value_vector
{
public:
	//! \brief constructor
	terimber_db_value_vector_impl(size_t count				//!< vector size
					);

	//! \brief destructor
	virtual 
	~terimber_db_value_vector_impl();

	//! \brief gets vector size
	virtual 
	size_t 
	get_size() const;

	//! sets condition value
	//! \brief sets value as null
	virtual 
	bool 
	set_value_as_null(size_t index,							//!< index
					dbtypes type							//!< db type
					);
	//! \brief sets value as a boolean
	virtual 
	bool 
	set_value_as_bool(size_t index,							//!< index
					bool val								//!< value
					);
	//! \brief sets value as a char
	virtual 
	bool 
	set_value_as_char(size_t index,							//!< index
					sb1_t val								//!< value
					);
	//! \brief sets value as a byte
	virtual 
	bool 
	set_value_as_byte(size_t index, 						//!< index
					ub1_t val								//!< value
					);
	//! \brief sets value as a short
	virtual 
	bool 
	set_value_as_short(size_t index, 						//!< index
					sb2_t val								//!< value
					);
	//! \brief sets value as an unsigned short
	virtual 
	bool 
	set_value_as_word(size_t index, 						//!< index
					ub2_t val								//!< value
					);
	//! \brief sets value as a long
	virtual 
	bool 
	set_value_as_long(size_t index, 						//!< index
					sb4_t val								//!< value
					);
	//! \brief sets value as an unsigned long
	virtual 
	bool 
	set_value_as_dword(size_t index, 						//!< index
					ub4_t val								//!< value
					);
	//! \brief sets value as a float
	virtual 
	bool 
	set_value_as_float(size_t index, 						//!< index
					float val								//!< value
					);
	//! \brief sets value as a double
	virtual 
	bool 
	set_value_as_double(size_t index, 						//!< index
					double val								//!< value
					);
	//! \brief sets value as a signed int64
	virtual 
	bool 
	set_value_as_long64(size_t index, 						//!< index
					sb8_t val								//!< value
					);
	//! \brief sets value as an unsigned int64
	virtual 
	bool 
	set_value_as_dword64(size_t index, 						//!< index
					ub8_t val								//!< value
					);
	//! \brief sets value as a guid
	virtual 
	bool 
	set_value_as_guid(size_t index, 						//!< index
					const guid_t& val						//!< value
					);

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
					);
	//! \brief sets value as a decimal
	virtual 
	bool 
	set_value_as_decimal(size_t index,  					//!< index
					const char* val, 						//!< value
					char delimeter							//!< delimeter
					);
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
					);

	//! \brief sets value as a string
	//! for 0 terminated string len can be -1
	virtual 
	bool 
	set_value_as_string(size_t index,						//!< index
					const char* val,						//!< string pointer
					size_t len								//!< string length
					);
	
	//! \brief sets value as a string pointer
	//! to avoid copying already allocated memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statement has been executed.
	//! it is supposed to work only for input parameters, not for output or input/output ones
	virtual 
	bool 
	set_value_as_string_ptr(size_t index,					//!< index
					const char* val							//!< string pointer
					);
	
	//! \brief sets value as a wide string
	//! for 0 terminated string len can be -1
	virtual 
	bool 
	set_value_as_wstring(size_t index,						//!< index
					const wchar_t* val,						//!< string pointer
					size_t len								//!< string length
					);
	//! \brief sets value as a wide string pointer
	//! to avoid copying already allocated memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statement has been executed.
	virtual 
	bool 
	set_value_as_wstring_ptr(size_t index,					//!< index
					const wchar_t* val						//!< string pointer
					);
	//! \brief sets value as a binary
	//! val points to the valid buffer of raw byte of "len" length
	virtual 
	bool 
	set_value_as_binary(size_t index,						//!< index
					const ub1_t* val,						//!< value pointer
					size_t len								//!< length of bytes
					);
	//! \brief sets value as a binary pointer
	//! to avoid copying already allocated memory, only the pointer can be used
	//! be careful! the caller is responsible for the life time of the provided pointer, 
	//! which has to be valid until the sql statement has been executed.
	//! the format of the buffer must be as follows
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
	//! it is supposed to work only for input parameters, not for output or input/output ones
	virtual 
	bool 
	set_value_as_binary_ptr(size_t index,					//!< index
					const ub1_t* val						//!< value pointer
					);


	//! gets values
	//! \brief checks if the value is null
	virtual 
	bool 
	get_value_is_null(size_t index							//!< index
					) const;

	//! \brief gets the value as a boolean
	virtual 
	bool 
	get_value_as_bool(size_t index							//!< index
					) const;
	//! \brief gets the value as a char
	virtual 
	sb1_t 
	get_value_as_char(size_t index							//!< index
					) const;
	//! \brief gets the value as a byte
	virtual 
	ub1_t 
	get_value_as_byte(size_t index							//!< index
					) const;
	//! \brief gets the value as a short
	virtual 
	sb2_t 
	get_value_as_short(size_t index							//!< index
					) const;
	//! \brief gets the value as an unsigned short
	virtual 
	ub2_t 
	get_value_as_word(size_t index							//!< index
					) const;
	//! \brief gets the value as a long
	virtual 
	sb4_t 
	get_value_as_long(size_t index							//!< index
					) const;
	//! \brief gets the value as an unsigned long
	virtual 
	ub4_t 
	get_value_as_dword(size_t index							//!< index
					) const;
	//! \brief gets the value as a float
	virtual 
	float 
	get_value_as_float(size_t index							//!< index
					) const;
	//! \brief gets the value as a double
	virtual 
	double 
	get_value_as_double(size_t index						//!< index
					) const;
	//! \brief gets the value as a int32
	virtual 
	sb8_t 
	get_value_as_long64(size_t index						//!< index
					) const;
	//! \brief gets the value as an unsigned int32
	virtual 
	ub8_t 
	get_value_as_dword64(size_t index						//!< index
					) const;
	//! \brief gets the value as a guid
	//! for higher performance we are using reference
	virtual 
	bool 
	get_value_as_guid(size_t index, 						//!< index
					guid_t& val								//!< [out] value
					) const;
	// delimeter defines the desirable delimeter sign
	//! \brief gets the value as a numeric
	virtual 
	const char* 
	get_value_as_numeric(size_t index, 						//!< index
					char delimeter							//!< delimeter
					) const;
	// delimeter defines the desirable delimeter sign
	//! \brief gets the value as a deciaml
	virtual 
	const char* 
	get_value_as_decimal(size_t index, 						//!< index
					char delimeter							//!< delimeter
					) const;
	// to avoid confusions with different date formats, functions return parsed date
	//! \brief gets the value as a date
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
					) const;

	//! \brief gets the value as a string
	//! function returns the pointer to string 
	virtual 
	const char* 
	get_value_as_string(size_t index						//!< index
					) const;
	//! \brief gets the value as a wide string
	//! function returns the pointer to wide string 
	virtual 
	const wchar_t* 
	get_value_as_wstring(size_t index						//!< index
					) const;

	//! \brief gets the value as a binary
	//! returns pointer to the raw bytes
	//! size of bytes is put to len
	virtual 
	const ub1_t* 
	get_value_as_binary(size_t index, 						//!< index
					size_t& len								//!< [out] length
					) const;
	//! \brief gets the value as a binary pointer
	//! for reusing only
	//! the format of buffer is as follows
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
 	virtual 
	const ub1_t* 
	get_value_as_binary_ptr(size_t index					//!< index
					) const;

private:
	//! \brief converts one type to the other type
	terimber_xml_value 
	get_value_as_value(size_t index,						//!< index
					vt_types type							//!< output type
					) const;

public:
	mutable byte_allocator	_tmp_allocator;					//!< temporary allocator
	vector< binder >		_value;							//!< vector of binders
};

//! \class memdb_row
//! \brief db row
class memdb_row
{
public:
	_vector< terimber_db_value >		_row;				//!< DB row
	terimber_db_row_status				_status;			//!< row status
};

//! \typedef memdb_rowset_t
//! \brief list of db rows
typedef list< memdb_row >	memdb_rowset_t;
//! \typedef memdb_rowset_iterator_t
//! \brief iterator of memdb_rowset_t
typedef memdb_rowset_t::iterator memdb_rowset_iterator_t;
//! \typedef memdb_rowset_citerator_t
//! \brief const iterator of memdb_rowset_t
typedef memdb_rowset_t::const_iterator memdb_rowset_citerator_t;
//! \typedef terimber_index_column_array_t
//! \brief vector of index cloumn information
typedef vector< terimber_index_column_info_ex > terimber_index_column_array_t;


//! \brief compare two db values with the same type
static 
inline 
int 
compare_db_value(	dbtypes type,							//!< db type
					const terimber_db_value& first,			//!< first value
					const terimber_db_value second,			//!< second value
					bool case_insensitive					//!< case insesitive flag
					);

//! \class memdb_rowset_less
//! \brief predicate for row comparision
class memdb_rowset_less
{
public:
	//! \brief constructor
	memdb_rowset_less(const terimber_index_column_array_t& info //!< index columns array
					) : 
		_info(info) 
	{
	}
	//! \brief operator ()
	inline 
	bool 
	operator()(const memdb_rowset_citerator_t& first, const memdb_rowset_citerator_t& second) const;
	//! \brief returns index column info
	inline 
	const terimber_index_column_array_t& 
	get_info() const 
	{ 
		return _info; 
	}
private:
	terimber_index_column_array_t	_info;					//!< index column information
};

//! \typedef memdb_index_t
//! \brief maps rowset iterator to the correct position
typedef map< memdb_rowset_citerator_t, memdb_rowset_citerator_t, memdb_rowset_less, true > memdb_index_t;
//! \typedef memdb_index_citer_t
//! \brief const iterator of memdb_index_t
typedef memdb_index_t::const_iterator memdb_index_citer_t; 

// forward declaration
class memindex;
class memlookup;

//! \class memtable 
//! \brief interface to memory table
class memtable : public terimber_memtable
{
	//! \typedef list_indexes_t
	//! \brief list of memindex pointers
	typedef list< memindex* > list_indexes_t;
	//! \typedef list_values_t
	//! \brief list of value vector pointers
	typedef list< terimber_db_value_vector_impl* > list_values_t;
public:
	//! \brief constructor
	memtable();
	//! \brief destructor
	//! user is responsible for destoying memtable object
	virtual 
	~memtable();

	//! \brief returns the last occured error
	virtual 
	const char* 
	get_last_error();

	//! \brief create internal table structure from server object
	//! db_server has to be after executing query or stored procedure
	//! and ready to fetch data
	//! memtable automatically detects the list of columns in recordset
	//! and creates table in memory with the same columns' order and types.
	//! max_rows sets a restriction on max rows to be retrived, -1 - all of them
	//! start_row means absolute number of row to start with
	//! unless you specify 0, the memtable starts with the current row
	//! to create an empty table, the fake select can be used
	//! for instanse, 
	//! select CAST(0 as NUMBER) as col1, CAST('' as VARCHAR2(255)) as col2 from dual;
	//! or you can specifiy the max_rows = 0
	virtual 
	bool 
	populate(		dbserver* server,						//!< dbserver instance, connected to database and opened the recordset
					size_t start_row,						//!< row to fetch from, 0 - current row
					size_t max_rows							//!< max row to fetch
					);

	//! \brief the other alternative for creating an empty table in the memory is column specification 
	//! the list of columns descriptions explicitly
	virtual 
	bool 
	create(			size_t columns,							//!< number of columns
					const terimber_table_column_desc desc[]	//!< array of columns' descriptors
					);


	//! \brief resets all new, update rows status to original, removes deleted rows
	//! update all indexes
	virtual 
	void 
	refresh();
	//! \brief after population and any time after that you can request the number of rows in table
	virtual 
	size_t
	get_row_count();

	//! columns description is pretty much the same as db_server provides

	//! gets column count
	virtual 
	size_t 
	get_column_count() const;
	//! \brief returns the virtual terimber dbtype like db_bool, ...
	virtual 
	dbtypes 
	get_column_type(size_t index							//!< index
					) const;
	//! \brief returns the name of the column
	virtual 
	const char* 
	get_column_name(size_t index							//!< index
					) const;
	//! \brief returns the nullable - database specific
 	virtual 
	bool 
	get_column_nullable(size_t index						//!< index
					) const;
	//! \brief returns the scale
	virtual 
	size_t 
	get_column_scale(size_t index						//!< index
					) const;
	//! \brief returns the precision
	virtual 
	size_t 
	get_column_precision(size_t index					//!< index
					) const;
	//! \brief returns the length of the column (only for fixed length columns)
	virtual 
	size_t 
	get_column_max_length(size_t index					//!< index
					) const;

	//! index management
	//! there are no connection between the indexes in the memory and the indexes in the database

	//! \brief creates index to perform the quick search
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
					);

	//! \brief destroy index
	//! memtable data is still valid
	//! all memlookups on the current index will be destroyed! 
	virtual 
	bool 
	remove_index(	terimber_memindex* obj					//!< index pointer
					);

		
	//! \brief requests value array
	virtual 
	terimber_db_value_vector* 
	allocate_db_values(size_t count							//!< array size
					);

	//! \brief destroys value array
	virtual 
	void 
	destroy_db_values(terimber_db_value_vector* obj			//!< value array pointer
					);

public:
	//! \brief returns rowset
	inline 
	const memdb_rowset_t& 
	get_rowset() const 
	{ 
		return _rowset; 
	}
	//! \brief returns column binders
	inline 
	const binders_t& 
	get_cols() const 
	{ 
		return _cols; 
	}
	//! \brief inserts new row
	//! if row is modified it will still have a status = new
	//! if row is removed it's just deleted from recordset
	//! inserted row has all columns' value as null
	bool 
	insert_row(		const terimber_db_value_vector* info	//!< value vector
					);
	//! \brief updates row
	bool 
	update_row(		memdb_rowset_citerator_t iter,			//!< row iterator for update
					const terimber_db_value_vector* info	//!< value vector
					);
	//! \brief deletes existing row
	//! if row was in original recordset (modified or not), it will have a status = delete, and will not be accessible anymore
	//! deleting of new row will just remove row from recordset
	bool 
	delete_row(		memdb_rowset_citerator_t iter			//!< row iterator for deletion
					);

private:
	//! \brief uninit
	void 
	uninit();
private:
	byte_allocator		_allocator;							//!< internal allocator
	binders_t			_cols;								//!< columns binders
	memdb_rowset_t		_rowset;							//!< rowset
	string_t			_error;								//!< last error
	mutex				_mtx;								//!< mutex 
	list_indexes_t		_indexes;							//!< list of indexes
	list_values_t		_values;							//!< list of value vectors
};

//! \class memindex
//! \brief implementation of terimber_memindex interface
class memindex :	public terimber_memindex, 
					public terimber_log_helper
{
	//! memtable
	friend class memtable;
	//! \typedef list_lookups_t
	//! \brief list of lookups
	typedef list< memlookup* > list_lookups_t;
protected:
	//! \brief constructor
	memindex(		memtable& parent,						//!< parent memtable
					const memdb_rowset_less& pred			//!< predicate for row copmparision
					);
	//! \brief destructor
	//! users can't call destructor, memtable will take case about cleanup
	virtual 
	~memindex();
public:

	//! \brief for navigation the user has to create index lookup first
	//! the list of values should be the same length & type as the index columns or less
	//! for example, let us assume that we got the index with two columns (id, integer) & (name, string)
	//! if the user provides values for id & name then the lookup will search for rows with specified id & name
	//! however, if only id is specified, lookup will ignore the second column values.
	//! it's impossible to set search lookup on the second column without setting the first column
	//! to do such a thing the user needs to create index with a different order of index columns, for instance 
	//! name and id, and set search criteria for the name column.
	//! user is allowed to ignore search criteria completely,
	//! then lookup will move row by row in the order they came from the database, including the row with status = deleted
	virtual 
	terimber_memlookup* 
	add_lookup(		const terimber_db_value_vector* info	//!< value vector
					);

	//! \brief destroys lookup after using
	virtual 
	bool 
	remove_lookup(	terimber_memlookup* obj					//!< pointer to the lookup object
					);


public:
	//! \brief returns const memtable
	inline 
	const memtable& 
	get_table() const 
	{ 
		return _parent; 
	}
	//! \brief returns memtable
	inline 
	memtable& 
	get_table() 
	{ 
		return _parent; 
	}
	//! \brief returns index table
	inline 
	const memdb_index_t& 
	get_index() 
	{ 
		return _index; 
	}

private:
	//! \brief constructs index
	bool
	construct();
	//! \brief notifies all lookup about changes
	void 
	notify(			memdb_rowset_citerator_t iter,			//!< affected row iterator
					bool insert_or_delete					//!< deleted or not 
					);

private:
	memtable&		_parent;								//!< parent memtable
	memdb_index_t	_index;									//!< index table
	mutex			_mtx;									//!< mutex
	list_lookups_t	_lookups;								//!< list of lookups
};

//! \class memlookup
//! \brief implementation of terimber_memlookup interface
class memlookup :	public terimber_memlookup, 
					public terimber_log_helper
{
	//! memindex
	friend class memindex;
protected:
	//! \brief constructor
	memlookup(		memindex& parent						//!< parent index table
					);
	//! \brief destructor
	virtual 
	~memlookup();
	//! \brief constructs lookup
	bool 
	construct(		const terimber_db_value_vector_impl* info //!< value vector
					);
public:
	//! \brief copy method
	//! lookups must be created for the same index
	virtual 
	bool 
	assign(			const terimber_memlookup& x				//!< input lookup
					);

	//! navigation methods

	//! \brief sets lookup outside the row sequence
	//! and initialize the selection criteria
	virtual 
	bool 
	reset(			const terimber_db_value_vector* info	//!< value vector
					);

	//! \brief tries to find the next row
	//! according to lookup values
	virtual 
	bool 
	next() const;

	//! \brief tries to find the previous row
	//! according to lookup values
	virtual 
	bool 
	prev() const;

	//! access to the row
	//! this is pretty much the same as db_server provides.
	//! returns the value of the column at the current row position
    //! if the index is out of range then zero will be returnes
	//! if the type is mismatched then an attempt to convert will be made
	//! the convertion of internal values is made on the temporary allocator
	//! the returned pointer is valid until the next call
	//! the best practice to check type before call get value functions

	//! \brief checks if value is null
	virtual 
	bool 
	get_value_is_null(size_t index							//!< index
					) const;
	//! \brief gets the value as a boolean
	virtual 
	bool 
	get_value_as_bool(size_t index							//!< index
					) const;
	//! \brief gets the value as a char	
	virtual 
	sb1_t 
	get_value_as_char(size_t index							//!< index
					) const;
	//! \brief gets the value as a byte
	virtual 
	ub1_t 
	get_value_as_byte(size_t index							//!< index
					) const;
	//! \brief gets the value as a short	
	virtual 
	sb2_t 
	get_value_as_short(size_t index							//!< index
					) const;
	//! \brief gets the value as a unsigned short
	virtual 
	ub2_t 
	get_value_as_word(size_t index							//!< index
					) const;
	//! \brief gets the value as a long
	virtual 
	sb4_t 
	get_value_as_long(size_t index							//!< index
					) const;
	//! \brief gets the value as a unsigned long
	virtual 
	ub4_t 
	get_value_as_dword(size_t index							//!< index
					) const;
	//! \brief gets the value as a float
	virtual 
	float 
	get_value_as_float(size_t index							//!< index
					) const;
	//! \brief gets the value as a double
	virtual 
	double 
	get_value_as_double(size_t index						//!< index
					) const;
	//! \brief gets the value as a int64
	virtual 
	sb8_t 
	get_value_as_long64(size_t index						//!< index
					) const;
	//! \brief gets the value as a unsigned int64
	virtual 
	ub8_t 
	get_value_as_dword64(size_t index						//!< index
					) const;

	//! \brief gets the value as a guid
	//! for high performance we are using reference
	virtual 
	bool 
	get_value_as_guid(size_t index, 						//!< index
					guid_t& val
					) const;
	//! \brief gets the value as a numeric
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_value_as_numeric(size_t index,  					//!< index
					char delimeter
					) const;
	//! \brief gets the value as a decimal
	//! delimeter defines the desirable delimeter sign
	virtual 
	const char* 
	get_value_as_decimal(size_t index,  					//!< index
					char delimeter
					) const;
	//! \brief gets the value as a date
	//! to avoid confusions with different date formats, functions return parsed date
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
					) const;
	//! \brief gets the value as a string
	//! function returns the pointer to string 
	virtual 
	const char* 
	get_value_as_string(size_t index  						//!< index
					) const;
	//! \brief gets the value as a wide string
	//! function returns the pointer to wide string 
	virtual 
	const wchar_t* 
	get_value_as_wstring(size_t index 						//!< index
					) const;

	//! \brief gets the value as a binary
	//! returns pointer to the raw bytes
	//! size of bytes is put to len
	virtual 
	const ub1_t* 
	get_value_as_binary(size_t index,  						//!< index
					size_t& len
					) const;
	//! \brief gets the value as a binary pointer
	//! for reusing only
	//! the format of buffer is as follows
	//! first size_t bytes the length of the rest of buffer,
	//! which contains the raw byte data
 	virtual 
	const ub1_t* 
	get_value_as_binary_ptr(size_t index  					//!< index
					) const;


	//! row modificators
	//! the current row can be changed
	//! there are possible scenarios
	//! 1. inserts the new row - it will become the current one
	//! 2. updates the current row - it will be the same current row, only index columns are affected
	//! 3. deletes the current row - the current row will be next to the current one or current will point to the out of row sequence

	//! \brief inserts new row
	//! if the row is modified it will still have a status = new
	//! if the row is removed it's just deleted from recordset
	//! inserted row has all columns' value as null
	virtual 
	bool 
	insert_row(	const terimber_db_value_vector* info		//!< value vector
				);


	//! \brief delete existing row
	//! if row was in original recordset (modified or not), it will have a status = delete, and will not be accessible anymore
	//! deleting of new row will just remove row from recordset
	virtual 
	bool 
	delete_row();

	//! \brief updates the column value
	//! update of new row will preserve status = new
	//! update of original row will change status from original to update.
	virtual 
	bool 
	update_row(	const terimber_db_value_vector* info		//!< value vector
				);

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
	get_row_status();

private:
	//! \brief make changes according to the action 
	void 
	notify(			memdb_index_citer_t iter,				//!< affected row iterator
					bool insert_or_delete					//!< remove row or not
					);
	//! \brief converts value to the different type
	terimber_xml_value 
	get_value_as_value(size_t index,						//!< index
					vt_types type							//!< output type
					) const;

private:
	mutable byte_allocator			_tmp_allocator;			//!< temporary allocator
	memindex&						_parent;				//!< parent memory index
	memdb_index_citer_t				_low_bounder;			//!< lower bound iterator
	memdb_index_citer_t				_upper_bounder;			//!< upper bound iterator
	mutable memdb_index_citer_t		_current_iter;			//!< current iterator
	byte_allocator					_condition_allocator;	//!< consition allocator
	memdb_rowset_t					_condition_rowset;		//!< condition rowset
};


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_memdb_h_

