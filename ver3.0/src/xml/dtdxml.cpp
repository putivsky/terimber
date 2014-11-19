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

#include "xml/dtdxml.h"
#include "xml/declxml.hpp"
#include "xml/defxml.hpp"
#include "xml/mngxml.hpp"
#include "xml/sxml.hpp"
#include "xml/sxs.hpp"
#include "xml/storexml.hpp"
#include "xml/miscxml.hpp"

#include "base/list.hpp"
#include "base/map.hpp"
#include "base/stack.hpp"
#include "base/string.hpp"
#include "base/common.hpp"
#include "base/memory.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

dtd_processor::dtd_processor(	byte_source& stream, 
								xml_document& doc,
								mem_pool_t& small_pool, 
								mem_pool_t& big_pool,
								size_t xml_size) :
	byte_manager(stream, doc, small_pool, big_pool, xml_size)
{
}
//////////////////////////////////
// we can parse dtd as external and then stopSymbol = 0
// but also it can be a build-in dtd so we are parsing dtd 
// until stopSymbol = ']' appears

void 
dtd_processor::parse()
{
	// [' (markupdecl | DeclSep)* ']'
	parseSubSet(!get_subset() ? ch_close_square : ch_null, get_subset());
	validate();
	// set standalone
	_doc._standalone = 1;
}

void 
dtd_processor::parseSubSet(char stopSymbol, bool include_allowed)
{
	ub1_t symbol = 0;
	while ((symbol = pick()) && symbol != stopSymbol)
	{
		switch (symbol)
		{
			// [28a]    DeclSep    ::=    PEReference | S 
			// [69]    PEReference    ::=    '%' Name ';' 
			// [29]    markupdecl    ::=    elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment  
			case ch_percent:
				// [69]    PEReference    ::=    '%' Name ';' 
				parsePEReference(false, true);
				break;
			case ch_open_angle:
				// [45]    elementdecl    ::=    '<!ELEMENT' S Name S contentspec S? '>' 
				// [52]    AttlistDecl    ::=    '<!ATTLIST' S Name AttDef* S? '>' 
				// [70]    EntityDecl    ::=    GEDecl | PEDecl 
				// [71]    GEDecl    ::=    '<!ENTITY' S Name S EntityDef S? '>' 
				// [72]    PEDecl    ::=    '<!ENTITY' S '%' S Name S PEDef S? '>' 
				// [82]    NotationDecl    ::=    '<!NOTATION' S Name S (ExternalID | PublicID) S? '>' 
				// [16]    PI    ::=    '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>' 
				// [15]    Comment    ::=    '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->' 
				switch (pop()) // skip '<' and check next char
				{
					case ch_question:
						// '<?'
						parsePI();
						break;
					case ch_bang:
						// '<!ELEMENT'
						// '<!ATTLIST'
						// '<!ENTITY'
						// '<!NOTATION'
						// '<!--'
						switch (pop()) // skip '!' and checks the next char
						{
							case ch_E:
								switch (pop())
								{
									case ch_L: // '<!ELEMENT'
										parseElement();
										break;
									case ch_N: // '<!ENTITY'
										parseEntity();
										break;
									default:
										throw_exception("Invalid ELEMENT or ENTITY sections syntax");
								}
								break;
							case ch_A: // '<!ATTRLIST'
								parseAttrList();
								break;
							case ch_N: // '<!NOTATION'
								parseNotation();
								break;
							case ch_dash: // '<!-'
								parseComment();
        						break;
							case ch_open_square: 
								//if (stopSymbol != ch_null) // internal subset
								if (!include_allowed)
									throw_exception("IGNORE or INCLUDE sections are not allowed in internal dtd subset");

								pop(); // skip '['
								skip_sign(ch_I, true, false, "Expected IGNORE or INCLUDE sections");
								switch (pick()) // tests the next char
								{
									case ch_G:
										// '<![' S? 'IGNORE'
										parseIgnore();
										break;
									case ch_N:
										// '<![' S? 'INCLUDE'
										parseInclude();
										break;
									default:
										throw_exception("Invalid IGNORE or INCLUDE sections syntax");
										break;
								} // switch
								break;
							default:
								throw_exception("Invalid markup instruction syntax in the internal dtd section");
								break;
						} // switch 
						break;
					default:
						throw_exception("Unexpected char in dtd markup language");
						break;
				} // switch
				break;
			default: // only 'S'
				skip_white_space(true, "White space expected");
				break;
		} // switch

		reset_all_tmp(true);
		_doc.get_tmp_allocator().reset();
	} // while

}

