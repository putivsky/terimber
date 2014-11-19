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

#ifndef _terimber_varobj_h_
#define _terimber_varobj_h_

#include "smart/varvalue.h"
#include "base/vector.h"
#include "base/list.h"
#include "base/stack.h"
#include "base/map.h"
#include "base/memory.h"
#include "smart/byterep.h"
#include "smart/varmap.h"
#include "xml/xmlaccss.h"
#include "fuzzy/fuzzyaccess.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class var_property_schema
//! \brief class describes the object property schema
class var_property_schema
{
public:
	vt_types	_type;										//!< type of field
	const char*	_name;										//!< name of field
	bool		_is_searchable;								//!< is searchable - applies to string types only
	bool		_is_fuzzy_match;							//!< is fuzzy match - applies to string types only
};

//! \typedef list_returns_index_t
//! \brief list of idents
typedef _list< size_t > list_returns_index_t;

//! \class pk_column
//! \brief primary key and column index info
class pk_column
{
public:
	//! \brief constructor
	pk_column(		size_t pk,								//!< primary key
					size_t col								//!< column index
					) : 
		_pk(pk), 
		_col(col)
	{
	}

	//! \brief compare operator
	inline 
	bool 
	operator<(const pk_column& x) const
	{
		//return _pk != x._pk ? _pk < x._pk : _col < x._col;
		return _col != x._col ? _col < x._col : _pk < x._pk;
	}
	//! \brief equal operator
	inline 
	bool 
	operator==(const pk_column& x) const
	{
		return _col == x._col && _pk == x._pk;
	}

	size_t	_pk;											//!< primary key
	size_t	_col;											//!< column index
};

//! \class var_container
//! \brief variant container
class var_container
{
public:
	//! \typedef sorted_container_allocator_t
	//! \brief allocator for map
	//typedef node_allocator< _map< size_t, bool >::_node >			sorted_container_allocator_t;
	typedef  byte_allocator											sorted_container_allocator_t;
	//! \typedef sorted_container_data_t
	//! \brief sorted container as a map
	typedef _map< size_t, bool, sorted_container_allocator_t >		sorted_container_data_t;


public:
	//! \brief constructor
	var_container(	sorted_container_allocator_t& all		//!< external allocator
					) : 
		_allocator(all) 
	{
	}

	//! \brief intersects containers
	void 
	intersect(		const var_container& x					//!< input container
					);
	//! \brief combines containers
	void 
	combine(		const var_container& x					//!< input container
					);
	//! \brief extracts sub key
	static 
	inline 
	size_t 
	clean_key(		const pk_column& x						//!< input info
					)
	{
		return x._pk;
	}

	//! \brief returns full key
	static 
	inline 
	pk_column 
	compound_key(	size_t x,								//!< input key
					const pk_column& c						//!< column key
					)
	{
		return  pk_column(x, c._col);
	}
	
	
	sorted_container_allocator_t&	_allocator;				//!< external allocator
	sorted_container_data_t			_container;				//!< Terimber map with external allocator
};

//! \typedef var_object_map_t
//! \brief maps pk, col info to variant container
typedef varmap< pk_column, var_container >		var_object_map_t;
//! \typedef var_object_schema
//! \brief vector of schemas
typedef _vector< var_property_schema >			var_object_schema;
//! \typedef var_object_values
//! \brief vector of variant values
typedef _vector< var_value >					var_object_values;

//! \typedef var_unique_key_t
//! \brief maps pk, col info to variant container
typedef _list< pk_column >						var_unique_key_t;

//! \class var_object_repository
//! \brief class where all maps resides
class var_object_repository
{
public:
	//! \typedef iter_vec_allocator_t
	//! \brief allocator for iterator vector
	typedef array_allocator< var_object_map_t::mainmap_iter_t >						iter_vec_allocator_t;
	//! \typedef iter_vector_t
	//! \brief vector of iterators
	typedef _vector< var_object_map_t::mainmap_iter_t, iter_vec_allocator_t >		iter_vector_t;
	//! \typedef object_map_allocator_t
	//! \brief map allocator
	typedef node_allocator< _map< size_t, iter_vector_t >::_node >					object_map_allocator_t;
	//! \typedef object_map_data_t
	//! \brief maps rowid to the vector of iterators
	typedef _map< size_t, iter_vector_t, object_map_allocator_t >					object_map_data_t;

	//! \brief copy constructor
	var_object_repository(const var_object_repository& x);
	//! \brief assign operator
	var_object_repository& operator=(const var_object_repository& x);

public:
	//! \brief constructor
	var_object_repository(const var_object_schema& schema	//!< schema
					);
	//! \brief destructor
	~var_object_repository();

	//! \brief clear and reuse object
	void
	reset();

	//! \brief count rows
	size_t
	count() const;

	//! \brief processes xml query
	bool 
	process_query(	xml_designer* parser,					//!< xml designer
					var_container::sorted_container_allocator_t& all, //!< container allocator
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl						//!< inline allocator
					);


	// the following functions must be used with care

	//! non xml level
	//! \brief find by pk
	bool
	select_object(size_t pk,								//!< primary key
					var_object_values& row,					//!< row values
					byte_allocator& tmp						//!< temporary allocator
					) const;

	//! non xml level
	//! \brief find by unique key
	bool
	select_object(const var_object_values& key,				//!< unique key
					var_object_values& row,					//!< row values
					byte_allocator& tmp,					//!< temporary allocator
					var_container::sorted_container_allocator_t& conall,	//!< container allocator
					size_t& pk								//!< returns internal pk
					) const;

	//! \brief adds new object to the repository
	//! the length of object must excatly match the length of var_object_schema
	//! returns primary key
	size_t 
	insert_object(	const var_object_values& values,		//!< db row
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					);

	//! \brief deletes existed object from the repository
	bool 
	delete_object(	size_t pk,								//!< primary key
					byte_allocator& tmp,					//!< temporary allocator
					string_t& err							//!< [out] error
					);

	//! \brief deletes existed object from the repository
	bool 
	delete_object(	const var_object_values& key,			//!< key
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,						//!< inline allocator
					string_t& err							//!< [out] error
					);

	//! \brief updates existed object from the repository
	bool 
	update_object(	size_t pk, 								//!< primary key
					const var_object_values& values,		//!< new db row
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					);

	//! \brief find pks by conditions
	bool 
	find_by_conditions(	const xml_designer* parser,			//!< xml designer
					var_container& container,				//!< [out] container of values
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					) const;

	//! \brief find pks by conditions
	bool 
	find_by_condition(	const xml_designer* parser,			//!< xml designer
					var_container& container,				//!< [out] container of values
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					) const;

	//! \brief find pks by foreign key
	bool 
	find_by_row(	const var_object_values& values,		//!< row with conditions
					var_container& container,				//!< [out] container of values
					byte_allocator& tmp						//!< temporary allocator
					) const;

private:
	//! \brief converts value to common type
	vt_types 
	cast_to_common_type(vt_types type,						//!< variant type
					const var_value& in,					//!< input value
					var_value& out,							//!< output value
					byte_allocator& tmp						//!< temporary allocator
					) const;
	//! \brief map types
	vt_types 
	map_type(		vt_types type							//!< variant type
					) const;

	//! \brief processes selected columns
	bool 
	process_returns(const xml_designer* parser,				//!< xml designer
					list_returns_index_t& container,		//!< [out] list of return columns
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					) const;
	//! \brief processes VALUES clause
	bool 
	process_values(	const xml_designer* parser,				//!< xml designer
					var_object_values& container,			//!< [out] container of values
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					) const;
	//! \brief processes WHERE clause
	bool 
	process_where(	const xml_designer* parser,				//!< xml designer
					var_container& container,				//!< [out] container of values
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					) const;
	//! \brief processes conditions
	bool 
	process_condition(const xml_designer* parser,			//!< xml designer
					var_container& container,				//!< [out] container of conditions
					bool intersect,							//!< flag intersect
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					) const;
	//! \brief processes group conditions recursively
	bool 
	process_group(	const xml_designer* parser,				//!< xml designer
					var_container& container,				//!< [out] container of conditions
					bool intersect,							//!< flag intersect
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err
					) const;

	//! \brief processes error
	bool 
	process_error(	xml_designer* parser,					//!< xml designer
					const char* err							//!< error
					) const;

	//! \brief returns the object by type const verstion
	const var_object_map_t* 
	get_v_object_pointer(vt_types type,						//!< variant type
					bool searchable							//!< flag searchable
					) const;

	//! \brief returns the object by type
	var_object_map_t* 
	get_v_object_pointer(vt_types type,						//!< variant type
					bool searchable							//!< flag searchable
					);

private:	
	const var_object_schema&	_schema;					//!< schema for this object
	unique_key_generator		_key_generator;				//!< unique key generator
	variant_factory				_var_factory;				//!< variant factory
	var_object_map_t			_uint32_type_map;			//!< bool, byte, short, long - all goes to the int_t map
	var_object_map_t			_double_type_map;			//!< float, double map
	var_object_map_t			_uint64_type_map;			//!< long long and date type
	var_object_map_t			_string_type_map;			//!< string, wstring (will be converted into utf-8)
	var_object_map_t			_searchable_string_type_map;//!< partial searchable, string, wstring (will be converted into utf-8)
	var_object_map_t			_fuzzy_string_type_map;		//!< fuzzy match searchable, string, wstring (will be converted into utf-8)
	var_object_map_t			_numeric_type_map;			//!< decimal, numeric
	var_object_map_t			_binary_type_map;			//!< binary
	var_object_map_t			_guid_type_map;				//!< guid
	object_map_allocator_t		_objs_all;					//!< size_t is a unique rowid pointing to the iterator for the correspondent map
	iter_vec_allocator_t		_iter_vec_all;				//!< allocator for iterators
	object_map_data_t			_objs_map;					//!< actual map to the iterators
	fuzzy_matcher*				_fuzzy_engine;				//!< fuzzy match engine
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif
