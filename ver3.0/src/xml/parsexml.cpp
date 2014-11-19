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

#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/stack.hpp"
#include "base/string.hpp"
#include "base/common.hpp"
#include "base/template.hpp"

#include "xml/parsexml.hpp"
#include "xml/declxml.hpp"
#include "xml/defxml.hpp"
#include "xml/miscxml.hpp"
#include "xml/mngxml.hpp"
#include "xml/sxml.hpp"
#include "xml/sxs.hpp"
#include "xml/storexml.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

///////////////////////////////////////////////////////
xml_processor::xml_processor(byte_source& stream,
							 xml_document& doc,
							 mem_pool_t& small_pool,
							 mem_pool_t& big_pool,
							 size_t xml_size,
							 bool validate) :
	byte_manager(stream, doc, small_pool, big_pool, xml_size),
	_doc(doc),
	_validate(validate)
{
	_white_space_allocator = _small_pool.loan_object();
}

xml_processor::~xml_processor()
{
	_small_pool.return_object(_white_space_allocator);
}

const char* 
xml_processor::get_error() const
{ 
	return _error; 
}

// [1]    document    ::=    prolog element Misc* 
bool
xml_processor::parse()
{
	try
	{
		// puts document to the stack
		_doc.container_reset();
		_white_space_stack.clear();
		_white_space_allocator->reset();

		_entity_map.clear();
		_entity_allocator->reset();
		_preserve_white_space = false;
    
		_doc.add_escaped_symbols();
		// we suppose that the byte_source is a UTF-8 sequence of chars
		// also byte_source internally try to define the current charset automatically
		// or we can set the encoding schema explicitly
		// but by default it should be automatic

		// [1]    document    ::=    prolog element Misc* 
		parseDocument();

		// skips tailing white chars
		skip_white_space();

		if (pick() != 0) // something unrecognized after document
			throw_exception("Unrecognized chars after document");

		resolve_references();
	}
	catch (exception& x)
	{
		_error = x.what();
		return false;
	}
	catch (...)
	{
		_error = "Unexpected exception has been thrown";
		return false;
	}

	return true;
}

void  
xml_processor::parseDocument()
{
	// [1]    document    ::=    prolog element Misc* 
	parseProlog();

	// sets standalone in advance
	if (_doc._standalone != os_minus_one)
		_doc._standalone = get_standalone();
	
	parseElement();
	// checks completion
	if (_doc.container_peak())
		throw_exception("Invalid Root element syntax");

	// checks root find or not
	if (!_doc.get_root_element()._decl)
		throw_exception("Can't find Root element");

	parseMisc();
}

void  
xml_processor::parseProlog()
{
	// [22]    prolog    ::=    XMLDecl? Misc* (doctypedecl Misc*)? 
	// however XMLDecl parsing is responsibility of base class byte_source
	// because this is only one way to know the current encoding
	// it's XMLDecl parsing
	
	// [27]    Misc    ::=    Comment | PI | S
	// [28] doctypedecl    ::=    '<!DOCTYPE' S Name (S ExternalID)? S? ('[' (markupdecl | DeclSep)* ']' S?)? '>' 
	skip_white_space();
	while (pick() == ch_open_angle)
	{		
		switch (pop()) // skips '<' and checks the next char
		{
			// Misc* (doctypedecl Misc*)?
			// doctype '<!DOCTYPE'
			// comment '<!--'
			// PI '<?'
			case ch_question:
				parsePI();
				break;
			case ch_bang:
				// doctype '<!DOCTYPE'
				// comment '<!--'
				switch (pop()) // skips '!' and checks the next char
				{
					case ch_dash: // '<!-'
						parseComment();
						break;
					case ch_D:
						parseDocTypeDecl();
						break;
					default:
						return;
				}
				break;
			default:
				// returns open tag to char stack
				push(ch_open_angle);
				return;
		} // switch

		skip_white_space();

		reset_all_tmp(true);
		_doc.get_tmp_allocator().reset();
	}
}

void
xml_processor::parseDocTypeDecl()
{
	// [28] doctypedecl    ::=    '<!DOCTYPE' S Name (S ExternalID)? S? ('[' (markupdecl | DeclSep)* ']' S?)? '>' 
	skip_string(str_DOCTYPE, "Invalid DOCTYPE syntax");
	skip_white_space(true, "Expected white char");

	// pushes
	// name of the document
	if (!_doc.set_doc_name(parseName()) && _doc.is_on_fly())
		throw_exception("Invalid DOCTYPE name");

	_doc.container_start_doctype();

	skip_white_space();
	// checks externalID
	// ExternalID    ::=    'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral 
	if (pick() == ch_S || pick() == ch_P)
	{
		if (_doc._standalone == os_minus_one)
		{
			if (get_standalone() == 1)
				throw_exception("Illegal external DOCTYPE in standalone document");
			else if (get_standalone() == os_minus_one)
				_doc._standalone = 0;
		}

		// (S ExternalID)?
		parseExternalID(_doc._system_id, _doc._public_id, true);
		if (_doc.is_on_fly() && _doc._system_id.length())
			parseDTD(_doc._system_id);
	}

	// checks markup
	skip_white_space();
	if (pick() == ch_open_square)
	{
		// '[' (markupdecl | DeclSep)* ']' S?)? '>'
		pop(); // skips '['
		if (_doc.is_on_fly())
			parseDTD(); // the close tag ']' will be skipped inside parseDTD
		else // skips internal dtd
			skipDTD();

		skip_white_space();
	}

	skip_sign(ch_close_angle, false, false, "Expected close tag for DOCTYPE");

	if (!_doc.check_root())
		throw_exception("Invalid root element name");

	_doc.container_stop_doctype();
}