void  
dtd_processor::parseIgnore()
{
	// [63]    ignoreSect    ::=    '<![' S? 'IGNORE' S? '[' ignoreSectContents* ']]>'
	// [64]    ignoreSectContents    ::=    Ignore ('<![' ignoreSectContents ']]>' Ignore)* 
	// [65]    Ignore    ::=    Char* - (Char* ('<![' | ']]>') Char*)  

	// first char of the IGNORE word has been eaten by caller
	skip_string(str_IGNORE + 1, "Invalid IGNORE section syntax");
	skip_sign(ch_open_square, true, true, "Expected IGNORE Bracket");

	// It assumes that
	//  we are already in the body, i.e. we've seen <![IGNORE[ at this point. So
	//  we just have to scan until we see a matching ]]> closing markup.
	//

	//  Depth starts at one because we are already in one section and want
    //  to parse until we hit its end.
    //
    size_t depth = 1;
	ub1_t symbol = 0;

    while (0 != (symbol = pick()))
    {
		switch (symbol)
		{
			case ch_open_angle:
				if (pop() == ch_bang && pop() == ch_open_square)
				{
					pop();
					++depth;        
				}
				continue;
			case ch_close_square:
				if (pop() == ch_close_square && pop() == ch_close_angle)
				{
					pop();
					if (!--depth)
						return;
				}
				continue;
			default:
				break;
        } // switch

		pop();
		skip_white_space();
    } // while

	// 
	throw_exception("Invalid IGNORE section syntax");
}

void  
dtd_processor::parseInclude()
{
	// [62]    includeSect    ::=    '<![' S? 'INCLUDE' S? '[' extSubsetDecl ']]>'
	// first char of the INCLUDE word has been eaten by caller
	skip_string(str_INCLUDE + 1, "Invalid INCLUDE section syntax");
	skip_sign(ch_open_square, true, true, "Expected open square tag symbol after INCLUDE section");
	parseSubSet(ch_close_square, true);
	skip_sign(ch_close_square, true, false, "Expected INCLUDE close bracket");
	skip_sign(ch_close_square, false, false, "Expected INCLUDE second close bracket");
	skip_sign(ch_close_angle, false, false, "Expected INCLUDE close angle");
}


void  
dtd_processor::parseElement()
{
	// [45]    elementdecl    ::=    '<!ELEMENT' S Name S contentspec S? '>' 
	// first char of the ELEMENT word has been eaten by caller
	skip_string(str_ELEMENT + 1, "Invalid ELEMENT section syntax");
	skip_white_space(true, "Expected white space");
   	
	parsePEReference(false, true);

	elementDecl& decl = _doc.add_element_decl(parseName(), false, true, false);
	skip_white_space(true, "Expected white space");
	parsePEReference(false, false);

    // And now scan the content model for this guy.
    parseContentSpec(decl);
    // Another check for a PE ref, but we don't require whitespace here
	skip_sign(ch_close_angle, true, false, "Expected close tag");
	_doc.add_element_desc(decl);
}

void  
dtd_processor::parseContentSpec(elementDecl& decl)
{
	skip_white_space();

	switch (pick())
	{
		case ch_E:
			skip_string(str_EMPTY, "Invalid EMPTY declaration");
			decl._content = CONTENT_EMPTY;
			break;
		case ch_A:
			skip_string(str_ANY, "Invalid ANY declaration");
			decl._content = CONTENT_ANY;
			break;
		case ch_open_paren:
			pop();
			// We could have a PE ref here, but don't require space
			parsePEReference(true, true);
			//  Now we look for a PCDATA string_t. If its PCDATA, then it must be a
			//  MIXED model. Otherwise, it must be a regular list of children in
			//  a regular expression perhaps.
			switch (pick())
			{
				case ch_pound:
					skip_string(str__PCDATA, "Invalid PCDATA declaration");
					decl._content = CONTENT_MIXED;
					decl._token = parseMixed();
					return;
				default: // must be children
					decl._content = CONTENT_CHILDREN;
					decl._token = parseChildren();
					return;
			} // switch
		default:
			throw_exception("Invalid element content");
    } // switch

	// validates Deterministic Content Models (Non-Normative)
	deterministic_model(decl._token);
}

