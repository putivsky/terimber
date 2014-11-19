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

#include "xml/mngxml.hpp"
#include "xml/declxml.hpp"
#include "xml/defxml.hpp"
#include "xml/miscxml.hpp"
#include "xml/storexml.hpp"
#include "xml/sxml.hpp"
#include "xml/sxs.hpp"

#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/stack.hpp"
#include "base/string.hpp"
#include "base/template.hpp"
#include "base/common.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

/////////////////////////////
byte_manager::byte_manager(byte_source& stream, 
						   xml_document& doc, 
						   mem_pool_t& small_pool,
						   mem_pool_t& big_pool,
						   size_t xml_size) :
	_xml_size(__max(xml_size, os_def_size)),
	_small_pool(small_pool),
	_big_pool(big_pool),

	_depot_store1_allocator(_xml_size <= os_def_size ? _small_pool.loan_object() :  _big_pool.loan_object(_xml_size)),
	_depot_store2_allocator(_xml_size <= os_def_size ? _small_pool.loan_object() :  _big_pool.loan_object(_xml_size)),
	_depot_store3_allocator(_xml_size <= os_def_size ? _small_pool.loan_object() :  _big_pool.loan_object(_xml_size)),

	_tmp_allocator(_small_pool.loan_object()),	
 
	_tmp_store1_allocator(_small_pool.loan_object()),	
	_tmp_store2_allocator(_small_pool.loan_object()),	
	_tmp_store3_allocator(_small_pool.loan_object()),	

	_entity_allocator(_small_pool.loan_object()),

	_doc(doc),

	_stream(stream),

	_tmp_store1(*_depot_store1_allocator, *_tmp_store1_allocator, _xml_size - 1),
	_tmp_store2(*_depot_store2_allocator, *_tmp_store2_allocator, _xml_size - 1),
	_tmp_store3(*_depot_store3_allocator, *_tmp_store3_allocator, _xml_size - 1)
{ 
}

byte_manager::~byte_manager()
{
	_small_pool.return_object(_tmp_allocator);
	_small_pool.return_object(_entity_allocator);

	_small_pool.return_object(_tmp_store1_allocator);
	_small_pool.return_object(_tmp_store2_allocator);
	_small_pool.return_object(_tmp_store3_allocator);

	if (_xml_size <= os_def_size)
	{
		_small_pool.return_object(_depot_store1_allocator);
		_small_pool.return_object(_depot_store2_allocator);
		_small_pool.return_object(_depot_store3_allocator);
	}
	else
	{
		_big_pool.return_object(_depot_store1_allocator);
		_big_pool.return_object(_depot_store2_allocator);
		_big_pool.return_object(_depot_store3_allocator);
	}
}

const char* 
byte_manager::parseValue()
{
	// resets allocator
	_tmp_store1.reset();
	// gets first symbol
	ub1_t symbol = pick();
	while (symbol && is_name_char(symbol))
	{
		_tmp_store1 << symbol;
		symbol = pop();
	}

	return _tmp_store1.persist();
}

void
byte_manager::parseAttributeValue(string_t& name, string_t& value)
{
	skip_white_space();
	
	name = parseName();
	skip_sign(ch_equal, true, true, "Can't find equal symbol");
	// allocates memory through the data allocator
	value = parseQuotedValue(true, true, is_attribute_char, "Invalid characters in attribute value");
}

void  
byte_manager::parseComment()
{
	assert(pick() == ch_dash);

	_tmp_store1.reset();

	// skips the second dash
	pop(); // skips '-' first dash
	skip_sign(ch_dash, false, false, "Invalid char after <!- in comment"); // skip second dash

	ub1_t state = 0; // 0 - didn't meet '-', 1 - met '-', 2- met '--'
	// looks for close comment tag '-->'
	// reads from stack first
	ub1_t symbol = pick();
	while (symbol)
	{
		if (state == 2)
		{
			if (symbol != ch_close_angle)	// '--' is a forbiden sequence
				throw_exception("Invalid sequence (--) in comment");

			pop(); // skips '>'
			_doc.add_comment(_tmp_store1.persist());
			_tmp_store1.reset();
			return;			
		}

		if (symbol == ch_dash)
			++state;
		else
		{
			while (state)
			{
	            _tmp_store1 << ch_dash;
				--state;
			}

			_tmp_store1 << symbol;
		}
		symbol = pop();
	}

	// we are here
	// it means it didn't find the end of the comment - throws exception
	throw_exception("Can't find the close tag --> for comment");
}

