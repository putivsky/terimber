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

#ifndef _terimber_common_hpp_
#define _terimber_common_hpp_

#include "base/common.h"
#include "base/except.h"
#include "base/string.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// declaration for heximal conversion
//! \brief period char
const ub1_t _ch_period = '.';
//! \brief 0 char
const ub1_t _ch0 = '0';
//! \brief 1 char
const ub1_t _ch1 = '1';
//! \brief 7 char
const ub1_t _ch7 = '7';
//! \brief 9 char
const ub1_t _ch9 = '9';
//! \brief A char
const ub1_t _chA = 'A';
//! \brief a char
const ub1_t _cha = 'a';
//! \brief F char
const ub1_t _chF = 'F';
//! \brief f char
const ub1_t _chf = 'f';


inline 
void* 
check_pointer(void* ptr)
{ 
	if (!ptr) 
		exception::_throw("Not enough memory"); 
	return ptr; 
}

inline 
size_t 
get8bits(unsigned char x)
{
	return (size_t)tolower(x);
}

inline 
size_t 
get16bits(const unsigned char* x)
{
	return (size_t)tolower(x[0]) + (size_t)(tolower(x[1]) << 8);
}

template < class T >
inline 
size_t 
do_hash(const T* x, size_t len)
{
	if (!len || !x)
		return 0;

	// gets correct length
	if (len == os_minus_one)
		len = str_template::strlen(x);

	const unsigned char* ptr = (unsigned char*)x;
	size_t hash = len;
	size_t rem = len & 3;
	len >>= 2;

	// hashes by 4 bytes
	for (; len > 0; --len)
	{
		hash += get16bits(ptr);
		size_t tmp = (get16bits(ptr + 2) << 11) ^ hash;
		hash += ((hash << 16) ^ tmp) >> 11;
		ptr += 4;
	}

	// hashes the remainder
	switch (rem)
	{
		case 3:
			hash += get16bits(ptr);
			hash ^= hash << 16;
			hash ^= get8bits(ptr[2]) << 18;
			hash += hash >> 11;
			break;
		case 2:
			hash += get16bits(ptr);
			hash ^= hash << 11;
			hash += hash >> 17;
			break;
		case 1:
			hash += get8bits(ptr[0]);
			hash ^= hash << 10;
			hash += hash >> 1;
	}

	// final hashing
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

template < class T >
inline
T* 
copy_string(const T* x, byte_allocator& _allocator, size_t len)
{
	T* dest = 0;
	if (!x)
		return 0;

	len = (len == os_minus_one) ? str_template::strlen(x) : len;
	dest = (T*)_allocator.allocate((len + 1) * sizeof(T));
	if (dest)
	{
		memcpy(dest, x, len * sizeof(T));
		dest[len] = 0;
	}

	return dest;
}

template < class T >
inline
bool 
hex_to_binary(ub1_t* dest, const T* x, size_t len)
{
	if (!dest)
		return false;

	*(size_t*)dest = 0;

	// sets the correct length
	len = (len == os_minus_one) ? (x ? strlen(x) : 0) : len;

	if (!len)
		return false;

	// gets the pointer to buffer
	ub1_t* buf = dest + sizeof(size_t);

	size_t index = 0;
	for (; index < len / 2; ++index)
	{
		if (!hex_to_byte(*buf, x, RADIX16))
			return false;
		++buf;
		x += 2;
	}

	// sets the buffer length
	*(size_t*)dest = index;
	return true;
}

template < class T >
inline 
bool 
symbol_to_byte(ub1_t &dest, T x, numeric_radix radix_)
{
	switch (radix_)
	{
		case RADIX16:
			if (x >= _ch0 && x <= _ch9)
				dest = x - _ch0;
			else if (x >= _chA && x <= _chF)
				dest = x - _chA + 0x0A;
			else if (x >= _cha && x <= _chf)
				dest = x - _cha + 0x0A;
			else
				return false;
			break;
		case RADIX2:
			if (x >= _ch0 && x <= _ch1)
				dest = x - _ch0;
			else
				return false;
			break;
		case RADIX8:
			if (x >= _ch0 && x <= _ch7)
				dest = x - _ch0;
			else
				return false;
			break;
		case RADIX10:
			if (x >= _ch0 && x <= _ch9)
				dest = x - _ch0;
			else
				return false;
			break;
	} // switch

	return true;
}

template < class T >
inline
bool 
hex_to_byte(ub1_t &dest, const T* x, numeric_radix radix_)
{
	if (!x)
		return false;

	ub1_t uplimit = 0;
	switch (radix_)
	{
		case RADIX16:
			uplimit = 2;
			break;
		case RADIX2:
			uplimit = 8;
			break;
		case RADIX8:
			uplimit = 4;
			break;
		case RADIX10:
			uplimit = 3;
			break;
	} // switch

	dest = 0;
	for (ub1_t index = 0; index < uplimit; ++index)
	{				
		dest *= radix_;
		ub1_t dummy = 0;
		if (symbol_to_byte(dummy, x[index], radix_))
			dest += dummy;
		else
			return false;
	}

	return true;
}

template < class T >
inline
void 
binary_to_hex(T* dest, const ub1_t* x)
{
	if (!dest || !x)
		return;

	*dest = 0;

	// gets the buffer length
	size_t len = *(size_t*)x;
	dest[2 * len] = 0;
	const ub1_t* buf = x + sizeof(size_t);

	for (size_t index = 0; index < len; ++index)
	{
		byte_to_hex(dest, *buf);
		++buf;
		dest += 2;
	}
}

template < class T >
inline
void 
byte_to_symbol(T& dest, ub1_t x)
{
	dest = (x & 0x0F) < 0x0A ? _ch0 + (x & 0x0F) : _chA + ((x - 0x0A) & 0x0F);
}

template < class T >
inline
void 
byte_to_hex(T* dest, ub1_t x)
{
	assert(dest);
	byte_to_symbol(dest[1], x);
	x >>= 4;
	byte_to_symbol(dest[0], x);
}

template < class T >
inline
T* 
guid_to_string(T* dest, const guid_t& x)
{
	assert(dest);
	T* retVal = dest;
	byte_to_hex(dest, (ub1_t)((0xff000000 & x.Data1) >> 24));
	dest += 2;
	byte_to_hex(dest, (ub1_t)((0x00ff0000 & x.Data1) >> 16));
	dest += 2;
	byte_to_hex(dest, (ub1_t)((0x0000ff00 & x.Data1) >> 8));
	dest += 2;
	byte_to_hex(dest, (ub1_t)(0x000000ff & x.Data1));
	dest += 2;

	byte_to_hex(dest, (ub1_t)((0xff00 & x.Data2) >> 8));
	dest += 2;
	byte_to_hex(dest, (ub1_t)(0x00ff & x.Data2));
	dest += 2;

	byte_to_hex(dest, (ub1_t)((0xff00 & x.Data3) >> 8));
	dest += 2;
	byte_to_hex(dest, (ub1_t)(0x00ff & x.Data3));
	dest += 2;

	for (size_t index = 0; index < 8; ++index)
	{
		byte_to_hex(dest, x.Data4[index]);
		dest += 2;
	}

	*dest = 0;

	return retVal;
}

template < class T >
inline
bool 
string_to_guid(guid_t& dest, const T* x)
{
	ub1_t b1, b2, b3, b4;

	memset(&dest, 0, sizeof(guid_t));
	if (!x) 
		return false;
	if (!hex_to_byte(b1, x, RADIX16)) 
		return false;
	x += 2;
	if (!hex_to_byte(b2, x, RADIX16)) 
		return false;
	x += 2;
	if (!hex_to_byte(b3, x, RADIX16)) 
		return false;
	x += 2;
	if (!hex_to_byte(b4, x, RADIX16)) 
		return false;

	dest.Data1 =	(ub4_t)(b1 << 24) | 
					(ub4_t)(b2 << 16) |
					(ub4_t)(b3 << 8)  |
					(ub4_t)b4;

	x += 2;
	if (!hex_to_byte(b1, x, RADIX16)) 
		return false;
	x += 2;
	if (!hex_to_byte(b2, x, RADIX16)) 
		return false;

	dest.Data2 =	(ub2_t)(b1 << 8) | 
					(ub2_t)b2;

	x += 2;
	if (!hex_to_byte(b1, x, RADIX16)) 
		return false;
	x += 2;
	if (!hex_to_byte(b2, x, RADIX16)) 
		return false;

	dest.Data3 =	(ub2_t)(b1 << 8) | 
					(ub2_t)b2;

	for (size_t index = 0; index < 8; ++index)
	{
		x += 2;
		if (!hex_to_byte(dest.Data4[index], x, RADIX16)) 
			return false;
	}

	return true;
}

template < class T >
inline 
int 
simple_compare(const T& v1, const T& v2)
{ 
	return v1 != v2 ? (v1 < v2 ? -1 : 1 ) : 0; 
}

template < class T >
inline 
int 
memory_compare(const T& v1, const T& v2)
{ 
	return !v1 ? (!v2 ? 0 : -1) : (!v2 ? 1 : memcmp(v1, v2, sizeof(T))); 
}

inline 
int 
memory_compare_binary(const size_t* v1, const size_t* v2)
{ 
	return !v1 ? (!v2 ? 0 : -1) : (!v2 ? 1 : memcmp(v1 + 1, v2 + 1, __min(*v1, *v2))); 
}

template < class T >
inline 
int 
simple_compare_ptr(const T* v1, const T* v2)
{
	return !v1 ? (!v2 ? 0 : -1) : (!v2 ? 1 : simple_compare(*v1, *v2)); 
}

template < class T, class F >
inline 
int 
string_compare(const T& v1, const T& v2, F fn)
{ 
	return !v1 ? (!v2 ? 0 : -1) : (!v2 ? 1 : fn(v1, v2)); 
}

inline 
size_t 
do_hash(vt_types type, const terimber_xml_value& x)
{ 
	size_t res = 0;

	switch(type)
	{
		case vt_binary:
			{
				size_t len = x.bufVal ? *(size_t*)x.bufVal : 0;
				const char* ptr = len ? ((const char*)x.bufVal + sizeof(size_t)) : 0;
				res = do_hash(ptr, len);
			}
			break;
		case vt_string:
			do_hash(x.strVal, os_minus_one);
			break;
		case vt_wstring:
			do_hash(x.wstrVal, os_minus_one);
			break;
		default:
			break;
	}
	
	return res;
}

inline 
int 
compare_value(vt_types type, const terimber_xml_value& first, const terimber_xml_value& second, bool use_hash, bool case_insensitive)
{
	switch(type)
	{
		case vt_empty:
		case vt_null:
			return 0;
		case vt_bool:
			return simple_compare(first.boolVal, second.boolVal);
		case vt_sb1:
			return simple_compare(first.cVal, second.cVal);
		case vt_ub1:
			return simple_compare(first.bVal, second.bVal);
		case vt_sb2:
			return simple_compare(first.iVal, second.iVal);
		case vt_ub2:
			return simple_compare(first.uiVal, second.uiVal);
		case vt_sb4:
			return simple_compare(first.lVal, second.lVal);
		case vt_ub4:
			return simple_compare(first.ulVal, second.ulVal);
		case vt_float:
			return simple_compare(first.fltVal, second.fltVal);
		case vt_double:
		case vt_date:
#ifdef OS_64BIT
			return simple_compare(first.dblVal, second.dblVal);
#else
			return simple_compare_ptr(first.dblVal, second.dblVal);
#endif
		case vt_guid:
			return memory_compare(first.guidVal, second.guidVal);
		case vt_sb8:
#ifdef OS_64BIT
			return simple_compare(first.intVal, second.intVal);
#else
			return simple_compare_ptr(first.intVal, second.intVal);
#endif
		case vt_ub8:
#ifdef OS_64BIT
			return simple_compare(first.uintVal, second.uintVal);
#else
			return simple_compare_ptr(first.uintVal, second.uintVal);
#endif
		case vt_binary:
			return memory_compare_binary((const size_t*)first.bufVal, (const size_t*)second.bufVal);
		case vt_string:
			return use_hash ? simple_compare(first.lVal, second.lVal) : (case_insensitive ? str_template::strnocasecmp(first.strVal, second.strVal, os_minus_one) : str_template::strcmp(first.strVal, second.strVal, os_minus_one));
		case vt_wstring:
			return use_hash ? simple_compare(first.lVal, second.lVal) : (case_insensitive ? str_template::strnocasecmp(first.wstrVal, second.wstrVal, os_minus_one) : str_template::strcmp(first.wstrVal, second.wstrVal, os_minus_one));
		default:
			return -1;
	}
}

inline 
terimber_xml_value 
copy_value(vt_types type, const terimber_xml_value& x, byte_allocator& _allocator)
{
	terimber_xml_value dest;
	memset(&dest, 0, sizeof(terimber_xml_value));

	switch(type)
	{
		case vt_empty:
		case vt_null:
		case vt_bool:
		case vt_sb1:
		case vt_ub1:
		case vt_sb2:
		case vt_ub2:
		case vt_sb4:
		case vt_ub4:
		case vt_float:
			memcpy(&dest, &x, sizeof(terimber_xml_value));
			break;
		case vt_double:
			if (x.dblVal)
			{
#ifdef OS_64BIT
				dest.dblVal = x.dblVal;
#else
				double* dummy = 0;
				dummy = (double*)check_pointer(_allocator.allocate(sizeof(double)));
				*dummy = *x.dblVal;
				dest.dblVal = dummy;
#endif
			}
			break;
		case vt_guid:
			if (x.guidVal)
			{
				guid_t* dummy = (guid_t*)check_pointer(_allocator.allocate(sizeof(guid_t)));
				*dummy = *x.guidVal;
				dest.guidVal = dummy;
			}
			break;
		case vt_sb8:
		case vt_date:
			if (x.intVal)
			{
#ifdef OS_64BIT
				dest.intVal = x.intVal;
#else
				sb8_t* dummy = (sb8_t*)check_pointer(_allocator.allocate(sizeof(sb8_t)));
				memcpy(dummy, x.intVal, sizeof(sb8_t));
				dest.intVal = dummy;
#endif
			}
			break;
		case vt_ub8:
			if (x.uintVal)
			{
#ifdef OS_64BIT
				dest.uintVal = x.uintVal;
#else
				ub8_t* dummy = (ub8_t*)check_pointer(_allocator.allocate(sizeof(ub8_t)));
				memcpy(dummy, x.uintVal, sizeof(ub8_t));
				dest.uintVal = dummy;
#endif
			}
			break;
		case vt_binary:
			if (x.bufVal)
			{
				size_t len = *(size_t*)x.bufVal;
				ub1_t* dummy = (ub1_t*)check_pointer(_allocator.allocate(len + sizeof(size_t)));
				// sets length
				*(size_t*)dummy = len;
				if (len) // copy data
					memcpy(dummy + sizeof(size_t), x.bufVal + sizeof(size_t), len);

				dest.bufVal = dummy;
			}
			break;
		case vt_decimal:
		case vt_numeric:
		case vt_string:
			if (x.strVal)
				dest.strVal = copy_string(x.strVal, _allocator, os_minus_one);
			break;
		case vt_wstring:
			if (x.wstrVal)
				dest.wstrVal = copy_string(x.wstrVal, _allocator, os_minus_one);
			break;
		default:
			assert(false);
			break;
	}

	return dest;
}

////////////////////////////////////////////////////////////
inline 
bool 
operator==(const guid_t& first, const guid_t& second)
{ 
	return memcmp(&first, &second, sizeof(guid_t)) == 0; 
}

inline 
bool 
operator!=(const guid_t& first, const guid_t& second)
{ 
	return memcmp(&first, &second, sizeof(guid_t)) != 0; 
}

inline 
bool 
operator<(const guid_t& first, const guid_t& second)
{ 
	return memcmp(&first, &second, sizeof(guid_t)) < 0; 
}

inline bool operator>(const guid_t& first, const guid_t& second)
{ return memcmp(&first, &second, sizeof(guid_t)) > 0; }

////////////////////////////////////////////////////////////
// push symbol to buffer
inline 
paged_buffer& 
paged_buffer::operator<<(ub1_t symbol)
{
	if (_pos == _size)
		add_page();

	_ptr[_pos++] = symbol;

	return *this;
}

// pushes string to buffer
inline 
paged_buffer& 
paged_buffer::operator<<(const char* x)
{
	while (*x)
	{
		if (_pos == _size)
			add_page();
		_ptr[_pos++] = *x++;
	} // while

	return *this;
}

// pushes bytes to buffer
inline 
paged_buffer& 
paged_buffer::append(const ub1_t* x, size_t size)
{
	size_t len;
	while (size)
	{
		if (_pos == _size)
			add_page();
		else
		{
			len = __min(size, _size - _pos);
			memcpy(_ptr + _pos, x, len);
			size -= len;
			_pos += len;
			x += len;
		}
	} // while

	return *this;
}

// persists paged blocks into one permanent block
inline 
const char* 
paged_buffer::persist()
{
	// checks for one block
	if (_buffer.empty())
	{
		_primary[_pos] = 0;
		return (const char*)_primary;
	}
	else
		return _persist();

}

// persists paged blocks into one permanent block
inline 
const ub1_t* 
paged_buffer::persist(size_t& size_)
{
	// checks for one block
	if (_buffer.empty())
	{
		size_ = _pos;
		return _primary;
	}
	else
		return _persist(size_);
}

// persists paged blocks into one permanent block (internal allocator)
// if internally only one block has been allocated
// then the class dosn't make a copy - it simply returns the pointer
inline 
void 
paged_buffer::reset()
{ 
	_pos = 0;
	_buffer.clear();
	_ptr = _primary;
}

// calculates the size of persisted chars
inline 
size_t 
paged_buffer::size() const
{
	if (_buffer.empty())
		return _pos;
	else
	{
		size_t retVal = _size;
		for (paged_store_t::const_iterator iter = _buffer.begin(); iter != _buffer.end(); ++iter)
		{
			retVal += (iter != --_buffer.end()) ? _size : _pos;
		}
		return retVal;
	}
}

// shares memory resource for a while
inline 
byte_allocator& 
paged_buffer::get_tmp_allocator()
{ 
	return _tmp_allocator; 
}
	
inline 
size_t 
paged_buffer::get_page_size() const
{ 
	return _size; 
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_common_hpp_

