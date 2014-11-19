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

#include "xml/sxml.hpp"
#include "xml/sxs.hpp"
#include "xml/xmlmodel.h"
#include "xml/defxml.hpp"
#include "xml/declxml.hpp"
#include "xml/miscxml.h"

#include "base/common.hpp"
#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/stack.hpp"
#include "base/string.hpp"
#include "base/template.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

///////////////////////////////////////////////
xml_node::~xml_node()
{
}

xml_node::xml_node(const namedNodeDecl* decl) :
	_decl(decl)
{
}

void 
xml_node::clear()
{
	_decl = 0;
}

///////////////////////////////////////////////
xml_tree_node::xml_tree_node(const namedNodeDecl* decl, xml_tree_node* parent) : 
	xml_node(decl), _parent(parent), _left(0), _right(0)
{
}

xml_tree_node::~xml_tree_node()
{
}

void 
xml_tree_node::clear()
{
	xml_node::clear();
	_parent = 0;
	_left = 0;
	_right = 0;
}

////////////////////////////////
xml_value_node::~xml_value_node()
{
}

xml_value_node::xml_value_node(const namedNodeDecl* decl, xml_tree_node* parent) :
	xml_tree_node(decl, parent)
{
	_value.strVal = 0;
}

const char* 
xml_value_node::persist(byte_allocator& allocator_) const
{
	switch (_decl->get_type())
	{
		case ATTRIBUTE_NODE:
			return cast_to_attribute()->persist_attribute(_value, &allocator_);
		default:
			return _value.strVal;
	}
}

void 
xml_value_node::clear()
{
	xml_node::clear();
	_value.strVal = 0;
}

///////////////////////////////
// class with children elements
xml_container::xml_container(const namedNodeDecl* decl, xml_tree_node* parent) :
	xml_tree_node(decl, parent), _first_child(0), _last_child(0)
{
}

void 
xml_container::clear()
{
	xml_tree_node::clear();
	_first_child = 0;
	_last_child = 0;
}



/////////////////////////////
xml_element::xml_element(const elementDecl* decl, xml_tree_node* parent) :
	xml_container(decl, parent), _first_attr(0), _last_attr(0)
{
}

//////////////////////////////////////////////////////
xml_document::xml_document(	mem_pool_t& small_manager, mem_pool_t& big_manager,
						    size_t xml_size,
							const xml_grammar* grammar_) :
	xml_container(get_document_decl(), 0),
	xml_grammar(small_manager, big_manager, xml_size),
	_standalone(os_minus_one),
	_model_allocator(1024*64),
	_on_fly(grammar_ == 0),
	_root(0, this),
	_doc_type(&_doctype_decl, this)
{
	if (grammar_)
	{
		copy(*grammar_);
		_decl = grammar_->find_element_decl(grammar_->_doc_name);
		if (!_decl)
			exception::_throw("Invalid root element name");
	}

	// adds first node to the contatiner as doc_type
	container_reset();
}

xml_document::~xml_document()
{
}

void 
xml_document::clear()
{
	_model_map.clear();
	xml_container::clear();
	xml_grammar::clear();
	_data_allocator.clear_extra();
	_model_allocator.clear_extra();
	_tmp_allocator.clear_extra();
	_root.clear();
	// reset parent
	_root._parent = this;
	// reset doc decl
	_decl = get_document_decl();
	// clear doc typ
	_doc_type.clear();
	// reset parent & decl
	_doc_type._decl = &_doctype_decl;
	_doc_type._parent = this;

	container_reset();
	_on_fly = true;
	_standalone = os_minus_one;
}

void  
xml_document::add_escaped_symbols()
{
	bool wasAdded = false;
	entityDecl& decl_apos = add_entity_decl(str_apos, wasAdded);
	decl_apos._is_encoded_char = true;
	if (wasAdded)
		decl_apos._value = str_ch_apos;

	entityDecl& decl_quote = add_entity_decl(str_quote, wasAdded);
	decl_quote._is_encoded_char = true;
	if (wasAdded)
		decl_quote._value = str_ch_quote;

	entityDecl& decl_amp = add_entity_decl(str_amp, wasAdded);
	decl_amp._is_encoded_char = true;
	if (wasAdded)
		decl_amp._value = str_ch_amp;

	entityDecl& decl_lt = add_entity_decl(str_lt, wasAdded);
	decl_lt._is_encoded_char = true;
	if (wasAdded)
		decl_lt._value = str_ch_lt;

	entityDecl& decl_gt = add_entity_decl(str_gt, wasAdded);
	decl_gt._is_encoded_char = true;
	if (wasAdded)
		decl_gt._value = str_ch_gt;
}