dfa_token*  
dtd_processor::parseMixed()
{
    //  Creates an initial content spec node. Its just a leaf node with a
    //  PCDATA element id. This current node pointer will be pushed down the
    //  tree as we go.
    
	dfa_token* curToken = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token)))) dfa_token(DFA_LEAF, 0, 0, 0);
	dfa_token* orgToken = curToken;
	dfa_token* headToken = orgToken;
   
	_list< const elementDecl* > uniqueList;
	//
    //  We just loop around, getting the | character at the top and then
    //  looking for the next element name. We keep up with the last node
    //  and add each new one to its last node.
    //
	bool hasElement = false;
	ub1_t symbol = 0;

    while (0 != (symbol = pick()))
    {
       	skip_white_space();   
		switch (symbol = pick())
		{
			case ch_percent:
				parsePEReference(false, false);
				break;
			case ch_pipe:	
				pop(); // skips '|'
				parsePEReference(true, true);

				// parses name of element
				{
					const elementDecl& decl = _doc.add_element_decl(parseName(), false, false, false);

					// checks the first element after #PCDATA
					if (curToken == orgToken)
					{
						hasElement = true;
						curToken = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token)))) 
													dfa_token(DFA_CHOICE, 0, curToken, new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token)))) 
																							dfa_token(DFA_LEAF, &decl, 0, 0));
						// Remembers the top token
						headToken = curToken;
					}
					else
					{
						// searches for the present
						for (_list< const elementDecl* >::const_iterator iter = uniqueList.begin(); iter != uniqueList.end(); ++iter)
							if (*iter == &decl)
								throw_exception("Dublicate elements in Mixed model");

						dfa_token* oldLast = curToken->_last;
						curToken->_last = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token))))
													dfa_token(DFA_CHOICE, 0, oldLast, new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token))))
																							dfa_token(DFA_LEAF, &decl, 0, 0));

						// Make the new last node the current node
						curToken = curToken->_last;
					}

					// pushes to list
					uniqueList.push_back(*_tmp_allocator, &decl);
				}
				break;
			case ch_close_paren:
				if (ch_asterisk == pop()) // skips ')'
					skip_sign(ch_asterisk, false, false, "Expected asterisk symbol");
				
                //
                //  Creates a zero or more node and makes the original head
                //  node its first child.
                //
                if (hasElement) 
				        {
                    headToken = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token))))
													dfa_token(DFA_ASTERISK, 0, headToken, 0);
                }

                return headToken;
			default: // unkonwn
				throw_exception("Invalid element PCDATA syntax");
        } // switch
    } // while

	throw_exception("Invalid element PCDATA syntax");
	return 0;
}

dfa_token*
dtd_processor::checkRepeation(ub1_t symbol, dfa_token* token)
{
	dfa_token* retVal = token;

	switch (symbol)
	{
		case ch_question: // '?'
			retVal = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token)))) dfa_token(DFA_QUESTION, 0, token, 0);
			pop();
			break;
		case ch_asterisk: // '*'
			retVal = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token)))) dfa_token(DFA_ASTERISK, 0, token, 0);
			pop();
			break;
		case ch_plus: // '+'
			retVal = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token)))) dfa_token(DFA_PLUS, 0, token, 0);
			pop();
			break;
		default:
			break;
	}

	return retVal;
}

dfa_token*
dtd_processor::parseChildren()
{
	parsePEReference(true, true);

	ub1_t symbol = 0;
	ub1_t type_char = 0;
	dfa_token* curToken = 0;
	dfa_token* headToken = 0;

	switch (pick())
	{
		case ch_open_paren: // group
			{
				pop(); // skips '('
				// here we don't know the sub group type
				// Lets call ourself and get back the resulting node
				curToken = parseChildren();
			}
			break;
		default: // element name
			{
				const elementDecl& decl = _doc.add_element_decl(parseName(), false, false, false);
				//  Creates a leaf token for it.
				curToken = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token)))) 
													dfa_token(DFA_LEAF, &decl, 0, 0);

				parsePEReference(true, true);
				curToken = checkRepeation(pick(), curToken);
			}
	} // switch

    // checks the next symbol
	parsePEReference(true, true);

	switch (type_char = pick())
	{
		case ch_comma:
		case ch_pipe:
			headToken = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token))))
						dfa_token(type_char == ch_comma ? DFA_SEQUENCE : DFA_CHOICE, 0, curToken, 0);
			curToken = headToken;
			break;
		case ch_close_paren:
			headToken = curToken;
			pop();
			break;
		default:
			throw_exception("Expected CHOICE or SEQUENCE or CLOSE PAREN syntax");
	} // switch

	if ((type_char == ch_comma) || (type_char == ch_pipe))
	{
		dfa_token* lastToken = 0;
		dfaRule typeRule = type_char == ch_comma ? DFA_SEQUENCE : DFA_CHOICE;

		while (pick())
		{
			parsePEReference(true, true);

			symbol = pick();

			if (symbol == ch_close_paren)
			{
				pop(); // skips ')'
				//  We've hit the end of this section, so break out. But, we
				//  need to see if we left a partial sequence of choice nodes
				//  without a second node. If so, we have to undo that and
				//  put its left child into the right node of the previous
				//  node.
				if (!curToken->_last)
				{
					dfa_token* oldFirst = curToken->_first;
					curToken->_first = 0;
					lastToken->_last = oldFirst;
					curToken = lastToken;
				}
				break;
			}
			else if (symbol == ch_comma || symbol == ch_pipe)
			{
				if (symbol != type_char)
					throw_exception("Expected legal CHOICE/SEQUENCE syntax");

				pop(); // skips '|' or ','
				parsePEReference(true, true);

				if ((symbol = pick()) == ch_open_paren)
				{
					pop(); // skips '('
					// Recurses to handle this new guy
					dfa_token* newToken = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token))))
												dfa_token(typeRule, 0, parseChildren(), 0);

					
					curToken->_last = newToken;
					lastToken = curToken;
					curToken = newToken;
				}
				else
				{
					//
					//  Has to be a leaf node, so gets a name. If it cannot get
					//  one, then it cleans up and gets out of here.
					//
					const elementDecl& decl = _doc.add_element_decl(parseName(), false, false, false);
					dfa_token* tmpToken = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token))))
												dfa_token(DFA_LEAF, &decl, 0, 0);

					parsePEReference(true, true);

					tmpToken = checkRepeation(pick(), tmpToken);

					//
					//  Creates a new sequence or choice node, with the leaf
					//  (or rep surrounding it) we just recieved as its first node.
					//  Makes the new node the second node of the current node,
					//  and then makes it the current node.
					//
					dfa_token* newToken = new(check_pointer(_doc.get_model_allocator().allocate(sizeof(dfa_token))))
												dfa_token(typeRule, 0, tmpToken, 0);

					curToken->_last = newToken;
					lastToken = curToken;
					curToken = newToken;
				}
			} // else if
			else
				throw_exception("Expected legal CHOICE/SEQUENCE syntax");
		} // while
	} // if

    //
    //  We saw the terminating parenthesis so lets check for any repetition
    //  character, and create a node for that, making the head node its child
    //  
    //
	return checkRepeation(pick(), headToken);
}