void 
xml_processor::parseMisc()
{
	// [27]    Misc    ::=    Comment | PI | S
	skip_white_space();

	while (pick() == ch_open_angle)
	{
		switch (pop())
		{
			case ch_question:
				// PI '<?'
				parsePI();
				break;
			case ch_bang:
				if (ch_dash == pop()) // skip '!'
				{
					// comment <!--
					parseComment();
					break;
				}
			default:
				throw_exception("Invalid markup instruction in Misc, PI or comment are allowed only");
				break;
		} // switch

		skip_white_space();
		reset_all_tmp(true);
		_doc.get_tmp_allocator().reset();
	} // while
}

void  
xml_processor::parseElement()
{
	skip_white_space();
	if (pick() == ch_ampersand)
		parseGeneralReference(true);

	// [39]    element    ::=    EmptyElemTag | STag content ETag 
	while (pick() == ch_open_angle)
	{
		switch (pop())
		{
			case ch_forward_slash: // '</'
				pop(); // skips slash
				parseEndTag();
				// checks element stack
				if (!_doc.container_peak())
					return;

				parseContent();
				break;
			default:
				// EmptyElemTag | STag
				parseStartTag();
				// check element stack
				if (!_doc.container_peak())
					return;

				// [43]    content    ::=    CharData? ((element | Reference | CDSect | PI | Comment) CharData?)* 
				parseContent();
				break;
		} // switch

		reset_all_tmp(true);
		_doc.get_tmp_allocator().reset();
	} // while
}

void 
xml_processor::parseAttributes(xml_element& el)
{
	// map of attribute
	attr_states_map_t attrStates;
	attr_states_map_t::iterator iterState;

	// fills map
	for (attribute_decl_map_t::const_iterator iterDecl = el.cast_decl()->_attributes.begin(); iterDecl != el.cast_decl()->_attributes.end(); ++iterDecl)
		attrStates.insert(*_tmp_allocator, &*iterDecl, 0);

	// skips junk
	skip_white_space();
	ub1_t symbol = pick();

	while (symbol && symbol != ch_forward_slash && symbol != ch_close_angle)
	{
		if (symbol == ch_ampersand)
			parseGeneralReference(true);

		string_t attr_name(_tmp_allocator);
		string_t attr_value(_tmp_allocator);
		// parses attribute
		parseAttributeValue(attr_name, attr_value);
		// adds attribute
		xml_value_node* attrRef = _doc.add_attribute(el, attr_name, attr_value);

		iterState = attrStates.find(attrRef->cast_to_attribute());
		
		if (iterState == attrStates.end())
			attrStates.insert(*_tmp_allocator, attrRef->cast_to_attribute(), attrRef);
		else if (*iterState)
			throw_exception("Dublicate attribute");
		else
			*iterState = attrRef;

		// checks spaces
		symbol = pick();
		switch (symbol)
		{
			case ch_forward_slash:
			case ch_close_angle:
				break;
			default:
				// checks next attribute
				skip_white_space(true, "Must be separator between attributes");
				symbol = pick();
		} // switch 
	} // while

	// adds default attributes here
	if (!_doc.is_on_fly() && attrStates.size())
		_doc.add_def_attributes(el, attrStates);

	// checks xml:space attribute
	const attributeDecl* decl_xml_space = _doc.find_attribute_decl(*el.cast_decl(), str_xml_space);
	if (decl_xml_space && (iterState = attrStates.find(decl_xml_space)) != attrStates.end() && *iterState)
	{		
		// resolves 
		for (_list< enumNodeDecl >::const_iterator iterEnum = decl_xml_space->_enum.begin(); iterEnum != decl_xml_space->_enum.end(); ++iterEnum)
		{
			if (iterEnum->_id == (*iterState)->_value.lVal)
			{
				_preserve_white_space = iterEnum->_value == str_preserve;
				xml_white_space_handler handler(&el, _preserve_white_space);
				_white_space_stack.push(*_white_space_allocator, handler);
				break;
			}
		}
	}
}

