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

#ifndef _terimber_sxml_h_
#define _terimber_sxml_h_

#include "xml/sxs.h"
#include "base/stack.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

/////////////////////////////
//! \class xml_node
//! \brief base class for all xml nodes (attributes, elements, ...)
//! no virtual methods to keep class size as small as possible
class xml_node
{
protected:
	//! \brief private copy constructor and assign operator
	xml_node(const xml_node& x);
	xml_node& operator=(const xml_node& x);
public:
	//! \brief constructor
	xml_node(		const namedNodeDecl* decl				//!< pointer to the node declaration
					);
	//! \brief destructor
	~xml_node();
	//! \brief casts operation to attrubute
	xml_forceinline
	const attributeDecl* 
	cast_to_attribute() const;
	//! \brief clears
	void 
	clear();
public:
	const namedNodeDecl*	_decl;							//!< pointer to the node declaration
};

/////////////////////////////
//! \class xml_tree_node
//! \brief base class for attributes and element container
class xml_tree_node : public xml_node
{
protected:
	//
	//! prevents direct copy
	//! \brief private copy constructor and assign operator
	xml_tree_node(const xml_tree_node& x);
	xml_tree_node& operator=(const xml_tree_node& x);
public:
	//! \brief constructor
	xml_tree_node(	const namedNodeDecl* decl,				//!< pointer to the node declaration
					xml_tree_node* parent					//!< pointer to the parent node
					);
	//! \brief destructor
	~xml_tree_node();
	//! \brief clears
	void 
	clear();
public:
	xml_tree_node*			_parent;						//!< pointer to the parent node
	xml_tree_node*			_left;							//!< pointer to left sibling
	xml_tree_node*			_right;							//!< pointer to the right sibling
};

//! \class xml_value_node
//! \brief class for all value contained nodes 
//! like comment, pi, CDATA and text
class xml_value_node : public xml_tree_node
{
protected:
	//! prevents direct copying
	//! \brief private copy constructor and assign operator
	xml_value_node(const xml_value_node& x);
	xml_value_node& operator=(const xml_value_node& x);
public:
	//! \brief constructor
	xml_value_node(	const namedNodeDecl* decl,				//!< pointer to the node declaration
					xml_tree_node* parent					//!< pointer to the parent node
					);
	//! \brief destructor
	~xml_value_node();
	//! \brief persists value to the string
	const char* 
	persist(		byte_allocator& allocator				//!< external allocator
					) const;
	//! \brief type cast to const value node
	xml_forceinline
	static 
	const xml_value_node* 
	cast_to_node_value(const xml_tree_node* x				//!< pointer to the const tree node
					);
	//! \brief type cast to value node
	xml_forceinline
	static 
	xml_value_node* 
	cast_to_node_value(xml_tree_node* x						//!< pointer to the tree node
					);
	//! \brief clears
	void 
	clear();


public:
	terimber_xml_value				_value;					//!< value uninon
};


// forward declaration
class xml_document;
class xml_element;

//! \class xml_container
//! \brief class with children elements
class xml_container : public xml_tree_node
{
public:
	//! \brief constructor
	xml_container(	const namedNodeDecl* decl,				//!< pointer to the node declaration
					xml_tree_node* parent					//!< pointer to the parent node
					);
	//! \brief clears content
	void 
	clear();
public:
	//! \brief adds node to the end of children list
	xml_forceinline
	xml_tree_node* 
	add_node(		xml_tree_node* node						//!< node pointer
					);
	//! \brief adds node after specified node
	xml_forceinline
	xml_tree_node* 
	append_node(	xml_tree_node* after,					//!< sibling node
					xml_tree_node* node						//!< node pointer
					);
	//! \brief adds node before specified node
	xml_forceinline
	xml_tree_node* 
	insert_node(	xml_tree_node* before,					//!< sibling node
					xml_tree_node* node						//!< node pointer
					);
	//! \brief removes node from the children list
	xml_forceinline
	xml_tree_node* 
	remove_node(	xml_tree_node* node						//!< node pointer
					);
	//! \brief checks if node has children
	xml_forceinline
	bool 
	has_children() const;
	//! \brief checks if node is a container
	xml_forceinline
	bool 
	is_container() const;
	//! \brief type cast to the const container
	xml_forceinline
	static 
	const xml_container* 
	cast_to_container(const xml_tree_node* x				//!< pointer to the tree node
					);
	//! \brief type cast to the container
	xml_forceinline
	static 
	xml_container* 
	cast_to_container(xml_tree_node* x						//!< pointer to the tree node
					);
public:
	xml_tree_node*		_first_child;						//!< pointer to the first child
	xml_tree_node*		_last_child;						//!< pointer to the last child
};

