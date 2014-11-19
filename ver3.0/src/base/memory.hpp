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

#ifndef _terimber_memory_hpp_
#define _terimber_memory_hpp_

#include "base/memory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// class to keep free chunks of memory for reusing
// clears stack
inline
chunk_stack::chunk_stack() : 
	_head(0) 
{
}

inline 
void 
chunk_stack::clear()
{ 
	_head = 0; 
}

// pushes new chunk
inline 
void 
chunk_stack::push(size_t* chunk)
{ 
	if (!chunk) 
		return; 
	*chunk = reinterpret_cast< size_t >(_head); 
	_head = chunk; 
}

// removes the head chunk
inline 
size_t* 
chunk_stack::pop() 
{ 
	size_t* chunk = _head; 
	if (chunk) 
		_head = reinterpret_cast< size_t* >(*chunk); 
	return chunk; 
}

// return the head chunk
inline 
size_t* 
chunk_stack::top() 
{ 
	return _head; 
}

// returns the head chunk
inline 
bool
chunk_stack::empty()
{ 
	return _head == 0; 
}

inline 
void 
byte_allocator::reset(bool secure)
{ 
	if (secure && _start_chunk)
	{
		_using_chunk = _start_chunk;
		while (_using_chunk) // while memory is there
		{
			memset(_using_chunk->_mem, 0, _using_chunk->_chunk_size);
			_using_chunk = _using_chunk->_next_chunk; // move to next chunk
		}
	}

	_using_chunk = _start_chunk; 
	_free_pos = _start_chunk ? _start_chunk->_mem : 0;
}

inline 
void* 
byte_allocator::allocate(size_t size)
{ 
	if (!size) return 0;
		//corect size
	size_t correct_size = ALIGNED_SIZEOF(size);
	// try do it easy
	if (_using_chunk && _using_chunk->_chunk_size - (_free_pos - _using_chunk->_mem) >= correct_size)
	{ 
		unsigned char* pos = _free_pos;
		_free_pos += correct_size; 
		return pos; 
	}
	else // you can get nothing easy in the current life
		return next_chunk(correct_size);
}

// deallocates
// we don't support deallocation by pointer
// everything or nothing
inline 
void 
byte_allocator::deallocate(void*) 
{
}

// returns the capacity of chunk
inline 
size_t 
byte_allocator::capacity() const 
{ 
	return _capacity; 
}

// returns the count of chunks
inline 
size_t 
byte_allocator::count() const 
{ 
	return _count; 
}

/////////////////////////////////////////////////////
inline 
void
rep_allocator::clear_extra(bool secure) 
{ 
	_rep.clear(); 
	byte_allocator::clear_extra(secure); 
}

inline 
void
rep_allocator::clear_all(bool secure) 
{ 
	_rep.clear(); 
	byte_allocator::clear_all(secure); 
}

inline 
void 
rep_allocator::reset(bool secure) 
{ 
	_rep.clear(); 
	byte_allocator::reset(secure); 
}

// push pointer to object back to repository stack
inline 
void 
rep_allocator::deallocate(void* p) 
{ 
	_rep.push((size_t*)p); 
}

////////////////////////////////////////////////
// high performance template class for allocation
// one object at a time
// constructor
template < class T >
node_allocator< T >::node_allocator(size_t capacity) : 
	rep_allocator((capacity ? capacity : os_def_size) * ALIGNED_SIZEOF(sizeof(T))) 
{
}

template < class T >
inline
T*
node_allocator< T >::allocate()
{ 
	return _rep.empty() ? (T*)byte_allocator::allocate(ALIGNED_SIZEOF(sizeof(T))) : (T*)_rep.pop(); 
}

////////////////////////////////////////////////////
// template class for allocation of the array of objects
// it seems like this class covers the functionality of previous one
// but the previous class (node_allocator) has better performance
// constructor
template < class T >
array_allocator< T >::array_allocator(size_t capacity) : 
	rep_allocator((capacity ? capacity : os_def_size) * ALIGNED_SIZEOF(sizeof(T))) 
{
}

// destructor
template < class T >
array_allocator< T >::~array_allocator() 
{
}

template < class T >
inline
T*
array_allocator< T >::allocate(size_t n)
{ 
	if (!n) 
		return 0; // nothing to do
	// loans from stack
	T* p = pop(n);
	if (!p)
	{
		size_t* ob = (size_t*)byte_allocator::allocate(n * ALIGNED_SIZEOF(sizeof(T)) + sizeof(size_t));
		if (!ob) return 0;
		*ob = n;
		p = (T*)(ob + 1);
	}
    
	return p;
}

template < class T >
inline
T*
array_allocator< T >::pop(size_t n)
{
	// looks for right size
	size_t* cur = _rep.top();
	size_t* prev = 0;
	while (cur && *(cur - 1) < n) // sets the next chunk in stack
	{
		prev = cur;
		cur = (size_t*)*cur;
	}

	if (!cur) 
		return 0; // nothing available in stack
	if (prev == 0) 
		return (T*)_rep.pop();// head is a right item
	// removes item from stak manually
	*prev = *cur;
	// returns result
	return (T*)cur;
}

//////////////////////////////////////////////////////////////
template < class T >
// static 
inline
node_allocator< T >* 
node_allocator_creator< T >::create(size_t size) 
{ 
	return new node_allocator< T >(size); 
}

template < class T >
// static 
inline
bool 
node_allocator_creator< T >::find(node_allocator< T >* obj, size_t size) 
{ 
	return obj->capacity() >= size; 
}

template < class T >
// static 
inline
void 
node_allocator_creator< T >::back(node_allocator< T >* obj, size_t) 
{ 
	obj->count() > 1 ? obj->clear() : obj->reset(); 
}

template < class T >
// static 
inline
void 
node_allocator_creator< T >::deactivate(node_allocator< T >* obj, size_t) 
{ 
	obj->clear(); 
}

////////////////////////////////////////////////////////////
template < class T >
// static
inline
array_allocator< T >*
array_allocator_creator< T >::create(size_t size) 
{ 
	return new array_allocator< T >(size); 
}

template < class T >
// static
inline
bool
array_allocator_creator< T >::find(array_allocator< T >* obj, size_t size) 
{ 
	return obj->capacity() >= size; 
}

template < class T >
// static
inline
void
array_allocator_creator< T >::back(array_allocator< T >* obj, size_t) 
{ 
	obj->count() > 1 ? obj->clear() : obj->reset(); 
}

template < class T >
// static
inline
void
array_allocator_creator< T >::deactivate(array_allocator< T >* obj, size_t) 
{ 
	obj->clear(); 
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_memory_hpp_



