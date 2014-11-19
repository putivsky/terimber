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

#ifndef _terimber_vector_hpp_
#define _terimber_vector_hpp_

#include "base/vector.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//
// constructor
//
template < class T >
base_vector< T >::base_vector() : 
	_first(0), _length(0) 
{
}
//
// destructor
//
template < class T >
base_vector< T >::~base_vector() 
{
}
//
// copy constructor
//
template < class T >
base_vector< T >::base_vector(const base_vector& x) : 
	_first(0), _length(0) 
{
}

//
// checks empty
//
template < class T >
inline
bool
base_vector< T >::empty() const 
{ 
	return !_length; 
}
//
// returns size
//
template < class T >
inline
size_t
base_vector< T >::size() const 
{ 
	return _length; 
}
//
// vector doesn't check boundaries
//
template < class T >
inline
const T&
base_vector< T >::operator[](size_t p) const 
{ 
	return *(const T*)(_first + p); 
}
//
// vector doesn't check boundaries
//
template < class T >
inline
T&
base_vector< T >::operator[](size_t p) 
{ 
	return *(_first + p); 
}
//
// supports iterators for STL's algorithms
//
template < class T >
inline
TYPENAME base_vector< T >::const_iterator
base_vector< T >::begin() const 
{ 
	return const_iterator(_first); 
}
template < class T >
inline
TYPENAME base_vector< T >::iterator
base_vector< T >::begin() 
{ 
	return iterator(_first); 
}

template < class T >
inline
TYPENAME base_vector< T >::const_iterator
base_vector< T >::end() const 
{ 
	return const_iterator(_first + _length); 
}

template < class T >
inline
TYPENAME base_vector< T >::iterator
base_vector< T >::end() 
{ 
	return iterator(_first + _length); 
}

// class supports vector with external allocator
// size of empty vector is 8 byte
//
// constructor
//
template < class T, class A >
_vector< T, A >::_vector() : 
	base_vector< T >() 
{
}
//
// destructor
//
template < class T, class A >
_vector< T, A >::~_vector()
{ 
	clear(); 
}
//
// copy constructor
//
template < class T, class A >
_vector< T, A >::_vector(const _vector& x) : 
	base_vector< T >(x) 
{ 
	*this = x; 
}
//
// copies only pointer to first element and size
// NO memory reallocation or copy
//
template < class T, class A >
inline
_vector< T, A >&
_vector< T, A >::operator=(const _vector& x) 
{ 
	if (this != &x) 
	{
		this->_first = x._first; 
		this->_length = x._length;	
	}
	return *this; 
}
//
// assigns range of iterators
//
template < class T, class A >
inline
bool
_vector< T, A >::assign(A& allocator_, TYPENAME _vector< T, A >::const_iterator first, TYPENAME _vector< T, A >::const_iterator last)
{ 
	clear(); 
	size_t len = last - first; 
	if (!resize(allocator_, len))
		return false;
	size_t I = 0;
	for(first; first != last; ++first, ++I) 
		new(this->_first + I) T(*first); 
	return true;
}
//
// assigns n equal values
//
template < class T, class A >
inline
bool
_vector< T, A >::assign(A& allocator_, size_t n, const T& x)
{ 
	clear(); 
	return resize(allocator_, n, x); 
}
//
// assigns another vector 
// copies full values
//
template < class T, class A >
inline
bool
_vector< T, A >::assign(A& allocator_, const _vector& x)
{ 
	clear(); 
	if (!resize(allocator_, x._length))
		return false;
	for(size_t I = 0; I < x._length; ++I) 
		new(this->_first + I) T(*(x._first + I)); 
	return true;
}
//
// expands boundaries, if needed
// copy constructor must be defined for template T
//
template < class T, class A >
inline
bool
_vector< T, A >::resize(A& allocator_, size_t n, const T& x)
{ 
	if (n > this->_length) // check size
	{
		// allocates new n requested elements
		T* ptr = _buynodes(allocator_, n);	
		if (!ptr)
			return false;
		// copies previous elements
		memcpy(ptr, this->_first, sizeof(T) * this->_length);
		// initializes new elements
		for (size_t I = this->_length; I < n; ++I) { new( ptr + I ) T(x); }
		// sets members
		this->_first = ptr, this->_length = n;
	}
	else if (n < this->_length) 
		_reduce(n);

	return true;
}
//
// expands boundaries, if needed
// for performance - T should be a simple data struct
// with possibility zeroing memory
//
template < class T, class A >
inline
bool
_vector< T, A >::resize(A& allocator_, size_t n)
{
	if (n > this->_length) // check size
	{
		// allocates new n requested elements
		T* ptr = _buynodes(allocator_, n);
		if (!ptr)
			return false;
		// copies previous elements
		memcpy(ptr, this->_first, sizeof(T) * this->_length);
		// initializes new elements
		{ memset(ptr + this->_length, 0, sizeof(T) * (n - this->_length)); } 
		// sets members
		this->_first = ptr, this->_length = n;
	}
	else if (n < this->_length)
		_reduce(n);

	return true;
}


