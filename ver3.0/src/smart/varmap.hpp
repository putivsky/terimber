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

#include "smart/varmap.h"

#ifndef _terimber_varmap_hpp_
#define _terimber_varmap_hpp_

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//////////////////////////////////////////////////////////////////////
inline 
bool 
main_map_key::operator<(const main_map_key& x) const
{ 
	return str_template::strnocasecmp(_var_res._key._res ? _var_res._key._res : "", x._var_res._key._res ? x._var_res._key._res : "", os_minus_one) < 0; 
}

inline 
bool 
main_map_key::partial_match(const main_map_key& x) const
{
	return !str_template::strnocasecmp(_var_res._key._res ? _var_res._key._res : "", 
					x._var_res._key._res ? x._var_res._key._res : "", 
					x._var_res._key._res ? strlen(x._var_res._key._res) : 0);
}

template < class T, class C >
size_t
varmap< T, C >::remove_all_resource(const T& x)
{
	size_t ret = 0;

	for (mainmap_iter_t miter = _mainmap.begin(); miter != _mainmap.end();)
	{
		// removes ident from list
		TYPENAME mainmap_object_t::iterator iter = miter->find(x);
		if (iter != miter->end())
		{
			++ret;
			// returns key to the factory
			miter->erase(_main_object_allocator, iter);
		}

		if (miter->empty()) // there are still entries here
		{
			var_value val;

			if (_partial || _fuzzy)
			{
				val._not_null = true;
				val._value.strVal = miter.key()._var_res._key._res;
				_factory.destroy(_type, val);
				val._value.bufVal = (const ub1_t*)miter.key()._var_res._key._offsets;
				if (val._value.bufVal)
					_factory.destroy(vt_numeric, val);
			}
			else
			{
				val = miter.key()._var_res._val;
				_factory.destroy(_type, val);
			}

			miter = _mainmap.erase(miter);
		}
		else
			++miter;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////
template < class T, class C >
varmap< T, C >::varmap(vt_types type, bool partial, bool fuzzy, variant_factory& factory) : 
	_type(type), _partial(partial), _fuzzy(fuzzy), _compare(type, partial, fuzzy), _mainmap(_compare), _factory(factory)
{
	assert(!_partial || !_fuzzy);
}

template < class T, class C >
varmap< T, C >::~varmap()
{
	clear();
}

template < class T, class C >
TYPENAME varmap< T, C >::mainmap_iter_t 
varmap< T, C >::end()
{
	return _mainmap.end();
}

template < class T, class C >
bool 
varmap< T, C >::find_exact_resource(const main_map_key& res, C& x, const T& min_filter, const T& max_filter) const
{
	bool ret = false;
	// tries to find
	mainmap_citer_t ifind = _mainmap.find(res);
	if (ifind != _mainmap.end()) // found
	{
		// copies only the uniques idents
		ret |= add_unique_items(ifind, x, min_filter, max_filter);
	}

	return ret;
}

// finds the range of resources
// "<" , "<="
template < class T, class C >
bool 
varmap< T, C >::find_less_resource(const main_map_key& res, C& x, const T& min_filter, const T& max_filter, bool boundary_include) const
{
	bool ret = false;

	// boundaries [begin -> end]
	// no boundaries [begin -> end[

	// tries to find
	mainmap_citer_t istart = _mainmap.begin();
	mainmap_citer_t iend = boundary_include ? _mainmap.upper_bound(res) : _mainmap.lower_bound(res); // [begin -> end]

	while (istart != iend) 
	{
		// copies only the uniques idents
		ret |= add_unique_items(istart, x, min_filter, max_filter);
		++istart;
		ret = true;
	}

	return ret;
}

// ">", ">='
template < class T, class C >
bool 
varmap< T, C >::find_greater_resource(const main_map_key& res, C& x, const T& min_filter, const T& max_filter, bool boundary_include) const
{
	bool ret = false;

	// boundaries [start -> end[
	// no boundaries ]start -> end[

	// tries to find
	mainmap_citer_t istart = _mainmap.end();
	mainmap_citer_t iend = _mainmap.end();

	if (boundary_include) // boundaries [start -> end[
		istart = _mainmap.find(res);

	if (istart == _mainmap.end()) //]start -> end[
		istart = _mainmap.upper_bound(res);

	while (istart != iend) // found
	{
		// copies only the uniques idents
		ret |= add_unique_items(istart, x, min_filter, max_filter);
		++istart;
	}

	return ret;
}

template < class T, class C >
bool 
varmap< T, C >::intersect_exact_resource(const main_map_key& res, C& x, const T& min_filter, const T& max_filter) const
{
	// tries to find
	mainmap_citer_t ifind = _mainmap.find(res);
	if (ifind != _mainmap.end()) // found
	{
		// copies only the uniques idents
		remove_uncommon_items(ifind, x, min_filter, max_filter);
	}
	else
		x._container.clear();

	return !x._container.empty();
}

// finds the range of resources
// "<" , "<="
template < class T, class C >
bool 
varmap< T, C >::intersect_less_resource(const main_map_key& res, C& x, const T& min_filter, const T& max_filter, bool boundary_include) const
{
	// tries to find
	// tries to find
	mainmap_citer_t istart = _mainmap.begin();
	mainmap_citer_t iend = boundary_include ? _mainmap.upper_bound(res) : _mainmap.lower_bound(res); // [begin -> end]

	while (istart != iend) 
	{
		// copies only the uniques idents
		mark_common_items(istart, x, min_filter, max_filter);
		++istart;
	}

	// removes all negatives
	for (TYPENAME C::sorted_container_data_t::iterator it = x._container.begin(); it != x._container.end();)
	{
		if (!*it)
			it = x._container.erase(it);
		else // reset
			*it = false, ++it;
	}

	return !x._container.empty();
}

// ">", ">='
template < class T, class C >
bool 
varmap< T, C >::intersect_greater_resource(const main_map_key& res, C& x, const T& min_filter, const T& max_filter, bool boundary_include) const
{
	// tries to find
	mainmap_citer_t ifind = _mainmap.end();

	if (boundary_include)
		ifind = _mainmap.find(res);

	if (ifind == _mainmap.end())
		ifind = _mainmap.upper_bound(res);

	while (ifind != _mainmap.end()) // found
	{
		// copies only the uniques idents
		mark_common_items(ifind, x, min_filter, max_filter);
		++ifind;
	}

	// removes all negatives
	for (TYPENAME C::sorted_container_data_t::iterator it = x._container.begin(); it != x._container.end();)
	{
		if (!*it)
			it = x._container.erase(it);
		else // reset
			*it = false, ++it;
	}

	return !x._container.empty();
}


template < class T, class C >
void 
varmap< T, C >::fetch_all(C& x) const
{
	T min_filter(0, 0);
	T max_filter(~0, ~0);

	for (mainmap_citer_t miter = _mainmap.begin(); miter != _mainmap.end(); ++miter)
	{
		add_unique_items(miter, x, min_filter, max_filter);
	}
}

template < class T, class C >
void 
varmap< T, C >::clear()
{
	for (mainmap_citer_t miter = _mainmap.begin(); miter != _mainmap.end(); ++miter)
	{
		var_value val;
		if (_partial)
		{
			val._not_null = true;
			val._value.strVal = miter.key()._var_res._key._res;
			_factory.destroy(_type, val);
			val._value.bufVal = (const ub1_t*)miter.key()._var_res._key._offsets;
			if (val._value.bufVal)
				_factory.destroy(vt_numeric, val);
		}
		else
		{
			val = miter.key()._var_res._val;
			_factory.destroy(_type, val);
		}
	}

	_mainmap.clear();
	_main_object_allocator.clear_all();
	if (_partial)
		_offsetmap.clear();
}

// static
template < class T, class C >
void 
varmap< T, C >::get_common_items(mainmap_citer_t c, const C& r, const T& min_filter, const T& max_filter, size_t& entries)
{
	if (c->size() < r._container.size())
	{
		TYPENAME mainmap_object_t::const_iterator clower = c->lower_bound(min_filter);
		TYPENAME mainmap_object_t::const_iterator cupper = c->upper_bound(max_filter);

		for (; clower != cupper; ++clower)
		{
			size_t ckey = r.clean_key(clower.key());
			if (r._container.end() != r._container.find(ckey))
				++entries;
		}
	}
	else
	{
		for (TYPENAME C::sorted_container_data_t::const_iterator riter = r._container.begin(); riter != r._container.end(); ++riter)
		{
			T ckey = r.compound_key(riter.key(), min_filter);
			if (c->end() != c->find(ckey))
				++entries;
		}
	}
}

// static
template < class T, class C >
bool 
varmap< T, C >::add_unique_items(mainmap_citer_t c, C& r, const T& min_filter, const T& max_filter)
{
	bool ret = false;

	TYPENAME mainmap_object_t::const_iterator clower = c->lower_bound(min_filter);
	TYPENAME mainmap_object_t::const_iterator cupper = c->upper_bound(max_filter);

	for (; clower != cupper; ++clower)
	{
		size_t ckey = r.clean_key(clower.key());
		if (r._container.end() == r._container.find(ckey))
		{
			r._container.insert(r._allocator, ckey, false);
			ret = true;
		}
	}

	return ret;
}

template < class T, class C >
size_t
varmap< T, C >::get_main_object_allocator_pages() const
{
	return _main_object_allocator.count();
}

template < class T, class C >
size_t
varmap< T, C >::get_main_object_allocator_capacity() const
{
	return _main_object_allocator.capacity();
}

template < class T, class C >
TYPENAME varmap< T, C >::mainmap_iter_t 
varmap< T, C >::add_resource(const main_map_key& mkey, const T& x)
{
	// tries to find
	mainmap_iter_t ifind = _mainmap.find(mkey);
	if (ifind == _mainmap.end()) // not found
	{
		mainmap_object_t obj;

		// here we make a full copy
		if (_partial)
		{
			main_map_key clone_key(mkey._var_res._key._res, &_factory);
			ifind = _mainmap.insert(clone_key, obj).first;
		}
		else if (_fuzzy)
		{
			main_map_key clone_key(mkey._var_res._ngram._fuzzy_key, mkey._var_res._key._res, &_factory);
			ifind = _mainmap.insert(clone_key, obj).first;
		}
		else
		{
			main_map_key clone_key(_type, mkey._var_res._val, &_factory); 
			ifind = _mainmap.insert(clone_key, obj).first;
		}

		if (ifind == _mainmap.end())
			return ifind;

		// assigns value
		ifind->insert(_main_object_allocator, x, false);

		if (_partial)
		{
			// inserts into offset
			for (ub1_t i = 0; ifind.key()._var_res._key._offsets && i < *ifind.key()._var_res._key._offsets; ++i)
			{
				offset_map_key skey(ifind, i);
				_offsetmap.insert(skey, true);
			}
		}
	}
	else // add to list
	{
		// checks if already in list
		TYPENAME mainmap_object_t::const_iterator iter = ifind->find(x);

		if (iter == ifind->end())
			ifind->insert(_main_object_allocator, x, false);
	}

	return ifind;
}


template < class T, class C >
bool
varmap< T, C >::remove_resource(mainmap_iter_t ierase, const T& x)
{
	// tries to find
	TYPENAME mainmap_object_t::iterator iter = ierase->find(x);
	if (iter == ierase->end())
		return false;

	// removes ident from list
	ierase->erase(_main_object_allocator, iter);

	if (!ierase->empty()) // there are still entries here
		return true;

	var_value val;

	if (_partial)
	{
		// removes resource
		for (ub1_t i = 0; ierase.key()._var_res._key._offsets && i < *ierase.key()._var_res._key._offsets; ++i)
		{
			// multimap, we have to find exact main map iterator and then delete it
			offset_map_key skey(ierase, i);
			offsetmap_iter_t iLower = _offsetmap.lower_bound(skey);
			offsetmap_iter_t iUpper = _offsetmap.upper_bound(skey);

			if (iLower != _offsetmap.end()) // if we found
			{
				// checks all iterators
				while (iLower != iUpper)
				{
					if (iLower.key()._iter == ierase)
						iLower = _offsetmap.erase(iLower);
					else
						++iLower;
				}
			}
		}

		// destroys string resource
		val._not_null = true;
		val._value.strVal = ierase.key()._var_res._key._res;
		if (val._value.strVal)
			_factory.destroy(_type, val);
		// destroy offsets
		val._value.bufVal = (const ub1_t*)ierase.key()._var_res._key._offsets;
		if (val._value.bufVal)
			_factory.destroy(vt_numeric, val);
	}
	else if (_fuzzy)
	{
		// destroys string
		val._not_null = true;
		val._value.strVal = ierase.key()._var_res._ngram._res;
		if (val._value.strVal)
			_factory.destroy(_type, val);
	}
	else
	{
		val = ierase.key()._var_res._val;
        _factory.destroy(_type, val);
	}

	_mainmap.erase(ierase);
	return true;
}

template < class T, class C >
bool 
varmap< T, C >::find_partial_resource(const main_map_key& mkey, bool deep, C& x, const T& min_filter, const T& max_filter) const
{
	bool ret = false;
	assert(_partial);
	// tries to find
	mainmap_citer_t bmfind = _mainmap.lower_bound(mkey);
	while (bmfind != _mainmap.end()
		&& bmfind.key().partial_match(mkey)
		)
	{
		// copies only the uniques idents
		ret |= add_unique_items(bmfind, x, min_filter, max_filter);
		++bmfind;
	}

	if (deep && !_offsetmap.empty())
	{
		// finds the longest offset
		offsetmap_citer_t bsupper = _offsetmap.end();
		--bsupper;

		size_t max_tokens = bsupper.key()._offset;

		// looking offset maps
		for (size_t i = 0; deep && i <= max_tokens; ++i)
		{
			offset_map_key skey(mkey._var_res._key._res, i);
			offsetmap_citer_t bsfind = _offsetmap.lower_bound(skey);
			while (bsfind != _offsetmap.end()
				&& bsfind.key()._offset == i
				&& bsfind.key().partial_match(mkey)
				)
			{
				// copies only the uniques idents
				ret |= add_unique_items(bsfind.key()._iter, x, min_filter, max_filter);
				++bsfind;
			}
		}
	}

	return ret;
}

template < class T, class C >
bool 
varmap< T, C >::intersect_partial_resource(const main_map_key& mkey, bool deep, C& x, const T& min_filter, const T& max_filter) const
{
	TYPENAME C::sorted_container_data_t::iterator it = x._container.begin();

	// tries to find
	mainmap_citer_t bmfind = _partial ? _mainmap.lower_bound(mkey) : _mainmap.find(mkey);
	while (bmfind != _mainmap.end()
		&& (!_partial || _partial && bmfind.key().partial_match(mkey))
		)
	{
		// copies only the uniques idents
		mark_common_items(bmfind, x, min_filter, max_filter);
		++bmfind;
	}

	// finds the longest offset
	if (_partial && deep && !_offsetmap.empty())
	{
		// finds the longest offset
		offsetmap_citer_t bsupper = _offsetmap.end();
		--bsupper;

		size_t max_tokens = bsupper.key()._offset;

		// looking offset maps
		for (size_t i = 0; deep && i <= max_tokens; ++i)
		{
			offset_map_key skey(mkey._var_res._key._res, i);
			offsetmap_citer_t bsfind = _offsetmap.lower_bound(skey);
			while (bsfind != _offsetmap.end()
				&& bsfind.key()._offset == i
				&& bsfind.key().partial_match(mkey)
				)
			{
				// copies only the uniques idents
				mark_common_items(bmfind, x, min_filter, max_filter);
				++bsfind;
			}
		}
	}

	// removes all negatives
	for (it = x._container.begin(); it != x._container.end();)
	{
		if (!*it)
			it = x._container.erase(it);
		else
			*it = false, ++it;
	}

	return !x._container.empty();
}

template < class T, class C >
bool 
varmap< T, C >::find_fuzzy_resource(const _list< size_t >& fuzzy_container, C& x, const T& min_filter, const T& max_filter) const
{
	TYPENAME C::sorted_container_data_t::iterator it = x._container.begin();

	bool ret = false;
	assert(_fuzzy);

	// finds all fuzzy candidates
	for (_list< size_t >::const_iterator iter_candidates = fuzzy_container.begin(); iter_candidates != fuzzy_container.end(); ++iter_candidates)
	{
		main_map_key mkey(*iter_candidates, 0, 0); 
		// tries to find
		mainmap_citer_t mfind = _mainmap.lower_bound(mkey);
		if (mfind != _mainmap.end())
		{
			// copies only the uniques idents
			ret |= add_unique_items(mfind, x, min_filter, max_filter);
		}
	}

	return ret;
}

template < class T, class C >
bool 
varmap< T, C >::intersect_fuzzy_resource(const _list< size_t >& fuzzy_container, C& x, const T& min_filter, const T& max_filter) const
{
	TYPENAME C::sorted_container_data_t::iterator it = x._container.begin();

	// finds all fuzzy candidates
	for (_list< size_t >::const_iterator iter_candidates = fuzzy_container.begin(); iter_candidates != fuzzy_container.end(); ++iter_candidates)
	{
		main_map_key mkey(*iter_candidates, 0, 0); 
		// tries to find
		mainmap_citer_t mfind = _mainmap.lower_bound(mkey);
		if (mfind != _mainmap.end())
		{
			// copies only the uniques idents
			mark_common_items(mfind, x, min_filter, max_filter);
		}
	}

	// removes all negatives
	for (it = x._container.begin(); it != x._container.end();)
	{
		if (!*it)
			it = x._container.erase(it);
		else
			*it = false, ++it;
	}

	return !x._container.empty();
}

// static
template < class T, class C >
void 
varmap< T, C >::mark_common_items(mainmap_citer_t c, C& r, const T& min_filter, const T& max_filter)
{

	if (c->size() < r._container.size())
	{
		TYPENAME mainmap_object_t::const_iterator clower = c->lower_bound(min_filter);
		TYPENAME mainmap_object_t::const_iterator cupper = c->upper_bound(max_filter);

		for (; clower != cupper; ++clower)
		{
			TYPENAME C::sorted_container_data_t::iterator riter = r._container.find(r.clean_key(clower.key()));
			if (riter != r._container.end())
				*riter = true;
		}
	}
	else
	{
		for (TYPENAME C::sorted_container_data_t::iterator riter = r._container.begin(); riter != r._container.end(); ++riter)
		{
			T ckey = r.compound_key(riter.key(), min_filter);
			TYPENAME mainmap_object_t::const_iterator citer = c->find(ckey);
			if (citer != c->end())
				*riter = true;
		}
	}
}

// static
template < class T, class C >
void 
varmap< T, C >::remove_uncommon_items(mainmap_citer_t c, C& r, const T& min_filter, const T& max_filter)
{
	for (TYPENAME C::sorted_container_data_t::iterator riter = r._container.begin(); riter != r._container.end();)
	{
		T ckey = r.compound_key(riter.key(), min_filter);
		TYPENAME mainmap_object_t::const_iterator citer = c->find(ckey);
		if (citer != c->end())
			++riter;
		else
			riter = r._container.erase(riter);
	}
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_varmap_hpp_
