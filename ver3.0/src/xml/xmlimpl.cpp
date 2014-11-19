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


#include "xml/xmlimpl.hpp"
#include "xml/miscxml.hpp"
#include "xml/parsexml.hpp"
#include "xml/mngxml.hpp"
#include "xml/storexml.hpp"
#include "xml/declxml.hpp"
#include "xml/sxml.hpp"
#include "xml/sxs.hpp"
#include "xml/persxml.h"
#include "xml/dtdxml.h"

#include "base/common.hpp"
#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/stack.hpp"
#include "base/template.hpp"
#include "base/keymaker.h"
#include "base/string.hpp"

#include "ossock.h"

//////////////////////////////////////////////////////////
xml_factory::xml_factory()
{
	// inits socket library
	_sockStartup();
}

xml_factory::~xml_factory()
{
	_sockCleanup();
}

xml_designer* 
xml_factory::get_xml_designer(size_t xml_size)
{
	return new terimber::xml_designer_impl(xml_size);
}

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

///////////////////////////////////////////////////////
xml_designer_impl::xml_designer_impl(size_t block_size) :
_xml_size(block_size <= os_def_size ? os_def_size : big_xml_size), _doc(_small_manager, _big_manager, block_size <= os_def_size ? os_def_size : big_xml_size, 0)
{
	_cur_node = &_doc;
	_tmp_allocator = _small_manager.loan_object();
}

// 
xml_designer_impl::~xml_designer_impl()
{
	_small_manager.return_object(_tmp_allocator);
}

// loads/saves xml from/to memory/file
bool 
xml_designer_impl::load(const char* name, const char* grammar)
{
	byte_source* in_xml = 0; 
	byte_source* in_grammar = 0;

	stream_input_common stream_xml(_small_manager, _big_manager, _xml_size, false);
	if (name && name[0])
	{
		xml_stream_attribute attr(name, true);
		if (!stream_xml.open(attr))
		{
			_error = "Can't open file ";
			_error += name;
			return false;
		}

		in_xml = &stream_xml;
	}

	stream_input_common stream_grammar(_small_manager, _big_manager, 0, false);
	if (grammar && grammar[0])
	{
		xml_stream_attribute attr(grammar, true);
		if (!stream_grammar.open(attr))
		{
			_error = "Can't open file ";
			_error += grammar;
			return false;
		}

		in_grammar = &stream_grammar;
	}

	return _load(in_xml, in_grammar);
}

bool 
xml_designer_impl::load(const void* name, size_t length, const char* grammar)
{
	byte_source* in_xml = 0;
	byte_source* in_grammar = 0;

	stream_input_memory stream_xml((const ub1_t*)name, length, _small_manager, _big_manager, _xml_size, false);	
	if (name && length)
		in_xml = &stream_xml;

	stream_input_common stream_grammar(_small_manager, _big_manager, 0, false);
	if (grammar && grammar[0])
	{
		xml_stream_attribute attr(grammar, true);
		if (!stream_grammar.open(attr))
		{
			_error = "Can't open file ";
			_error += grammar;
			return false;
		}

		in_grammar = &stream_grammar;
	}

	return _load(in_xml, in_grammar);
}
bool 
xml_designer_impl::load(const char* name, const void* grammar, size_t grammar_length)
{
	byte_source* in_xml = 0;
	byte_source* in_grammar = 0;

	stream_input_common stream_xml(_small_manager, _big_manager, _xml_size, false);
	if (name && name[0])
	{
		xml_stream_attribute attr(name, true);
		if (!stream_xml.open(attr))
		{
			_error = "Can't open file ";
			_error += name;
			return false;
		}

		in_xml = &stream_xml;
	}

	stream_input_memory stream_grammar((const ub1_t*)grammar, grammar_length, _small_manager, _big_manager, 0, false);	
	if (grammar && grammar_length)
		in_grammar = &stream_grammar;

	return _load(in_xml, in_grammar);
}

bool 
xml_designer_impl::load(const void* name, size_t length, const void* grammar, size_t grammar_length)
{
	byte_source* in_xml = 0;
	byte_source* in_grammar = 0;

	stream_input_memory stream_xml((const ub1_t*)name, length, _small_manager, _big_manager, _xml_size, false);	
	if (name && length)
		in_xml = &stream_xml;

	stream_input_memory stream_grammar((const ub1_t*)grammar, grammar_length, _small_manager, _big_manager, 0, false);	
	if (grammar && grammar_length)
		in_grammar = &stream_grammar;

	return _load(in_xml, in_grammar);
}

const char* 
xml_designer_impl::error() const
{ 
	return _error; 
}

// resets the navigator to the document
void 
xml_designer_impl::select_document() const
{
	_cur_node = const_cast< xml_document* >(&_doc);
}

// resets the navigator to the root element
bool 
xml_designer_impl::select_root() const
{
	// check the root presence
	if (!_doc.get_root_element()._decl)
		return false;

	_cur_node = const_cast< xml_element* >(&_doc.get_root_element());
	return true;
}

// navigates to the next level
bool 
xml_designer_impl::has_children() const
{
	return is_container() && xml_container::cast_to_container(_cur_node)->has_children();
}

bool 
xml_designer_impl::select_first_child() const
{
	if (!has_children())
		return false;

	_cur_node = xml_container::cast_to_container(_cur_node)->_first_child;
	return true;
}

bool 
xml_designer_impl::select_last_child() const
{
	if (!has_children())
		return false;

	_cur_node = xml_container::cast_to_container(_cur_node)->_last_child;
	return true;
}

// navigates through the same level
bool 
xml_designer_impl::select_next_sibling() const
{
	if (!_cur_node->_right)
		return false;

	_cur_node = _cur_node->_right;
	return true;
}


bool 
xml_designer_impl::select_prev_sibling() const
{
	if (!_cur_node->_left)
		return false;

	_cur_node = _cur_node->_left;
	return true;
}

// navigates up a level
bool 
xml_designer_impl::select_parent() const
{
	if (!_cur_node->_parent)
		return false;

	_cur_node = _cur_node->_parent;
	return true;
}

// attribute navigation
bool 
xml_designer_impl::has_attributes() const
{
	return is_element() && xml_element::cast_to_element(_cur_node)->has_attributes();
}


bool 
xml_designer_impl::select_first_attribute() const
{
	if (!has_attributes())
		return false;

	_cur_node = xml_element::cast_to_element(_cur_node)->_first_attr;
	return true;
}

bool 
xml_designer_impl::select_last_attribute() const
{
	if (!has_attributes())
		return false;

	_cur_node = xml_element::cast_to_element(_cur_node)->_last_attr;
	return true;
}

bool 
xml_designer_impl::select_attribute_by_name(const char* name) const
{
	if (!has_attributes())
		return false;

	// finds decl of attribute
	const namedNodeDecl* decl = _doc.find_attribute_decl(*xml_element::cast_to_element(_cur_node)->cast_decl(), name);
	if (!decl)
		return false;
	for (xml_tree_node* attr = xml_element::cast_to_element(_cur_node)->_first_attr; attr; attr = attr->_right)
		if (attr->_decl == decl)
		{
			_cur_node = attr;
			return true;
		}

	return false;
}

// where we are?
xmlNodeType
xml_designer_impl::get_type() const
{
	return get_cur_type();
}

const char* 
xml_designer_impl::get_name() const
{
	switch (get_cur_type())
	{
		case DOCUMENT_NODE:
		case DOCUMENT_TYPE_NODE:
			return _doc.get_doc_name();
		case ELEMENT_NODE:
		case ATTRIBUTE_NODE:
		case PROCESSING_INSTRUCTION_NODE:
			return _cur_node->_decl->_name;
		default:
			return 0;
	}
}

