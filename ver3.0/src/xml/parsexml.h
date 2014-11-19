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

#ifndef _terimber_parsexml_h_
#define _terimber_parsexml_h_

#include "xml/mngxml.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//////////////////////////////////////////////////////////
//! \class xml_white_space_handler
//! \brief a stack to keep white space info
class xml_white_space_handler
{
public:
	//! \brief constructor
	xml_white_space_handler(const xml_container* el,		//!< pointer to xml node
					bool preserve							//!< preserve flag
					) :	
		_preserve(preserve),
		_el(el)
	{
	}

	bool					_preserve;						//!< preserve flag
	const xml_container*	_el;							//!< pointer to xml node
};

//! \typedef xml_white_space_stack_t
//! \brief stack of white spaces informations
typedef _stack< xml_white_space_handler >	xml_white_space_stack_t;

//! \class xml_processor
//! \brief xml processor main class
class xml_processor : public byte_manager
{
public:
	//! \brief consrtuctor
	xml_processor(	byte_source& stream,					//!< byte stream
					xml_document& doc,						//!< xml document
					mem_pool_t& small_pool,					//!< small memory pool
					mem_pool_t& big_pool,					//!< big memory pool
					size_t xml_size,						//!< xml size - just a tip
					bool validate							//!< validation flag
					);
	//! \brief destructor
	~xml_processor();
	//! \brief parser document
	bool 
	parse();
	//! \brief returns the last error
	const char* 
	get_error() const;

private:
	//! \brief parses document
	//! [1]    document    ::=    prolog element Misc* 
	void 
	parseDocument();

	//! \brief parses prolog
	//! [22]    prolog    ::=    XMLDecl? Misc* (doctypedecl Misc*)? 
	void 
	parseProlog();
	//! \brief parses document type declaration
	//! [23]    XMLDecl    ::=    '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
	//! [28] doctypedecl    ::=    '<!DOCTYPE' S Name (S ExternalID)? S? ('[' (markupdecl | DeclSep)* ']' S?)? '>' 
	void 
	parseDocTypeDecl();
	//! \brief parses xml element
	//! [39]    element    ::=    EmptyElemTag | STag content ETag 
	void 
	parseElement();
	//! \brief parses xml element open tag
	//! [44] EmptyElemTag    ::=    '<' Name (S Attribute)* S? '/>' 
	//! [40] STag    ::=    '<' Name (S Attribute)* S? '>' 
	xml_forceinline 
	void 
	parseStartTag();
	//! \brief parses attributes
	//! [41]    Attribute    ::=    Name Eq AttValue 
    void 
	parseAttributes(xml_element& el							//!< parent element
					);
	//! \brief parses element closing tag
	//! [42]    ETag    ::=    '</' Name S? '>' 
	xml_forceinline 
	void 
	parseEndTag();
	//! \brief parses element content
	//! [43]    content    ::=    CharData? ((element | Reference | CDSect | PI | Comment) CharData?)* 
	xml_forceinline 
	void 
	parseContent();
	//! \brief CDATA section
	//! [18]    CDSect    ::=    CDStart CData CDEnd 
	//! [19]    CDStart    ::=    '<![CDATA[' 
	//! [20]    CData    ::=    (Char* - (Char* ']]>' Char*))  
	//! [21]    CDEnd    ::=    ']]>' 
	void 
	parseCDATA();
	//! \brief parses char data - text node, internal version
	// [14]    CharData    ::=  ,//  [^<&]* - ([^<&]* ']]>' [^<&]*) 
	void 
	_parseCharData();
	//! \brief parses char data - text node inline wrapper
	xml_forceinline 
	void 
	parseCharData();
	//! \brief parses general reference
	//! [67]    Reference    ::=    EntityRef | CharRef 
	//! [68]    EntityRef    ::=    '&' Name ';' 
	//! [66]    CharRef    ::=    '&#' [0-9]+ ';'  | '&#x' [0-9a-fA-F]+ ';'
	void 
	parseGeneralReference(bool skip_after					//!< flag to skip white spaces after
					);
	//! \brief parses misc
	//! [27]    Misc    ::=    Comment | PI | S 
	void 
	parseMisc();
	//! \brief parses DTD
	void 
	parseDTD(		const char* location = 0				//!< optional external location
					);
	//! \brief parses DTD but does not build grammar
	void 
	skipDTD();
	//! \brief resolves references
	void 
	resolve_references();

private:
	xml_document&				_doc;						//!< xml document
	bool						_preserve_white_space;		//!< flag to preserve white spaces
	byte_allocator*				_white_space_allocator;		//!< white space stack allocator
	xml_white_space_stack_t		_white_space_stack;			//!< white space stack
	string_t					_error;						//!< last error
	bool						_validate;					//!< flag to do validation
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_parsexml_h_ 
