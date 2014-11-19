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

#ifndef _terimber_stack_hpp_
#define _terimber_stack_hpp_

#include "base/stack.h"
#include "base/common.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// class provides the stack functionality
//
// constructor
//
template< class T >
base_stack< T >::base_stack() 
	: _head() 
{
	clear();
}

//
// destructor
//
template< class T >
base_stack< T >::~base_stack() 
{
}

//
// checks empty, uses instead of size
//
template< class T >
inline
bool 
base_stack< T >::empty() const 
{ 
	return _head._next == head();  
}

//
// function doesn't check boundaries
//
template< class T >
inline
const T& 
base_stack< T >::top() const 
{ 
	return _head._next->_value; 
}

//
// function doesn't check boundaries
//
template< class T >
inline
T& 
base_stack< T >::top() 
{ 
	return _head._next->_value; 
}

template< class T >
inline
TYPENAME base_stack< T >::const_iterator 
base_stack< T >::begin() const 
{ 
	return const_iterator(_head._next); 
}

template< class T >
inline
TYPENAME base_stack< T >::iterator 
base_stack< T >::begin() 
{ 
	return iterator(_head._next); 
}

template< class T >
inline
TYPENAME base_stack< T >::const_iterator 
base_stack< T >::end() const 
{ 
	return const_iterator(head()); 
}

template< class T >
inline
TYPENAME base_stack< T >::iterator 
base_stack< T >::end() 
{ 
	return iterator(head()); 
}

template < class T >
inline 
void
base_stack< T >::clear() 
{ 
	_head._next = head(); 
}

template < class T >
inline 
TYPENAME base_stack< T >::_node*
base_stack< T >::head() 
{ 
	return static_cast< TYPENAME base_stack< T >::_node* >(&_head); 
}

template < class T >
inline 
TYPENAME base_stack< T >::_node*
base_stack< T >::head() const 
{ 
	return const_cast< TYPENAME base_stack< T >::_node* >(static_cast< const TYPENAME base_stack< T >::_node* >(&_head)); 
}
//////////////////////////////////////////////////
//
// constructor
//
template< class T, class A >
_stack< T, A >::_stack() 
	: base_stack< T >() 
{
}

//
// destructor
//
template< class T, class A >
_stack< T, A >::~_stack() 
{ 
	base_stack< T >::clear(); 
}

//
// copy constructor
// simple assignment
//
template< class T, class A >
_stack< T, A >::_stack(const _stack< T, A >& x) : 
	base_stack< T >(x) 
{ 
	*this = x; 
}

template< class T, class A >
inline
_stack< T, A >& 
_stack< T, A >::operator=(const _stack< T, A >& x) 
{ 
	if (this != &x)
	{
		base_stack< T >::clear();
		if (!x.empty()) 
			this->_head._next = x._head._next;
	}
	return *this; 
}

// expensive operation
template < class T, class A >
inline
size_t
_stack< T, A >::size()
{ 
	size_t count = 0; 
	for (TYPENAME _stack< T, A >::const_iterator I = this->begin(); I != this->end(); ++I, ++count); 
	return count; 
}

//
// copies only pointers to head element
// NO memory reallocation or copy
//
template< class T, class A >
inline
TYPENAME
_stack< T, A >::iterator
_stack< T, A >::push(A& allocator_, const T& x)
{  	
	// allocates new _node
	TYPENAME _stack< T, A >::_node* n = _buynode(allocator_, x);
	if (!n)
		return this->end();
	n->_next = this->_head._next;
	this->_head._next = n;
	return TYPENAME _stack< T, A >::iterator(n);
}

//
// removes first element from stack
// no memory deallocation
//
template< class T, class A >
inline
bool
_stack< T, A >::pop()
{  	
	if (base_stack< T >::empty()) 
		return false;
	
	this->_head._next = this->_head._next->_next;
	return true;
}

template< class T, class A >
inline
bool
_stack< T, A >::pop(node_allocator< TYPENAME base_stack< T >::_node >& allocator_)
{  	
	if (this->empty()) 
		return false;
	
	TYPENAME _stack< T, A >::_node* f = this->_head._next;
	this->_head._next = this->_head._next->_next;
	_freenode(allocator_, f);

	return true;
}

template< class T, class A >
inline
TYPENAME _stack< T, A >::_node* 
_stack< T, A >::_buynode(byte_allocator& allocator_, const T& x)
{ 
	void* ptr = allocator_.allocate(sizeof(TYPENAME _stack< T, A >::_node));
	if (!ptr)
		return 0;

	return new (ptr) TYPENAME _stack< T, A >::_node(x); 
}