void  
dtd_processor::parseEntity()
{
	// [70]    EntityDecl    ::=    GEDecl | PEDecl 
	// first char of the ENTITY word has been eaten by caller
	skip_string(str_ENTITY + 1, "Invalid ENTITY section syntax");
	skip_white_space(true, "Expected white space");

	// next must be either the persent or name or entity
	bool bPEDecl = false;
	if (pick() == ch_percent)
	{
		pop(); // skips '%'
		skip_white_space(true, "Expected white space");
		bPEDecl = true;
	}


	// checks in the entity map
	bool wasAdded = false;
	entityDecl dummy(0, _tmp_allocator);
	entityDecl& entry = _doc.add_entity_decl(parseName(), wasAdded);
	entityDecl& entity = wasAdded ? entry : dummy;

	entity._is_parameter = bPEDecl;

	skip_white_space(true, "Expected white space");
	parsePEReference(false, true);

	// [73]    EntityDef    ::=    EntityValue | (ExternalID NDataDecl?) 
	parseEntityDef(entity);
	skip_sign(ch_close_angle, true, false, "Expected close tag");
	if (wasAdded)
		_doc.add_entity_desc(entry);
}

void  
dtd_processor::parseEntityDef(entityDecl& decl)
{
	// [73]    EntityDef    ::=    EntityValue | (ExternalID NDataDecl?)
	reset_all_tmp();

	ub1_t symbol = 0;
	size_t counter = 0;
	bool met_xD = false;

	// checks quote
	if ((symbol = pick()) == ch_double_quote || symbol == ch_single_quote)
	{
		ub1_t quote = skip_quote(0);
		while (0 != (symbol = pick()))
		{
			// checks the next char
			if(symbol == ch_percent) // PERef
			{
				// expands the Parameter Entity
				// and continue the parsing process
				// preventing recursive reference
				if (&decl == expandPEReference(_tmp_store2))
					throw_exception("Recursive PE is not allowed");

				size_t len = 0;
				const ub1_t* ptr = _tmp_store2.persist(len);
				push(ptr, len);
				_tmp_store2.reset();
				continue;
			}
			else if (symbol == ch_ampersand)
			{
				symbol = pop(); // skips '&'
				if (symbol == ch_pound) // CharRef
					parseCharRef(_tmp_store3);
				else // Entity Reference as it is
				{
					// NB!!! inside _tmp_store1 will be used
					const char* value = parseName();
					skip_sign(ch_semicolon, false, false, "Expected semicolon after Entity Reference");
					_tmp_store3 << ch_ampersand << value << ch_semicolon;
				}
			}
			else if (symbol == quote)
			{
				pop();
				// checks the last xD
				if (met_xD) _tmp_store3 << ch_cr;
				decl._value = _tmp_store3.persist();
				return;
			}
			else
			{
				// translates the two-character sequence #xD #xA 
				// and any #xD that is not followed by #xA to #xA 
				// on input before parsing
				switch (symbol)
				{
					case ch_lf: // #xA
						_tmp_store3 << ch_lf;
						// resets boolean
						met_xD = false; // all xD before xA were skipped
						break;
					case ch_cr:	// #xD
						// sets boolean
						met_xD = true;
						break;
					default:
						if (met_xD) // we previously got xD but there isn't a next xA
						{
							// replaces to xA
							_tmp_store3 << ch_lf;
							// resets boolean
							met_xD = false;
						}
                        _tmp_store3 << symbol;
				} // switch
				pop();
			}
		} // while

		if (!symbol)
			throw_exception("Invalid ENTITY syntax");
	} // if

	// [75]    ExternalID    ::=    'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral  
	skip_white_space();
	string_t value_system(_tmp_allocator);
	string_t value_public(_tmp_allocator);
	parseExternalID(value_system, value_public, true);

	decl._systemId = value_system;
	decl._publicId = value_public;

	if (!decl._is_parameter && is_white_space(pick())) // NDATA can be
	{
		// [76]    NDataDecl    ::=    S 'NDATA' S Name
		parsePEReference(true, true);

		if (pick() == ch_N)
		{
			skip_string(str_NDATA, "Invalid NDATA syntax");
			skip_white_space(true, "Expected white space");
			parsePEReference(false, true);

			decl._notation = parseName();
			decl._is_unparsed = true;
		} // if
	} // else

	decl._is_in_subset = true;
}
	
