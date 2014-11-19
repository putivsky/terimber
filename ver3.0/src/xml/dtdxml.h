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

#ifndef _terimber_dtdxml_h_
#define _terimber_dtdxml_h_

#include "xml/mngxml.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//////////////////////////////////////////////////////////
//! \class dtd_processor
//! \brief class process DTD into internal grammar
class dtd_processor : public byte_manager
{
public:
	//! \brief constructor
	dtd_processor(	byte_source& stream,					//!< input byte sequence
					xml_document& doc,						//!< document that will keep the grammar as a result of DTD parsing
					mem_pool_t& small_pool,					//!< pool of allocators
					mem_pool_t& big_pool,					//!< pool of allocators
					size_t xml_size							//!< default size for allocators
					);

	//! \brief parse DTD
	void 
	parse();

private:
	//! \brief parses subset
	//! dtd can be built into main document text
	//! [' (markupdecl | DeclSep)* ']'
	//! [30]    extSubset    ::=    TextDecl? extSubsetDecl 
	void 
	parseSubSet(	char stopSymbol,						//!< stop symbol
					bool include_allowed					//!< flag if aditional includes are allowed
					);


	//! \brief parser element
	//! [29]    markupdecl    ::=    elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment 
	//! [28a]    DeclSep    ::=    PEReference | S 
	//! [45]    elementdecl    ::=    '<!ELEMENT' S Name S contentspec S? '>' 
	//! element starts with '<!ELEMENT'
	void parseElement();

	//! \brief parses content of element
	//! [46]    contentspec    ::=    'EMPTY' | 'ANY' | Mixed | children 
	void 
	parseContentSpec(elementDecl& decl						//!< element declaration
					);

	//! \brief parses mixed content type
	//! [51]    Mixed    ::=    '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*'  | '(' S? '#PCDATA' S? ')'  
	dfa_token* 
	parseMixed();

	//! \brief parses children content type
	//! [47]    children    ::=    (choice | seq) ('?' | '*' | '+')? 
	//! [48]    cp    ::=    (Name | choice | seq) ('?' | '*' | '+')? 
	//! [49]    choice    ::=    '(' S? cp ( S? '|' S? cp )+ S? ')'
    //! [50]    seq    ::=    '(' S? cp ( S? ',' S? cp )* S? ')' 
	dfa_token* 
	parseChildren();

	//! \brief parses attribute list
	//! [52]    AttlistDecl    ::=    '<!ATTLIST' S Name AttDef* S? '>' 
	void 
	parseAttrList();

	//! \brief parses attribute definition
	//! [53]    AttDef    ::=    S Name S AttType S DefaultDecl 
	void 
	parseAttDef(	elementDecl& decl						//!< element declaration
					);

	//! \brief attribute as enumaration
	//! [54]    AttType    ::=    StringType | TokenizedType | EnumeratedType  
	//! [55]    StringType    ::=    'CDATA' 
	//! [56]    TokenizedType    ::=    'ID' | 'IDREF' | 'IDREFS' | 'ENTITY' | 'ENTITIES' | 'NMTOKEN' | 'NMTOKENS'
	//! [57]    EnumeratedType    ::=    NotationType | Enumeration  
	//! [58]    NotationType    ::=    'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')'
	//! [59]    Enumeration    ::=    '(' S? Nmtoken (S? '|' S? Nmtoken)* S? ')' 
	void 
	parseAttrEnumeration(attributeDecl& decl				//!< attribute declaration
					);
	
	//! \brief parses attribute default value
	//! [60]    DefaultDecl    ::=    '#REQUIRED' | '#IMPLIED' | (('#FIXED' S)? AttValue) 
	void 
	parseDefaultDecl(attributeDecl& decl					//!< attribute declaration
					);

	//! \brief parses reference
	//! [10]    AttValue    ::=    '"' ([^<&"] | Reference)* '"' |  "'" ([^<&'] | Reference)* "'" 
	//! [67]    Reference    ::=    EntityRef | CharRef 
	//! [68]    EntityRef    ::=    '&' Name ';'
	//! [69]    PEReference    ::=    '%' Name ';'
	void 
	parsePEReference(bool skip_junk_before,					//!< flag to skip whitespaces before reference
					bool skip_junk_after					//!< flag to skip whitespaces after reference
					);

	//! \brief parses entity
	//! [70]    EntityDecl    ::=    GEDecl | PEDecl 
	void 
	parseEntity();

	//! \brief parser entity definition
	//! [71]    GEDecl    ::=    '<!ENTITY' S Name S EntityDef S? '>' 
	void 
	parseEntityDef(	entityDecl& decl						//!< entity declaration
					);
	//! \brief below parsing defined in base class
	//! [72]    PEDecl    ::=    '<!ENTITY' S '%' S Name S PEDef S? '>' 
	//! [73]    EntityDef    ::=    EntityValue | (ExternalID NDataDecl?) 
	//! [74]    PEDef    ::=    EntityValue | ExternalID 
	//! [9]    EntityValue    ::=    '"' ([^%&"] | PEReference | Reference)* '"'  |  "'" ([^%&'] | PEReference | Reference)* "'" 
	//! [75]    ExternalID    ::=    'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral  
	//! [76]    NDataDecl    ::=    S 'NDATA' S Name 
	
	//! \brief parses notation
	//! [82]    NotationDecl    ::=    '<!NOTATION' S Name S (ExternalID | PublicID) S? '>'
	void 
	parseNotation();

	//! \brief following parses defined in base class
	//! [83]    PublicID    ::=    'PUBLIC' S PubidLiteral 
	//! [16]    PI    ::=    '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>' 
	//! [17]    PITarget    ::=    Name - (('X' | 'x') ('M' | 'm') ('L' | 'l')) 
	//! [15]    Comment    ::=    '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->' 
	//! [77]    TextDecl    ::=    '<?xml' VersionInfo? EncodingDecl S? '?>' 
	
	//! [61]    conditionalSect    ::=    includeSect | ignoreSect  
	//! \brief parse include set
	//! [62]    includeSect    ::=    '<![' S? 'INCLUDE' S? '[' extSubsetDecl ']]>'
	void 
	parseInclude();

	//! \brief parses ignore list
	// [63]    ignoreSect    ::=    '<![' S? 'IGNORE' S? '[' ignoreSectContents* ']]>'
	// [64]    ignoreSectContents    ::=    Ignore ('<![' ignoreSectContents ']]>' Ignore)* 
	// [65]    Ignore    ::=    Char* - (Char* ('<![' | ']]>') Char*)  
	void 
	parseIgnore();

	//! \brief checks if the symbol is repeated
	dfa_token* 
	checkRepeation(	ub1_t symbol,							//!< symbol
					dfa_token* token						//!< DFA token
					);

	//! \brief expands the reference into entity declaration
	const entityDecl* 
	expandPEReference(paged_buffer& buffer					//!< input buffer
					);

	//! \brief converts CTYPE type to terimber virtual type
	//! non standard terimber extension of DTD
	vt_types 
	convert_ctype(	const char* x							//!< input buffer
					);
	//! \brief validates DTD
	void 
	validate();
	//! \brief validates the deterministic model
	void 
	deterministic_model(const dfa_token* token				//!< DFA root token
					);
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_dtdxml_h_ 
