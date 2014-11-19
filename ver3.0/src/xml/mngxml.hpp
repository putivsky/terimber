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

#ifndef _terimber_mngxml_hpp_
#define _terimber_mngxml_hpp_

#include "xml/mngxml.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

/////////////////////////////
// the base class for utf-8 stream management with
// additional stack support
// however, the parsing process assumes 
// that some entries must be substituted
// so we need additional stores like a stack

// incapsulation stream
xml_forceinline 
ub1_t 
byte_manager::pick() 
{ 
	return _stream.pick(); 
}

xml_forceinline 
ub1_t 
byte_manager::pop() 
{ 
	return _stream.pop(); 
}

xml_forceinline 
ub1_t 
byte_manager::pip() 
{ 
	return _stream.pip(); 
}

xml_forceinline 
void 
byte_manager::skip_white_space(bool mustPresent, const char* message) 
{ 
	_stream.skip_white_space(mustPresent, message); 
}

xml_forceinline 
void 
byte_manager::push(ub1_t x) 
{ 
	_stream.push(x); 
}

xml_forceinline 
void 
byte_manager::push(const ub1_t* x, size_t len) 
{ 
	_stream.push(x, len); 
}

xml_forceinline 
void 
byte_manager::throw_exception(const char* msg_text) 
{ 
	_stream.throw_exception(msg_text); 
}

xml_forceinline 
void 
byte_manager::skip_string(const char* x, const char* message) 
{ 
	_stream.skip_string(x, message); 
}

xml_forceinline 
void 
byte_manager::skip_sign(ub1_t symbol, bool skip_before, bool skip_after, const char* message) 
{ 
	_stream.skip_sign(symbol, skip_before, skip_after, message); 
}

xml_forceinline 
ub1_t 
byte_manager::skip_quote(ub1_t symbol) 
{ 
	return _stream.skip_quote(symbol); 
}

// parses xml valid name
// the last parameter (use_tmp_allocator) define which internal allocator (temporary or data must be used)
xml_forceinline 
const char* 
byte_manager::parseName()
{
	// checks first symbol
	ub1_t symbol = pick();
	if (!symbol || !is_name_first_char(symbol))
		_stream.throw_exception("Illegal first symbol in name");

	_tmp_store1.reset();
	_tmp_store1 << symbol;

	while ((symbol = pop()) && is_name_char(symbol))
		_tmp_store1 << symbol;

	return _tmp_store1.persist();
}
	
// returns the current subset value
inline 
bool 
byte_manager::get_subset() const 
{ 
	return _stream.get_subset(); 
}

// returns the current standalone value
inline 
size_t 
byte_manager::get_standalone() const 
{ 
	return _stream.get_standalone(); 
}

// returns the current version
inline 
size_t 
byte_manager::get_version() const 
{ 
	return _stream.get_version(); 
}

// resets internal temporary allocator and buffers
xml_forceinline 
void 
byte_manager::reset_all_tmp(bool reset_allocator)
{
	_tmp_store1.reset();
	_tmp_store2.reset();
	_tmp_store3.reset();
	if (reset_allocator) 
		_tmp_allocator->reset();
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_mngxml_hpp_ 