void  
dtd_processor::parseAttrList()
{
	skip_string(str_ATTRLIST, "Invalid ATTRLIST section syntax");
	// Space is required here
	skip_white_space(true, "Expected white space");
	parsePEReference(false, true);


	elementDecl& decl = _doc.add_element_decl(parseName(), false, false, false);

	while (pick())
	{
		skip_white_space();

		switch (pick())
		{
			case ch_close_angle:
				pop(); // skips '>'
				return;
			case ch_percent:
				parsePEReference(false, false);
				break;
			default: // attribute Def
				parseAttDef(decl);
				break;
		}
	} // while 

	throw_exception("Invalid ATTLIST syntax");
}

void  
dtd_processor::parseAttDef(elementDecl& decl)
{
	bool wasAdded = false;
	attributeDecl dummy(0, _tmp_allocator);

	attributeDecl& new_decl = _doc.add_attribute_decl(decl, parseName(), false, wasAdded);
	attributeDecl& attr_decl = wasAdded ? new_decl : dummy;

	skip_white_space(true, "Expected white space");
	// next must be type
	parsePEReference(false, false);

	switch (pick())
	{
		case ch_C: // CDATA
			if (pop() == ch_D)
			{
				skip_string(str_CDATA + 1, "Invalid CDATA syntax");
				attr_decl._atype = ATTR_TYPE_CDATA;
				attr_decl._ctype = vt_string;
			}
			else
			{
				skip_string(str_CTYPE + 1, "Invalid CTYPE syntax");
				attr_decl._atype = ATTR_TYPE_CDATA;
				skip_white_space(true, "Expected white space");
				attr_decl._ctype = convert_ctype(parseName());
			}
			break;
		case ch_I: // ID, IDREF, IDREFS
			skip_string(str_ID, "Invalid CDATA syntax");
			if (pick() != ch_R)
			{
				attr_decl._atype = ATTR_TYPE_ID;
				if (wasAdded)
				{
					// checks id uniqueness
					for (attribute_decl_map_t::const_iterator iter = decl._attributes.begin(); iter != decl._attributes.end(); ++iter)
						if (&*iter != &new_decl && iter->_atype == ATTR_TYPE_ID)
							throw_exception("Dublicate ID type for the same element");
				}
			}
			else
			{
				skip_string(str_REF, "Invalid IDREF syntax");
				if (pick() == ch_S)
				{
					attr_decl._atype = ATTR_TYPE_IDREFS;
					pop(); // skips 'S'
				}
				else
					attr_decl._atype = ATTR_TYPE_IDREF;
			}

			attr_decl._ctype = vt_string;
			break;
		case ch_E:
			skip_string(str_ENTIT, "Invalid ENTITY syntax");
			if (pick() == ch_Y)
			{
				attr_decl._atype = ATTR_TYPE_ENTITY;
				pop(); // skips 'Y'
			}
			else if (pick() == ch_I && pop() == ch_E && pop() == ch_S)
			{
				attr_decl._atype = ATTR_TYPE_ENTITIES;
				pop(); // skips 'S'
			}
			else
				throw_exception("Invalid ENTITY syntax");

			attr_decl._ctype = vt_string;
			break;
		case ch_N:
			if (pop() == ch_M)
			{
				skip_string(str_NMTOKEN + 1, "Invalid NMTOKEN syntax");
				if (pick() == ch_S)
				{
					attr_decl._atype = ATTR_TYPE_NMTOKENS;
					pop(); // skips 'S'
				}
				else
					attr_decl._atype = ATTR_TYPE_NMTOKEN;

				attr_decl._ctype = vt_string;
			}
			else// notation
			{
				skip_string(str_NOTATION + 1, "Invalid NOTATION syntax");
				skip_white_space(true, "Expected white space");
				attr_decl._atype = ATTR_TYPE_NOTATION;
				attr_decl._ctype = vt_enum;

				if (wasAdded)
				{
					// checks empty element
					if (decl._content == CONTENT_EMPTY)
						throw_exception("An attribute of type NOTATION must not be declared on an element declared EMPTY");

					// checks id uniqueness
					for (attribute_decl_map_t::const_iterator iter = decl._attributes.begin(); iter != decl._attributes.end(); ++iter)
						if (&*iter != &new_decl && iter->_atype == ATTR_TYPE_NOTATION)
							throw_exception("No element type may have more than one NOTATION attribute specified");
				}

				parseAttrEnumeration(attr_decl);
			}
			break;
		case ch_open_paren:
			{
				attr_decl._atype = ATTR_TYPE_ENUMERATION;
				attr_decl._ctype = vt_enum;
				parseAttrEnumeration(attr_decl);
			}
			break;
		default:
			throw_exception("Unexpected char in attribute definition");
    } // switch

	skip_white_space(true, "Expected white space");
	parsePEReference(false, true);

    // And then scans for the optional default value declaration
    parseDefaultDecl(attr_decl);

	// validate there something if first
	if (!wasAdded)
		return;

	if (attr_decl._atype == ATTR_TYPE_ID && attr_decl._rule != ATTR_RULE_IMPLIED && attr_decl._rule != ATTR_RULE_REQUIRED)
		throw_exception("Invalid AttrDef syntax");

	// A special attribute named xml:space may be attached to an element to signal an intention that in that element, 
	// white space should be preserved by applications. In valid documents, this attribute, like any other, 
	// must be declared if it is used. 
	// When declared, it must be given as an enumerated type whose values are one or both of "default" and "preserve". 
	if (!strcmp(str_xml_space, attr_decl._name))
	{
		if (attr_decl._atype == ATTR_TYPE_ENUMERATION)
		{
			size_t count = attr_decl._enum.size();

			if (count < 1 || count > 2)
				throw_exception("Invalid xml space syntax");

			if (count == 1 && 
				!(	attr_decl._enum.front()._value == str_default 
					//|| attr_decl._enum.front()._value != str_preserve)
					|| attr_decl._enum.front()._value == str_preserve)
				)
				throw_exception("Invalid xml space syntax");

			if (count == 2 && 
				!(attr_decl._enum.front()._value == str_default
					&& attr_decl._enum.back()._value == str_preserve
					|| attr_decl._enum.back()._value == str_default
					&& attr_decl._enum.front()._value == str_preserve)
				)
				throw_exception("Invalid xml space syntax");
		}
		else if (attr_decl._atype == ATTR_TYPE_CDATA && attr_decl._rule == ATTR_RULE_FIXED)
		{
			if (attr_decl._defval != str_default
				&& attr_decl._defval != str_preserve)
				throw_exception("Invalid xml space syntax");
		}
		else
			throw_exception("Invalid xml space syntax");
	}
	
	// check default
	if (attr_decl._defval.length())
	{
		const char* value_ = attr_decl._defval;
		switch (attr_decl._atype)
		{
			case ATTR_TYPE_ENUMERATION:
				{
					bool findDefault = false;
					for (_list< enumNodeDecl >::const_iterator iter = attr_decl._enum.begin(); iter != attr_decl._enum.end(); ++iter)
					{
						if (!findDefault && attr_decl._defval == iter->_value)
						{
                            findDefault = true;
							break;
						}
					}

					if (!findDefault)
						throw_exception("Default value doesn't match the enumeration items");
				}
				break;
			case ATTR_TYPE_ID:
			case ATTR_TYPE_NMTOKEN:
				//check_string_content(attr_decl._defval, attr_decl._defval.length(), is_name_char, ATTR_TYPE_ID ? "Invalid char in ID default value" : "Invalid char in NMTOKEN default value");
				break;
			case ATTR_TYPE_NMTOKENS:
				{
					_list< const char* > values;
					tokenValues(value_, values, *_tmp_allocator);
					_list< const char* > defvalues;
					tokenValues(attr_decl._defval, defvalues, *_tmp_allocator); 

					if (values.empty())
						throw_exception("Invalid ENTITIES default value syntax");

					for (_list< const char* >::const_iterator iter = values.begin(); iter != values.end(); ++iter)
					{
						//check_string_content(*iter, -1, is_name_char, "Illigal token char in ENTITIES default value");
						bool findDefault = false;
						for (_list< const char* >::const_iterator defiter = defvalues.begin(); defiter != defvalues.end(); ++defiter)
						{
							if (!findDefault && !strcmp(*defiter, *iter))
							{
								findDefault = true;
								break;
							}
						}
					
						if (!findDefault)
							throw_exception("Default value doesn't match the enumeration items");
					}
				}
				break;
			default:
				break;
		} // switch
	} // if
}