const char* 
xml_designer_impl::get_value() const
{
	_tmp_allocator->reset();
	switch (get_cur_type())
	{
		case ATTRIBUTE_NODE:
			// attribute value needs to be converted to string
			return _cur_node->cast_to_attribute()->persist_attribute(xml_value_node::cast_to_node_value(_cur_node)->_value, _tmp_allocator);
		case TEXT_NODE:
		case PROCESSING_INSTRUCTION_NODE:
		case CDATA_SECTION_NODE:
		case COMMENT_NODE:
			return persist_value(vt_string, xml_value_node::cast_to_node_value(_cur_node)->_value, _tmp_allocator);
		default:
			return 0;
	}
}

//
// returns the xpath (a/b/c format, where a root element c is current element) of the current node
// NB!!! value is allocated on a temporary allocator
// char pointer will be valid until next function call only
//
const char* 
xml_designer_impl::get_xpath() const
{
	if (!is_element())
		return 0;

	_tmp_allocator->reset();
    _list< const string_t* > tmp_store; 
	const xml_tree_node* node = _cur_node;
	const xml_tree_node* doc = &_doc;

	while (node != doc)
	{
		tmp_store.push_front(*_tmp_allocator, &node->_decl->_name);
		node = node->_parent;
	}

	// restores path
	size_t len = 0;
	for (_list< const string_t* >::const_iterator iter = tmp_store.begin(); iter != tmp_store.end(); ++iter)
		len += (len ? 1 : 0) + (*iter)->length();

	// allocates length
	char* dest = (char*)_tmp_allocator->allocate(len + 1);
	dest[len] = 0;
	size_t shift = 0;
	for (_list< const string_t* >::const_iterator iter1 = tmp_store.begin(); iter1 != tmp_store.end(); ++iter1)
	{
		if (shift)
			dest[shift++] = ch_forward_slash;

		memcpy(dest + shift, (const char*)**iter1, (*iter1)->length());
		shift += (*iter1)->length();
	}

	return dest;
}
//
// global navigation looking for element specified according to format (a/b/c)
//
bool 
xml_designer_impl::select_xpath(const char* path) const
{
	if (!path)
		return false;

	_tmp_allocator->reset();
	// constructs list of token
	_list< const elementDecl* > tmp_store;
	const char* path_ = path;
	string_t tmp_name(0, _tmp_allocator);
	size_t length = 0;
	const elementDecl* decl = 0;
	xml_tree_node* node = 0;

	while (*path_)
	{
		if (*path_ == ch_forward_slash || *path_ == ch_back_slash)
		{
			tmp_name.assign(path, length);
			decl = _doc.find_element_decl(tmp_name);
			if (!decl)
			{
				_error = "Can't find element ";
				_error += tmp_name;
				return false;
			}

			tmp_store.push_back(*_tmp_allocator, decl);
			path += length + 1;
			path_ = path;
			length = 0;
		}
		else
			++length, ++path_;
	}

	if (length)
	{
		tmp_name.assign(path, length);
		decl = _doc.find_element_decl(tmp_name);
		if (!decl)
		{
			_error = "Can't find element ";
			_error += tmp_name;
			return false;
		}

        tmp_store.push_back(*_tmp_allocator, decl);
	}
	

    node = _cur_node;
	if (!tmp_store.empty() && node->_decl != tmp_store.front())
	{
		_error = "Invalid path";
		return false;
	}

	tmp_store.pop_front();
	while (!tmp_store.empty())
	{
		// looks for children
		for (node = static_cast< const xml_element* >(node)->_first_child; node; node = node->_right)
		{
			if (node->_decl->get_type() != ELEMENT_NODE)
				continue;

			if (node->_decl == tmp_store.front())
				break;
		} // for

		if (!node)
		{
			_error = "Invalid path";
			return false;
		}

		tmp_store.pop_front();
	}
	// while


	_cur_node = node;
	return true;
}

