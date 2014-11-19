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

#include "xml/storexml.hpp"
#include "xml/declxml.hpp"
#include "xml/defxml.hpp"
#include "xml/miscxml.hpp"
#include "xml/mngxml.hpp"

#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/string.hpp"
#include "base/template.hpp"
#include "base/vector.hpp"
#include "base/common.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

const size_t xml_decl_max_len = 4096;
/////////////////////////////////
byte_source::byte_source(mem_pool_t& small_pool, mem_pool_t& big_pool, size_t xml_size, const char* url, bool subset) : 
	_small_pool(small_pool),
	_big_pool(big_pool),
	_xml_size(__max(xml_size, os_def_size)),
	_url(url),
	_subset(subset),
	_buffer_pos(_xml_size),
	_symbol(0), 
	_line_counter(0),
	_char_counter(0),
	_pos_counter(0),
	_encodingSchema(AUTO), 
	_version(1),
	_standalone(os_minus_one),
	_end(false)
{ 
	_convert_allocator = _small_pool.loan_object();
	_list_allocator = _small_pool.loan_object();

	if (_xml_size <= os_def_size)
	{
		_depot_allocator = _small_pool.loan_object();
		_store_allocator = _small_pool.loan_object();
	}
	else
	{
		_depot_allocator = _big_pool.loan_object(_xml_size);
		_store_allocator = _big_pool.loan_object(_xml_size);
	}

	_buffer = (ub1_t*)_depot_allocator->allocate(_xml_size);
	_convert_buffer = (ub1_t*)_convert_allocator->allocate(_xml_size);
}

byte_source::~byte_source()
{
	_small_pool.return_object(_convert_allocator);
	_small_pool.return_object(_list_allocator);
	if (_xml_size <= os_def_size)
	{
		_small_pool.return_object(_depot_allocator);
		_small_pool.return_object(_store_allocator);
	}
	else
	{
		_big_pool.return_object(_depot_allocator);
		_big_pool.return_object(_store_allocator);
	}
}

void  
byte_source::reset_buffer() 
{ 
	_active_store.clear();
	_used_store.clear();
	_store_allocator->reset();
	_list_allocator->reset();

	_symbol = 0;
	_buffer_pos = _xml_size;
	_line_counter = 0;
	_pos_counter = 0;
	_char_counter = 0;
	_encodingSchema = AUTO;
	_version = 1;
	_standalone = os_minus_one;
	_end = false;
}

void  
byte_source::throw_exception(const char* msg_text)
{
	char x[64] = {0};
	str_template::strprint(x, 64, "Error on line: %d, position: %d", _line_counter + 1, _char_counter);
	string_t ex = x;

	if (msg_text)
	{
		ex += ", error message: \"";
		ex.append(msg_text, __min(strlen(msg_text), (size_t)128));
		ex += "\"";
	}

	exception::_throw(ex);
}

// inserts into head of sequence
void 
byte_source::push(const ub1_t* x, size_t len)
{
	size_t remain = len;
	while (remain)
	{
		size_t copy_len = __min(remain, _buffer_pos);
		// copies the rest of buffer
		memcpy(_buffer + _buffer_pos - copy_len, x + remain - copy_len, copy_len);

		// adjusts len
		remain -= copy_len;
		_buffer_pos -= copy_len;

		if (!_buffer_pos && remain) // we on top of buffer and there are more chars
		{
			// copies buffer to store
			ub1_t* vec = 0;
			// checks used
			if (!_used_store.empty())
			{
				vec = _used_store.front();
				_used_store.pop_front();
			}
			else
				vec = (ub1_t*)_store_allocator->allocate(_xml_size);

			if (!vec)
				throw_exception("Not enough memory");
			
			// saves to store
			_active_store.push_front(*_list_allocator, vec);

			memcpy(vec, _buffer, _xml_size);
			_buffer_pos = _xml_size;
		}
	}

	_pos_counter -= len;
	_symbol = _buffer_pos == _xml_size ? 0x00 : _buffer[_buffer_pos];
}

size_t 
byte_source::pull(ub1_t* x, size_t len)
{
	size_t requested = len;
	len = 0;
	size_t copy_len;
		
	while (!_end && len < requested)
	{
		if (_buffer_pos == _xml_size)
				go_shopping();
		else
		{
			copy_len = __min(requested - len, _xml_size - _buffer_pos);
			memcpy(x + len, _buffer + _buffer_pos, copy_len);
			_buffer_pos += copy_len;
			len += copy_len;
		}
	}

	_pos_counter += len;
	_char_counter += len;

	return len;
}