void  
dtd_processor::parseAttrEnumeration(attributeDecl& decl)
{
	//if (decl._atype == ATTR_TYPE_NOTATION)
	//	skip_white_space(true, "Expected white space");

	skip_sign(ch_open_paren, false, false, "Expected open paren symbol");

	size_t counter = 0;

    while (pick())
    {
		parsePEReference(true, true);

		enumNodeDecl attrEnum(&_doc.get_model_allocator());
		attrEnum._id = counter++;
		attrEnum._value = parseValue();
		decl._enum.push_back(_doc.get_model_allocator(), attrEnum);

		skip_white_space();
        // Checks for the terminating paren
		if (pick() == ch_close_paren)
		{
			pop();
			return;
		}

		skip_sign(ch_pipe, false, false, "Expected Enum Separator");
    }
}

void  
dtd_processor::parseDefaultDecl(attributeDecl& decl)
{
	switch (pick())
	{
		case ch_pound: // required or implied
			if (pop() == ch_R)
			{
				skip_string(str_REQUIRED, "Invalid REQUIRED syntax");
				decl._rule = ATTR_RULE_REQUIRED;
			}
			else if (pick() == ch_I)
			{
				skip_string(str_IMPLIED, "Invalid IMPLIED syntax");
				decl._rule = ATTR_RULE_IMPLIED;
			}
			else if (pick() == ch_F) // fixed
			{
				skip_string(str_FIXED, "Invalid FIXED syntax");
				decl._rule = ATTR_RULE_FIXED;

				skip_white_space(true, "Expected white space");
				// must be default value
				// resolves entities
				//decl._defval = parseQuotedValue(true, false, is_name_char, "Illigal token char in attribute default value");
				decl._defval = parseQuotedValue(true, false, 0, 0);
			}
			break;
		default:
			{
				// checks for ID constrain
				if (decl._atype == ATTR_TYPE_ID)
					throw_exception("An ID attribute must have a declared default of #IMPLIED or #REQUIRED");
				decl._rule = ATTR_RULE_REQUIRED;
				// must be default value
				//decl._defval = parseQuotedValue(true, false, is_name_char, "Illigal token char in attribute default value");
				decl._defval = parseQuotedValue(true, false, 0, 0);
			}
			break;
	} // switch
}