bool 
xml_designer_impl::save(const char* name, bool add_doc_type) const
{
	stream_output_file stream(_small_manager, _big_manager, _xml_size);
	if (!stream.open(name))
	{
		_error = "Can't open file ";
		_error += name;
		return false;
	}

	// persists xml document
	xml_persistor pr(stream, _doc, _small_manager, _big_manager, false, add_doc_type, _xml_size);
	if (!pr.persist())
	{
		stream.close();
		_error = pr.get_error();
		return false;
	}

	stream.close();
	return true;
}

bool 
xml_designer_impl::save(void* buffer, size_t& length, bool add_doc_type) const
{
	memory_output_stream stream(_small_manager, _big_manager, _xml_size, (ub1_t*)buffer, length);
	// persists xml document
	xml_persistor pr(stream, _doc, _small_manager, _big_manager, false, add_doc_type, _xml_size);
	if (!pr.persist() || stream.is_overflow())
	{
		_error = pr.get_error();
		length = stream.get_required_size();
		return false;
	}

	// saves input length
	size_t length_ = length;
	length = stream.get_required_size();

	return length_ >= length;
}

// node management
// element, attribute - name == name
// text, cdata, comment - name is ignored
// processing instruction - name == target
bool 
xml_designer_impl::add_child(xmlNodeType type, const char* name, const char* value, bool dont_move)
{
	return _import_node(type, name, value, false, false, dont_move);
}

bool 
xml_designer_impl::insert_sibling(xmlNodeType type, const char* name, const char* value, bool dont_move)
{
	return _import_node(type, name, value, true, false, dont_move);
}

bool 
xml_designer_impl::append_sibling(xmlNodeType type, const char* name, const char* value, bool dont_move)
{
	return _import_node(type, name, value, true, true, dont_move);
}

bool 
xml_designer_impl::remove_node()
{
	// removes the current node
	switch (_cur_node->_decl->get_type())
	{
		case DOCUMENT_NODE:
		case DOCUMENT_TYPE_NODE:
			// can't remove
			_error = "Can't remove specified node";
			return false;
		case ATTRIBUTE_NODE:
			_cur_node = xml_element::cast_to_element(_cur_node->_parent)->remove_attribute(_cur_node);
			break;
		case ELEMENT_NODE:
			// checks if it's a root
			if (_cur_node->_parent->_decl == _doc._decl) // root
				// clears root
				_doc.clear_root();
			
				
			_cur_node = xml_container::cast_to_container(_cur_node->_parent)->remove_node(_cur_node);
			break;
		default: // removes as usual
			_cur_node = xml_container::cast_to_container(_cur_node->_parent)->remove_node(_cur_node);
			break;
	} // switch
	return true;
}

// attribute value == value -- check enumaration
// processing instruction value = text
// cdata, text, comment value == text
bool 
xml_designer_impl::update_value(const char* value)
{
	try
	{
		switch (_cur_node->_decl->get_type())
		{
			case ATTRIBUTE_NODE:
				_doc.update_attribute(xml_element::cast_to_element(_cur_node->_parent), xml_value_node::cast_to_node_value(_cur_node), value);
				break;
			case PROCESSING_INSTRUCTION_NODE:
			case TEXT_NODE:
			case CDATA_SECTION_NODE:
			case COMMENT_NODE:
				xml_value_node::cast_to_node_value(_cur_node)->_value.strVal = copy_string(value, _doc.get_data_allocator(), os_minus_one);
				break;
			default:
				_error = "Specified type is not supported";
				return false;
		}
	}
	catch (exception& x)
	{
		_error = x.what();
		return false;
	}

	return true;
}

