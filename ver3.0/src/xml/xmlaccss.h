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

#ifndef _terimber_xmlaccess_h_
#define _terimber_xmlaccess_h_

#include "xmltypes.h"

//! \class xml_designer
//! \brief class supports xml parsing, navigating, construction, and persistence
class xml_designer
{
public:
	//! \brief destructor
	virtual 
	~xml_designer() 
	{
	}
	//! \brief  loads xml from file
	//! if the name is not null and the grammar is not null
	//! then parses external xml according to the provided external grammar
	//! if the name is not null and the grammar is null
	//! then parses external xml according to the internal grammar if specified
	//! if the name is null and the grammar is not null
	//! then parses external grammar and clear internal document
	//! if the name is null and the grammar is not null
	// then clears internal document and uses external grammar for validation
	virtual 
	bool 
	load(			const char* name,						//!< xml file name
					const char* grammar						//!< external DTD file
					) = 0;
	//! \brief loads xml from memory
	//! DTD data comes from file
	virtual 
	bool 
	load(			const void* buffer,						//!< memory buffer
					size_t length,							//!< buffer length
					const char* grammar						//!< external DTD file
					) = 0;
	//! \brief loads xml from file
	//! DTD data comes from memory
	virtual 
	bool 
	load(			const char* name,						//!< xml file name
					const void* grammar,					//!< DTD memory buffer
					size_t grammar_length					//!< buffer length
					) = 0;
	//! \brief loads xml from memory
	//! DTD data comes from memory
	virtual 
	bool 
	load(			const void* buffer,						//!< xml memory buffer
					size_t length,							//!< xml buffer length
					const void* grammar,					//!< DTD memory buffer
					size_t grammar_length					//!< DTD buffer length
					) = 0;
	//! \brief returns the last error
	virtual 
	const char* 
	error() const = 0;
	//! \brief resets navigator to the document
	virtual 
	void 
	select_document() const = 0;
	//! \brief resets navigator to the root element
	virtual 
	bool 
	select_root() const = 0;
	//! \brief checks the presence of children
	virtual 
	bool 
	has_children() const = 0;
	//! \brief navigates to the next level looking for the first child
	virtual 
	bool 
	select_first_child() const = 0;
	//! \brief navigates to the next level looking for the last child
	virtual 
	bool 
	select_last_child() const = 0;
	//! \brief navigates up a level
	virtual 
	bool 
	select_parent() const = 0;
	//! attribute navigation
	//! \brief checks attributes presence
	virtual 
	bool 
	has_attributes() const = 0;
	//! \brief navigates to the first attribute
	virtual 
	bool 
	select_first_attribute() const = 0;
	//! \brief navigates to the last attribute
	virtual 
	bool 
	select_last_attribute() const = 0;
	//! \brief navigates to the attribute by name
	virtual 
	bool 
	select_attribute_by_name(const char* name				//!< attribute name
					) const = 0;
	//! \brief navigates through the same level go to the next node
	//! for attributes and nodes
	virtual 
	bool 
	select_next_sibling() const = 0;
	//! \brief navigates through the same level go to the previous node
	//! for attributes and nodes
	virtual 
	bool 
	select_prev_sibling() const = 0;
	//! where am I?
	//! \brief returns type of the current node
	virtual 
	xmlNodeType 
	get_type() const = 0;
	//! \brief returns name of the current node
	//! text, comment, cdata - name == null
	//! element, attribute - name == name
	//! document, doc_type - document name
	//! processing instruction - name == target
	virtual 
	const char* 
	get_name() const = 0;
	//! \brief returns value of the current node
	//! for attributes this is an attribute value
	//! for element this is a text
	//! for processing instruction - instruction text
	//! for cdata, comment - text
	//! NB!!! value is allocated on temporary allocator
	//! char pointer will be valid until next function call
	virtual 
	const char* 
	get_value() const = 0;
	//! \brief returns the xpath (a/b/c format, where a root element c is current element) of the current node
	//! NB!!! value is allocated on temporary allocator
	//! char pointer will be valid until next function call
	virtual 
	const char* 
	get_xpath() const = 0;
	//! \brief global navigation looking for element specified according to format (a/b/c)
	virtual 
	bool 
	select_xpath(	const char* path						//!< xpath
					) const = 0;
	//! \brief save xml to file
	//! adding DTD is optional
	virtual 
	bool 
	save(			const char* name,						//!< file name
					bool add_doc_type						//!< flag save the grammar to the output xml
					) const = 0;
	//! \brief saves xml to memory
	//! adding DTD is optional
	//! if buffer is null function returns the required memory size through length
	virtual
	bool 
	save(			void* buffer,							//!< output buffer
					size_t& length,							//!< buffer length
					bool add_doc_type						//!< flag save the grammar to the output xml
					) const = 0;
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
					) = 0;
	//! \brief inserts node before current node beneath the same parent
	virtual 
	bool 
	insert_sibling(	xmlNodeType type,						//!< xml node type
					const char* name,						//!< name
					const char* value,						//!< value
					bool dont_move							//!< do not navigate from current position
					) = 0;
	//! \brief appends node after current node beneath the same parent
	virtual 
	bool 
	append_sibling(	xmlNodeType type,						//!< xml node type
					const char* name,						//!< name
					const char* value,						//!< value
					bool dont_move							//!< do not navigate from current position
					) = 0;
	//! \brief removes the current node
	virtual 
	bool 
	remove_node() = 0;
	//! \brief update the current node
	//! attribute value == value
	//! processing instruction value = text
	//! cdata, text, comment value == text
	virtual 
	bool 
	update_value(	const char* value						//!< value
					) = 0;
	//
	//! \brief adds, beneath the current node, an external xml fragment starting with the current node
	//
	virtual 
	bool merge(		const xml_designer* x,					//!< source xml document
					bool recursive							//!< merge recursively
					) = 0;
	//! \brief validates the xml node accorging to the current grammar
	virtual 
	bool 
	validate(		bool recursive							//!< validate recursively
					) = 0;
};

//! \class xml_factory
//! \brief create xml_designer instance
class xml_factory
{
public:
	//! \brief constructor
	xml_factory();
	//! \brief destructor
	virtual 
	~xml_factory();
	//! \brief creates xml_designer object
	virtual 
	xml_designer* 
	get_xml_designer(size_t xml_size = 1024					//!< xml size - just a tip
					);
};

#endif // _terimber_xmlaccess_h_


