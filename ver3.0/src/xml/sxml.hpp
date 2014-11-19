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

#ifndef _terimber_sxml_hpp_
#define _terimber_sxml_hpp_

#include "xml/sxml.h"
#include "base/except.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

/////////////////////////////
// base class for all xml nodes (attributes, elements, ...)
xml_forceinline
const attributeDecl* 
xml_node::cast_to_attribute() const 
{ 
	assert(_decl->get_type() == ATTRIBUTE_NODE); 
	return static_cast< const attributeDecl* >(_decl); 
}

/////////////////////////////////////////////////////////////
//
// type cast
//
xml_forceinline
const xml_value_node* 
xml_value_node::cast_to_node_value(const xml_tree_node* x)
{ 
	switch (x->_decl->get_type())
	{
		case TEXT_NODE:
		case PROCESSING_INSTRUCTION_NODE:
		case CDATA_SECTION_NODE:
		case COMMENT_NODE:
		case ATTRIBUTE_NODE:
			return static_cast< const xml_value_node* >(x);
		default:
			assert(false);
			return 0;
	}
}

//
// type cast
//
xml_forceinline
xml_value_node* 
xml_value_node::cast_to_node_value(xml_tree_node* x)
{ 
	switch (x->_decl->get_type())
	{
		case TEXT_NODE:
		case PROCESSING_INSTRUCTION_NODE:
		case CDATA_SECTION_NODE:
		case COMMENT_NODE:
		case ATTRIBUTE_NODE:
			return static_cast< xml_value_node* >(x);
		default:
			assert(false);
			return 0;
	}
}

////////////////////////////////////////////////////////////////////////
//
// adds node to the end of children
//
xml_forceinline
xml_tree_node* 
xml_container::add_node(xml_tree_node* node)
{ 
	// this is the new last node
	node->_right = 0;
	node->_left = _last_child;
	// if not the first node
	return _last_child = (_first_child ? _last_child->_right : _first_child) = node;
}

xml_forceinline
xml_tree_node*
xml_container::append_node(xml_tree_node* after, xml_tree_node* node)
{
	// acts like a static function
	node->_right = after->_right;
	node->_left = after;
	return after->_right = (after->_right ? after->_right->_left : _last_child) = node;
}

//
// adds node before specified node
//
xml_forceinline
xml_tree_node*
xml_container::insert_node(xml_tree_node* before, xml_tree_node* node)
{
	// acts like a static function
	node->_left = before->_left;
	node->_right = before;
	return before->_left = (before->_left ? before->_left->_right : _first_child) = node;
}

xml_forceinline
xml_tree_node* 
xml_container::remove_node(xml_tree_node* node)
{
	(node->_left ? node->_left->_right : _first_child) = node->_right;
	(node->_right ? node->_right->_left : _last_child) = node->_left;
	// after removing, the current node will be chosen
	// according to the follow priority
	// 1. right node 
	// 2. left node
	// 3. parent node
	return node->_right ? node->_right : (node->_left ? node->_left : node->_parent);
}

//
// checks children
//
xml_forceinline
bool 
xml_container::has_children() const 
{ 
	return _first_child != 0; 
}

xml_forceinline
bool 
xml_container::is_container() const 
{ 
	switch (_decl->get_type())
	{
		case ELEMENT_NODE:
		case DOCUMENT_NODE:
			return true;
		default:
			return false;
	}
}

//
// type cast
//
xml_forceinline
const xml_container* 
xml_container::cast_to_container(const xml_tree_node* x)
{ 
#ifdef _DEBUG
	switch (x->_decl->get_type())
	{
		case ELEMENT_NODE:
		case DOCUMENT_NODE:
#endif
			return static_cast< const xml_container* >(x);
#ifdef _DEBUG
		default:
			assert(false);
			return 0;
	}
#endif
}

