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

#ifndef _terimber_bitset_hpp_
#define _terimber_bitset_hpp_

#include "base/bitset.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

static const unsigned char low_zero_in_byte[] =
{
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 0xff
};


inline 
base_bitset::base_bitset(size_t capacity) : _capacity(capacity), _bits(0)
{
}

inline 
bool 
base_bitset::set(size_t index, bool value)
{
	// checks bounderies
	if (index >= _capacity)
		return false;

	// finds offset
	size_t off = offset(index);

	// sets mask
	if (value)
		_bits[off] |= mask(index);
	else
		_bits[off] &= ~(mask(index));

	return true;
}

inline 
bool 
base_bitset::get(size_t index) const
{
	// checks bounderies
	if (index >= _capacity)
		return false;

	// finds offset
	size_t off = offset(index);
	// retreives value by mask
	return 0 != (_bits[off] & mask(index));
}

inline 
bool 
base_bitset::find_first_free(size_t& index) const
{
	// assigns pointers
	const unsigned char* begin = _bits;
	const unsigned char* ptr = begin;
	const unsigned char* end = _bits + get_byte_len(_capacity);

	// skips taken bytes
	while (ptr != end && *ptr == 0xff) ++ptr;

	if (ptr == end) // no more room available
		return false;

	// gets the index of the first available bit
	index = bs_bits_in_byte * (ptr - begin) + low_zero_in_byte[*ptr];
	// check bounderies, because capacity can be less than total number of allocated bits
	return index < _capacity;
}

//static 
inline 
size_t
base_bitset::get_byte_len(size_t bitlen)
{
	// gets bits
	return (bitlen >> bs_offset) + ((bitlen & bs_mask) != 0 ? 1 : 0);
}

//static 
inline 
size_t 
base_bitset::offset(size_t index)
{
	return index >> bs_offset;
}

//static 
inline 
unsigned char 
base_bitset::mask(size_t index)
{
	return (0x01 << (unsigned char)(index & bs_mask));
}

inline
bool 
base_bitset::operator==(const base_bitset& x) const
{
	// size is different
	if (_capacity != x._capacity)
		return false;

	if (!_capacity)
		return true;

	return !memcmp(_bits, x._bits, 	get_byte_len(_capacity));
}

inline
bool
base_bitset::operator!=(const base_bitset& x) const
{
	return !operator==(x);
}

inline
base_bitset&
base_bitset::operator|=(const base_bitset& x)
{
	assert(_capacity = x._capacity);
	size_t len = get_byte_len(_capacity);
	for (size_t index = 0; index < len; ++index)
      _bits[index] |= x._bits[index];
	return *this;
}

inline
base_bitset& 
base_bitset::operator&=(const base_bitset& x)
{
	assert(_capacity = x._capacity);
	size_t len = get_byte_len(_capacity);
	for (size_t index = 0; index < len; ++index)
       _bits[index] &= x._bits[index];
	return *this;
}

inline
void
base_bitset::reset()
{
	if (_capacity)
		memset(_bits, 0, get_byte_len(_capacity));
}

inline
bool 
base_bitset::empty() const
{
	// assigns pointers
	const unsigned char* begin = _bits;
	const unsigned char* ptr = begin;
	const unsigned char* end = _bits + get_byte_len(_capacity);

	// skips taken bytes
	while (ptr != end && *ptr == 0x00) ++ptr;

	return ptr == end;
}

inline 
bool 
base_bitset::operator<(const base_bitset& x) const
{
	if (!_capacity)
		return x._capacity != 0;
	else if (!x._capacity)
		return false;
	else
	{
		size_t min_capacity = __min(_capacity, x._capacity);
		int ret = memcmp(_bits, x._bits, get_byte_len(min_capacity));
		return ret ? ret < 0 : _capacity < x._capacity;
	}
}
////////////////////////////////////////////////////////////////////////////
inline 
_bitset::_bitset() :
	base_bitset(0)
{
}

inline
_bitset::_bitset(TERIMBER::byte_allocator& all, size_t capacity) :
	  base_bitset(capacity)
{
	size_t len = get_byte_len(capacity);
	if (len)
	{
		_bits = (unsigned char*)all.allocate(len);
		memset(_bits, 0, len);
	}
}

inline
_bitset& 
_bitset::operator=(const _bitset& x)
{
	_bits = x._bits;
	_capacity = x._capacity;
	return *this;
}

inline
_bitset& 
_bitset::assign(byte_allocator& all, const _bitset& x)
{
	_capacity = x._capacity;
	size_t len = get_byte_len(x._capacity);
	if (len)
	{
		_bits = (unsigned char*)all.allocate(len);
		memcpy(_bits, x._bits, len);
	}
	else
		_bits = 0;

	return *this;
}

inline
_bitset&
_bitset::resize(byte_allocator& all, size_t capacity)
{
	size_t oldlen = get_byte_len(_capacity);
	size_t newlen = get_byte_len(capacity);
	if (newlen > oldlen)
	{
		// allocates new size
		unsigned char* bits = (unsigned char*)all.allocate(newlen);
		// preserves old bits
		if (oldlen)
			memcpy(bits, _bits, oldlen);
		// sets zero new bits
		memset(bits + oldlen, 0, newlen - oldlen);
		// resets pointer
		_bits = bits;
	}
	else if (!newlen)
		_bits = 0;

	_capacity = capacity;
	return *this;

}

inline
void
_bitset::clear()
{
	if (_capacity)
		_bits = 0, _capacity = 0;
}

////////////////////////////////////////////////////////////////////////////
inline 
bitset::bitset() :
	base_bitset(0)
{
}

inline
bitset::bitset(size_t capacity) :
	  base_bitset(capacity)
{
	size_t len = get_byte_len(capacity);
	if (len)
	{
		_bits = (unsigned char*)malloc(len);
		memset(_bits, 0, len);
	}
}

inline
bitset::~bitset()
{
	free(_bits);
}

inline
bitset& 
bitset::operator=(const bitset& x)
{
	if (_bits)
		free (_bits), _bits = 0;

	_capacity = x._capacity;
	size_t len = get_byte_len(_capacity);
	if (len)
	{
		_bits = (unsigned char*)malloc(len);
		memcpy(_bits, x._bits, len);
	}

	return *this;
}

inline
bitset&
bitset::resize(size_t capacity)
{
	size_t oldlen = get_byte_len(_capacity);
	size_t newlen = get_byte_len(capacity);
	if (newlen > oldlen)
	{
		// allocates new size
		unsigned char* bits = (unsigned char*)malloc(newlen);
		// preserves old bits
		if (oldlen)
			memcpy(bits, _bits, oldlen);
		// sets zero new bits
		memset(bits + oldlen, 0, newlen - oldlen);
		// resets pointer
		_bits = bits;
	}
	else if (!newlen && _bits)
	{
		free(_bits), _bits = 0;
	}

	_capacity = capacity;
	return *this;
}

//! \brief sets capacity to zero
inline
void
bitset::clear()
{
	if (_capacity)
		free(_bits), _bits = 0, _capacity = 0;
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif

