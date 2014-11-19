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

#ifndef _terimber_varmap_h_
#define _terimber_varmap_h_

#include "smart/varvalue.h"
#include "smart/varfactory.h"
#include "base/map.h"
#include "base/list.h"
#include "base/string.h"
#include "smart/byterep.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class main_map_key
//! \brief template map class has to expose the interface 
//! for exact match "less", "greater", and so on exact match comparision
//! and optionaly "like" for partial match comparision
//! class creates the wrappper around var_value for string resources
class main_map_key
{
	//! \brief hidden assign operator
	main_map_key& operator=(const main_map_key& x);

public:
	//! \brief copy constructor
	main_map_key(const main_map_key& x);

	//! \brief constuctor with memory reallocation
	//! ordinary value constructor
	//! factory can be null for temporary objects
	main_map_key(	vt_types type,							//!< variant type
					const var_value& val,					//!< input variant value
					variant_factory* factory				//!< optional pointer to variant factory
					);

	//! \brief constuctor with memory reallocation
	//! for partial match only
	main_map_key(	const char* val,						//!< string pointer
					variant_factory* factory				//!< memory factory
					);

	//! \brief constructor with memory reallocation
	//! for fuzzy match only
	main_map_key(	size_t fuzzyident,						//!< fuzzy ident
					const char* val,						//!< string pointer
					variant_factory* factory				//!< memory factory
					);


	//! \brief less operator
	inline 
	bool 
	operator<(const main_map_key& x) const;
	//! \brief compare only x.length() chars
	inline 
	bool 
	partial_match(const main_map_key& x) const;

public:
	var_key					_var_res;					// united resource
};

//! \class main_map_key_compare
//! \brief predicate for main key comparision
class main_map_key_compare
{
public:
	//! \brief constructor
	main_map_key_compare(vt_types type,						//!< variant type
					bool partial,							//!< flag partial match
					bool fuzzy								//!< flag fuzzy match
					) : 
		_type(type), 
		_partial(partial), 
		_fuzzy(fuzzy) 
	{
	}
	//! \brief compare operator
	inline 
	bool 
	operator()(const main_map_key& x, const main_map_key& y) const
	{
		if (_partial)
			return x < y; // compare as strings
		else if (_fuzzy)
			return x._var_res._ngram._fuzzy_key < y._var_res._ngram._fuzzy_key;
		else
			return operator_less(_type, x._var_res._val, y._var_res._val); // compare as variant
	}
public:
	vt_types	_type;										//!< variant type
	bool		_partial;									//!< flag partial match
	bool		_fuzzy;										//!< flag fuzzy match
};

//! \class varmap
//! \brief template class with main map (searchable property -->map of unique objects)
template <	class T											//!< unique primary key, must be supported by caller
			, class C										//!< high-performance sorted container must expose public properties such as
															//!< _container - terimber _map, _allocator - allocator for _container member
		>
class varmap
{
	//! \typedef mainmap_object_allocator_t
	//! \brief allocator for map
	typedef node_allocator< TYPENAME base_map< T, bool >::_node >		mainmap_object_allocator_t;
	//! \typedef mainmap_object_t
	//! \brief main map list of objects
	typedef _map< T, bool, mainmap_object_allocator_t >					mainmap_object_t;

public:
	//! mandatory defined types
	//! \typedef mainmap_t
	//! \brief main map itself
	typedef map< main_map_key, mainmap_object_t, main_map_key_compare >	mainmap_t;
	//! \typedef mainmap_citer_t
	//! \brief const iterator for main map
	typedef TYPENAME mainmap_t::const_iterator							mainmap_citer_t;
	//! \typedef mainmap_iter_t
	//! \brief iterator for main map
    typedef TYPENAME mainmap_t::iterator								mainmap_iter_t;
	//! \typedef mainmap_pairib_t
	//! \brief pair < iterator, bool > for main map
    typedef TYPENAME mainmap_t::pairib_t								mainmap_pairib_t;

public:
	//! \brief constructor
	varmap(			vt_types type,										//!< variant type
					bool partial,										//!< flag partial match
					bool fuzzy,											//!< flag fuzzy match
					variant_factory& factory							//!< memory factory
					);
	//! \brief destructor
	~varmap();

	//! \brief finds/adds resource
	mainmap_iter_t 
	add_resource(	const main_map_key& res,							//!< input key
					const T& x											//!< input value
					); 
	//! \brief returns end iterator
	mainmap_iter_t 
	end();
	
	//! \brief finds the exact resource
	//! adding to those that is already in the container if any
	bool 
	find_exact_resource(const main_map_key& res,						//!< input key
					C& x,												//!< output container
					const T& min_filter,								//!< min boundary
					const T& max_filter									//!< max boundary
					) const;

	//! \brief finds the range of resources
	//! "<" , "<="
	bool 
	find_less_resource(const main_map_key& res,							//!< input key
					C& x,												//!< output container
					const T& min_filter,								//!< min boundary
					const T& max_filter,								//!< max boundary
					bool boundary_include								//!< flag include bounderies
					) const;
	//! \brief finds the range of resources
	//! ">", ">='
	bool 
	find_greater_resource(const main_map_key& res,						//!< input key
					C& x,												//!< output container
					const T& min_filter,								//!< min boundary
					const T& max_filter,								//!< max boundary
					bool boundary_include								//!< flag include bounderies
					) const;

	//! \brief finding the exact resource and keep in the container 
	//! only those that intersect with the new found ones
	bool 
	intersect_exact_resource(const main_map_key& res,					//!< input key
					C& x,												//!< output container
					const T& min_filter,								//!< min boundary
					const T& max_filter									//!< max boundary
					) const;
	
	//! \brief intersects the range of resources
	//! "<" , "<="
	bool 
	intersect_less_resource(const main_map_key& res,					//!< input key
					C& x,												//!< output container
					const T& min_filter,								//!< min boundary
					const T& max_filter,								//!< max boundary
					bool boundary_include								//!< flag include bounderies
					) const;
	//! \brief intersects the range of resources
	//! ">", ">='
	bool 
	intersect_greater_resource(const main_map_key& res,					//!< input key
					C& x,												//!< output container
					const T& min_filter,								//!< min boundary
					const T& max_filter,								//!< max boundary
					bool boundary_include								//!< flag include bounderies
					) const;

	//! \brief removes the resource from the container by iter(key)/value pair
	bool 
	remove_resource(mainmap_iter_t iter,								//!< interator to the main map
					const T& x											//!< resource for removal
					);

	//! \brief removes specific resource from all keys 
	//! returns the number removed items
	size_t 
	remove_all_resource(const T& x										//!< resource for removal
					);

	//! \brief extracts all resources from varmap
	void 
	fetch_all(		C& x												//!< output container
					) const;

	//! \brief removes all resources from container
	void 
	clear();

	//! \brief finds partial match
	bool 
	find_partial_resource(const main_map_key& res,						//!< input key
					bool deep,											//!< flag match partially deeply
					C& x,												//!< output container
					const T& min_filter,								//!< min boundary
					const T& max_filter									//!< max boundary
					) const;
	//! \brief intersects partial match
	bool 
	intersect_partial_resource(const main_map_key& res,					//!< input key
					bool deep,											//!< flag match partially deeply
					C& x,												//!< output container
					const T& min_filter,								//!< min boundary
					const T& max_filter									//!< max boundary
					) const;
	//! \brief finds fuzzy match
	bool 
	find_fuzzy_resource(const _list< size_t >& fuzzy_container,			 //!< input ident container
					C& x,												 //!< output resource container
					const T& min_filter,								//!< min boundary
					const T& max_filter									//!< max boundary
					) const;
	//! \brief intersects fuzzy match
	bool 
	intersect_fuzzy_resource(const _list< size_t >& fuzzy_container,	//!< input ident container
					C& x,												//!< output resource container
					const T& min_filter,								//!< min boundary
					const T& max_filter									//!< max boundary
					) const;
	//! \brief finds common items
	static 
	void 
	get_common_items(mainmap_citer_t c,									//!< input iterator to main map
					const C& r,											//!< input resource container
					const T& min_filter,								//!< min boundary
					const T& max_filter,								//!< max boundary
					size_t& entries										//!< [out] entries
					);
	//! \brief adds unique items to the container
	static 
	bool 
	add_unique_items(mainmap_citer_t c,									//!< input iterator to main map
					C& r,												//!< resource container
					const T& min_filter,								//!< min boundary
					const T& max_filter									//!< max boundary
					);
	//! \brief removes uncommon items from container
	static 
	void 
	remove_uncommon_items(mainmap_citer_t c,							//!< input iterator to main map
					C& r,												//!< resource container
					const T& min_filter,								//!< min boundary
					const T& max_filter									//!< max boundary
					);
	//! \brief marks common items
	static 
	void 
	mark_common_items(mainmap_citer_t c,								//!< input iterator to main map
					C& r,												//!< resource container
					const T& min_filter,								//!< min boundary
					const T& max_filter									//!< max boundary
					);

