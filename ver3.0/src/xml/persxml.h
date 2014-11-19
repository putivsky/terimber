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

#ifndef _terimber_persxml_h_
#define _terimber_persxml_h_

#include "base/stack.h"
#include "xml/sxml.h"
#include "xml/miscxml.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//////////////////////////////////////////////////////////
//! \typedef persistor_stack_t
//! \brief stack of xml nodes
typedef _stack< const xml_tree_node* > persistor_stack_t;

//! \class xml_persistor
//! \brief class save internal xml document into stream
class xml_persistor
{
public:
	//! \brief constructor
	xml_persistor(	byte_consumer& stream,					//!< output stream
					const xml_document& doc,				//!< xml document
					mem_pool_t& small_pool,					//!< small memory pool
					mem_pool_t& big_pool,					//!< big memory pool
					bool validate,							//!< flag to do validation
					bool save_doc_type,						//!< flag to save internal DTD to the output xml
					size_t size								//!< xml size - just a tip
					);

	//! \brief destructor
	~xml_persistor();
	//! \brief saves xml to stream
	bool 
	persist();
	//! \brief returns last error
	const char* 
	get_error() const;

protected:
	//! \brief persists document
	void 
	persistDocument();
	//! \brief persists DTD
	void 
	persistDocType();
	//! \brief persists xml element
	void 
	persistElement();
	//! \brief persists attributes
	void 
	persistAttributes();
	//! \brief persists text node
	void 
	persistText();
	//! \brief persists CDATA section
	void 
	persistCDATA();
	//! \brief persists comment
	void 
	persistComment();
	//! \brief persists processing instruction
	void 
	persistPI();
	//! \brief persists entity declaration
	void 
	persistEntityDecl(const entityDecl* decl = 0			//!< optional external entity declaration
					);
	//! \brief persists notation declaration
	void 
	persistNotationDecl(const notationDecl* decl = 0		//!< optional external notation declaration
					);
	//! \brief persists element declaration
	void 
	persistElementDecl(const elementDecl* decl = 0			//!< optional external element declaration
					);
	//! \brief persists
	void 
	persistAttributeDecl(const attributeDecl& decl			//!< optional external attribute declaration
					);

	//! \brief persists mixed element content
	void 
	persistMixed(	const dfa_token* parent					//!< DFA element pointer
					);
	//! \brief persists children of element
	void 
	persistChildren(const dfa_token* parent,				//!< DFA element pointer
					dfaRule prevRule						//!< previous DFA rule
					);
	//! \brief restores element stack
	void 
	restore_stack(	bool makeShift = true					//!< flag to make a shift
					);
	//! \brief persists value
	void 
	persistValue(	const char* value,						//!< value
					bool charData = false					//!< flag if this a char data
					);
private:
	const size_t					_xml_size;				//!< xml size
	byte_consumer&					_stream;				//!< output byte stream
	const xml_document&				_doc;					//!< xml document
	mem_pool_t&						_small_pool;			//!< small memory pool
	mem_pool_t&						_big_pool;				//!< big memory pool
	persistor_stack_t				_element_stack;			//!< xml element stack
	string_t						_error;					//!< last error
	bool							_validate;				//!< validation flag
	bool							_save_doc_type;			//!< flag to save DTD
	byte_allocator*					_tmp_allocator;			//!< temporary allocator
	byte_allocator*					_stack_allocator;		//!< stack allocator
	bool							_in_doc_type;			//!< flag if we are inside DTD saving
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_persxml_h_ 
