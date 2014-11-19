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

#ifndef _terimber_storexml_hpp_
#define _terimber_storexml_hpp_

#include "xml/storexml.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

////////////////////////////////
inline
size_t 
byte_source::get_standalone() const 
{ 
	return _standalone; 
}

inline
bool 
byte_source::get_subset() const 
{ 
	return _subset; 
}

inline
size_t 
byte_source::get_version() const 
{ 
	return _version; 
}

inline 
size_t 
byte_source::get_xml_size() const 
{ 
	return _xml_size; 
}

xml_forceinline 
ub1_t 
byte_source::pick()
{ 
	return _buffer_pos == _xml_size ? go_shopping() : _symbol; 
}


xml_forceinline 
ub1_t 
byte_source::pop()
{ 
	++_pos_counter;
	++_char_counter;
	ub1_t symbol = _buffer_pos >= _xml_size - 1 ? go_shopping() : (_symbol = _buffer[++_buffer_pos]);
	switch (symbol)
	{
		case ch_lf:
			++_line_counter;
		case ch_cr:
			_char_counter = 0;
			break;
	}

	return symbol;
}

xml_forceinline 
ub1_t 
byte_source::pip()
{ 
	ub1_t symbol = pick();
	pop();
	return symbol; 
}

xml_forceinline 
void 
byte_source::skip_white_space(bool mustPresent, const char* message)
{ 
	ub1_t fsymbol = pick();
	ub1_t symbol = fsymbol;

	while (symbol)
	{
		switch (symbol)
		{
			case ch_lf:
			case ch_cr:
			case ch_hor_tab:
			case ch_space:
				break;
			default:
				if (mustPresent && fsymbol == symbol)
					throw_exception(message);
				return;
		} // switch
		
		symbol = pop(); 
	}
}

xml_forceinline 
size_t 
byte_source::current_pos() const
{ 
	return _pos_counter; 
}

xml_forceinline 
void 
byte_source::push(ub1_t x)
{ 
	push(&x, 1); 
}

inline
encodingSchema 
byte_source::get_encoding() const
{ 
	return _encodingSchema; 
}

inline 
void 
byte_source::set_encoding(encodingSchema schema)
{ 
	_encodingSchema = schema; 
}

inline 
const char* 
byte_source::get_location() const
{ 
	return _url; 
}

//////////////////////////////////////////////////////////////////
inline 
void 
byte_consumer::push(ub1_t x) 
{ 
	push(&x, 1); 
}

inline 
void 
byte_consumer::push(const char* x) 
{ 
	if (x) 
	{ 
		push((ub1_t*)x, strlen(x)); 
	} 
}

inline 
size_t 
byte_consumer::get_xml_size() const 
{ 
	return _xml_size; 
}

inline 
size_t 
byte_consumer::get_buffer_pos() const 
{ 
	return _buffer_pos; 
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_storexml_hpp_ 
