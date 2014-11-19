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

#ifndef _terimber_string_hpp_
#define _terimber_string_hpp_

#include "base/string.h"
#include "base/memory.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

static const ub1_t str_leadingByte[6] = {0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

//
// default constructor
//
template < class T >
base_string< T >::base_string(byte_allocator* allocator_) : 
	_allocator(allocator_), _handle(0), _length(0) 
{
}

//
// destructor
//
template < class T >
base_string< T >::~base_string() 
{ 
	_destroy(); 
}
//
// constructs from another string
//
template < class T >
base_string< T >::base_string(const T* x, byte_allocator* allocator_) : 
	_allocator(allocator_), _handle(0), _length(0)
{ 
	*this = x; 
}
//
// copy constructor
//
template < class T >
base_string< T >::base_string(const base_string< T >& x) : 
	_allocator(x._allocator), _handle(0), _length(0) 
{ 
	*this = x; 
}
//
// assign operator from string
//
template < class T >
inline 
base_string< T >&
base_string< T >::operator=(const T* x) 
{ 
	_assign(x); 
	return *this; 
}
//
// assign operator
//
template < class T >
inline 
base_string< T >&
base_string< T >::operator=(const base_string< T >& x) 
{ 
	_assign(x._handle, x._length); 
	return *this; 
} 
//
// compare operators
//
template < class T >
inline 
bool
base_string< T >::operator==(const base_string< T >& x) const 
{ 
	return _compare(x._handle, x._length) == 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator!=(const base_string< T >& x) const 
{ 
	return _compare(x._handle, x._length) != 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator<(const base_string< T >& x) const
{ 
	return _compare(x._handle, x._length) < 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator<=(const base_string< T >& x) const 
{ 
	return _compare(x._handle, x._length) <= 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator>(const base_string< T >& x) const
{ 
	return _compare(x._handle, x._length) > 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator>=(const base_string< T >& x) const
{ 
	return _compare(x._handle, x._length) >= 0; 
} 
//
// copmare operator for input pointers
//
template < class T >
inline 
bool
base_string< T >::operator==(const T* x) const 
{ 
	return _compare(x) == 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator!=(const T* x) const 
{ 
	return _compare(x) != 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator<(const T* x) const 
{ 
	return _compare(x) < 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator<=(const T* x) const 
{ 
	return _compare(x) <= 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator>(const T* x) const 
{ 
	return _compare(x) > 0; 
} 

template < class T >
inline 
bool
base_string< T >::operator>=(const T* x) const 
{ 
	return _compare(x) >= 0; 
} 

template < class T >
inline 
int
base_string< T >::compare(const T* x, size_t size) const
{ 
	return _compare(x, size);
}
//
// useful operators
//
template < class T >
inline 
base_string< T >&
base_string< T >::operator+=(const T* x) 
{ 
	_add(x); 
	return *this; 
}

template < class T >
inline 
base_string< T >&
base_string< T >::operator+=(const base_string< T >& x)
{ 
	_add(x._handle, x._length); 
	return *this; 
} 
//
// appends string
//
template < class T >
inline 
base_string< T >&
base_string< T >::append(const T* x, size_t size) 
{ 
	_add(x, size); 
	return *this; 
}
//
// assigns string
//
template < class T >
inline 
base_string< T >&
base_string< T >::assign(const T* x, size_t size) 
{ 
	_assign(x, size); 
	return *this; 
}
//
// returns length
//
template < class T >
inline 
size_t
base_string< T >::length() const 
{ 
	return _length; 
}
//
// reserves memory 
//
template < class T >
inline 
T*
base_string< T >::reserve(size_t size)
{
	if (_length < size) 
		_destroy(), _length = size;
	else if (_length > size)
	{
		_length = size;
		_handle[_length] = 0;
	}

	if (_length >= 0) _create();
	return _handle;
}

template < class T >
inline 
byte_allocator*
base_string< T >::get_allocator()
{ 
	return _allocator; 
}

// compares function
template < class T >
inline 
int
base_string< T >::_compare(const T* x, size_t len) const
{	
	if (!_handle || !*_handle) 
		return (x && *x) ? -1 : 0; 
	if (!x || !*x) 
		return 1;

	// checks source length
	size_t slen = (len == os_minus_one) ? str_template::strlen(x) : len;
	int res = str_template::strcmp(x, _handle, __min(slen, _length));
	return (res != 0) ? res : (_length == slen ? 0 : (_length < slen ? -1 : 1));
}

// allocates string if need & assign
template < class T >
inline 
void
base_string< T >::_assign(const T* x, size_t len)
{ 
	if (!x)  // null assign
	{
		_destroy();
		return;
	}

	// can be self assignment may be partly
	// saves current value
	T* handle_ = _handle;
	_handle = 0;
	_length = 0;
	_add(x, len);

	if (handle_) 
		_allocator ? _allocator->deallocate(handle_) : delete [] handle_;
}

// adds new string to the one assigned previously
template < class T >
inline 
void
base_string< T >::_add(const T* x, size_t len)
{ 
	if (!x) 
		return; // null add
	size_t _len = len == os_minus_one ? str_template::strlen(x) : len;
	if (!_handle)
	{
		_length = _len;
		if (_length >= 0) _create(), memcpy(_handle, x, S * _length); 
	}
	else // saves previous data
	{
		// saves current value
		T* handle_ = _handle;
		// saves length
		size_t len_ = _length;
		// sets null
		_handle = 0;
		// calculates new length
		_length = len_ + _len;
		// create string
		_create();
		// copies old string
		memcpy(_handle, handle_, S * len_); 
		// copies new string
		memcpy(_handle + len_, x, S * _len);
		// frees previous memory
		_allocator ? _allocator->deallocate(handle_) : delete [] handle_; 
	}
}

// create
template < class T >
inline 
void
base_string< T >::_create()
{ 
	if (_handle) 
		return;
	_handle = _allocator ? (T*)_allocator->allocate(S * (_length + 1)) : new T[_length + 1]; 
	_handle[_length] = 0;		
} 

// destroys
template < class T >
inline 
void
base_string< T >::_destroy()
{ 
	if (!_handle) 
		return;
	_allocator ? _allocator->deallocate(_handle) : delete [] _handle; 
	_length = 0; 
	_handle = 0; 
}

////////////////////////////////////////////////////////////////
namespace str_template
{

inline 
const char* 
strformat(char*, const char* format)
{ 
	return format; 
}

inline 
const wchar_t* 
strformat(wchar_t* buf, const char* format)
{ 
	wchar_t* retVal = buf;
	for (; format && *format; *buf++ = (ub2_t)*format++);
	return retVal;
}

inline 
int 
strscan(const char* buf, size_t len, const char* format, void* res)
{ 
#if OS_TYPE == OS_WIN32 && defined(_MSC_VER) && _MSC_VER > 1200
	return _snscanf(buf, len, format, res);
#else
	return sscanf(buf, format, res);
#endif
}

inline 
int 
strscan(const wchar_t* buf, size_t len, const wchar_t* format, void* res)
{ 
#if OS_TYPE == OS_WIN32 && defined(_MSC_VER) && _MSC_VER > 1200
	return _snwscanf(buf, len, format, res);
#else
	return swscanf(buf, format, res);
#endif
}

inline 
int
strprint(char* buf, size_t len, const char* format, ...)
{ 
	va_list va;
	va_start(va, format);

#if OS_TYPE == OS_WIN32 && defined(_MSC_VER) && _MSC_VER < 1400
	int res = _vsnprintf(buf, len, format, va);
#else
	int res = vsnprintf(buf, len, format, va);
#endif

	va_end(va);
	return res;
}

inline 
int 
strprint(wchar_t* buf, size_t len, const wchar_t* format, ...)
{ 
	va_list va;
	va_start(va, format);

#if OS_TYPE == OS_WIN32 && defined(_MSC_VER) && _MSC_VER <= 1200
	int res = _vsnwprintf(buf, len, format, va);
#else
	int res = vswprintf(buf, len, format, va);
#endif

	va_end(va);
	return res;
}

// 
// template function for string length calculation
// 
template < class T > 
inline 
size_t 
strlen(const T* x) 
{ 
	if (!x) return 0;
	const T* end = x;
	while (*end) ++end;
	return end - x;
}
//
// template function for string coping
//
template < class T > 
inline 
T* 
strcpy(T* dest, const T* x, size_t len) 
{ 
	T* begin = dest;
	while (len-- && *x) *dest++ = *x++;
	*dest = 0;
	return begin;
}
//
// template function for string comparision
//
template < class T > 
inline 
int 
strcmp(const T* dest, const T* x, size_t len) 
{ 
	int ret = 0;
	while (len-- && !(ret = (*dest - *x)) && *x) ++dest, ++x;
	return ret; 
}

inline 
int 
strnocasecmp(const char* dest, const char* x, size_t len)
{
	if (len == os_minus_one)
		len = __min(strlen(dest), strlen(x)) + 1;
#if OS_TYPE == OS_WIN32
	return strnicmp(dest, x, len);
#else
	return strncasecmp(dest, x, len);
#endif
}

inline 
int 
strnocasecmp(const wchar_t* dest, const wchar_t* x, size_t len)
{
	if (len == os_minus_one)
		len = __min(strlen(dest), strlen(x)) + 1;
#if   OS_TYPE == OS_WIN32
	return wcsnicmp(dest, x, len);
#elif OS_TYPE == OS_MACOSX
	return wcsncmp(dest, x, len); 	// (0902-08) ToDo: wcsncmp() is case-sensitive. Make sure this replacement won't break the software.
#else
	return wcsncasecmp(dest, x, len);
#endif
}

///////////////////////////////////////////////////////////////
inline
size_t 
multibyte_to_unicode_len(const char* x, size_t len)
{
	// saves return value
	size_t size = 0;
	// sets dest length t0 zero
	size_t len_ = 0;
	while (x[len_] && len_ < len)
	{
		ub4_t value = 0;
		ub1_t byte_count = 0;
		ub1_t source = x[len_];
		// check
		if (source <= 0x7F) // simple byte
		{
			value = source;
		}
		else
		{
			if (source < 0xC0 || source > 0xFD)
				return 0; //exception::_throw("Invalid char UTF-8 encoding");

			if (source < 0xE0)
				byte_count = 1;
			else if (source < 0xF0)
				byte_count = 2;
			else if (source < 0xF8)
				byte_count = 3;
			else if (source < 0xFC)
				byte_count = 4;
			else
				byte_count = 5;

			value |= source & ((2 << (5 - byte_count)) - 1);
			while (byte_count)
			{
				++len_;
				source = x[len_];
				if (len_ >= len)
					return 0;

				if (source > 0xBF || source < 0x80)
					return 0;

				value <<= 6;
				value |= source & 0x3F;
				--byte_count;
			}

	// 0000 0000-0000 007F   0xxxxxxx
	// 0000 0080-0000 07FF   110xxxxx 10xxxxxx
	// 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
	// 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	// 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	// 0400 0000-7FFF FFFF   1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		} // else
		// assign here
		if (value >= 0x10000) // surrogate pair required
			size += 2;
		else
			++size;

		++len_;
	} // while

	return size;
}

inline
const wchar_t* 
multibyte_to_unicode(wchar_t* dest, size_t dest_len, const char* x, size_t src_len)
{
	// saves return value
	wchar_t* retVal = dest;
	// sets dest length to zero
	size_t len_ = 0;
	while (x[len_] && len_ < src_len)
	{
		ub4_t value = 0;
		ub1_t byte_count = 0;
		ub1_t source = (ub1_t)x[len_];
		// check
		if (source <= 0x7F) // simple byte
		{
			value = source;
		}
		else
		{
			if (source < 0xC0 || source > 0xFD)
				return 0; //exception::_throw("Invalid char UTF-8 encoding");

			if (source < 0xE0)
				byte_count = 1;
			else if (source < 0xF0)
				byte_count = 2;
			else if (source < 0xF8)
				byte_count = 3;
			else if (source < 0xFC)
				byte_count = 4;
			else
				byte_count = 5;

			value |= source & ((2 << (5 - byte_count)) - 1);
			while (byte_count)
			{
				++len_;
				source = x[len_];
				if (len_ >= src_len)
					return 0;

				if (source > 0xBF || source < 0x80)
					return 0;

				value <<= 6;
				value |= source & 0x3F;
				--byte_count;
			}

	// 0000 0000-0000 007F   0xxxxxxx
	// 0000 0080-0000 07FF   110xxxxx 10xxxxxx
	// 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
	// 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	// 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	// 0400 0000-7FFF FFFF   1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		} // else

		// assigns here
		if (value >= 0x10000) // surrogate pair required
		{
			if (dest_len < 2)
				return 0;

			// substracts
			value -= 0x10000;
			// extracts surrogate
			ub2_t surrogate = (ub2_t)(value >> 10) + 0xD800;
			value = (value & 0x03FF) + 0xDC00;
			*dest++ = surrogate;
			*dest++ = (ub2_t)value;
			dest_len -= 2;
		}
		else
		{
			if (dest_len < 1)
				return 0;

			*dest++ = (ub2_t)value;
			--dest_len;
		}

		++len_;
	} // while

	return retVal;
}

inline
const wchar_t* 
multibyte_to_unicode(wstring_t& dest, const char* x, size_t len)
{
	dest = 0;

	if (!x)
		return 0;

	size_t src_len = len == os_minus_one ? strlen(x) : len;
	size_t _len = multibyte_to_unicode_len(x, src_len);
	return multibyte_to_unicode(dest.reserve(_len), _len, x, src_len);
}

inline
const wchar_t* 
multibyte_to_unicode(byte_allocator& _allocator, const char* x, size_t len)
{
	wchar_t* dest = 0;
	
	if (!x)
		return 0;

	size_t src_len = len == os_minus_one ? strlen(x) : len;
	size_t _len = multibyte_to_unicode_len(x, src_len);
	dest = (wchar_t*)_allocator.allocate((_len + 1) * sizeof(ub2_t));
	dest[_len] = 0;
	return multibyte_to_unicode(dest, _len, x, src_len);
}

inline
size_t 
unicode_to_multibyte_len(const wchar_t* x, size_t len)
{
	// saves return value
	size_t size = 0;
	// sets dest length to zero
	size_t len_ = 0;
	// while length
	while (x[len_] && len_ < len)
	{
		// sets value
		ub4_t value = x[len_];
		// if surrogate bytes
		if (value >= 0xD800 && value <= 0xDBFF)
		{
			// moves length
			++len_;
			if (!x[len_])
				return 0; // error here
			// saves surrogate
			ub4_t surrogate = value;
			// 
			value = x[len_];
			value = ((surrogate - 0xD800) << 10) + ((value - 0xDC00) + 0x10000);
		}

		ub1_t byte_count = 0;
		// detects value
		if (value < 0x80)
			byte_count = 1;	
		else if (value < 0x800)
			byte_count = 2;
		else if (value < 0x10000)
			byte_count = 3;
		else if (value < 0x200000)
			byte_count = 4;
		else if (value < 0x4000000)
			byte_count = 5;
		else if (value <= 0x7FFFFFFF)
			byte_count = 6;
		else
			return 0;

		size += byte_count;
		++len_;
	} // while

	return size;
}

inline
const char* 
unicode_to_multibyte(char* dest, size_t dest_len, const wchar_t* x, size_t src_len)
{
	// saves return value
	char* retVal = dest;
	// sets dest length to zero
	size_t len_ = 0;
	// while length
	while (x[len_] && len_ < src_len)
	{
		// sets value
		ub4_t value = x[len_];
		// if surrogate bytes
		if (value >= 0xD800 && value <= 0xDBFF)
		{
			// moves length
			++len_;
			if (!x[len_])
				return 0; // error here
			// saves surrogate
			ub4_t surrogate = value;
			// 
			value = x[len_];
			value = ((surrogate - 0xD800) << 10) + ((value - 0xDC00) + 0x10000);
		}

		ub1_t byte_count = 0;
		// detects value
		if (value < 0x80)
			byte_count = 1;	
		else if (value < 0x800)
			byte_count = 2;
		else if (value < 0x10000)
			byte_count = 3;
		else if (value < 0x200000)
			byte_count = 4;
		else if (value < 0x4000000)
			byte_count = 5;
		else if (value <= 0x7FFFFFFF)
			byte_count = 6;
		else
			return 0;;

		if (dest_len < byte_count)
			return 0; // handle error

		// moves dest to the end
		dest += byte_count;
		// reduces the buffer length
		dest_len -= byte_count;
		switch (byte_count)
		{
			case 6 : *--dest = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 5 : *--dest = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 4 : *--dest = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 3 : *--dest = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 2 : *--dest = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 1 : *--dest = (ub1_t)(value | str_leadingByte[byte_count - 1]);
		} // switch

		// moves dest again
		dest += byte_count;
		++len_;
	} // while

	return retVal;
}

inline
const char* 
unicode_to_multibyte(string_t& dest, const wchar_t* x, size_t len)
{
	dest = 0;

	if (!x)
		return 0;

	size_t src_len = len == os_minus_one ? wcslen(x) : len;
	size_t _len = unicode_to_multibyte_len(x, src_len);	
	return unicode_to_multibyte(dest.reserve(_len), _len, x, src_len);
}

inline
const char* 
unicode_to_multibyte(byte_allocator& _allocator, const wchar_t* x, size_t len)
{
	char* dest = 0;

	if (!x)
		return 0;

	size_t src_len = len == os_minus_one ? wcslen(x) : len;
	size_t _len = unicode_to_multibyte_len(x, src_len);
	dest = (char*)_allocator.allocate(_len + 1);
	dest[_len] = 0;
	return unicode_to_multibyte(dest, _len, x, src_len);
}
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_string_hpp_