ub1_t
byte_source::go_shopping()
{ 
	if (_end)
		return 0;

	if (!_active_store.empty())
	{
		memcpy(_buffer, _active_store.front(), _xml_size);
		// saves here
		_used_store.push_front(*_list_allocator, _active_store.front());
		_active_store.pop_front();

		_buffer_pos = 0;
		_symbol = _buffer[_buffer_pos];
		return _symbol; 
	}

	// resets buffer
	_buffer_pos = 0;

	switch (_encodingSchema)
	{
		case AUTO: // first entry
			// tries to define incoming encoding and converts xml decl into utf-8
			// checks the specified encoding in xml decl and set it if any 
			_end = !taste_buffer();
			break;
		default: // needs to convert to utf-8
			// converts data into utf-8
			_end = !auto_convert();
			break;
	} // switch

	// moves to the right position
	if (!_end)
	{
		if (_buffer_pos != _xml_size)
			memmove(_buffer + _xml_size - _buffer_pos, _buffer, _buffer_pos);

		_buffer_pos = _xml_size - _buffer_pos;
		_symbol = _buffer[_buffer_pos]; 
	}
	else
		_symbol = 0;

	return _symbol;
}

bool
byte_source::taste_buffer()
{ 
	size_t len = 4;
	size_t more = 0;
	size_t processed = 0;
	size_t converted = 0;


	// begins with a small chunk of 4 bytes
	if (!data_request(_buffer, len) || len != 4)
	{
		_encodingSchema = UTF_8;
		_buffer_pos = len;
		return _buffer_pos != 0;
	}
	else if (!memcmp(_buffer, UCS4BBOM, 4)) // BOM UCS_4B
	{
		_encodingSchema = UCS_4B;
		if (!convert_chars(20) || _buffer_pos != 5 || memcmp(_buffer, UTF8Pre, 5))
			return true;
		else
			return parseXMLDeclInfo();
	}
	else if (!memcmp(_buffer, UCS4LBOM, 4)) // BOM UCS_4L
	{
		_encodingSchema = UCS_4L;
		if (!convert_chars(20) || _buffer_pos != 5 || memcmp(_buffer, UTF8Pre, 5))
			return true;
		else
			return parseXMLDeclInfo();
	}
	else if (!memcmp(_buffer, UTF8BOM, 3)) // BOM UTF-8
	{
		_encodingSchema = UTF_8;
		// we already have one char
		// and that char is utf-8 encoded
		// however, this char is located on the 4th position
		_buffer_pos = 1;
		_buffer[0] = _buffer[3];		
		if (!convert_chars(4) || _buffer_pos != 5 || memcmp(_buffer, UTF8Pre, 5))
			return true;
		else
			return parseXMLDeclInfo();
	}
	else if (!memcmp(_buffer, UTF16BBOM, 2)) // BOM UTF-16B
	{
		_encodingSchema = UTF_16B;
		// we already have two chars
		// but they are in UTF16-B
		// converts two last chars to the UTF-8
		// skips BOM

		_convert_buffer[0] = _buffer[2];
		_convert_buffer[1] = _buffer[3];
		len = 8;
		if (!data_request(_convert_buffer + 2, len) || len != 8)
			return true;
		
		_buffer_pos = 0;
		len = 10;
				
		if (!fixedN_to_utf8(_encodingSchema, _convert_buffer, len, _buffer + _buffer_pos, converted, processed, more))
		{
			_symbol = 0;
			throw_exception("Invalid char token while char conversion");
		}

		if (!more)
			_buffer_pos += converted;
		else
		{
			size_t more_ = more;
			size_t processed_;
			size_t converted_;

			if (!data_request(_convert_buffer + len, more_) || more != more_)
				return false;

			if (!fixedN_to_utf8(_encodingSchema, _convert_buffer + processed, len - processed + more, _buffer + _buffer_pos + converted, converted_, processed_, more_))
			{
				_symbol = 0;
				throw_exception("Invalid char token while char conversion");
			}
			
			_buffer_pos += converted + converted_;
		}
	
		// checks xml
		if (_buffer_pos != 5 || memcmp(_buffer, UTF8Pre, 5))
			return true;
		else
			return parseXMLDeclInfo();
	}
	else if (!memcmp(_buffer, UTF16LBOM, 2)) // BOM UTF-16L
	{
		_encodingSchema = UTF_16L;
		// we already have two chars
		// but they are in UTF16-B
		// converts two last chars to the UTF-8
		// skips BOM 
		_convert_buffer[0] = _buffer[2];
		_convert_buffer[1] = _buffer[3];
		len = 8;
		if (!data_request(_convert_buffer + 2, len) || len != 8)
			return true;
		
		_buffer_pos = 0;
		len = 10;
				
		if (!fixedN_to_utf8(_encodingSchema, _convert_buffer, len, _buffer + _buffer_pos, converted, processed, more))
		{
			_symbol = 0;
			throw_exception("Invalid char token while char conversion");
		}

		if (!more)
			_buffer_pos += converted;
		else
		{
			size_t more_ = more;
			size_t processed_;
			size_t converted_;

			if (!data_request(_convert_buffer + len, more_) || more != more_)
				return false;

			if (!fixedN_to_utf8(_encodingSchema, _convert_buffer + processed, len - processed + more, _buffer + _buffer_pos + converted, converted_, processed_, more_))
			{
				_symbol = 0;
				throw_exception("Invalid char token while char conversion");
			}
			
			_buffer_pos += converted + converted_;
		}
	
		// checks xml
		if (_buffer_pos != 5 || memcmp(_buffer, UTF8Pre, 5))
			return true;
		else
			return parseXMLDeclInfo();
	}
	else // unknown BOM tries to check '<?xml' directly
	{
		// expands buffer up to 5 chars
		len = 1;
		if (!data_request(_buffer + 4, len) || len != 1)
		{
			_encodingSchema = UTF_8;
			_buffer_pos = 4 + len;
			return true;
		}

		if (!memcmp(_buffer, UTF8Pre, 5))
		{
			_encodingSchema = UTF_8;
			return parseXMLDeclInfo();
		}
//		else if (!memcmp(_buffer, EBCDICPre, 5))
//		{
//			_encodingSchema = EBCDIC
//			return parseXMLDeclInfo();
//		}
		else // go ahead for 10 chars
		{
			len = 5;
			if (!data_request(_buffer + 5, len) || len != 5)
			{
				_encodingSchema = UTF_8;
				_buffer_pos = 5 + len;
				// checks here ut8
				if (!utf8_to_utf8(_buffer, _buffer_pos, processed, more))
				{
					_symbol = 0;
					throw_exception("Invalid char token while char conversion");
				}
	
				return true;
			}
			else if (!memcmp(_buffer, UTF16BPre, 10))
			{
				_encodingSchema = UTF_16B;
				return parseXMLDeclInfo();
			}
			else if (_buffer_pos >= 10 && !memcmp(_buffer, UTF16LPre, 10))
			{
				_encodingSchema = UTF_16L;
				return parseXMLDeclInfo();
			}
			else // go ahead for 20 chars
			{
				len = 10;
				if (!data_request(_buffer + 10, len) || len != 10)
				{
					_encodingSchema = UTF_8;
					_buffer_pos = 10 + len;
					return true;
				}
				else if (!memcmp(_buffer, UCS4BPre, 20))
				{
					_encodingSchema = UCS_4B;
					return parseXMLDeclInfo();
				}
				else if (!memcmp(_buffer, UCS4LPre, 20))
				{
					_encodingSchema= UCS_4L;
					return parseXMLDeclInfo();
				}
			}
		}

		_buffer_pos = 20;
		_encodingSchema = UTF_8; // by default
		return true;
	}
}