content_interface* 
xml_document::find_model(const elementDecl* decl)
{
	model_map_t::iterator iter = _model_map.find(decl);
	return iter != _model_map.end() ? *iter : 0;
}

void 
xml_document::add_model(const elementDecl* decl, content_interface* model)
{
	if (_model_map.find(decl) != _model_map.end())
		xml_exception_throw("Dublicate xml model", 0);

	_model_map.insert(_model_allocator, decl, model);
}

//////////////////////////////////////////////////////////////////
void 
xml_document::validate(xml_element& el)
{
	validate_attributes(el);
	validate_children(el);
}

void 
xml_document::validate_attributes(xml_element& el)
{
	for (attribute_decl_map_t::const_iterator iter = el.cast_decl()->_attributes.begin(); iter != el.cast_decl()->_attributes.end(); ++iter)
	{
		const namedNodeDecl* itdecl = &*iter;

		switch (iter->_rule)
		{
			// dtd support
			case attributeRule_MIN:
            case attributeRule_MAX:
				xml_exception_throw("no rule, for element: ",
									(const char*)el._decl->_name,
									" and attribute: ",
									(const char*)iter->_name,
									0);
				break;
			case ATTR_RULE_DEFAULT: // must be
			case ATTR_RULE_REQUIRED:
			case ATTR_RULE_FIXED:
				{
					// tries to find correspondent attribute
					bool find = false;
					if (el.has_attributes())
					{
						for (const xml_tree_node* iterAttr = el._first_attr; iterAttr; iterAttr = iterAttr->_right)
						{
							if (iterAttr->_decl == itdecl)
							{
								find = true;
								if (iter->_atype == ATTR_TYPE_CDATA && iter->_rule == ATTR_RULE_FIXED)
								{
									terimber_xml_value val;
									val.strVal = iterAttr->cast_to_attribute()->_defval;
									if (compare_value(iterAttr->cast_to_attribute()->_ctype, xml_value_node::cast_to_node_value(&*iterAttr)->_value, val, false, false))
										xml_exception_throw("Invalid fixed attribute value, for element: ",
															(const char*)el._decl->_name,
															" and attribute: ",
															(const char*)iter->_name,
															0);
								}
								break;
							}
						}
					}

					if (!find)
						xml_exception_throw("Required attribute not found, for element: ",
											(const char*)el._decl->_name,
											" and attribute: ",
											(const char*)iter->_name,
											0);
				}
				break;
			default: // ATTR_RULE_IMPLIED
				break;
		} // switches rules
	} // for
}

void 
xml_document::validate_children(xml_element& el)
{
	// checks rules for existing elements
	switch (el.cast_decl()->_content)
	{
		case contentSpec_MIN:
		case contentSpec_MAX:
			xml_exception_throw("no rule, for element: ",
					(const char*)el._decl->_name,
					0);
			break;
		case CONTENT_EMPTY:
			if (el.has_children())
				xml_exception_throw("EMPTY rule doesn't allow children, for element: ",
					(const char*)el._decl->_name,
					0);
			break;
		case CONTENT_ANY: // can be anything
			break;
		case CONTENT_MIXED: // (#PCDATA|a|b)* | (#PCDATA)
			{
				// tries to find model
				content_interface* model = find_model(el.cast_decl());
				if (!model) // adds model
				{
					model = new (check_pointer(_model_allocator.allocate(sizeof(content_mixed)))) content_mixed(el.cast_decl()->_token, _model_allocator);
					add_model(el.cast_decl(), model);
				}

				model->validate(el);
			}
			break;
		case CONTENT_CHILDREN:
			{
				// tries to find model
				content_interface* model = find_model(el.cast_decl());
				if (!model) // adds model
				{
					model = new (check_pointer(_model_allocator.allocate(sizeof(content_children)))) content_children(el.cast_decl()->_token, _model_allocator);
					add_model(el.cast_decl(), model);
				}

				model->validate(el);
			}
			break;
		default:
			assert(false);
			break;
	}
}