void  
dtd_processor::parseNotation()
{
	skip_string(str_NOTATION, "Invalid NOTATION section syntax");
	skip_white_space(true, "Expected white space");
	parsePEReference(false, true);

	notationDecl& decl = _doc.add_notation_decl(parseName());

	skip_white_space(true, "Expected white space");
	parsePEReference(false, true);

	string_t value_system(_tmp_allocator);
	string_t value_public(_tmp_allocator);

	parseExternalID(value_system, value_public, false);

	decl._publicId = value_public;
	decl._systemId = value_system;
	skip_sign(ch_close_angle, true, false, "Expected close tag");
}


void  
dtd_processor::validate()
{
}

void  
dtd_processor::parsePEReference(bool skip_junk_before, bool skip_junk_after)
{
	// [69]    PEReference    ::=    '%' Name ';'
	if (skip_junk_before && is_white_space(pick()))
		skip_white_space();

	if (pick() == ch_percent)
	{
		// resolves value
		_tmp_store2.reset();
		expandPEReference(_tmp_store2);
		size_t len = 0;
		const ub1_t* ptr = _tmp_store2.persist(len);
		push(ptr, len);
		_tmp_store2.reset();

		if (skip_junk_after && is_white_space(pick()))
			skip_white_space();
	}
}

const entityDecl*  
dtd_processor::expandPEReference(paged_buffer& buffer)
{
	assert(pick() == ch_percent);
	pop(); // skips '%'
	
	// parses name
	const entityDecl* entry = _doc.find_entity_decl(parseName());
	if (!entry)
		throw_exception("Unresolved parameter entity");

	skip_sign(ch_semicolon, false, false, "Expected semicolon symbol");

	// we have entity value
	if (entry->_value.length())
		buffer << entry->_value;
	else if (entry->_systemId.length())
		// loads
		buffer_loader::load(_stream.get_location(), entry->_systemId, _small_pool, _big_pool, buffer, false);

	return entry;
}