	//! any statistics
	//! \brief returns allocated pages
	size_t 
	get_main_object_allocator_pages() const;
	//! \brief returns allocators capacity
	size_t 
	get_main_object_allocator_capacity() const;
	//! \brief rertives statistic
	template < class F >
	void 
	retrive_stats(	F& r									//!< fuction pointer
					)
	{
		for (mainmap_citer_t miter = _mainmap.begin(); miter != _mainmap.end(); ++miter)
		{
			r(miter);
		}
	}

protected:
	vt_types					_type;						//!< variant type
	main_map_key_compare		_compare;					//!< compare predicate
	mainmap_t					_mainmap;					//!< main map
	mainmap_object_allocator_t	_main_object_allocator;		//!< main map allocator
	variant_factory&			_factory;					//!< refrence to memory factory
	bool						_partial;					//!< flag partial match
	bool						_fuzzy;						//!< flag fuzzy match

private:
	//! \class offset_map_key
	//! \brief offset map key
	class offset_map_key
	{
		//! \brief hidden assign operator
		offset_map_key& operator=(const offset_map_key& x);
	public:
		//! \brief constructor - for insertion and search
		offset_map_key(mainmap_iter_t iter,					//!< iterator to main map
					size_t offset							//!< offset
					) :
			_iter(iter), 
			_offset(offset), 
			_pointer(0) 
		{
		}
		//! \brief constructor for partial comparision only
		offset_map_key(const char* x,						//!< string pointer
					size_t offset							//!< offset
					):
			_pointer(x), _offset(offset)
		{
		}
		//! \brief copy constructor
		offset_map_key(const offset_map_key& x): 
			_iter(x._iter), _offset(x._offset), _pointer(x._pointer) 
		{
		}

		//! \brief less operator
		inline 
		bool 
		operator<(const offset_map_key& x) const
		{ 
			if (x._pointer)
				return _offset != x._offset ? _offset < x._offset :
					str_template::strnocasecmp(_iter.key()._var_res._key._res + _iter.key()._var_res._key._offsets[1 + _offset], x._pointer, os_minus_one) < 0; 
			else if (_pointer)
				return _offset != x._offset ? _offset < x._offset :
						str_template::strnocasecmp(_pointer, x._iter.key()._var_res._key._res + x._iter.key()._var_res._key._offsets[1 + x._offset], os_minus_one) < 0; 
			else
				return _offset != x._offset ? _offset < x._offset :
						str_template::strnocasecmp(_iter.key()._var_res._key._res + _iter.key()._var_res._key._offsets[1 + _offset],
							x._iter.key()._var_res._key._res + x._iter.key()._var_res._key._offsets[1 + x._offset], os_minus_one) < 0; 
		}
		//! \brief specific partial match function
		inline 
		bool 
		partial_match(const main_map_key& x) const
		{
			return !str_template::strnocasecmp((const char*)_iter.key()._var_res._key._res + _iter.key()._var_res._key._offsets[1 + _offset], 
								x._var_res._key._res ? x._var_res._key._res : "", 
								x._var_res._key._res ? strlen(x._var_res._key._res) : 0);
		}

	public:
		
		size_t					_offset;					//!< offset to the start token
		mainmap_iter_t			_iter;						//!< iter to main map entry
		const char*				_pointer;					//!< pointer for the partial match only
	};

	//! \typedef offsetmap_t
	//! \brief offset map
	typedef map< offset_map_key, bool, less< offset_map_key >, true >	offsetmap_t;
	//! \typedef offsetmap_citer_t
	//! \brief const iterator for offsetmap_t
	typedef typename offsetmap_t::const_iterator						offsetmap_citer_t;
	//! \typedef offsetmap_iter_t
	//! \brief iterator for offsetmap_t
	typedef typename offsetmap_t::iterator								offsetmap_iter_t;
private:
	offsetmap_t					_offsetmap;					//!< offset map
};


#pragma pack()
END_TERIMBER_NAMESPACE

#endif