//
// adds node to the list of children
//
xml_element* 
xml_document::add_element(const char* name, xml_tree_node* sibling, bool after)
{
	xml_element* retVal = 0;
	if (_container_stack.top() == this) // root ???
	{
		// it can be two way
		// internal parsing process contains the build-in DTD
		// or there are no schema at all
		if (!set_doc_name(name))
			exception::_throw("Invalid root element");

		if (!check_root())
			exception::_throw("Invalid root element");
		
		retVal = &_root;
		//add_node(retVal);
		sibling ? (after ? append_node(sibling, retVal) : insert_node(sibling, retVal)) : add_node(retVal);
	}
	else
	{
		// adds element to the parent
		xml_container* parent = _container_stack.top();
		// checks if the parent has content ANY
		xml_element* pel = xml_element::cast_to_element(parent);
		// get parent declaration
		const elementDecl* pdecl = pel->cast_decl();
		bool pany = pdecl->_content == CONTENT_ANY || pdecl->_content == CONTENT_CHILDREN && pdecl->_token->find_any_resursively();
		// allocate new object
		retVal = new(check_pointer(_data_allocator.allocate(sizeof(xml_element)))) xml_element(&add_element_decl(name, _on_fly, false, pany), parent);
		sibling ? (after ? parent->append_node(sibling, retVal) : parent->insert_node(sibling, retVal)) : parent->add_node(retVal);
		// There is exactly one element, called the root, or document element, 
		// no part of which appears in the content of any other element
		if (retVal->_decl == _root._decl)
			exception::_throw("There is exactly one element, called the root");
			//return 0;
	}

	return retVal;
}

xml_value_node* 
xml_document::add_attribute(xml_element& el, const char* name, const char* value_org, xml_tree_node* sibling, bool after)
{
	bool wasAdded = false;
	const attributeDecl& attr_decl = add_attribute_decl(*el.cast_decl(), name, !is_on_fly(), wasAdded);

	// checks the duplicate attributes
	for (const xml_tree_node* node = el._first_attr; node; node = node->_right)
		if (node->_decl == &attr_decl)
			xml_exception_throw("Dublicate attribute found: ",
								name,
								" beneath parent element: ",
								(const char*)el._decl->_name,
								0);


	xml_value_node* retVal = new(check_pointer(_data_allocator.allocate(sizeof(xml_value_node)))) xml_value_node(&attr_decl, &el);
	sibling ? (after ? el.append_attribute(sibling, retVal) :  el.insert_attribute(sibling, retVal)) : el.add_attribute(retVal);

	assign_attribute_value(el, attr_decl, retVal, value_org);
	return retVal;
}