bool 
byte_source::parseXMLDeclInfo()
{
	_buffer_pos = 0;
	
	if (!convert_chars(1))
		return false;

	// gets the max chars to buffer
	size_t prev_pos = _buffer_pos - 1;

	while (_buffer_pos < _xml_size - 6 && convert_chars(1))
	{
		if (_buffer_pos != prev_pos + 2) // must be one byte utf-8 char
			throw_exception("Invalid XML declaration syntax");

		if ((char)_buffer[prev_pos] == ch_question 
			&& (char)_buffer[_buffer_pos - 1] == ch_close_angle)
			break;
		else
			++prev_pos;
	}

	if (_buffer_pos == _xml_size)
		throw_exception("Invalid XML declaration syntax");

	// now in buffer something like that
	// ' version="1.0" encoding="UTF-16" standalone="yes">'
	// creates in memory stream
	stream_input_memory stream(_buffer, _buffer_pos, _small_pool, _big_pool, 0, _subset);
	stream.set_encoding(UTF_8);
	encodingSchema detectedSchema = _subset ? stream.parseTextDecl() : stream.parseXMLDecl();

	// set encoding if found
	if (detectedSchema != AUTO)
	{
		if (detectedSchema == UTF_16)
		{
			if (_encodingSchema != UTF_16B && _encodingSchema != UTF_16L)
				throw_exception("Unknown encoding schema");
		}
		else if (detectedSchema == UCS_4)
		{
			if (_encodingSchema != UCS_4B && _encodingSchema != UCS_4L && _encodingSchema != UCS_4BS && _encodingSchema != UCS_4LS)
				throw_exception("Unknown encoding schema");
		}
		else
			_encodingSchema = stream.get_encoding();
	}

	_standalone = stream.get_standalone();
	_version = stream.get_version();
	// one more block bytes before leave request
	_buffer_pos = 0;
	return auto_convert();
}

