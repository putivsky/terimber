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

#include "base/except.h"
#include "base/common.h"
#include "base/string.hpp"
#include "base/memory.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

/////////////////////////////////////////////////////////
bool 
exception_item::operator <(const exception_item& x) const
{ 
	return _code < x._code; 
}

bool 
exception_item::operator ==(const exception_item& x ) const
{ 
	return _code == x._code; 
}

//////////////////////////////////////////////////////////
exception_table::exception_table(exception_item* items) :
	_items(items), _len(0)
{
	// counts items
	for (exception_item* ptr = items; ptr->_desc; ++_len)
		++ptr;

	// sorts items
	std::sort(items, items + _len);
}

// returns exception description according to the code
const char* 
exception_table::get_error(size_t code_)
{
	// creates ethalon item
	exception_item ethalon;
	// assigns code
	ethalon._code = code_;
	// no description
	ethalon._desc = 0;
	// finds iterator
	exception_item* iter = std::lower_bound(_items, _items + _len, ethalon);
	// returns description
	return iter == _items + _len || iter->_code != code_ ? 0 : iter->_desc;
}

/////////////////////////////////////////////////////
exception::exception(size_t id, const wchar_t* desc) : 
	_id(id)
{ 
	// converts wide char string to unicode
	str_template::unicode_to_multibyte(_reason, desc); 
}

exception::exception(size_t id, const char* desc) : 
	_id(id), _reason(desc) 
{
}

// virtual 
exception::~exception() 
{
}

size_t 
exception::get_code() const
{ 
	return _id; 
}

const 
char* 
exception::what() const
{ 
	return _reason; 
}

// static 
void 
exception::_throw(size_t code_, exception_table* table_)
{ 
	if (table_)
		throw exception(code_, table_->get_error(code_));
	else
	{
		// declares buffer
		char err[2048];
		// extracts OS error
		os_get_error(code_, err, 2048);
		// throws exception
		throw exception(code_, err); 
	}
}

// static 
void 
exception::_throw(const wchar_t* desc)
{ 
	throw exception(0, desc); 
}

// static 
void 
exception::_throw(const char* desc)
{ 
	throw exception(0, desc); 
}

// static 
void 
exception::extract_os_error(size_t code, string_t& err)
{
	// declares buffer
	char buf[2048];
	// extracts OS error
	os_get_error(code, buf, 2048);
	// assigns error
	err = buf;
}

#pragma pack()
END_TERIMBER_NAMESPACE