void 
xml_document::assign_attribute_value(xml_element& el, const attributeDecl& attr_decl, xml_value_node* retVal, const char* value_org)
{
	// cuts leading and trailing white space
	size_t start_char = 0;
	//const char* value = value_org;
	const char* value = value_org;
	if (value && attr_decl._atype != ATTR_TYPE_CDATA)
	{
		while (*value == ch_space)
		{
			++start_char;
			++value;
		}

		if (*value) // is there something not space?
		{
			const char* first_samuri = value;
			const char* last_samuri = value;


			while (*first_samuri)
			{
				switch (*first_samuri)
				{
					case ch_space:
						break;
					default:
						 // shifts from the begining of original string
						last_samuri = first_samuri;
				}

				++first_samuri;
			}

			if (last_samuri + 1 != first_samuri)
			{ 
				// make a copy
				char* value_ = (char*)_tmp_allocator.allocate(last_samuri - value + 2);
				memcpy(value_, value_org, last_samuri - value + 1);
				value_[last_samuri - value + 1] = 0;

				value = value_;
			}
			else
				value = value;
		}
		//	const_cast< char* >(value)[end_char - start_char + 1] = 0;
	}

	// converts the value into the correct type
	switch (attr_decl._atype)
	{
		case attributeType_MIN:
		case attributeType_MAX:
			assert(false);
			break;
		case ATTR_TYPE_ID:
			{
				attributeRefDecl* decl = find_reference_decl(value);

				if (!decl)
					decl = &add_reference_decl(value, true);
				else if (decl->_declared)
					xml_exception_throw("Dublicate ID found: ",
										value,
										" beneath parent element: ",
										(const char*)el._decl->_name,
										0);
				else
					decl->_declared = true;


				// assigns attribute value
				retVal->_value.strVal = copy_string(value, _data_allocator, os_minus_one);

				if (!is_name_first_char(*value))
					xml_exception_throw("Illigal ID char: ",
										value,
										" beneath parent element: ",
										(const char*)el._decl->_name,
										0);
				
				const char* value_ = value + 1;
				while (*value_)
					if (!is_name_char(*value_))
						xml_exception_throw("Illigal ID char: ",
											value_,
											" beneath parent element: ",
											(const char*)el._decl->_name,
											0);
					else
						++value_;

			}
			break;
		case ATTR_TYPE_IDREF:
			{
				attributeRefDecl* decl = find_reference_decl(value);
				if (!decl)
					decl = &add_reference_decl(value, false);
				else if (!decl->_refered)
					decl->_refered = true;


				// assigns attribute value
				retVal->_value.strVal = copy_string(value, _data_allocator, os_minus_one);
			}
			break;
		case ATTR_TYPE_IDREFS:
			{
				_list< const char* > values;
				tokenValues(value, values, _tmp_allocator);
				if (values.empty())
					xml_exception_throw("Invalid IDREFS syntax found: ",
										value,
										" beneath parent element: ",
										(const char*)el._decl->_name,
										0);

				for (_list< const char* >::iterator iter = values.begin(); iter != values.end(); ++iter)
				{
					attributeRefDecl* decl = find_reference_decl(*iter);
					if (!decl)
						decl = &add_reference_decl(value, false);
					else if (!decl->_refered)
						decl->_refered = true;
				}

				// assigns attribute value
				retVal->_value.strVal = copy_string(value, _data_allocator,os_minus_one);
			}
			break;
		case ATTR_TYPE_ENTITY:
			{
				const entityDecl* decl = find_entity_decl(value);
				if (!decl)
					xml_exception_throw("Unresolved entity: ",
										value,
										" beneath parent element: ",
										(const char*)el._decl->_name,
										0);

				// checks external 
				if (decl->_is_in_subset)
					xml_exception_throw("Illigal external entity: ",
										value,
										" beneath parent element: ",
										(const char*)el._decl->_name,
										0);


				// assigns attribute value
				retVal->_value.strVal = copy_string((const char*)decl->_value, _data_allocator, os_minus_one);
			}
			break;
		case ATTR_TYPE_ENTITIES:
			{
				_list< const char* > values;
				tokenValues(value, values, _tmp_allocator); 

				if (values.empty())
					xml_exception_throw("Invalid ENTITIES syntax found: ",
										value,
										" beneath parent element: ",
										(const char*)el._decl->_name,
										0);

				// assigns attribute value
				string_t strDummy(&_tmp_allocator);
				char str_ch_space[2] = {ch_space, ch_null};

				for (_list< const char* >::const_iterator iter = values.begin(); iter != values.end(); ++iter)
				{
					const entityDecl* decl = find_entity_decl(*iter);
					if (!decl)
						xml_exception_throw("Unresolved entity: ",
											*iter,
											" beneath parent element: ",
											(const char*)el._decl->_name,
											0);

					if (decl->_is_in_subset)
						xml_exception_throw("Illigal external entity: ",
											*iter,
											" beneath parent element: ",
											(const char*)el._decl->_name,
											0);

		
					if (strDummy.length())
                        strDummy += str_ch_space;

					strDummy += decl->_value;
				}
				
				retVal->_value.strVal = copy_string((const char*)strDummy, _data_allocator, os_minus_one);
			}
			break;
		case ATTR_TYPE_NMTOKEN:
			// checks legal value chars
			{
				const char* value_ = value;
				while (*value_)
					if (!is_name_char(*value_))
						xml_exception_throw("Illigal token char: ",
											value_,
											" beneath parent element: ",
											(const char*)el._decl->_name,
											0);
					else
						++value_;

				retVal->_value.strVal = copy_string(value, _data_allocator, os_minus_one);
			}
			break;
		case ATTR_TYPE_NMTOKENS:
			{
				_list< const char* > values;
				tokenValues(value, values, _tmp_allocator); 

				if (values.empty())
					xml_exception_throw("Invalid NMTOKENS syntax found: ",
										value,
										" beneath parent element: ",
										(const char*)el._decl->_name,
										0);

				// assigns attribute value
				string_t strDummy(&_tmp_allocator);
				char str_ch_space[2] = {ch_space, ch_null};

				for (_list< const char* >::const_iterator iter = values.begin(); iter != values.end(); ++iter)
				{
					if (strDummy.length())
                        strDummy += str_ch_space;

					const char* value_ = *iter;
					while (*value_)
						if (!is_name_char(*value_))
							xml_exception_throw("Illigal token char: ",
												value_,
												" beneath parent element: ",
												(const char*)el._decl->_name,
												0);
						else
							++value_;

					strDummy += *iter;
				}
				
				
				retVal->_value.strVal = copy_string((const char*)strDummy, _data_allocator, os_minus_one);
			}
			break;
		case ATTR_TYPE_NOTATION:
			{
				const notationDecl* decl = find_notation_decl(retVal->_decl->_name);
				if (!decl)
					xml_exception_throw("Can't find notation: ",
										(const char*)retVal->_decl->_name,
										" beneath parent element: ",
										(const char*)el._decl->_name,
										0);
			}
			// continues to search as an enumaration
		case ATTR_TYPE_ENUMERATION:
			{
				bool find = false;
				for (_list< enumNodeDecl >::const_iterator iter = retVal->cast_to_attribute()->_enum.begin(); iter != retVal->cast_to_attribute()->_enum.end(); ++iter)
				{
					if (!strcmp(iter->_value, value))
					{
						retVal->_value.lVal = (sb4_t)iter->_id;
						find = true;
						break;
					}
				}

				if (!find)
					xml_exception_throw("Invalid enumaration value: ",
										value,
										" beneath parent element: ",
										(const char*)el._decl->_name,
										0);
			}
			break;
		default: // CDATA
			// value needs to be converted there
			retVal->_value = parse_value(retVal->cast_to_attribute()->_ctype, value, os_minus_one, &_data_allocator);
			break;
	} // switch
}