template < class T, class A >
inline
void
_vector< T, A >::reduce(size_t n)
{
	if (n < this->_length)
		this->_reduce(n);
}
//
// clears vector
// NO memory deallocation
//
template < class T, class A >
inline
void
_vector< T, A >::clear() 
{ 
	this->_first = 0; 
	this->_length = 0; 
}

template < class T, class A >
inline 
void 
_vector< T, A >::clear(array_allocator< T >& allocator_)
{
	allocator_.deallocate(this->_first);
}


// shrinks size of vector
// NO memory deallocation
template < class T, class A >
inline
void
_vector< T, A >::_reduce(size_t n)
{
	if (n >= 0 && n < this->_length)
	{
		// sets new length
		this->_length = n;
		if (!this->_length) 
			this->_first = 0; // reduces to zero
	}
}

template < class T, class A >
inline
T*
_vector< T, A >::_buynodes(byte_allocator& allocator_, size_t n)
{ 
	return (T*)allocator_.allocate(sizeof(T)*n); 
}

template < class T, class A >
inline
T*
_vector< T, A >::_buynodes(array_allocator< T >& allocator_, size_t n)
{ 
	return (T*)allocator_.allocate(n); 
}

// class supports vector with internal allocator
//
// constructor
//
template < class T >
vector< T >::vector(size_t capacity) : 
	base_vector< T >(), _allocator(capacity) 
{
}
//
// destructor
//
template < class T >
vector< T >::~vector() 
{ 
	clear(); 
}
//
// copy constructor
//
template < class T >
vector< T >::vector(const vector& x) : 
	base_vector< T >(x), _allocator(x._allocator.capacity())
{ 
	*this = x; 
}
//
// full memory copy
//
template < class T >
inline
vector< T >&
vector< T >::operator=(const vector& x)
{
	if (this != &x)
	{
		clear();
		_allocator.reset();
		resize(x._length);
		for(size_t I = 0; I < x._length; ++I)	
			new(this->_first + I) T(*(x._first + I));
	}

	return *this;
}
//
// assigns range of iterators
//
template < class T >
inline
bool
vector< T >::assign(TYPENAME vector< T >::const_iterator first, TYPENAME vector< T >::const_iterator last)
{ 
	clear(); 
	size_t len = last - first; 
	if (!resize(len))
		return false;
	size_t I = 0;
	for(first; first != last; ++first, ++I) 
		new(this->_first + I) T(*first); 
	return true;
}
//
// assigns n equal copies
//
template < class T >
inline
bool
vector< T >::assign(size_t n, const T& x) 
{ 
	clear(); 
	return resize(n, x); 
}
//
// assigns another vector
//
template < class T >
inline
bool
vector< T >::assign(const vector& x) 
{ 
	clear(); 
	if (!resize(x._length))
		return false;
	for(size_t I = 0; I < x._length; ++I)	
		new(this->_first + I) T(*(x._first + I)); 

	return true;
}
//
// expands boundaries, if needed
// must be a defined copy constructor for template T
//
template < class T >
inline
bool
vector< T >::resize(size_t n, const T& x)
{
	if (n > this->_length) // checks size
	{
		// allocates new n requested elements
		T* ptr = _buynodes(n);
		if (!ptr)
			return false;
		// copies previous elements
		for (size_t I = 0; I < this->_length; ++I) 
			new(ptr + I) T(*(this->_first + I));
		// initializes new elements
		for (size_t J = this->_length; J < n; ++J) 
			new(ptr + J) T(x);
		// sets members
		clear();
		this->_first = ptr, this->_length = n;
	}
	else if (n < this->_length) 
		this->_reduce(n);

	return true;
}
//
// clears vector
//
template < class T >
inline
void
vector< T >::clear() 
{ 
	for (size_t I = 0; I < this->_length; ++I) 
		(this->_first + I)->~T(); 
	_allocator.deallocate(this->_first);
	this->_first = 0, this->_length = 0; 
}
//
// shrinks size of vector
//
template < class T >
inline
void
vector< T >::_reduce(size_t n)
{
	// initializes new elements
	for (size_t I = n; I < this->_length; ++I) 
		(this->_first + I)->~T(); 
	// sets a new length
	this->_length = n;
	if (!this->_length) 
	{ 
		_allocator.reset(); 
		this->_first = 0; 
	} // reduces to zero
}
//
// allocates an array of nodes
//
template < class T >
inline
T*
vector< T >::_buynodes(size_t n) 
{ 
	return (T*)_allocator.allocate(n); 
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_vector_hpp_