// import xml fragment as a child of xpath element
bool 
xml_designer_impl::merge(const xml_designer* nav, bool recursive)
{
	switch (_cur_node->_decl->get_type())
	{
		case DOCUMENT_NODE:
			// checks if the current navigator is empty
			// otherwise we will have two roots
			if (_doc.get_root_element()._decl)
			{
				_error = "The final document would have two root elements";
				return false;
			}

			// 
			if (nav->get_type() != ELEMENT_NODE)
			{
				_error = "Only element type is allowed when current documet is empty";
				return false;
			}

		case ELEMENT_NODE:

			// this->cur, nav->cur
			if (!add_child(nav->get_type(), nav->get_name(), nav->get_value(), false))
				return false;

			// this->cur/root, nav->cur
			if (nav->get_type() == ELEMENT_NODE)
			{
				// imports attributes
				if (nav->select_first_attribute())
				{
					// this->cur/root, nav->cur/attr1
					do
					{
						// add attribute
						if (!add_child(nav->get_type(), nav->get_name(), nav->get_value(), true))
							return false;
						// this->cur/root/attr1, nav->cur/attr1

						// returns to the element
						// select_parent();
						// this->cur/root, nav->cur/attr1
					}
					while (nav->select_next_sibling());

					// this->cur/root, nav->cur/attrN

					// restores navigator to the current element
					nav->select_parent();
					// this->cur/root, nav->cur
				} // ends import attributes
			
				// imports children
				if (recursive && nav->select_first_child())
				{			
					// this->cur/root, nav->cur/el1
					do
					{
						// this->cur/root, nav->cur/el1
						if (!merge(nav, true))
							return false;
						// this->cur/root, nav->cur/el1
					}
					while (nav->select_next_sibling());

					// this->cur/root, nav->cur/elN
					// restores navigator to the current element
					nav->select_parent();
					// this->cur/root, nav->cur
					// restores cur node
					//select_parent();
				}			
			}

			// restores cur node
			select_parent();
			// this->cur, nav->cur
			break;
		default:
			_error = "Specified type is not supported";
			return false;
	};
	
	return true;
}

//
// validates the xml node accorging to the current grammar
//
bool
xml_designer_impl::validate(bool recursive)
{
	if (!is_element())
	{
		_error = "Only element can be validated";
		return false;
	}

	try
	{
		_validate(xml_element::cast_to_element(_cur_node), recursive);
	}
	catch (exception& x)
	{
		_error = x.what();
		return false;
	}

	return true;
}

/////////////////////////////////////////////
void 
xml_designer_impl::_validate(xml_element* el, bool recursive)
{
	// checks the current element
	_doc.validate(*el);
	// checks children
	if (recursive)
	{
		for (xml_tree_node* child = el->_first_child; child; child = child->_right)
			if (child->_decl->get_type() != ELEMENT_NODE)
				continue;
			else
				_validate(xml_element::cast_to_element(child), recursive);
	}
}