// [66]    CharRef    ::=    '&#' [0-9]+ ';'  | '&#x' [0-9a-fA-F]+ ';' [WFC: Legal Character] 
void
byte_manager::parseCharRef(paged_buffer& buffer)
{
	assert(pick() == ch_pound);
	ub4_t result = 0;
	numeric_radix radix_ = RADIX10;

	ub1_t symbol = pop(); // skip '#'
	// '&#' [0-9]+ ';'  | '&#x' [0-9a-fA-F]+ ';' 
	// ISO/IEC 10646 
	if (symbol == ch_x || symbol == ch_X) // heximal
	{
		symbol = pop(); // skip 'x' | 'X'
		radix_ = RADIX16;
	}

	while (symbol && symbol != ch_semicolon)
	{
		result *= radix_;
		if (symbol >= ch_0 && symbol <= ch_9)
			result += symbol - ch_0;
		else if (radix_ == RADIX16 && symbol >= ch_A && symbol <= ch_F)
			result += symbol - ch_A + 0x0A;
		else if (radix_ == RADIX16 && symbol >= ch_a && symbol <= ch_f)
			result += symbol - ch_a + 0x0A;
		else
			throw_exception("Invalid char value");

		symbol = pop();
	}

	skip_sign(ch_semicolon, false, false, "Expected semicolon after Entity Reference");
	
	// converts ISO/IEC 10646 to UTF-8
	ub1_t utf8Buf[7] = {0};
	size_t count = 0;
	if (!usascii_to_utf8(result, utf8Buf, count))
		throw_exception("Invalid char token while char reference conversion");
	buffer.append(utf8Buf, count);
}

const char*  
byte_manager::parseQuotedValue(bool resolve_entities, bool normalize, bool (*fn)(ub1_t), const char* message)
{
	const bool check = fn != 0;
	// skips open quote and remembers it
	// the close quote must be the same
	ub1_t quote_symbol = _stream.skip_quote();
	// resets all allocators
	reset_all_tmp();
	// room for space meeting
	bool white_space_met = false;
	// gets the first simbol
	ub1_t symbol = pick();

	// looking until exists byte stream or close quote found
	while (symbol && symbol != quote_symbol)
	{
		switch (symbol)
		{
			case ch_space:
			case ch_hor_tab:
			case ch_lf:
			case ch_cr: // all space here
				if (normalize) // asks to normalize
					white_space_met = true;
				else // pushes symbol to store
					_tmp_store3 << symbol;

				// gets the next symbol
				symbol = pop();
				break;
			case ch_ampersand: // entity
				if (resolve_entities) // asks to resolve
				{
					if (resolveEntity(_tmp_store2)) // resolves entity
					{
						// this is normal entity
						size_t len = 0;
						const ub1_t* pbuf = _tmp_store2.persist(len);
						// pushes back to the stream
						push(pbuf, len);
					}
					else // this char reference or escaped symbol
					{
						if (white_space_met) // there were space symbols before
						{
							// pushes one blank symbol
							_tmp_store3 << ch_space;
							// resets keeper
							white_space_met = false;
						}
						_tmp_store3 << _tmp_store2.persist(); // push to store
					}

					// resest store
					_tmp_store2.reset();
					// gets the next symbol
					symbol = pick();
					break;
				}
			default:
				// checks if the checking function has specified any symbol
				if (check && !fn(symbol))
					throw_exception(message); // throws message

				if (white_space_met) // there were space symbols before
				{
					// pushes one blank symbol
					_tmp_store3 << ch_space;
					// resets keeper
					white_space_met = false;
				}

				// pushes symbol to store
				_tmp_store3 << symbol;
				// gets next symbol
				symbol = pop();
		} // switch
	}

	// skips close quote
	_stream.skip_quote(quote_symbol);
	// returns the result
	return _tmp_store3.persist();
}

