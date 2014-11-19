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

#include "base/numeric.h"
#include "smart/varobj.h"
#include "base/vector.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/string.hpp"
#include "base/stack.hpp"
#include "smart/varmap.hpp"
#include "smart/varvalue.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

extern const char* response_dtd;
/////////////////////////////////////////////////////////////////////////////////////////////
// intersect 
void 
var_container::intersect(const var_container& x)
{
	sorted_container_data_t::const_iterator isrc = x._container.begin();
	sorted_container_data_t::iterator ithis = _container.begin();

	while (isrc != x._container.end()
		&& ithis != _container.end()
		)
	{
		if (isrc.key() < ithis.key())
			++isrc;
		else if (ithis.key() < isrc.key())
			ithis = _container.erase(ithis);
		else
		{
			++isrc;
			++ithis;
		}
	}

	while (ithis != _container.end())
		ithis = _container.erase(ithis);
}

// combine 
void 
var_container::combine(const var_container& x)
{
	sorted_container_data_t::const_iterator isrc = x._container.begin();
	sorted_container_data_t::iterator ithis = _container.begin();

	while (isrc != x._container.end()
		&& ithis != _container.end()
		)
	{
		if (isrc.key() < ithis.key())
		{
			ithis = _container.insert(_allocator, isrc.key(), *isrc).first;
			++isrc;
		}
		else if (ithis.key() < isrc.key())
			++ithis;
		else
		{
			++isrc;
			++ithis;
		}
	}

	while (isrc != x._container.end())
	{
		_container.insert(_allocator, isrc.key(), *isrc);		
		++isrc;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////


var_object_repository::var_object_repository(const var_object_schema& schema) : 
	_schema(schema),
	_uint32_type_map(vt_ub4, false, false, _var_factory),
	_uint64_type_map(vt_ub8, false, false, _var_factory),
	_double_type_map(vt_double, false, false, _var_factory),
	_string_type_map(vt_string, false, false, _var_factory),
	_searchable_string_type_map(vt_string, true, false, _var_factory),
	_fuzzy_string_type_map(vt_string, false, true, _var_factory),
	_numeric_type_map(vt_numeric, false, false, _var_factory),
	_binary_type_map(vt_binary, false, false, _var_factory),
	_guid_type_map(vt_guid, false, false, _var_factory)
{
	fuzzy_matcher_factory acc;
	_fuzzy_engine = acc.get_fuzzy_matcher(1024*1024*64);
}

var_object_repository::~var_object_repository()
{
	if (_fuzzy_engine)
		delete _fuzzy_engine;
}

//! \brief clear and reuse object
void
var_object_repository::reset()
{
	_key_generator.clear();

	_objs_map.clear();
	_objs_all.reset();

	_uint32_type_map.clear();
	_double_type_map.clear();
	_uint64_type_map.clear();
	_string_type_map.clear();		
	_searchable_string_type_map.clear();
	_fuzzy_string_type_map.clear();
	_numeric_type_map.clear();
	_binary_type_map.clear();
	_guid_type_map.clear();

	_iter_vec_all.reset();

	_var_factory.reset();
	_fuzzy_engine->reset();
}

//! \brief count rows
size_t
var_object_repository::count() const
{
	return _objs_map.size();
}

// selects all objects (GT (>), GE (>=), LT (<), LE (<=), EQ (=), NQ (!=), PM (partial match)) 
// query is xml request like
// <query what="SELECT">
//	<returns>
//		<col name="name" />
//	</returns>
//	<where>
//		<group join="AND">
//			<cond name="ident" how="GT" val="45" />
//			<cond name="ident" how="LE" val="75" />
//			<cond name="name" how="PM" val="John" />
//		</group>
//	</where>
// </query>

// <query what="INSERT">
//	<returns>
//		<col name="name" />
//	</returns>
//	<values>
//		<col name="ident" val="76" /> 
//	</values>
// </query>

// <query what="UPDATE">
//	<returns>
//		<col name="name" />
//	</returns>
//	<values>
//		<col name="ident" val="76" /> 
//	</values>
//	<where>
//		<group join="AND">
//			<cond name="ident" how="GT" val="45" />
//			<cond name="ident" how="LE" val="75" />
//			<cond name="name" how="PM" val="John" />
//		</group>
//	</where>
// </query>

// deletes by "rowid" - name of condition is omitted
// <query what="DELETE">
//	<returns>
//		<col name="name" />
//	</returns>
//	<where>
//		<group join="AND">
//			<cond how="EQ" val="1" />
//		</group>
//	</where>
// </query>

// processes xml query
bool 
var_object_repository::process_query(xml_designer* parser, var_container::sorted_container_allocator_t& all, byte_allocator& tmp, byte_allocator& inl)
{
	// fills up container (replaces allocator on the external one)
	var_container where(all);

	list_returns_index_t returns;
	var_object_values values;

	parser->select_attribute_by_name("what");
	const char* what = parser->get_value();
	char w = what[0];
	parser->select_parent();

	string_t err(0, &tmp);

	switch (w)
	{
		case 'S': // selects
			// checks returns - optional
			if (parser->select_xpath("query/returns"))
			{
				if (!process_returns(parser, returns, tmp, inl, err))
					return process_error(parser, err);

				parser->select_parent();
			}

			// checks where - required
			if (!parser->select_xpath("query/where"))
				return process_error(parser, "Can not find the required element \"where\"");
			else
			{
				if (!process_where(parser, where, tmp, inl, err))
					return process_error(parser, err);

				parser->select_parent();
			}
			break;
		case 'I': // inserts
			// checks returns - optional
			if (parser->select_xpath("query/returns"))
			{
				if (!process_returns(parser, returns, tmp, inl, err))
					return process_error(parser, err);

				parser->select_parent();
			}

			// checks values - required
			if (!parser->select_xpath("query/values"))
				return process_error(parser, "Can not find the required element \"values\"");
			else
			{
				if (!process_values(parser, values, tmp, inl, err))
					return process_error(parser, err);

				parser->select_parent();
			}

			{
				//  inserts all values and looks up all returns
				size_t pk = insert_object(values, tmp, inl, err);

				if (!pk)
					return process_error(parser, err);

				where._container.insert(where._allocator, pk, true);
			}

			break;
		case 'U': // updates
			// checks returns - optional
			if (parser->select_xpath("query/returns"))
			{
				if (!process_returns(parser, returns, tmp, inl, err))
					return process_error(parser, err);

				parser->select_parent();
			}

			// checks values - required
			if (!parser->select_xpath("query/values"))
				return process_error(parser, "Can not find the required element \"values\"");
			else
			{
				if (!process_values(parser, values, tmp, inl, err))
					return process_error(parser, err);

				parser->select_parent();
			}

			// checks where - required
			if (!parser->select_xpath("query/where"))
				return process_error(parser, "Can not find the required element \"where\"");
			else
			{
				if (!process_where(parser, where, tmp, inl, err))
					return process_error(parser, err);

				parser->select_parent();
			}

			{
				// updates all rows
				for (var_container::sorted_container_data_t::const_iterator it = where._container.begin(); it != where._container.end(); ++it)
				{
					size_t pk = it.key();
					if (!update_object(pk, values, tmp, inl, err))
						return process_error(parser, err);
				}
			}
			break;
		case 'D': // deletes
			// checks returns - optional
			if (parser->select_xpath("query/returns"))
			{
				if (!process_returns(parser, returns, tmp, inl, err))
					return process_error(parser, err);

				parser->select_parent();
			}

			// checks where - required
			if (!parser->select_xpath("query/where"))
				return process_error(parser, "Can not find the required element \"where\"");
			else
			{
				if (!process_where(parser, where, tmp, inl, err))
					return process_error(parser, err);

				parser->select_parent();
			}
			break;
		default:
			assert(false);
	}

	parser->load(0, 0, response_dtd, strlen(response_dtd));
	parser->add_child(ELEMENT_NODE, "response", 0, false);
	parser->add_child(ATTRIBUTE_NODE, "errCode", "0", true);

	// fills out returns
	for (var_container::sorted_container_data_t::const_iterator it = where._container.begin(); it != where._container.end(); ++it)
	{
		size_t pk = it.key();

		parser->add_child(ELEMENT_NODE, "row", 0, false);
		char buf[32];
		str_template::strprint(buf, 32, "%d", pk);
		parser->add_child(ATTRIBUTE_NODE, "rowid", buf, true);

		object_map_data_t::const_iterator ival = _objs_map.find(pk);
		assert(ival != _objs_map.end());

		for (list_returns_index_t::const_iterator ic = returns.begin(); ic != returns.end(); ++ic)
		{
			parser->add_child(ELEMENT_NODE, "col", 0, false);
			parser->add_child(ATTRIBUTE_NODE, "name", _schema[*ic]._name, true);

			// adds value
			const char* value = 0;

			try
			{
				if (_schema[*ic]._is_searchable)
				{
					value = (*ival)[*ic].key()._var_res._key._res;
				}
				else if (_schema[*ic]._is_fuzzy_match)
				{
					value = (*ival)[*ic].key()._var_res._ngram._res;
				}
				else
				{
					value = persist_value(_schema[*ic]._type, (*ival)[*ic].key()._var_res._val._value, &tmp);
				}
			}
			catch (exception& x)
			{
				return process_error(parser, x.what());
			}

			parser->add_child(ATTRIBUTE_NODE, "val", value, true);
			parser->select_parent();
		}
		
		parser->select_parent();
	}

	switch (w)
	{
		case 'S': // selects
			break;
		case 'I': // inserts
			break;
		case 'U': // updates
			break;
		case 'D': // deletes
			// deletes all rows
			{
				for (var_container::sorted_container_data_t::const_iterator it = where._container.begin(); it != where._container.end(); ++it)
				{
					size_t pk = it.key();
					if (!delete_object(pk, tmp, err))
						return process_error(parser, err);
				}
			}
			break;
		default:
			assert(false);
	}

	return true;
}

bool 
var_object_repository::process_where(const xml_designer* parser, 
									var_container& container, 
									byte_allocator& tmp, 
									byte_allocator& inl, 
									string_t& err) const
{
	// loop for all conditions
	if (!parser->select_first_child())
	{
		err = "where cluase is empty";
		return false;
	}

	do
	{
		// can be group or condition
		if (ELEMENT_NODE != parser->get_type())
			continue;

		const char* name = parser->get_name();

		switch (name[0])
		{
			case 'c': // condition
				if (!process_condition(parser, container, false, tmp, inl, err))
					return false;
				break;
			case 'g': // group
				if (!process_group(parser, container, false, tmp, inl, err))
					return false;
				break;
			default:
				assert(false);
		}
	}
	while (parser->select_next_sibling());

	parser->select_parent();

	return true;
}

bool 
var_object_repository::process_returns(const xml_designer* parser, 
									   list_returns_index_t& container, 
									   byte_allocator& tmp, 
									   byte_allocator& inl, 
									   string_t& err) const
{
	// loop for all values
	if (!parser->select_first_child())
		return true;

	do
	{
		// can be group or condition
		if (ELEMENT_NODE != parser->get_type())
			continue;

		// this is a column
		parser->select_attribute_by_name("name");
		const char* name = parser->get_value();

		size_t index = 0;
		for (var_object_schema::const_iterator iter = _schema.begin(); iter != _schema.end(); ++iter, ++index)
		{
			if (!str_template::strnocasecmp(name, (const char*)_schema[index]._name, os_minus_one))
				break;
		}

		if (index == _schema.size())
		{
			err = "Invalid column name: ";
			err += name;
			return false;
		}

		container.push_back(tmp, index);

		parser->select_parent();
	}
	while (parser->select_next_sibling());

	parser->select_parent();

	return true;
}

bool 
var_object_repository::process_values(const xml_designer* parser, 
									  var_object_values& container, 
									  byte_allocator& tmp, 
									  byte_allocator& inl, 
									  string_t& err) const
{
	// resizes container
	container.resize(tmp, _schema.size());

	// loop for all values
	if (!parser->select_first_child())
		return true;

	do
	{
		// can be group or condition
		if (ELEMENT_NODE != parser->get_type())
			continue;

		// this is a column
		parser->select_attribute_by_name("name");
		const char* name = parser->get_value();

		size_t index = 0;
		for (var_object_schema::const_iterator iter = _schema.begin(); iter != _schema.end(); ++iter, ++index)
		{
			if (!str_template::strnocasecmp(name, (const char*)_schema[index]._name, os_minus_one))
				break;
		}

		if (index == _schema.size())
		{
			err = "Invalid column name: ";
			err += name;
			return false;
		}

		parser->select_parent();

		const char* value = 0;
		if (parser->select_attribute_by_name("val"))
		{
			value = parser->get_value();

			container[index]._not_null = true;
		
			try
			{
				container[index]._value = parse_value(_schema[index]._type, value, os_minus_one, &tmp);
			}
			catch (exception& x)
			{
				err = x.what();
				return false;
			}

			parser->select_parent();
		}

		container[index]._specified = true;
	}
	while (parser->select_next_sibling());

	parser->select_parent();

	return true;
}

bool 
var_object_repository::process_condition(const xml_designer* parser, 
										 var_container& container, 
										 bool intersect, 
										 byte_allocator& tmp, 
										 byte_allocator& inl, 
										 string_t& err) const
{
// <!ATTLIST cond how %condition; #REQUIRED 
//	name CDATA #IMPLIED
//	val CDATA #REQUIRED >

	parser->select_attribute_by_name("how");
	char how[2];
	memcpy(how, parser->get_value(), 2);
	parser->select_parent();

	bool deep = false;
	if (parser->select_attribute_by_name("deep"))
	{
		deep = !strcmp(parser->get_value(), "1") || !strcmp(parser->get_value(), "true");
		parser->select_parent();
	}

	ngram_quality nq = nq_high;
	if (parser->select_attribute_by_name("nq"))
	{
		if (!strcmp(parser->get_value(), "low"))
			nq = nq_low;
		else if (!strcmp(parser->get_value(), "normal"))
			nq = nq_normal;

		parser->select_parent();
	}

	phonetic_quality pq = pq_high;
	if (parser->select_attribute_by_name("pq"))
	{
		if (!strcmp(parser->get_value(), "low"))
			pq = pq_low;
		else if (!strcmp(parser->get_value(), "normal"))
			pq = pq_normal;

		parser->select_parent();
	}

	size_t index = 0;
	string_t name(0, &inl);
	if (parser->select_attribute_by_name("name"))
	{
		name = parser->get_value();
		parser->select_parent();

		// finds name in schema 
		for (var_object_schema::const_iterator iter = _schema.begin(); iter != _schema.end(); ++iter, ++index)
		{
			if (name == iter->_name)
				break;
		}
		
		// no name
		if (index == _schema.size())
		{
			err = "Invalid column name: ";
			err += name;
			return false;
		}
	}

	bool by_rowid = name.length() == 0;
	vt_types type = by_rowid ? vt_ub4 : _schema[index]._type;
	bool searchable = by_rowid ? false : _schema[index]._is_searchable;
	bool fuzzymatch = by_rowid ? false : _schema[index]._is_fuzzy_match;

	// finds condition sign
	parser->select_attribute_by_name("val");
	const char* value = parser->get_value();

	var_value val;
	val._not_null = (value != 0);

	try
	{
		val._value = parse_value(type, value, os_minus_one, &tmp);
	}
	catch (exception& x)
	{
		err = x.what();
		return false;
	}

	parser->select_parent();

	var_value castval;
	vt_types casttype = cast_to_common_type(type, val, castval, tmp);

	if (by_rowid)
	{
		if (_objs_map.find(castval._value.ulVal) != _objs_map.end())
			container._container.insert(container._allocator, castval._value.ulVal, true); 
	}
	else
	{
		const var_object_map_t* pv = get_v_object_pointer(casttype, searchable);
		if (!pv)
		{
			err = "Not enough memory";
			return false;
		}

		main_map_key key(casttype, castval, 0);
		pk_column min_filter(0, index);
		pk_column max_filter(~0, index);

		switch (how[0]) // GT | GE | LT | LE | EQ | NE | PM | FM
		{
			case 'G':
				{
					switch (how[1])
					{
						case 'T':
							intersect ? 
								pv->intersect_greater_resource(key, container, min_filter, max_filter, false) :
								pv->find_greater_resource(key, container, min_filter, max_filter, false);
								break;
						case 'E':
							intersect ? 
								pv->intersect_greater_resource(key, container, min_filter, max_filter, true) :
								pv->find_greater_resource(key, container, min_filter, max_filter, true);
								break;
						default:
							assert(false);
					}
				}
				break;
			case 'L':
				{
					switch (how[1])
					{
						case 'T':
							intersect ? 
								pv->intersect_less_resource(key, container, min_filter, max_filter, false) :
								pv->find_less_resource(key, container, min_filter, max_filter, false);
								break;
						case 'E':
							intersect ? 
								pv->intersect_less_resource(key, container, min_filter, max_filter, true) :
								pv->find_less_resource(key, container, min_filter, max_filter, true);
								break;
						default:
							assert(false);
					}
				}
				break;
			case 'E':
				intersect ? 
					pv->intersect_exact_resource(key, container, min_filter, max_filter) :
					pv->find_exact_resource(key, container, min_filter, max_filter);
				break;
			case 'N':
				intersect ? 
					pv->intersect_exact_resource(key, container, min_filter, max_filter) :
					pv->find_exact_resource(key, container, min_filter, max_filter);
				break;
			case 'P':
				if (casttype != vt_string)
					return false;

				// TO DO - if fuzzy matching -> goes to the specific fuzzy engine and gets suggestions
				// applies for temporary container to fill out all possible candidates
				// for intersect_exact_resource
				if (searchable)
				{
					main_map_key pkey(castval._value.strVal, 0);
					intersect ? 
						pv->intersect_partial_resource(pkey, deep, container, min_filter, max_filter) :
						pv->find_partial_resource(pkey, deep, container, min_filter, max_filter);
				}
				break;
			case 'F':
				if (casttype != vt_string)
					return false;

				if (fuzzymatch)
				{
					// first calls the fuzzy matcher
					_list< size_t > fuzzy_container;
					_fuzzy_engine->match(nq, pq, castval._value.strVal, tmp, inl, fuzzy_container);
					// TO DO - calls fuzzy matcher
					intersect ? 
						pv->intersect_fuzzy_resource(fuzzy_container, container, min_filter, max_filter) :
						pv->find_fuzzy_resource(fuzzy_container, container, min_filter, max_filter);
				}

				break;
			default:
				assert(false);
				return false;
		}
	}
	return true;
}

bool 
var_object_repository::process_group(const xml_designer* parser, 
									 var_container& container, 
									 bool intersect, 
									 byte_allocator& tmp, 
									 byte_allocator& inl, 
									 string_t& err) const
{
	parser->select_attribute_by_name("join");
	char join = *parser->get_value();
	parser->select_parent();


	var_container groupcontainer(container._allocator);
	bool group_intersect = false;

	// loop for all conditions
	if (parser->select_first_child())
	{
		do
		{
			// can be group or condition
			if (ELEMENT_NODE != parser->get_type())
				continue;

			const char* name = parser->get_name();

			switch (name[0])
			{
				case 'c': // condition
					if (!process_condition(parser, groupcontainer, group_intersect && join == 'A', tmp, inl, err))
						return false;
					break;
				case 'g': // group
					if (!process_group(parser, groupcontainer, group_intersect && join == 'A', tmp, inl, err))
						return false;
					break;
				default:
					assert(false);
			}

			group_intersect = true;
		}
		while (parser->select_next_sibling());
		parser->select_parent();
	}

	intersect ? container.intersect(groupcontainer) : container.combine(groupcontainer);

	return true;
}

// could all objects be in a string presentation???
size_t 
var_object_repository::insert_object(const var_object_values& values, byte_allocator& tmp, byte_allocator& inl, string_t& err)
{
	// detects if the array  provided matches with the schema
	 if (_schema.size() != values.size())
	 {
		 assert(false);
		 err = "Number of values does not match number of columns";
		 return 0;
	 }

	iter_vector_t iters;
	iters.resize(_iter_vec_all, _schema.size());

	// generates unique key
	size_t pk = _key_generator.generate();

	
	size_t index = 0;
	for (var_object_schema::const_iterator iter = _schema.begin(); iter != _schema.end(); ++iter, ++index)
	{
		// gets correspondent value
		var_value val;
		vt_types mtype = cast_to_common_type(iter->_type, values[index], val, tmp);
		var_object_map_t* pv = get_v_object_pointer(mtype, iter->_is_searchable);
		if (!pv)
		{
			err = "Not enough memory";
			return 0;
		}

		pk_column dkey(pk, index);

		if (iter->_is_searchable)
		{
			main_map_key key(val._value.strVal, 0);
			iters[index] = pv->add_resource(key, dkey);
		}
		else if (iter->_is_fuzzy_match)
		{
			size_t ident = _fuzzy_engine->add(val._value.strVal, tmp);
			main_map_key key(ident, val._value.strVal, 0);
			iters[index] = pv->add_resource(key, dkey);
		}
		else
		{
			main_map_key key(mtype, val, 0);
			iters[index] = pv->add_resource(key, dkey);
		}
	}

	 // inserts pk and iterators into object map
	 _objs_map.insert(_objs_all, pk, iters);
	 return pk;
}

// could all objects be in a string presentation???
bool 
var_object_repository::delete_object(size_t pk, byte_allocator& tmp, string_t& err)
{
	// tries to find the correspondent object
	object_map_data_t::iterator iter_object = _objs_map.find(pk);
	if (iter_object == _objs_map.end())
	{
		err = "Object not found";
		return false;
	}
	
	// removes the correspondent entries
	size_t index = 0;
	for (var_object_schema::const_iterator iter = _schema.begin(); iter != _schema.end(); ++iter, ++index)
	{
		// checks the type
		vt_types mtype = map_type(iter->_type);
		var_object_map_t* pv = get_v_object_pointer(mtype, iter->_is_searchable);
		if (!pv)
		{
			err = "Not enough memory";
			return false;
		}

		if (iter->_is_fuzzy_match)
		{
			_fuzzy_engine->remove((*iter_object)[index].key()._var_res._ngram._fuzzy_key, tmp);
		}

		pk_column dkey(pk, index);
		if (!pv->remove_resource((*iter_object)[index], dkey))
		{
			err = "Object not found";
			return false;
		}
	}

	// removes resource
	iter_object->clear(_iter_vec_all);
	_objs_map.erase(iter_object);

	_key_generator.save(pk);
	return true;
}

// updates existing object from the repository
bool 
var_object_repository::update_object(size_t pk, const var_object_values& values, byte_allocator& tmp, byte_allocator& inl, string_t& err)
{
	// detects if the array provided matches the schema
	if (_schema.size() != values.size())
	 {
		 assert(false);
		 err = "Number of values does not match number of columns";
		 return 0;
	 }

	// tries to find the correspondent object
	object_map_data_t::iterator iter_object = _objs_map.find(pk);
	if (iter_object == _objs_map.end())
	{
		err = "Object not found";
		return false;
	}
	
	// removes the correspondent entries
	size_t index = 0;
	for (var_object_schema::const_iterator iter = _schema.begin(); iter != _schema.end(); ++iter, ++index)
	{
		if (!values[index]._specified)
			continue;

		// gets correspondent value
		var_value val;
		vt_types mtype = cast_to_common_type(iter->_type, values[index], val, tmp);

		var_object_map_t* pv = get_v_object_pointer(mtype, iter->_is_searchable);
		if (!pv)
		{
			err = "Not enough memory";
			return false;
		}

		if (iter->_is_fuzzy_match)
		{
			_fuzzy_engine->remove((*iter_object)[index].key()._var_res._ngram._fuzzy_key, tmp);
		}

		pk_column dkey(pk, index);
		if (!pv->remove_resource((*iter_object)[index], dkey))
		{
			err = "Object not found";
			return false;
		}
		

		if (iter->_is_searchable)
		{
			//main_map_key key(val._value.strVal, &_var_factory);
			main_map_key key(val._value.strVal, 0);
			(*iter_object)[index] = pv->add_resource(key, dkey);
		}
		else if (iter->_is_fuzzy_match)
		{
			size_t ident = _fuzzy_engine->add(val._value.strVal, tmp);
			main_map_key key(ident, val._value.strVal, 0);
			(*iter_object)[index] = pv->add_resource(key, dkey);
		}
		else
		{
			//main_map_key key(mtype, val, &_var_factory);
			main_map_key key(mtype, val, 0);
			(*iter_object)[index] = pv->add_resource(key, dkey);
		}
	} // for

	return true;
}

vt_types 
var_object_repository::cast_to_common_type(vt_types type, 
										   const var_value& in, 
										   var_value& out, 
										   byte_allocator& tmp) const
{
	out._not_null = in._not_null;

	switch (type)
	{
		case vt_enum:
		case vt_unknown:
		case vt_empty:
		case vt_null: // not supported
			assert(false);
			return vt_unknown;
		case vt_bool:
			out._value.ulVal = in._not_null ? (in._value.boolVal ? 1 : 0) : 0;
			return vt_ub4;
		case vt_sb1:
			out._value.ulVal = in._not_null ? (ub4_t)in._value.cVal : 0;
			return vt_ub4;
		case vt_ub1:
			out._value.ulVal = in._not_null ? (ub4_t)in._value.bVal : 0;
			return vt_ub4;
		case vt_sb2:
			out._value.ulVal = in._not_null ? (ub4_t)in._value.iVal : 0;
			return vt_ub4;
		case vt_ub2:
			out._value.ulVal = in._not_null ? (ub4_t)in._value.uiVal : 0;
			return vt_ub4;
		case vt_sb4:
			out._value.ulVal = in._not_null ? (ub4_t)in._value.lVal : 0;
			return vt_ub4;
		case vt_ub4:
			out._value.ulVal = in._not_null ? in._value.ulVal : 0;
			return vt_ub4;
		case vt_float:
#ifdef OS_64BIT
			out._value.dblVal = (double)in._value.fltVal;
#else
			{
				double* p = (double*)tmp.allocate(sizeof(double));
				if (!p)
				{
					assert(false);
					return vt_unknown;
				}

				*p = (double)in._value.fltVal;
				out._value.dblVal = p;
			}
#endif
			return vt_double;
		case vt_double:
			out._value.dblVal = in._value.dblVal;
			return vt_double;
		case vt_sb8:
		case vt_date:
#ifdef OS_64BIT
			out._value.uintVal = (ub8_t)in._value.intVal;
#else
			if (in._value.intVal)
			{
				ub8_t* p = (ub8_t*)tmp.allocate(sizeof(ub8_t));
				if (!p)
				{
					assert(false);
					return vt_unknown;
				}

				*p = (ub8_t)*in._value.intVal;
				out._value.uintVal = p;
			}
			else
				out._value.uintVal = 0;
#endif
			return vt_ub8;
		case vt_ub8:
			out._value.uintVal = in._value.uintVal;
			return vt_ub8;
		case vt_string:
			out._value.strVal = in._value.strVal;
			return vt_string;
		case vt_wstring:
			out._value.strVal = str_template::unicode_to_multibyte(tmp, out._value.wstrVal, os_minus_one);
			return vt_string;
		case vt_decimal:
		case vt_numeric:
			out._value.bufVal = in._value.bufVal;
			return vt_numeric;
		case vt_binary:
			out._value.bufVal = in._value.bufVal;
			return vt_binary;
		case vt_guid:
			out._value.guidVal = in._value.guidVal;
			return vt_guid;
	}

	assert(false);
	return vt_unknown;
}

vt_types 
var_object_repository::map_type(vt_types type) const
{
	switch (type)
	{
		case vt_enum:
		case vt_unknown:
		case vt_empty:
		case vt_null: // not supported
			assert(false);
			return vt_unknown;
		case vt_bool:
		case vt_sb1:
		case vt_ub1:
		case vt_sb2:
		case vt_ub2:
		case vt_sb4:
		case vt_ub4:
			return vt_ub4;
		case vt_float:
		case vt_double:
			return vt_double;
		case vt_sb8:
		case vt_date:
		case vt_ub8:
			return vt_ub8;
		case vt_string:
			return vt_string;
		case vt_wstring:
			return vt_string;
		case vt_decimal:
		case vt_numeric:
			return vt_numeric;
		case vt_binary:
			return vt_binary;
		case vt_guid:
			return vt_guid;
	}

	assert(false);
	return vt_unknown;
}

var_object_map_t* 
var_object_repository::get_v_object_pointer(vt_types type, bool searchable)
{
	switch (type)
	{
		case vt_ub4:
			return &_uint32_type_map;
		case vt_double:
			return &_double_type_map;
		case vt_ub8:
			return &_uint64_type_map;
		case vt_string:
			return searchable ? &_searchable_string_type_map : &_string_type_map;
		case vt_numeric:
			return &_numeric_type_map;
		case vt_binary:
			return &_binary_type_map;
		case vt_guid:
			return &_guid_type_map;
		case vt_unknown:
		default:
			assert(false);
			return 0;
	}
}

const var_object_map_t* 
var_object_repository::get_v_object_pointer(vt_types type, bool searchable) const
{
	switch (type)
	{
		case vt_ub4:
			return &_uint32_type_map;
		case vt_double:
			return &_double_type_map;
		case vt_ub8:
			return &_uint64_type_map;
		case vt_string:
			return searchable ? &_searchable_string_type_map : &_string_type_map;
		case vt_numeric:
			return &_numeric_type_map;
		case vt_binary:
			return &_binary_type_map;
		case vt_guid:
			return &_guid_type_map;
		case vt_unknown:
		default:
			assert(false);
			return 0;
	}
}

bool 
var_object_repository::process_error(xml_designer* parser, const char* err) const
{
	parser->load(0, 0, response_dtd, strlen(response_dtd));
	parser->add_child(ELEMENT_NODE, "result", 0, false);
	parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);
	parser->add_child(ATTRIBUTE_NODE, "errDesc", err, true);
	return false;
}

///////////////////////////
// no xml funcions
//! \brief find by pk
bool
var_object_repository::select_object(size_t pk,						//!< primary key
								var_object_values& row,					//!< row values
								byte_allocator& tmp						//!< temporary allocator
			) const
{
	// try to find
	object_map_data_t::const_iterator ival = _objs_map.find(pk);
	if (ival == _objs_map.end())
		return false;

	size_t cols = ival->size();

	if (!row.resize(tmp, cols))
		return false;

	for (size_t c = 0; c < cols; ++c)
	{
		if (_schema[c]._is_searchable)
		{
			row[c]._value.strVal = (*ival)[c].key()._var_res._key._res;
			row[c]._not_null = true;
			row[c]._specified = true;
		}
		else if (_schema[c]._is_fuzzy_match)
		{
			row[c]._value.strVal = (*ival)[c].key()._var_res._ngram._res;
			row[c]._not_null = true;
			row[c]._specified = true;
		}
		else
		{
			row[c] = (*ival)[c].key()._var_res._val;
			row[c]._specified = true;
		}
	}

	return true;
}

//! non xml level
//! \brief find by unique key
bool
var_object_repository::select_object(const var_object_values& key,				//!< unique key
				var_object_values& row,					//!< row values
				byte_allocator& tmp,						//!< temporary allocator
				var_container::sorted_container_allocator_t& conall,
				size_t& pk
				) const
{
	// lookup the correspondent keys
	var_container container(conall);


	if (!find_by_row(key, container, tmp))
		return false;

	if (container._container.size() != 1)
		return false;

	pk = container._container.begin().key();

	return select_object(pk, row, tmp);
}

//! non xml level
//! \brief find by unique key
bool
var_object_repository::delete_object(const var_object_values& key,				//!< unique key
									byte_allocator& tmp,
									byte_allocator& inl,
									string_t& err
									)
{
	// lookup the correspondent keys
	var_container container(tmp);

	if (!find_by_row(key, container, tmp))
		return false;


	for (var_container::sorted_container_data_t::const_iterator it = container._container.begin(); it != container._container.end(); ++it)
	{
		inl.reset();
		if (!delete_object(it.key(), inl, err))
			return false;
	}

	return true;
}

//! \brief find pks by conditions
bool 
var_object_repository::find_by_conditions(	const xml_designer* parser,			//!< xml designer
					var_container& container,				//!< [out] container of values
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					) const
{
	return process_where(parser, container, tmp, inl, err);
}

//! \brief find pks by a single condition
bool 
var_object_repository::find_by_condition(	const xml_designer* parser,			//!< xml designer
					var_container& container,				//!< [out] container of values
					byte_allocator& tmp,					//!< temporary allocator
					byte_allocator& inl,					//!< inline allocator
					string_t& err							//!< [out] error
					) const
{
	return process_condition(parser, container, false, tmp, inl, err);
}
//! \brief find pks by foreign key
bool 
var_object_repository::find_by_row(	const var_object_values& values,		//!< row with conditions
				var_container& container,				//!< [out] container of values
				byte_allocator& tmp					//!< temporary allocator
				) const
{
	// check sizes
	if (values.size() != _schema.size())
		return false;

	// lookup the correspondent keys
	size_t index = 0;
	bool intersect = false;

	for (var_object_values::const_iterator iter = values.begin(); iter != values.end(); ++iter, ++index)
	{
		if (!iter->_specified)
			continue;

		// gets correspondent value
		var_value castval;
		vt_types casttype = cast_to_common_type(_schema[index]._type, values[index], castval, tmp);

		const var_object_map_t* pv = get_v_object_pointer(casttype, _schema[index]._is_searchable);
		if (!pv)
			return false;

		main_map_key key(casttype, castval, 0);
		pk_column min_filter(0, index);
		pk_column max_filter(~0, index);


		intersect ? 
			pv->intersect_exact_resource(key, container, min_filter, max_filter) :
			pv->find_exact_resource(key, container, min_filter, max_filter);

		intersect = true;
	} // for

	return true;
}

#pragma pack()
END_TERIMBER_NAMESPACE