template< class T, class A >
inline
TYPENAME _stack< T, A >::_node* 
_stack< T, A >::_buynode(node_allocator< TYPENAME base_stack< T >::_node >& allocator_, const T& x) 
{ 
	void* ptr = allocator_.allocate();
	if (!ptr)
		return 0;
	return new (ptr) TYPENAME _stack< T, A >::_node(x); 
}

template< class T, class A >
inline 
void 
_stack< T, A >::_freenode(node_allocator< TYPENAME base_stack< T >::_node >& allocator_, TYPENAME _stack< T, A >::_node* p)
{
	allocator_.deallocate(p);
}


///////////////////////////////////////////////////
//
// constructor
//
template< class T >
stack< T >::stack(size_t size) 
	: base_stack< T >(), _length(0), _allocator(size) 
{
}

//
// destructor
//
template< class T >
stack< T >::~stack()
{ 
	clear(); 
}

//
// copy constructor
//
template< class T >
stack< T >::stack(const stack< T >& x) 
	: base_stack< T >(x), _length(0), _allocator(x._allocator.capacity())
{ 
	*this = x; 
}

//
// assign operator
//
template< class T >
inline
stack< T >& 
stack< T >::operator=(const stack< T >& x)
{
	if (this != &x)
	{
		clear();
		// copies only pointers in reverse order
		stack< const T* > tmp(x._allocator.capacity());
        for (TYPENAME stack< T >::const_iterator iter = x.begin(); iter != x.end(); ++iter) tmp.push(&*iter);
        for (TYPENAME stack< const T* >::const_iterator iter_ = tmp.begin(); iter_ != tmp.end(); ++iter_) push(**iter_);
	}

	return *this;
}

//
// returns length
//
template< class T >
inline 
size_t 
stack< T >::size() const 
{ 
	return _length; 
}

//
// inserts new object to the front of stack
//
template< class T >
inline 
TYPENAME
stack< T >::iterator
stack< T >::push(const T& x)
{  	
	// allocates new _node
	TYPENAME stack< T >::_node* n = _buynode(x);
	if (!n)
		return this->end();
	n->_next = this->_head._next;
	this->_head._next = n;
	// increment length
	++_length;
	// returns result
	return TYPENAME stack< T >::iterator(n);
}

//
// removes top element from stack
//
template< class T >
inline
bool 
stack< T >::pop()
{  	
	if (!_length) 
		return false;
	
	TYPENAME stack< T >::_node* f = this->_head._next;
	this->_head._next = this->_head._next->_next;
	// decrement
	--_length;
	_freenode(f);
	return true;
}

// clears stack
// calls only destructors
template< class T >
inline 
void 
stack< T >::clear()
{ 
	for (TYPENAME stack< T >::iterator I = this->begin(); I != this->end();)
	{
		TYPENAME stack< T >::_node* f = I.node();
		++I;
		_freenode(f);
	}

	base_stack< T >::clear();
	_allocator.clear_all();
	_length = 0;
}

template< class T >
inline
TYPENAME stack< T >::_node* 
stack< T >::_buynode(const T& x) 
{ 
	void* ptr = _allocator.allocate();
	if (!ptr)
		return 0;
	return new (ptr) TYPENAME stack< T >::_node(x); 
}

template< class T >
inline
void 
stack< T >::_freenode(TYPENAME stack< T >::_node* p) 
{ 
	p->_value.~T(); 
	_allocator.deallocate(p); 
}
////////////////////////////////////////////////////////////
//
// constructor
//
inline
unique_key_generator::unique_key_generator(size_t capacity) : _last(0), _rep(capacity) 
{
}

//
// destructor
//
inline
unique_key_generator::~unique_key_generator() 
{ 
	clear(); 
}

//
// clears all internal resources
//
inline 
void 
unique_key_generator::clear() 
{ 
	_rep.clear(); 
	_last = 0; 
}

//
// generates the new unique key or takes it from repository
//
inline 
size_t 
unique_key_generator::generate() 
{ 
	if (_rep.empty())
		return ++_last;
	else
	{
		size_t retVal = _rep.top();
		_rep.pop(); 
		return retVal;
	}
}

//
// saves the used key and stores it in repository
//
inline 
void 
unique_key_generator::save(size_t key) 
{ 
	_rep.push(key); 
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_stack_hpp_