bool 
byte_source::auto_convert()
{
	convert_chars(_xml_size - 6);
	return _buffer_pos != 0;
}

bool 
byte_source::convert_chars(size_t count)
{
	ub4_t in = 0, surrogate = 0;	
	size_t len = 0;
	size_t more = 0;
	size_t processed = 0;
	size_t converted = 0;
	size_t available = 0;
	size_t actual = 0;

	size_t counter = count;

	if (!counter)
		return false;

	while (counter > 0 // no more than requested
		&& (available = _xml_size - _buffer_pos - 6) > 0 // prevent buffer overflow
		)
	{
		switch (_encodingSchema)
		{
			//case EBCDIC: // 
			//	assert(false);
			//	break;
			case UTF_ISO88591: // 1 byte
				actual = available / 2;
				if (actual == 0) // no risk here
				{
					counter = 0;
					break;
				}

				if (counter == 1)
					len = 1;
				else if (counter == 2)
					len = 2;
				else
					len = counter / 2;

				// takes into account available bytes in buffer
				if (len > actual)
					len = actual;


				if (!data_request(_convert_buffer, len))
					return false;

				isoN_to_utf8(_encodingSchema, _convert_buffer, len, _buffer + _buffer_pos, converted, processed);
				_buffer_pos += converted;
				counter -= processed;
				break;
			case WINDOWS_1251: // 1 byte
				
				actual = available / 3;

				if (actual == 0) // no risk here
				{
					counter = 0;
					break;
				}

				if (counter == 1)
					len = 1;
				else if (counter == 2)
					len = 2;
				else
					len = counter / 3;

				// takes into account available bytes in buffer
				if (len > actual)
					len = actual;

				if (!data_request(_convert_buffer, len))
					return false;
				
				windowsN_to_utf8(_encodingSchema, _convert_buffer, len, _buffer + _buffer_pos, converted, processed);
				_buffer_pos += converted;
				counter -= processed;
				break;
			case UTF_8: // 1 byte
			case US_ASCII:
				len = counter;
				if (!data_request(_buffer + _buffer_pos, len))
					return false;
				// checks UTF-8 compatibility
				if (!utf8_to_utf8(_buffer + _buffer_pos, len, processed, more))
				{
					_symbol = 0;
					string_t err = "Invalid utf8 char token: ";
					err.append((const char*)_buffer + _buffer_pos, __min(processed, (size_t)32));
 					throw_exception(err);
				}

				if (!more)
				{
					_buffer_pos += len;
					counter -= processed;
				}
				else
				{
					size_t more_ = more;
					size_t processed_;

					if (!data_request(_buffer + _buffer_pos + len, more_))
						return false;

					if (!utf8_to_utf8(_buffer + _buffer_pos + processed, len - processed + more, processed_, more_))
					{
						_symbol = 0;
						string_t err = "Invalid utf8 char token: ";
						err.append((const char*)_buffer + _buffer_pos + processed, __min(processed_, (size_t)32));
						throw_exception(err);
					}

					_buffer_pos += len + more;
					counter -= __min(counter, len + more);
				}
				break;
			case UTF_16L: // 2 bytes little ending
			case UTF_16B: // 2 bytes big ending

				actual = available / 4;

				if (actual == 0) // no risk here
				{
					counter = 0;
					break;
				}

				// takes the estimated length + rounding
				if (counter == 1)
					len = 2;
				else
				{
					len = counter * 2 / 4;
					len += len % 2 ? 1 : 0;
				}

				// takes into account available bytes in buffer
				if (len > actual)
					len = actual;

				if (!data_request(_convert_buffer, len))
					return false;
				
				if (!fixedN_to_utf8(_encodingSchema, _convert_buffer, len, _buffer + _buffer_pos, converted, processed, more))
				{
					_symbol = 0;
					string_t err = "Invalid utf16 char token: ";
					err.append((const char*)_convert_buffer, __min(processed, (size_t)32));
					throw_exception(err);
				}

				if (!more)
				{
					_buffer_pos += converted;
					counter -= processed;
				}
				else
				{
					size_t more_ = more;
					size_t processed_;
					size_t converted_;

					if (!data_request(_convert_buffer + len, more_))
						return false;

					if (!fixedN_to_utf8(_encodingSchema, _convert_buffer + processed, len - processed + more, _buffer + _buffer_pos + converted, converted_, processed_, more_))
					{
						_symbol = 0;
						string_t err = "Invalid utf16 char token: ";
						err.append((const char*)_convert_buffer + processed, __min(processed_, (size_t)32));
						throw_exception(err);
					}

					_buffer_pos += converted + converted_;
					counter -= __min(counter, processed + processed_);
				}
				break;
			case UCS_4L: // 4 bytes 4321
			case UCS_4LS: // 4 bytes 2143
			case UCS_4B: // 4 bytes 1234
			case UCS_4BS: // 4 bytes 3412 

				actual = available / 6;

				if (actual == 0) // no risk here
				{
					counter = 0;
					break;
				}

				// takes estimated length + rounding
				if (counter == 1)
					len = 4;
				else
				{
					len = counter * 4 / 6;
					len += len % 4 ? len % 4 : 0;
				}

				// takes into account available bytes in buffer
				if (len > actual)
					len = actual;

				if (!data_request(_convert_buffer, len))
					return false;
				
				if (!fixedN_to_utf8(_encodingSchema, _convert_buffer, len, _buffer + _buffer_pos, converted, processed, more))
				{
					_symbol = 0;
					string_t err = "Invalid utf32 char token: ";
					err.append((const char*)_convert_buffer, __min(processed, (size_t)32));
					throw_exception(err);
				}

				if (!more)
				{
					_buffer_pos += converted;
					counter -= processed;
				}
				else
				{
					size_t more_ = more;
					size_t processed_;
					size_t converted_;

				 	if (!data_request(_convert_buffer + len, more_))
						return false;

					if (!fixedN_to_utf8(_encodingSchema, _convert_buffer + processed, len - processed + more, _buffer + _buffer_pos + converted, converted_, processed_, more_))
					{
						_symbol = 0;
						string_t err = "Invalid utf32 char token: ";
						err.append((const char*)_convert_buffer + processed, __min(processed_, (size_t)32));
						throw_exception(err);
					}

					_buffer_pos += converted + converted_;
					counter -= __min(counter, processed + processed_);
				}
				break;
			default:
				throw_exception("Unsupported encoding schema");
		} // switch
	} // while 
	return true;
}

