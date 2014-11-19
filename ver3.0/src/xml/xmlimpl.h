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

#ifndef _terimber_xmlimpl_h_
#define _terimber_xmlimpl_h_

#include "allinc.h"
#include "xml/xmlaccss.h"
#include "xml/sxml.h"
#include "xml/storexml.h"

#include "base/common.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class xml_designer_impl
//! \brief implements abstract xml_designer interface
class xml_designer_impl : public xml_designer
{
public:
	//! \brief constructor
	xml_designer_impl(size_t xml_size						//!< xml size - just a tip
					);
	//! \brief destructor
	virtual 
	~xml_designer_impl();
	//! \brief  load xml from file
	//! if the name is not null and the grammar is not null
	//! then parse external xml according to the provided external grammar
	//! if the name is not null and the grammar is null
	//! then parse external xml according to the internal grammar if specified
	//! if the name is null and the grammar is not null
	//! then parse external grammar and clear internal document
	//! if the name is null and the grammar is not null
	// then clear internal document and use external grammar for validation
	virtual 
	bool 
	load(			const char* name,						//!< xml file name
					const char* grammar						//!< external DTD file
					);
	//! \brief loads xml from memory
	//! DTD data comes from file
	virtual 
	bool 
	load(			const void* buffer,						//!< memory buffer
					size_t length,							//!< buffer length
					const char* grammar						//!< external DTD file
					);
	//! \brief loads xml from file
	//! DTD data comes from memory
	virtual 
	bool 
	load(			const char* name,						//!< xml file name
					const void* grammar,					//!< DTD memory buffer
					size_t grammar_length					//!< buffer length
					);
	//! \brief loads xml from memory
	//! DTD data comes from memory
	virtual 
	bool 
	load(			const void* buffer,						//!< xml memory buffer
					size_t length,							//!< xml buffer length
					const void* grammar,					//!< DTD memory buffer
					size_t grammar_length					//!< DTD buffer length
					);
	//! \brief returns the last error
	virtual 
	const char* 
	error() const;
	//! \brief resets navigator to the document
	virtual 
	void 
	select_document() const;
	//! \brief resets navigator to the root element
	virtual 
	bool 
	select_root() const;
	//! \brief checks the presence of children
	virtual 
	bool 
	has_children() const;
	//! \brief navigates to the next level looking for the first child
	virtual 
	bool 
	select_first_child() const;
	//! \brief navigates to the next level looking for the last child
	virtual 
	bool 
	select_last_child() const;
	//! \brief navigates up a level
	virtual 
	bool 
	select_parent() const;
	//! attribute navigation
	//! \brief checks the attributes' presence
	virtual 
	bool 
	has_attributes() const;
	//! \brief navigates to the first attribute
	virtual 
	bool 
	select_first_attribute() const;
	//! \brief navigates to the last attribute
	virtual 
	bool 
	select_last_attribute() const;
	//! \brief navigates to the attribute by name
	virtual 
	bool 
	select_attribute_by_name(const char* name				//!< attribute name
					) const;
	//! \brief navigates through the same level go to the next node
	//! for attributes and nodes
	virtual 
	bool 
	select_next_sibling() const;
	//! \brief navigates through the same level go to the previous node
	//! for attributes and nodes
	virtual 
	bool 
	select_prev_sibling() const;
	//! where am I?
	//! \brief returns type of the current node
	virtual 
	xmlNodeType 
	get_type() const;
	//! \brief returns name of the current node
	//! text, comment, cdata - name == null
	//! element, attribute - name == name
	//! document, doc_type - document name
	//! processing instruction - name == target
	virtual 
	const char* 
	get_name() const;
	//! \brief returns value of the current node
	//! for attributes this is an attribute value
	//! for element this is a text
	//! for processing instruction - instruction text
	//! for cdata, comment - text
	//! NB!!! value is allocated on temporary allocator
	//! char pointer will be valid until next function call
	virtual 
	const char* 
	get_value() const;
	//! \brief returns the xpath (a/b/c format, where a root element c is current element) of the current node
	//! NB!!! value is allocated on temporary allocator
	//! char pointer will be valid until next function call
	virtual 
	const char* 
	get_xpath() const;
	//! \brief global navigation looking for element specified according to format (a/b/c)
	virtual 
	bool 
	select_xpath(	const char* path						//!< xpath
					) const;
	//! \brief saves xml to file
	//! adding DTD is optional
	virtual 
	bool 
	save(			const char* name,						//!< file name
					bool add_doc_type						//!< flag save the grammar to the output xml
					) const;
	//! \brief saves xml to memory
	//! adding DTD is optional
	//! if buffer is null, the function returns the required memory size through length
	virtual
	bool 
	save(			void* buffer,							//!< output buffer
					size_t& length,							//!< buffer length
					bool add_doc_type						//!< flag save the grammar to the output xml
					) const;
	//! node management
	//! element, attribute - name == name
	//! text, cdata, comment - name is ignored
	//! processing instruction - name == target
	//! \brief add node as a child to the current node
	virtual
	bool 
	add_child(		xmlNodeType type,						//!< xml node type
					const char* name,						//!< name
					const char* value,						//!< value
					bool dont_move							//!< do not navigate from current position
					);
	//! \brief inserts node before current node beneath the same parent
	virtual 
	bool 
	insert_sibling(	xmlNodeType type,						//!< xml node type
					const char* name,						//!< name
					const char* value,						//!< value
					bool dont_move							//!< do not navigate from current position
					);
	//! \brief appends node after current node beneath the same parent
	virtual 
	bool 
	append_sibling(	xmlNodeType type,						//!< xml node type
					const char* name,						//!< name
					const char* value,						//!< value
					bool dont_move							//!< do not navigate from current position
					);
	//! \brief removes the current node
	virtual 
	bool 
	remove_node();
	//! \brief updates the current node
	//! attribute value == value
	//! processing instruction value = text
	//! cdata, text, comment value == text
	virtual 
	bool 
	update_value(	const char* value						//!< value
					);
	//
	//! \brief adds beneath the current node an external xml fragment starting with the current node
	//
	virtual 
	bool merge(		const xml_designer* x,					//!< source xml document
					bool recursive							//!< merge recursively
					);
	//! \brief validates the xml node accorging to the current grammar
	virtual 
	bool 
	validate(		bool recursive							//!< validate recursively
					);
protected:
	//! \brief gets the selected node type
	inline 
	xmlNodeType get_cur_type() const;
	//! \brief checks if the selected node is an attribute 
	inline 
	bool is_attribute() const;
	//! \brief checks if the selected node is an element
	inline 
	bool is_element() const;
	//! \brief checks if the selected node is a container (element or document)
	inline 
	bool is_container() const;

private:
	//! \brief inserts new xml node into document
	bool 
	_import_node(	xmlNodeType type,						//!< node type
					const char* name,						//!< node name
					const char* value,						//!< node value
					bool sibling,							//!< insert as a sibling to the selected node or as a child
					bool after,								//!< insert before or after selected node
					bool dont_move							//!< do not navigate to the new node
					);
	//! \brief validates element
	void 
	_validate(		xml_element* el,						//!< validate element
					bool recursive							//!< flag recursively
					);
	//! \brief parses xml document from abstract streams for xml and DTD sources
	bool 
	_load(			byte_source* stream,					//!< optional xml stream
					byte_source* grammar					//!< optional DTD stream
					);

private:
	size_t							_xml_size;				//!< xml size - just a tip
	mutable mem_pool_t				_small_manager;			//!< small memory pool
	mutable mem_pool_t				_big_manager;			//!< big memory pool
	mutable byte_allocator*			_tmp_allocator;			//!< temporary allocator
	xml_document					_doc;					//!< xml document
	mutable string_t				_error;					//!< last error
	mutable xml_tree_node*			_cur_node;				//!< selected node
};

//! \class xml_parser_creator
//! \brief class creator for xml designer
class xml_parser_creator : public proto_creator< xml_parser_creator, xml_designer, size_t >
{
public:
	//! \brief constructor
	xml_parser_creator();
	//! \brief creates xml-designer instance
	xml_designer* 
	create(			size_t n								//!< xml size
					);
	//! \brief destroys xml_designer instance
	void destroy(	xml_designer* obj
					);
private:
	xml_factory		_factory;								//!< factory
};

//! \typedef xml_designer_keeper_t
//! \brief smart pointer for xml_designer class
typedef smart_pointer< xml_parser_creator > xml_designer_keeper_t;

//! \class xml_designer_creator
//! \brief pool creator for xml_designer
class xml_designer_creator : public proto_creator< xml_designer_creator, xml_designer, size_t >
{
public: 
	//! \brief creates instance of xml_designer
	static
	xml_designer* 
	create(			size_t size								//!< xml size
					);
	//! \brief activates xml designer, clear resources
	static
	void 
	activate(		xml_designer* obj,						//!< input object
					size_t									
					);
};

//! \typedef xml_pool_t
//! \brief xml pool
typedef pool< xml_designer_creator > xml_pool_t;


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_xmlimpl_h_