void 
dtd_processor::deterministic_model(const dfa_token* token)
{
}

vt_types 
dtd_processor::convert_ctype(const char* x)
{
	if (!x)
		throw_exception("Expected valid type");

	// predefined types
	//! bool, sb1, ub1, sb2, ub2, sb4, ub4, float, double, sb8, ub8, date, 
	//! string, wstring, decimal, numeric, binary, guid
	// x can be one of the next
	if (*x != ch_v || *++x != ch_t || *++x != ch_underscore)
		throw_exception("Unknown ctype");

	switch (*++x)
	{
		case ch_u: // vt_unknown, vt_ub1, vt_ub2, vt_ub4, vt_ub8
			if (*++x != ch_b) throw_exception("Unknown ctype");
			switch (*++x)
			{
				case ch_1:
					if (*++x) throw_exception("Unknown ctype");
					return vt_ub1;
				case ch_2:
					if (*++x) throw_exception("Unknown ctype");
					return vt_ub2;
				case ch_4:
					if (*++x) throw_exception("Unknown ctype");
					return vt_ub4;
				case ch_8:
					if (*++x) throw_exception("Unknown ctype");
					return vt_ub8;
				default:
					throw_exception("Unknown ctype");
			}
		case ch_s: // vt_sb1, vt_sb2, vt_sb4, vt_sb8, vt_string
			switch (*++x)
			{
				case ch_b: // vt_sb1, vt_sb2, vt_sb4, vt_sb8
					switch (*++x)
					{
						case ch_1:
							if (*++x) throw_exception("Unknown ctype");
							return vt_sb1;
						case ch_2:
							if (*++x) throw_exception("Unknown ctype");
							return vt_sb2;
						case ch_4:
							if (*++x) throw_exception("Unknown ctype");
							return vt_sb4;
						case ch_8:
							if (*++x) throw_exception("Unknown ctype");
							return vt_sb8;
						default:
							throw_exception("Unknown ctype");
					}
				case ch_t:
					if (*++x != ch_r || *++x != ch_i || *++x != ch_n || *++x != ch_g || *++x) throw_exception("Unknown ctype");
					return vt_string;
				default:
					throw_exception("Unknown ctype");
			}
		case ch_f: // 
			if (*++x != ch_l || *++x != ch_t) throw_exception("Unknown ctype");
			switch (*++x)
			{
				case ch_3:
					if (*++x != ch_2 || *++x) throw_exception("Unknown ctype");
					return vt_float;
				case ch_6:
					if (*++x != ch_4 || *++x) throw_exception("Unknown ctype");
					return vt_double;
				default:
					throw_exception("Unknown ctype");
			}
		case ch_b:
			switch (*++x)
			{
				case ch_o:
					if (*++x != ch_o || *++x != ch_l || *++x) throw_exception("Unknown ctype");
					return vt_bool;
				case ch_i:
					if (*++x != ch_n || *++x != ch_a || *++x != ch_r || *++x != ch_y || *++x) throw_exception("Unknown ctype");
					return vt_binary;
				default:
					throw_exception("Unknown ctype");
			}
		case ch_g:
			if (*++x != ch_u || *++x != ch_i || *++x != ch_d || *++x) throw_exception("Unknown ctype");
			return vt_guid;
		case ch_d:
			switch (*++x)
			{
				case ch_a:
					if (*++x != ch_t || *++x != ch_e || *++x) throw_exception("Unknown ctype");
					return vt_date;
				case ch_e:
					if (*++x != ch_c || *++x != ch_i || *++x != ch_m || *++x != ch_a || *++x != ch_l || *++x) throw_exception("Unknown ctype");
					return vt_decimal;
				default:
					throw_exception("Unknown ctype");
			}
		case ch_w:
			if (*++x != ch_s || *++x != ch_t || *++x != ch_r || *++x != ch_i || *++x != ch_n || *++x != ch_g || *++x) throw_exception("Unknown ctype");
			return vt_wstring;
		case ch_n:
			if (*++x != ch_u || *++x != ch_m || *++x != ch_e || *++x != ch_r || *++x != ch_i || *++x != ch_c || *++x) throw_exception("Unknown ctype");
			return vt_numeric;
		default:
			throw_exception("Unknown ctype");
	} // switch

	return vt_unknown;
}

#pragma pack()
END_TERIMBER_NAMESPACE