//! \typedef attr_states_map_t
//! \brief mapping attribute declaration to the xml_attribute pointer
//! for internal using only while parsing
typedef _map< const attributeDecl*, const xml_value_node* > attr_states_map_t;

//! \class xml_element
//! \brief base xml element
class xml_element : public xml_container
{
	//! prevents copying
	//! \brief private copy constructor and assign operator
	xml_element(const xml_element& x);
	xml_element& operator=(const xml_element& x);
public:
	//! \brief constructor
	xml_element(	const elementDecl* decl,				//!< pointer to the node declaration
					xml_tree_node* parent					//!< pointer to the parent node
					);
	//! \brief checks if element has attributes
	xml_forceinline 
	bool 
	has_attributes() const;
	//! \brief casts to element declaration 
	xml_forceinline
	const elementDecl* 
	cast_decl() const;
	//! \brief clears element
	xml_forceinline 
	void 
	clear();
	//! \brief casts to const element
	xml_forceinline
	static 
	const xml_element* 
	cast_to_element(const xml_tree_node* x					//!< pointer to the tree node
					);
	//! \brief casts to the element
	xml_forceinline
	static 
	xml_element* 
	cast_to_element(xml_tree_node* x						//!< pointer to the tree node
					);

	//! \brief adds attribute to the tail
	xml_forceinline 
	xml_tree_node* 
	add_attribute(	xml_tree_node* node						//!< pointer to the tree node
					);

	//! \brief appends attribute after node
	xml_forceinline
	xml_tree_node* 
	append_attribute(xml_tree_node* after,					//!< sibling node
					xml_tree_node* node						//!< attribute node
					);
	//! \brief inserts attribute before node
	xml_forceinline
	xml_tree_node* 
	insert_attribute(xml_tree_node* before,					//!< sibling node
					xml_tree_node* node						//!< attribute node
					);
	//! \brief removes attribute
	xml_forceinline
	xml_tree_node* 
	remove_attribute(xml_tree_node* node					//!< pointer to the attribute
					);

public:
	xml_tree_node*		_first_attr;						//!< pointer to the first attribute
	xml_tree_node*		_last_attr;							//!< pointer to the last attribute
};

// forward declaration
class content_interface;
//! \typedef model_map_t
//! \brief maps element declaration to the content interface pointer
//! for validation purposes
typedef _map< const elementDecl*, content_interface*, byte_allocator, less< const elementDecl* >, false > model_map_t;
//! \typedef xml_container_stack_allocator_t
//! \brief node allocator for stack of pointers to xml container
typedef node_allocator< base_stack< xml_container* >::_node >			xml_container_stack_allocator_t; 
//! \typedef xml_container_stack_t
//! \brief stack of pointers to the xml container
typedef _stack< xml_container*, xml_container_stack_allocator_t >		xml_container_stack_t;

//! \class xml_document
//! \brief xml document class as a root class for xml document structure
class xml_document : public xml_container, 
					public xml_grammar
{
public:
	//! \brief constructor
	xml_document(	mem_pool_t& small_manager,				//!< small memory pool
					mem_pool_t& big_manager,				//!< big memory pool
					size_t xml_size,						//!< xml size - just a tip
					const xml_grammar* grammar				//!< optional external xml grammar
					);	
	//! \brief destructor
	~xml_document();
	//! \brief finds validation model by element declaration
	content_interface* 
	find_model(		const elementDecl* decl					//!< pointer to the element declaration
					);
	//! \brief adds validation model for element declaration
	void 
	add_model(		const elementDecl* decl,				//!< pointer to the element declaration
					content_interface* model				//!< pointer to the validation model
					);
	//! \brief returns data allocator
	xml_forceinline 
	byte_allocator&	
	get_data_allocator();
	//! \brief returns model allocator
	xml_forceinline 
	byte_allocator&	
	get_model_allocator();
	//! \brief returns temporary allocator
	xml_forceinline 
	byte_allocator&	
	get_tmp_allocator();
	//! \brief returns small memory pool
	xml_forceinline 
	mem_pool_t&	
	get_small_manager();
	//! \brief retunrs big memory pool
	xml_forceinline 
	mem_pool_t&	
	get_big_manager();
	//! \brief checks if grammar is built on the fly
	xml_forceinline 
	bool 
	is_on_fly() const;
	//! \brief gets root element
	xml_forceinline 
	xml_element& 
	get_root_element();
	//! \brief gets const root element
	xml_forceinline 
	const xml_element& 
	get_root_element() const;
	//! \brief adds xml escaped symbols to the internal map
	void 
	add_escaped_symbols();
	//! \brief gets document name 
	xml_forceinline 
	const string_t& 
	get_doc_name() const;
	//! \brief sets document name
	bool 
	set_doc_name(	const char* name						//!< document name
					);
	//! \brief checks if root is detected
	bool 
	check_root();
	//! \brief clears internal resources
	void clear();
	//! \brief clears root
	void 
	clear_root();
	//! element stack function
	//! \brief resets stack container
	void 
	container_reset();
	//! \brief push new element to the stack
	void 
	container_push(	xml_element* el							//!< pointeher to the element
					);
	//! \brief removes element from the stack
	xml_element* 
	container_pop();
	//! \brief checks if element stack is not empty
	bool 
	container_peak();
	//! \brief adds to the stack DTD node
	void container_start_doctype();
	//! \brief removes from the stack DTD node
	void container_stop_doctype();
	//! \brief validates element according to the provided document grammar
	void 
	validate(		xml_element& el							//!< element to validate
					);
	//! \brief adds entity node declaration to the grammar
	xml_forceinline
	void 
	add_entity_desc(const entityDecl& decl					//!< entity node declaration
					);
	//! \brief adds notation node declaration to the grammar
	xml_forceinline
	void 
	add_notation_desc(const notationDecl& decl				//!< notation node declaration
					);
	//! \brief adds element node declaration to the grammar
	xml_forceinline
	void 
	add_element_desc(const elementDecl& decl				//!< element node declaration
					);
	//! \brief adds comment to the document
	xml_forceinline
	xml_value_node* 
	add_comment(	const char* value,						//!< comment text
					xml_tree_node* sibling = 0,				//!< optional sibling node
					bool after = true						//!< optional insertion flag
					);
	//! \brief adds cdata to the document
	xml_forceinline
	xml_value_node* 
	add_cdata(		const char* value,						//!< CDATA text
					xml_tree_node* sibling = 0,				//!< optional sibling node
					bool after = true						//!< optional insertion flag
					);
	//! \brief adds pi to the document
	xml_forceinline
	xml_value_node* 
	add_pi(			const char* name,						//!< PI name
					const char* value,						//!< PI value
					xml_tree_node* sibling = 0,				//!< optional sibling node
					bool after = true						//!< optional insertion flag
					);
	//! \brief adds text node to the list document
	xml_forceinline
	xml_value_node* 
	add_text(		const char* value,						//!< Text value
					xml_tree_node* sibling = 0,				//!< optional sibling node
					bool after = true						//!< optional insertion flag
					);
	//! \brief adds element to the document
	xml_element* 
	add_element(	const char* name,						//!< element name
					xml_tree_node* sibling = 0,				//!< optional sibling node
					bool after = true						//!< optional insertion flag
					);
	//! \brief adds attribute to the document
	xml_value_node* 
	add_attribute(	xml_element& el,						//!< xml element
					const char* name,						//!< attribute name
					const char* value,						//!< attribute value
					xml_tree_node* sibling = 0,				//!< optional sibling node
					bool after = true						//!< optional insertion flag
					);
	//! \brief updates attribute value
	void 
	update_attribute(xml_element* el,						//!< element node
					xml_value_node* att,					//!< attribute node
					const char* value						//!< new attribute value
					);
	//! \brief adds default attributes if they aren't in the parsing document
	void 
	add_def_attributes(xml_element& el,						//!< element
					attr_states_map_t& attrStates			//!< attribute states map
					);
	//! \brief finds attribute value by attribute name
	//! if not assigned returns default value (be careful, value allocated on temporary allocator)
	//! if not found return empty xml value
	terimber_xml_value 
	find_attribute_value(const xml_element& el,				//!< element
					const char* name						//!< attribute name
					) const;
	//! \brief finds attribute by name, if not returns NULL
	xml_value_node* 
	find_attribute(	const xml_element& el,					//!< element
					const char* name						//!< attribute name
					);

private:
	//! \brief validates attributes according to the provided document grammar
	void 
	validate_attributes(xml_element& el						//!< element
					);
	//! \brief validates children nodes according to the provided document grammar
	void 
	validate_children(xml_element& el						//!< element
					);
	//! \brief assigns attribute value
	void 
	assign_attribute_value(xml_element& el,					//!< element
					const attributeDecl& attr_decl,			//!< attribute declaration
					xml_value_node* att,					//!< attribute node
					const char* value						//!< attribute value
					);
public:
	size_t								_standalone;		//!< standalone flag

private:
	xml_container_stack_t				_container_stack;	//!< stack of xml containers
	byte_allocator						_model_allocator;	//!< validation model allocator
	xml_container_stack_allocator_t		_stack_allocator;	//!< stack allocator
	bool								_on_fly;			//!< building grammar on the fly flag
	model_map_t							_model_map;			//!< model map
	xml_element							_root;				//!< root element
	xml_container						_doc_type;			//!< DTD container
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_sxml_h_ 