// CDATA section starts with '<![CDATA'
void  
xml_processor::parseCDATA()
{
	// caller has skipped '<![' already
	skip_string(str_CDATA, "Invalid CDATA syntax");
	skip_sign(ch_open_square, false, false, "Expected '[' symbol after <![CDATA instruction");
	size_t square_counter = 0;
	_tmp_store1.reset();
	ub1_t symbol = pick();
	while (symbol)
	{
		switch (symbol)
		{
			case ch_close_square:
				++square_counter;
				break;
			case ch_close_angle:
				if (square_counter >= 2)
				{
					pop();
					while (square_counter-- > 2)
						_tmp_store1 << ch_close_square;

					_doc.add_cdata(_tmp_store1.persist());
					return;
				}
			default:
				while (square_counter--)
					_tmp_store1 << ch_close_square;

				_tmp_store1 << symbol;
				square_counter = 0;
		} // switch

		symbol = pop();
  	} // while


	// we are here - something wrong with XML
	throw_exception("Incompleted CDATA section");
}

void  
xml_processor::_parseCharData()
{
	// [14]    CharData    ::=  ,//  [^<&]* - ([^<&]* ']]>' [^<&]*) 
	ub1_t symbol = pick();
	size_t illegal = 0;

	while (symbol && symbol != ch_open_angle)
	{
		// resolves entities
		switch (symbol)
		{
			case ch_ampersand: // if its special case &'"<> or charRef then
				if (_tmp_store1.size())
				{
					_tmp_store3 << _tmp_store1.persist();
					_tmp_store1.reset();
				}


				if (resolveEntity(_tmp_store2))
				{
					size_t len = 0;
					const ub1_t* bptr = _tmp_store2.persist(len);
					push(bptr, len);
 					if (_tmp_store3.size())
						_doc.add_text(_tmp_store3.persist());

					reset_all_tmp();
					return;
				}
				else
				{
	                _tmp_store3 << _tmp_store2.persist();
					illegal = 0;
				}

				_tmp_store2.reset();
				_tmp_store1.reset();
				symbol = pick();
				break;
			case ch_close_angle:
				if (illegal >= 2)
					throw_exception("Illegal char sequence ]]> in CharData");

				if (_tmp_store1.size())
				{
					_tmp_store3 << _tmp_store1.persist();
					_tmp_store1.reset();
				}

				illegal = 0;
				_tmp_store3 << symbol;
				symbol = pop();	
				break;
			case ch_close_square:
				if (_tmp_store1.size())
				{
					_tmp_store3 << _tmp_store1.persist();
					_tmp_store1.reset();
				}

				++illegal;
				_tmp_store3 << symbol;
				symbol = pop();
				break;
			default:
				if (is_white_space(symbol))
					_tmp_store1 << symbol;
				else
				{
					if (_tmp_store1.size())
					{
						_tmp_store3 << _tmp_store1.persist();
						_tmp_store1.reset();
					}

					_tmp_store3 << symbol;
				}

				illegal = 0;
				symbol = pop();
				break;
		} // switch
	} // while

	if (_tmp_store1.size() && _preserve_white_space)
		_tmp_store3 << _tmp_store1.persist();
	if (_tmp_store3.size())
		_doc.add_text(_tmp_store3.persist());

	reset_all_tmp();
}

void
xml_processor::parseDTD(const char* location)
{
	// working either with _stream or stream_
	if (location) // loads dtd
	{
		string_t full_path(0, _tmp_allocator);
		xml_stream_attribute attr_new;
		if (!attr_new.complete_to_full(location, _stream.get_location(), full_path))
		{
			string_t ex("Can't create full path from location: ");
			ex += location;
			ex += " and url: ";
			ex += _stream.get_location();
			exception::_throw(ex);
		}

		stream_input_common stream(_small_pool, _big_pool, _xml_size, true);
		if (!stream.open(attr_new))
		{
			string_t str = "Can't open location: ";
			str += location;
			exception::_throw(str);
		}

		// external
		dtd_processor dtd(stream, _doc, _small_pool, _big_pool, _xml_size);
		dtd.parse(); 
		stream.close();
	}
	else
	{
		dtd_processor dtd(_stream, _doc, _small_pool, _big_pool, _xml_size);
	//	dtd.set_encoding(UTF_8);
		dtd.parse(); // internal
		// skips close tag ']'
		pop();
	}
}

void  
xml_processor::skipDTD()
{
	xml_document skipSchema(_small_pool, _big_pool, _xml_size, 0);
	dtd_processor dtd(_stream, skipSchema, _small_pool, _big_pool, _xml_size);
	dtd.parse(); // internal
	// skips close tag
	pop();
}

void  
xml_processor::resolve_references()
{
	_doc.resolve_references();
}

void  
xml_processor::parseGeneralReference(bool skip_after)
{
	// resolves value
	_tmp_store1.reset();
	resolveEntity(_tmp_store1);
	size_t count = 0;
	const ub1_t* ptr = _tmp_store1.persist(count);
	push(ptr, count);
	_tmp_store1.reset();
	if (skip_after)
		skip_white_space();

}

#pragma pack()
END_TERIMBER_NAMESPACE