//
// type cast
//
xml_forceinline
xml_container* 
xml_container::cast_to_container(xml_tree_node* x)
{ 
#ifdef _DEBUG
	switch (x->_decl->get_type())
	{
		case ELEMENT_NODE:
		case DOCUMENT_NODE:
#endif
			return static_cast< xml_container* >(x);
#ifdef _DEBUG
		default:
			assert(false);
			return 0;
	}
#endif
}

////////////////////////////////////////////////////////////////
xml_forceinline 
bool 
xml_element::has_attributes() const 
{ 
	return _first_attr != 0; 
}


//
// cast declaration
//
xml_forceinline
const elementDecl* 
xml_element::cast_decl() const 
{ 
	return static_cast< const elementDecl* >(_decl); 
}

//
// clears element
//
xml_forceinline
void 
xml_element::clear()
{
	xml_container::clear();
	_first_attr = 0;
	_last_attr = 0;
}

//
//
//
xml_forceinline
const xml_element* 
xml_element::cast_to_element(const xml_tree_node* x)
{ 
#ifdef _DEBUG
	switch (x->_decl->get_type())
	{
		case ELEMENT_NODE:
#endif
			return static_cast< const xml_element* >(x);
#ifdef _DEBUG
		default:
			assert(false);
			return 0;
	}
#endif
}

xml_forceinline
xml_element* 
xml_element::cast_to_element(xml_tree_node* x)
{ 
#ifdef _DEBUG
	switch (x->_decl->get_type())
	{
		case ELEMENT_NODE:
#endif
			return static_cast< xml_element* >(x);
#ifdef _DEBUG
		default:
			assert(false);
			return 0;
	}
#endif
}

xml_forceinline
xml_tree_node* 
xml_element::add_attribute(xml_tree_node* node)
{ 
	// this is the new last node
	node->_right = 0;
	node->_left = _last_attr;
	// if not the first node
	return _last_attr = (_first_attr ? _last_attr->_right : _first_attr) = node;
}

//
// adds node after specified node
//
xml_forceinline
xml_tree_node* 
xml_element::append_attribute(xml_tree_node* after, xml_tree_node* node)
{
	// acts like a static function
	node->_right = after->_right;
	node->_left = after;
	return after->_right = (after->_right ? after->_right->_left : _last_attr) = node;
}

//
// adds node before specified node
//
xml_forceinline
xml_tree_node*
xml_element::insert_attribute(xml_tree_node* before, xml_tree_node* node)
{
	// acts like a static function
	node->_left = before->_left;
	node->_right = before;
	return before->_left = (before->_left ? before->_left->_right : _first_attr) = node;
}

xml_forceinline
xml_tree_node* 
xml_element::remove_attribute(xml_tree_node* node)
{
	(node->_left ? node->_left->_right : _first_attr) = node->_right;
	(node->_right ? node->_right->_left : _last_attr) = node->_left;
	// after removing, the current node will be chosen
	// according to the follow priority
	// 1. right node 
	// 2. left node
	// 3. parent node
	return node->_right ? node->_right : (node->_left ? node->_left : node->_parent);
}

////////////////////////////////////////////////////////////////
xml_forceinline 
byte_allocator&	
xml_document::get_data_allocator()
{ 
	return _data_allocator; 
}

xml_forceinline 
byte_allocator&	
xml_document::get_model_allocator()
{ 
	return _model_allocator; 
}

xml_forceinline 
byte_allocator&	
xml_document::get_tmp_allocator()
{ 
	return _tmp_allocator; 
}

xml_forceinline 
mem_pool_t&	
xml_document::get_small_manager()
{ 
	return _small_manager; 
}

xml_forceinline 
mem_pool_t&	
xml_document::get_big_manager()
{ 
	return _big_manager; 
}

xml_forceinline 
const string_t& 
xml_document::get_doc_name() const
{
	return _doc_name;
}

xml_forceinline 
bool 
xml_document::is_on_fly() const
{ 
	return _on_fly; 
}

xml_forceinline 
xml_element& 
xml_document::get_root_element()
{ 
	return _root; 
}