void 
xml_document::update_attribute(xml_element* el, xml_value_node* att, const char* value)
{
	const attributeDecl& attr_decl = *att->cast_to_attribute();
	assign_attribute_value(*el, attr_decl, att, value);
}

void 
xml_document::add_def_attributes(xml_element& el, attr_states_map_t& attrStates)
{
	for (attr_states_map_t::iterator iter = attrStates.begin(); iter != attrStates.end(); ++iter)
	{
		if (!*iter) // absent
		{
			const attributeDecl* attr_decl = iter.key();
			switch (attr_decl->_rule)
			{
				case ATTR_RULE_DEFAULT:
				case ATTR_RULE_FIXED:
					*iter = add_attribute(el, attr_decl->_name, attr_decl->_defval);
					break;
				case ATTR_RULE_REQUIRED:
					if (attr_decl->_defval.length()) // we have default value
						*iter = add_attribute(el, attr_decl->_name, attr_decl->_defval);
					break;
				default:
					break;
			}
		}
	}
}

// searches functions
terimber_xml_value 
xml_document::find_attribute_value(const xml_element& el, const char* name) const
{
	const attributeDecl* decl = find_attribute_decl(*el.cast_decl(), name);
	for (const xml_tree_node* iter = el._first_attr; iter; iter = iter->_right)
		if (iter->_decl == decl)
			return xml_value_node::cast_to_node_value(iter)->_value;

	return terimber_xml_value();
}

xml_value_node* 
xml_document::find_attribute(const xml_element& el, const char* name)
{
	const namedNodeDecl* decl = find_attribute_decl(*el.cast_decl(), name);
	for (xml_tree_node* iter = el._first_attr; iter; iter = iter->_right)
		if (iter->_decl == decl)
			return xml_value_node::cast_to_node_value(iter);

	return 0;
}

	
bool 
xml_document::set_doc_name(const char* name)
{ 
	if (_on_fly && _root._decl) // we already assigned
		return false;

	if (_on_fly || !_doc_name.length())
		_doc_name = name; 
	else
		return _doc_name == name;

	return true;
}

bool 
xml_document::check_root()
{
	if (_on_fly)
	{
		// can be in schema
		_root._decl = find_element_decl(_doc_name);
		if (!_root._decl)
			_root._decl = &add_element_decl(_doc_name, true, false, false);
	}
	else
	{
		_root._decl = find_element_decl(_doc_name);
		if (!_root._decl)
			return false;
	}

	return true;
}

void 
xml_document::clear_root() 
{ 
	_root.clear(); 
	_root._parent = this;
	_doc_name = 0; 
}

void 
xml_document::container_reset()
{
	_stack_allocator.clear_extra();
	_container_stack.clear();
	_container_stack.push(_stack_allocator, this);
}

void 
xml_document::container_push(xml_element* el)
{
	_container_stack.push(_stack_allocator, el);
}

void 
xml_document::container_start_doctype()
{
	_container_stack.push(_stack_allocator, &_doc_type);
	add_node(&_doc_type);
}

void 
xml_document::container_stop_doctype()
{
	container_pop();
	_on_fly = false;
}

xml_element* 
xml_document::container_pop()
{
	xml_container* el = _container_stack.top(); 
	if (el->_decl == _decl)
		return 0;
	else
	{
		_container_stack.pop(_stack_allocator);
		return static_cast< xml_element* >(el);
	}
}

bool 
xml_document::container_peak()
{
	xml_container* el = _container_stack.top(); 
	return el->_decl != _decl;
}


#pragma pack()
END_TERIMBER_NAMESPACE