bool 
xml_designer_impl::_import_node(xmlNodeType type, const char* name, const char* value, bool sibling, bool after, bool dont_move)
{
	// sibling == false if add_node
	// sibling == true if insert/append
	// if current node is document the siblings are not allowed
	if (sibling && _cur_node->_decl->get_type() == DOCUMENT_NODE)
	{
		_error = "Document node can't have siblings";
		return false;
	}

	// saves the previous current
	xml_tree_node* save_cur_node = _cur_node;

	xmlNodeType xmltype = sibling ? _cur_node->_parent->_decl->get_type() : _cur_node->_decl->get_type();
	xml_tree_node* cur_node = sibling ? _cur_node->_parent : _cur_node;
	xml_tree_node* cur_sibling = sibling ? _cur_node : 0;
	char* dest = 0;

	try
	{
		switch (type)
		{
			case DOCUMENT_NODE:
			case DOCUMENT_TYPE_NODE:
				// documents node and doctype can be added while automatic parsing
				return false;
			case ELEMENT_NODE:
				switch (xmltype)
				{
					case DOCUMENT_NODE:
						// attempts add root element
						_cur_node = _doc.add_element(name, cur_sibling, after);
						return true;
					case ELEMENT_NODE:
						_doc.container_push(static_cast< xml_element* >(cur_node));
						_cur_node = _doc.add_element(name, cur_sibling, after);
						break;
					default:
						if (sibling)
						{
							_doc.container_push(static_cast< xml_element* >(cur_node));
							_cur_node = _doc.add_element(name, cur_sibling, after);
						}
						else
						{
							_error = "Specified type is not supported";
							return false;
						}
						break;
				} // switch
				break;
			case ATTRIBUTE_NODE:
				if (xmltype != ELEMENT_NODE)
				{
					_error = "Attribute node is allowed only beneath element";
					return false;
				}
				_doc.container_push(static_cast< xml_element* >(cur_node));
				_cur_node = _doc.add_attribute(*static_cast< xml_element* >(cur_node), name, value, cur_sibling, after);
				break;
			case TEXT_NODE:
				if (xmltype != ELEMENT_NODE) 
				{
					_error = "Text node is allowed only beneath element";
					return false;
				}
				_doc.container_push(static_cast< xml_element* >(cur_node));
				_cur_node = _doc.add_text(value, cur_sibling, after);
				break;
			case CDATA_SECTION_NODE:
				if (xmltype != ELEMENT_NODE)
				{
					_error = "Cdata section node is allowed only beneath element";
					return false;
				}

				_doc.container_push(static_cast< xml_element* >(cur_node));
				_cur_node = _doc.add_cdata(value, cur_sibling, after);
				break;
			case PROCESSING_INSTRUCTION_NODE:
				if (xmltype != ELEMENT_NODE && xmltype != DOCUMENT_NODE)
				{
					_error = "Processing instruction node is allowed either beneath element or document node";
					return false;
				}
				_doc.container_push(static_cast< xml_element* >(cur_node));
				_cur_node = _doc.add_pi(name, value, cur_sibling, after);
				break;
			case COMMENT_NODE:
				if (xmltype != ELEMENT_NODE && xmltype != DOCUMENT_NODE)
				{
					_error = "Comment node is allowed either beneath element or document node";
					return false;
				}
				_doc.container_push(static_cast< xml_element* >(cur_node));
				_cur_node = _doc.add_comment(value, cur_sibling, after);
				break;
			default:
				assert(false);
				_error = "Specified type is not supported";
				return false;
		} // switch
	}
	catch (exception& x)
	{
		_doc.container_pop();
		_error = x.what();
		return false;
	}

	_doc.container_pop();

	if (dont_move)
		_cur_node = save_cur_node;

	return true;
}

bool 
xml_designer_impl::_load(byte_source* stream, byte_source* grammar)
{
	_doc.clear();
	_doc.add_escaped_symbols();
	_cur_node = &_doc;

	if (grammar)
	{
		// external
		dtd_processor dtd(*grammar, _doc, _small_manager, _big_manager, 0);
		try
		{
			_doc.container_start_doctype();
			dtd.parse();
			_doc.container_stop_doctype();
		}
		catch (exception& x)
		{
			_doc.clear();
			_doc.add_escaped_symbols();
			_error = x.what();
			return false;
		}
	}

	if (stream)
	{
		xml_processor pr(*stream, _doc, _small_manager, _big_manager, _xml_size, false);
		if (!pr.parse())
		{
			_doc.clear();
			_doc.add_escaped_symbols();
			_error = pr.get_error();
			return false;
		}
	}

	return true;
}


///////////////////////////////////////////////////////////
xml_parser_creator::xml_parser_creator() 
{
}

xml_designer* 
xml_parser_creator::create(size_t n) 
{ 
	return _factory.get_xml_designer(n); 
}

void 
xml_parser_creator::destroy(xml_designer* obj) 
{ 
	if (obj) 
		delete obj; 
}

////////////////////////////////////////////
// static 
xml_designer* 
xml_designer_creator::create(size_t size)
{
	xml_factory acc;
	return acc.get_xml_designer(size);
}

//static 
void 
xml_designer_creator::activate(xml_designer* obj, size_t)
{
	obj->load(0, 0, 0, 0);
}


#pragma pack()
END_TERIMBER_NAMESPACE