xml_forceinline 
const xml_element& 
xml_document::get_root_element() const
{ 
	return _root; 
}

//
// adds the node to the list of children
//
xml_forceinline
xml_value_node*
xml_document::add_comment(const char* value, xml_tree_node* sibling, bool after)
{
	xml_container* parent = _container_stack.top();
	string_t text(value, &_data_allocator);
	xml_value_node* retVal = new(check_pointer(_data_allocator.allocate(sizeof(xml_value_node)))) xml_value_node(&_comment_decl, parent);
	retVal->_value.strVal = text;
	sibling ? (after ? parent->append_node(sibling, retVal) : parent->insert_node(sibling, retVal)) : parent->add_node(retVal);
	return retVal;
}

//
// adds the node to the list of children
//
xml_forceinline
xml_value_node*
xml_document::add_cdata(const char* value, xml_tree_node* sibling, bool after)
{
	xml_container* parent = _container_stack.top();
	string_t text(value, &_data_allocator);
	xml_value_node* retVal = new(check_pointer(_data_allocator.allocate(sizeof(xml_value_node)))) xml_value_node(&_cdata_decl, parent);
	retVal->_value.strVal = text;
	sibling ? (after ? parent->append_node(sibling, retVal) : parent->insert_node(sibling, retVal)) : parent->add_node(retVal);
	return retVal;
}

//
// adds the node to the list of children
//
xml_forceinline
xml_value_node* 
xml_document::add_pi(const char* name, const char* value, xml_tree_node* sibling, bool after)
{
	// checks pi name
	if (!name || !*name)
		exception::_throw("Processing instruction must contain the name");
	// checks other than xml name
	if (!str_template::strnocasecmp(name, str_xml, os_minus_one))
		exception::_throw("Invalid Processing instruction name");

	xml_container* parent = _container_stack.top();
	string_t text(value, &_data_allocator);
	xml_value_node* retVal = new(check_pointer(_data_allocator.allocate(sizeof(xml_value_node)))) xml_value_node(&add_pi_decl(name), parent);
	retVal->_value.strVal = text;
	sibling ? (after ? parent->append_node(sibling, retVal) : parent->insert_node(sibling, retVal)) : parent->add_node(retVal);
	return retVal;
}

//
// adds text as Char Data
//
xml_forceinline
xml_value_node* 
xml_document::add_text(const char* value, xml_tree_node* sibling, bool after)
{
	xml_container* parent = _container_stack.top();
	string_t text(value, &_data_allocator);
	xml_value_node* retVal = new(check_pointer(_data_allocator.allocate(sizeof(xml_value_node)))) xml_value_node(&_text_decl, parent);
	retVal->_value.strVal = text;
	sibling ? (after ? parent->append_node(sibling, retVal) : parent->insert_node(sibling, retVal)) : parent->add_node(retVal);
	return retVal;
}

//
// adds the node to the list of children
//
xml_forceinline
void 
xml_document::add_entity_desc(const entityDecl& decl)
{
	xml_container* parent = _container_stack.top();
	xml_tree_node* retVal = new(check_pointer(_data_allocator.allocate(sizeof(xml_tree_node)))) xml_tree_node(&decl, parent);
	parent->add_node(retVal);
}

//
// adds the node to the list of children
//
xml_forceinline
void 
xml_document::add_notation_desc(const notationDecl& decl)
{
	xml_container* parent = _container_stack.top();
	xml_tree_node* retVal = new(check_pointer(_data_allocator.allocate(sizeof(xml_tree_node)))) xml_tree_node(&decl, parent);
	parent->add_node(retVal);
}

//
// adds the node to the list of children
//
xml_forceinline
void 
xml_document::add_element_desc(const elementDecl& decl)
{
	xml_container* parent = _container_stack.top();
	xml_tree_node* retVal = new(check_pointer(_data_allocator.allocate(sizeof(xml_tree_node)))) xml_tree_node(&decl, parent);
	parent->add_node(retVal);
}
	
#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_sxml_hpp_ 
