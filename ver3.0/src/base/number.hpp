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

#ifndef _terimber_number_hpp_
#define _terimber_number_hpp_

#include "base/number.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

template < class T >
room_array< T >::room_array(size_t size, byte_allocator* allocator_) : 
	_allocator(allocator_), _buffer(0), _size(size)
{
	assert(_size);
	_buffer = _allocator ? (T*)_allocator->allocate(S * _size) : new T[_size];
}

template < class T >
room_array< T >::room_array(const room_array< T >& x) : 
	_allocator(x._allocator), _buffer(0), _size(0)
{
	*this = x;
}

template < class T >
inline 
room_array< T >& 
room_array< T >::operator>>(size_t shift)
{
	assert(shift <= _size);
	memmove(_buffer, _buffer + shift * S, (_size - shift) * S);
	memset(_buffer + (_size - shift) * S, 0, shift * S);
	return *this;
}

template < class T >
inline 
room_array< T >& 
room_array< T >::operator<<(size_t shift)
{
	assert(shift <= _size);
	memmove(_buffer + shift * S, _buffer, (_size - shift) * S);
	memset(_buffer, 0, shift * S);
	return *this;
}

template < class T >
inline 
void
room_array< T >::swap(room_array< T >& x)
{
	byte_allocator*	allocator_ = _allocator;
	T*				buffer_ = _buffer;
	size_t			size_ = _size;

	_allocator = x._allocator;
	_buffer = x._buffer;
	_size = x._size;

	x._allocator = allocator_;
	x._buffer = buffer_;
	x._size = size_;
}

template < class T >
inline 
size_t
room_array< T >::size() const 
{ 
	return _size; 
}

template < class T >
inline 
byte_allocator*
room_array< T >::get_allocator() 
{ 
	return _allocator; 
}

template < class T >
room_array< T >& 
room_array< T >::operator=(const room_array< T >& x)
{
	if (this != &x)
	{
		// checks capacity
		if (_size < x._size)
			resize(x._size);

		memcpy(_buffer, x._buffer, x._size * S);
		memset(_buffer + x._size * S, 0, (_size - x._size) * S);
	}

	return *this;
}

template < class T >
room_array< T >::~room_array()
{
	clear();
}

template < class T >
room_array< T >& 
room_array< T >::resize(size_t size, bool clean)
{
	assert(size);
	if (_size < size)
	{
		T* new_buffer = _allocator ? (T*)_allocator->allocate(S * size) : new T[size];
		if (_size)
        	memcpy(new_buffer, _buffer, _size * S);

		memset(new_buffer + _size, 0, (size - _size) * S);

		clear();
		_buffer = new_buffer;
	}

	_size = size;
	if (clean)
		memset(_buffer, 0, _size * S);

	return *this;
}

template < class T >
room_array< T >& 
room_array< T >::reserve(size_t size)
{
	return _size < size ? resize(size) : *this;
}

template < class T >
room_array< T >& 
room_array< T >::fill(const T& x, size_t len)
{
	assert(len <= _size);
	memset(_buffer, x, len * S);
	return *this;
}

template < class T >
room_array< T >& 
room_array< T >::copy(const T* x, size_t len)
{
	assert(len <= _size);
	memcpy(_buffer, x, len * S);
	return *this;
}

template < class T >
void 
room_array< T >::back(T* x, size_t len) const
{
	memcpy(x, _buffer, __min(len, _size) * S);
}

template < class T >
room_array< T >& 
room_array< T >::clear()
{
	if (_buffer)
	{
		_allocator ? _allocator->deallocate(_buffer) : delete [] _buffer;
		_buffer = 0;
	}

	_size = 0;
	return *this;
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_number_hpp_