void 
byte_source::parseVersion()
{
	skip_string(str_version, "Invalid version syntax");
	// skips equal sign
	skip_sign(ch_equal, true, true, "Equal sign is expected");
	// reads value
	scanQuotedValue(_convert_buffer, xml_decl_max_len, is_versionnum_char, "Invalid version value");

	// [26]    VersionNum    ::=    ([a-zA-Z0-9_.:] | '-')+ 
	//check_string_content((const char*)_convert_buffer, -1, is_versionnum_char, "Invalid version value");
	// version is a required attribute for XMLDecl and must be 1.0
	if (str_template::strnocasecmp((const char*)_convert_buffer, "1.0", os_minus_one))
		throw_exception("Invalid xml version, must be 1.0");

	_version = 1;
}

encodingSchema 
byte_source::parseEncoding()
{
	skip_string(str_encoding, "Invalid encoding syntax");
	// skips equal sign
	skip_sign(ch_equal, true, true, "Equal sign is expected");
	// read value
	scanQuotedValue(_convert_buffer, xml_decl_max_len, is_encname_char, "Invalid char in encoding value");


	// encoding can't be empty
	if (!is_letter(_convert_buffer[0]))
		throw_exception("Invalid first char in encoding value");

	// [81]    EncName    ::=  ,//  [A-Za-z] ([A-Za-z0-9._] | '-')* 
	//check_string_content((const char*)_convert_buffer, -1, is_encname_char, "Invalid char in encoding value");

	return checkEncodingSchema((const char*)_convert_buffer);
}

size_t 
byte_source::parseStandalone()
{
	skip_string(str_standalone, "Invalid standalone syntax");
	// skips equal sign
	skip_sign(ch_equal, true, true, "Equal sign is expected");
	// reads value
	scanQuotedValue(_convert_buffer, xml_decl_max_len, 0, 0);
	// assigns standalone
	if (!strcmp((const char*)_convert_buffer, str_yes))
		return 1;
	else if (!strcmp((const char*)_convert_buffer, str_no))
		return 0;
	else
	{
		throw_exception("Invalid standalone value");
		return os_minus_one;
	}
}

// [23]    XMLDecl    ::=    '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>' 
encodingSchema
byte_source::parseXMLDecl()
{
	// set default version
	_version = 1;

	encodingSchema schema = AUTO;

	// skips junk
	skip_white_space(true, "Must be a separator after xml declaration open tag");
	//skip_white_space();
	
	// looks for required version
	if (pick() != ch_v) // version
		throw_exception("VersionInfo is required part of xml declaration");

	parseVersion();

	// skips junk
	skip_white_space(ch_question != pick(), "Must be a separator in xml declaration");

	// looks for encoding or standalone or close tag
	// reads the first char
	if (pick() == ch_e) // encoding
	{
		schema = parseEncoding();
		// skips junk
		skip_white_space(ch_question != pick(), "Must be a separator in xml declaration");
	}

	if (pick() == ch_s) // standalone
	{
		_standalone = parseStandalone();
		skip_white_space();
	}


	// the last must be question mark
	if (pick() != ch_question || pop() != ch_close_angle) // ?>
		throw_exception("Invalid close tag in Text declaration");

	pop(); // skips '>'

	if (schema != AUTO)
		_encodingSchema = schema;

	return schema;
}

// [77]    TextDecl    ::=    '<?xml' VersionInfo? EncodingDecl S? '?>' 
encodingSchema
byte_source::parseTextDecl()
{
	// sets default version
	_version = 1;

	// skips junk
	skip_white_space(true, "Must be a separator after xml declaration open tag");
	
	// looks for required version
	if (pick() == ch_v) // version
	{
		parseVersion();
		// skips junk
		skip_white_space(ch_question != pick(), "Must be a separator in xml declaration");
	}

	// looks for encoding or standalone or close tag
	// reads first char
	if (pick() != ch_e) // encoding
		throw_exception("Expected encoding attribute in xml declaration");

	encodingSchema schema = parseEncoding();
	skip_white_space();

	// the last must be question mark
	if (pick() != ch_question || pop() != ch_close_angle) // ?>
		throw_exception("Invalid close tag in xml declaration");

	pop(); // skips '>'
	_encodingSchema = schema;
	return schema;
}

// we send lower case, but compare also upper case
void 
byte_source::skip_string(const char* x, const char* message)
{
	assert(x);
	for (ub1_t symbol = pick(); symbol && *x && symbol == *x; symbol = pop(), ++x);
	
	if (*x)
		throw_exception(message);		
}

void 
byte_source::skip_sign(ub1_t symbol, bool skip_before, bool skip_after, const char* message)
{ 
	if (skip_before)
		skip_white_space();

	if (symbol != pip()) 
		throw_exception(message); 

	if (skip_after)
		skip_white_space();
}