void
byte_manager::parseExternalID(string_t& value_system, string_t& value_public, bool public_strick)
{
	// stores in stack 
	switch (pick())
	{
		// externalID
		case ch_S: // SYSTEM
			skip_string(str_SYSTEM, "Invalid syntax of the doctype SYSTEM declaration");
			// must be separator
			skip_white_space(true, "Can't find the separator");
			// system literal - dtd or schema file for example
			value_system = parseQuotedValue(false, false, 0, 0);
			break;
		case ch_P: // PUBLIC
			skip_string(str_PUBLIC, "Invalid syntax of the doctype SYSTEM declaration");
			// must be separator
			skip_white_space(true, "Can't find the separator");
			// first public literal
			value_public = parseQuotedValue(false, false, is_public_char, "Invalid syntax of the PUBLIC part of ExternalID declaration");
			if (public_strick)
			{
				skip_white_space(true, "Can't find the separator");
				// system literal - dtd or schema file for example
				value_system = parseQuotedValue(false, false, 0, 0);
			}
			else
			{
				skip_white_space();
				if (pick() != ch_close_angle)
					// system literal - dtd or schema file for example
					value_system = parseQuotedValue(false, false, 0, 0);	
			}
			break;
		default:
			throw_exception("Invalid syntax of the ExternalID declaration");
	} // switch
}

void  
byte_manager::parsePI()
{
	skip_sign(ch_question, false, false, "Expected question mark in PI");
	_tmp_store2.reset();

	const char* name = parseName();
	// checks first letter
	if (!str_template::strnocasecmp(name, str_xml, os_minus_one))
		throw_exception("Invalid PI target name");

	if (is_white_space(pick()))
	{
		// skips blank
		skip_white_space();
		// parses target
		ub1_t symbol = pick();
		while (symbol)
		{
			if (symbol == ch_question)
			{
				pop();
				skip_sign(ch_close_angle, false, false, "Expected close tag");
				_doc.add_pi(name, _tmp_store2.persist());
				return;
			}

			_tmp_store2 << symbol;
			symbol = pop();
		}

		throw_exception("Invalid PI target syntax");
	}

	skip_sign(ch_question, false, false, "Expected question mark");
	skip_sign(ch_close_angle, false, false, "Expected close tag");

	_doc.add_pi(name, _tmp_store2.persist());
}

bool  
byte_manager::resolveEntity(paged_buffer& buffer)
{
	pop(); // skips '&'

	if (pick() == ch_pound)
	{
		parseCharRef(buffer);
		return false;
	}

	// parses name
	const entityDecl* entry = _doc.find_entity_decl(parseName());
	if (!entry)
		throw_exception("Unresolved parameter entity");

	buffer.reset();
	skip_sign(ch_semicolon, false, false, "Expected semicolon symbol");


	// checks recursive call
	if (!entry->_is_encoded_char)
	{
		entity_map_t::iterator iter = _entity_map.find(entry);
		if (iter != _entity_map.end())
		{
			// checks position
			if (*iter >= _stream.current_pos())
				// recursive!!!
				throw_exception("Recursive entities are not allowed");
			else // updates position
				*iter = _stream.current_pos();
		}
		else
			_entity_map.insert(*_entity_allocator, entry, _stream.current_pos());
	}

	// checks standalone
	if (get_standalone() && entry->_is_in_subset)
		throw_exception("Illegal external reference in standalone document");

	// we have entity value
	if (entry->_value.length())
		// expands x
		buffer << entry->_value;
	else if (entry->_systemId.length())
	{
		if (entry->_is_unparsed)
		{
			buffer << entry->_notation;
			return false;
			//throw_exception("Can't resolve unparsed reference");
		}
		else
			// loads
			buffer_loader::load(_stream.get_location(), entry->_systemId, _small_pool, _big_pool, buffer, true);
	}

	return !entry->_is_encoded_char;
}


#pragma pack()
END_TERIMBER_NAMESPACE
