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

#ifndef _terimber_mngxml_h_
#define _terimber_mngxml_h_

#include "xml/miscxml.h"
#include "xml/sxml.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

/////////////////////////////
//! \class byte_manager
//! \brief base class for utf-8 stream management with
//! additional stack support
//! however the parsing process assumes 
//! that some entries must be substituted
//! so we need additional store like stack
class byte_manager
{
	//! \typedef entity_map_t
	//! \brief map of entires to the counts
	typedef _map< const entityDecl*, size_t > entity_map_t;
protected:
	//! \brief constructor
	byte_manager(	byte_source& stream,					//!< stream of bytes as input xml
					xml_document& doc,						//!< document that will hold the xml and grammar 
					mem_pool_t& small_pool,					//!< small pool of allocators
					mem_pool_t& big_pool,					//!< big pool of allocators
					size_t xml_size							// the default size to optimize the size of allocator
					);
	//! \brief destructor
	~byte_manager();

protected:
	//! inline functions
	//! \brief picks the byte
	xml_forceinline 
	ub1_t 
	pick();
	//! \brief pops the byte like operator++(int)
	xml_forceinline 
	ub1_t 
	pop();
	//! \brief picks than pop like operator++()
	xml_forceinline 
	ub1_t 
	pip();
	//! \brief skips white spaces
	xml_forceinline 
	void 
	skip_white_space(bool mustPresent = false,				//!< white space must be there
					const char* message = 0					//!< optional error message for exception
					);
	//! \brief pushes byte back on top of buffer
	xml_forceinline 
	void 
	push(			ub1_t x									//!< byte
					);
	//! \brief pushes array of bytes back to buffer
	xml_forceinline 
	void 
	push(			const ub1_t* x,							//!< pointer to array						
					size_t len								//!< array length
					);
	//! \brief throws exception, adding line and char position info	
	xml_forceinline 
	void 
	throw_exception(const char* msg_text					//!< error text
					);

	//! \brief skips specified string if not found throw exception
	xml_forceinline 
	void 
	skip_string(	const char* x,							//!< string to skip
					const char* message						//!< error message
					);
	//! \brief skips symbol with optional white spaces around it
	xml_forceinline 
	void 
	skip_sign(		ub1_t symbol,							//!< symbol to skip
					bool skip_before,						//!< before white spaces flag
					bool skip_after,						//!< after white spaces flag
					const char* message						//!< error message
					);
	//! \brief skips quote symbol (single or double)
	//! if input param is zero returns what type of quote has been found
	xml_forceinline 
	ub1_t 
	skip_quote(		ub1_t symbol = 0						//!< type of quote
					);

	//! \brief parses value wrapped with quoted symbols
	//! like '"' | "'" name '"' | "'"
	const char* 
	parseQuotedValue(bool resolve_entities,					//!< flag to resolve found entities
					bool normalize,							//!< flag to normalized value
					bool (*fn)(ub1_t),						//!< optional function pointer to check symbol validity
					const char* message						//!< error message 
					);
	//! \brief parses xml valid name
	xml_forceinline 
	const char* 
	parseName();
	//! \brief parses xml valid value
	const char* 
	parseValue();
	//! \brief parses attribute style xml part
	//! like: name S? = S? '"' | "'" value '"' | "'"
	//! returns the name and value
	void 
	parseAttributeValue(string_t& name,						//!< [out] name
					string_t& value							//!< [out] value
					);
	//! \brief parses comment
	//! [15]    Comment    ::=    '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->' 
	void 
	parseComment();
	//! \brief parses external ID
	//! [75]    ExternalID    ::=    'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral  
	//! [76]    NDataDecl    ::=    S 'NDATA' S Name 
	void 
	parseExternalID(string_t& value_system,					//!< [out] system value
					string_t& value_public,					//!< [out] public value
					bool public_strick						//!< force to find the public value
					);
	//! \brief parses processing instruction	
    //! [16]    PI    ::=    '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>' 
	//! [17]    PITarget    ::=    Name - (('X' | 'x') ('M' | 'm') ('L' | 'l')) 
	void 
	parsePI();

	//! \brief parses xml valid char &#[0-9]+ | &#x[0-9;a-F]+
	void 
	parseCharRef(	paged_buffer& buffer					//!< [out] buffer
					);
	//! \brief resolves entity
	bool 
	resolveEntity(	paged_buffer& buffer					//!< [out] buffer
					);
	//! \brief returns the current subset value
	inline 
	bool 
	get_subset() const;
	//! \brief returns the current standalone value
	inline 
	size_t 
	get_standalone() const;
	//! \brief returns the current version
	inline 
	size_t 
	get_version() const;
	//! NB!!! we don't care about encoding, 
	//! because stream is responsible for recognizing encoding and converting data to the UTF-8

protected:
	//! \brief resets internal temporary allocator and buffers
	xml_forceinline 
	void 
	reset_all_tmp(	bool reset_allocator = false			//!< flag to reset temporary allocator as well
					);

protected:
	const size_t		_xml_size;							//!< xml size tip
	mem_pool_t&			_small_pool;						//!< small memory pool
	mem_pool_t&			_big_pool;							//!< big memory pool
	byte_allocator*		_depot_store1_allocator;			//!< depot store #1 allocator
	byte_allocator*		_depot_store2_allocator;			//!< depot store #2 allocator
	byte_allocator*		_depot_store3_allocator;			//!< depot store #3 allocator
	byte_allocator*		_tmp_allocator;						//!< temporary allocator
	byte_allocator*		_tmp_store1_allocator;				//!< temporary store #1 allocator	
	byte_allocator*		_tmp_store2_allocator;				//!< temporary store #2 allocator	
	byte_allocator*		_tmp_store3_allocator;				//!< temporary store #3 allocator	
	byte_allocator*		_entity_allocator;					//!< entity allocator
	xml_document&		_doc;								//!< document that will hold the xml and grammar  
	byte_source&		_stream;							//!< stream of bytes as input xml
	entity_map_t		_entity_map;						//!< we need to keep track for recursive entities
	//! there are situation while parsing when we need three temporary containars simultaniously
	paged_buffer		_tmp_store1;						// temporary store #1
	paged_buffer		_tmp_store2;						// temporary store #2
	paged_buffer		_tmp_store3;						// temporary store #3
};


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_mngxml_h_ 
