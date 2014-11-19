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

#ifndef _terimber_parsexml_hpp_
#define _terimber_parsexml_hpp_

#include "xml/parsexml.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//////////////////////////////////////////////////////////

// [44] EmptyElemTag    ::=    '<' Name (S Attribute)* S? '/>' 
// [40] STag    ::=    '<' Name (S Attribute)* S? '>' 
// we can handle both
xml_forceinline 
void 
xml_processor::parseStartTag()
{
	// skips junk and resolves entity, if any
	if (pick() == ch_ampersand)
		parseGeneralReference(false);

	// adds element to document
	xml_element* el = _doc.add_element(parseName());

	// parses attribute
	parseAttributes(*el);

	// checks the close tag
	switch (pick())
	{
		case ch_close_angle: // '>'
			pop();
			// element isn't closed yet
			// next closeTag is expected
			// sets current element
			_doc.container_push(el);
			break;
		case ch_forward_slash:
			pop();
			skip_sign(ch_close_angle, false, false, "Expected close tag");
			// validate element before leaving
			if (_validate)
				_doc.validate(*el);

			if (!_white_space_stack.empty() && _white_space_stack.top()._el == el)
			{
				_white_space_stack.pop();
				if (_white_space_stack.empty())
					_white_space_allocator->reset();

				_preserve_white_space = _white_space_stack.empty() ? false : _white_space_stack.top()._preserve;
			}

			break;
		default: // 
			throw_exception("Expected close tag");		
	}
}


// [42]    ETag    ::=    '</' Name S? '>' 
xml_forceinline 
void 
xml_processor::parseEndTag()
{
	xml_element* el = _doc.container_pop();
	if (!el)
		throw_exception("Unexpected close tag");

	// checks name
	if (el->_decl->_name != parseName())
		throw_exception("Invalid close tag");

	if (!_white_space_stack.empty() && _white_space_stack.top()._el == el)
	{
		_white_space_stack.pop();
		if (_white_space_stack.empty())
			_white_space_allocator->reset();

		_preserve_white_space = _white_space_stack.empty() ? false : _white_space_stack.top()._preserve;
	}

	// validates element before leaving
	if (_validate) _doc.validate(*el);

	skip_sign(ch_close_angle, true, false, "Expected close tag");
}

// [43]    content    ::=    CharData? ((element | Reference | CDSect | PI | Comment) CharData?)* 
xml_forceinline 
void 
xml_processor::parseContent()
{
	// [43]    content    ::=    CharData? ((element | Reference | CDSect | PI | Comment) CharData?)* 
	reset_all_tmp();
	ub1_t symbol = pick();

	while (symbol)
	{
		switch (symbol)
		{
			case ch_open_angle:
			// [18]    CDSect    ::=    CDStart CData CDEnd 
			// [19]    CDStart    ::=    '<![CDATA[' 
			// OR
			// [16]    PI    ::=    '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>' 
			// OR
			// [15]    Comment    ::=    '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->' 
				switch (pop())
				{
					case ch_question:
						parsePI();
						break;
					case ch_bang:
						// CDStart OR Comment
						switch (pop())
						{
							case ch_dash:
								parseComment();
								break;
							case ch_open_square:
								pop();
								parseCDATA();
								break;
							default:
								throw_exception("Unexpected markup instruction");
								break;
						}
						break;
					default: // child element
						push(ch_open_angle);
						return;
				} // switch
				break;
			case ch_ampersand:
			// [67]    Reference    ::=    EntityRef | CharRef 
			// [68]    EntityRef    ::=    '&' Name ';' 
			// [66]    CharRef    ::=    '&#' [0-9]+ ';'  | '&#x' [0-9a-fA-F]+ ';'
			//	parseGeneralReference(false);
			//	break;
			default:
				parseCharData();
				break;
		} // switch

		symbol = pick();
	}
}

	
xml_forceinline 
void 
xml_processor::parseCharData()
{
	if (!_preserve_white_space)
		skip_white_space();

	ub1_t symbol = pick();
	if (symbol && symbol != ch_open_angle)
		_parseCharData();
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_parsexml_hpp_ 