encodingSchema 
byte_source::checkEncodingSchema(const char* schema)
{
	if (!str_template::strnocasecmp(schema, "UTF-8", os_minus_one) || !str_template::strnocasecmp(schema, "UTF8", os_minus_one))
		return UTF_8;
	else if (!str_template::strnocasecmp(schema, "US-ASCII", os_minus_one) || !str_template::strnocasecmp(schema, "USASCII", os_minus_one) || !str_template::strnocasecmp(schema, "ASCII", os_minus_one) || !str_template::strnocasecmp(schema, "US_ASCII", os_minus_one)) 
		return US_ASCII;
	else if (!str_template::strnocasecmp(schema, "UTF-16", os_minus_one))
		return UTF_16;
	else if (!str_template::strnocasecmp(schema, "UTF-16 (LE)", os_minus_one) || !str_template::strnocasecmp(schema, "UTF-16LE", os_minus_one))
		return UTF_16L;
	else if (!str_template::strnocasecmp(schema, "UTF-16 (BE)", os_minus_one) || !str_template::strnocasecmp(schema, "UTF-16BE", os_minus_one))
		return UTF_16B;
	else if (!str_template::strnocasecmp(schema, "USC-4 (LE)", os_minus_one) || !str_template::strnocasecmp(schema, "USC-4LE", os_minus_one))
		return UCS_4L;
   	else if (!str_template::strnocasecmp(schema, "USC-4 (BE)", os_minus_one) || !str_template::strnocasecmp(schema, "USC-4BE", os_minus_one))
		return UCS_4B;
  	else if (!str_template::strnocasecmp(schema, "USC-4", os_minus_one))
		return UCS_4;
	else if (!str_template::strnocasecmp(schema, "ISO-8859-1", os_minus_one) || !str_template::strnocasecmp(schema, "ISO88591", os_minus_one) || !str_template::strnocasecmp(schema, "ISO_8859-1", os_minus_one) || !str_template::strnocasecmp(schema, "ISO_8859_1", os_minus_one))
		return UTF_ISO88591;
	else if (!str_template::strnocasecmp(schema, "WINDOWS-1251", os_minus_one))
		return WINDOWS_1251;
	else
		return AUTO;
		//throw_exception("Invalid encoding schema");

	return AUTO;
}

ub1_t 
byte_source::skip_quote(ub1_t symbol)
{
	// checks rules
	if (!symbol) // we don't know single quote or double
	{
		// must be quote (single or double)
		if ((symbol = pick()) != ch_double_quote && symbol != ch_single_quote)
			throw_exception("Invalid open quote");
	}
	else if (symbol != pick()) // a symbol opened the quote, a symbol must also close the quote
			throw_exception("Invalid close quote");

	pop();
	return symbol;
}

void  
byte_source::scanQuotedValue(ub1_t* value, size_t len, bool (*fn)(ub1_t), const char* message)
{
	ub1_t quote_symbol = skip_quote();
	size_t count = 0;
	ub1_t symbol = pick();

	while (count < len - 1 && symbol && symbol != quote_symbol)
	{
		if (fn && !fn(symbol))
			throw_exception(message);
		value[count++] = symbol;
		symbol = pop();
	}

	value[count] = 0;

	skip_quote(quote_symbol);
}


/////////////////////////////////
/////////////////////////////////
byte_consumer::byte_consumer(mem_pool_t& small_pool, mem_pool_t& big_pool, size_t xml_size) : 
	_small_pool(small_pool),
	_big_pool(big_pool),
	_xml_size(__max(xml_size, os_def_size)),
	_buffer_pos(0)
{ 

	if (_xml_size <= os_def_size)
		_depot_allocator = _small_pool.loan_object(); // _buffer & _convert_buffer
	else
		_depot_allocator = _big_pool.loan_object(_xml_size); // _buffer & _convert_buffer

	_buffer = (ub1_t*)_depot_allocator->allocate(_xml_size);
}

byte_consumer::~byte_consumer()
{
	if (_xml_size <= os_def_size)
		_small_pool.return_object(_depot_allocator);
	else
		_big_pool.return_object(_depot_allocator);
}

// inserts into head of sequence
void 
byte_consumer::push(const ub1_t* x, size_t len)
{
	size_t remain = len;
	while (remain)
	{
		size_t copy_len = __min(remain, _xml_size - _buffer_pos);
		// copies the rest of buffer
		memcpy(_buffer + _buffer_pos, x + len - remain, copy_len);

		// adjusts len
		remain -= copy_len;
		_buffer_pos += copy_len;

		if (_buffer_pos == _xml_size)
		{
			if (!data_persist(_buffer, _buffer_pos))
				exception::_throw("Can't persist data");
			_buffer_pos = 0;
		}
	}
}

void 
byte_consumer::flush()
{
	if (_buffer_pos)
	{
		if (!data_persist(_buffer, _buffer_pos))
			exception::_throw("Can't persist data");
	
		_buffer_pos = 0;
	}
}

#pragma pack()
END_TERIMBER_NAMESPACE
