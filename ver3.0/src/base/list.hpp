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

#ifndef _terimber_list_hpp_
#define _terimber_list_hpp_

#include "base/list.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// constructor
template < class T >
inline
base_list< T >::base_list() : 
	_head() 
{ 
	clear(); 
}

// destructor
template < class T >
inline
base_list< T >::~base_list() 
{
}

// checks empty, uses instead of size
template < class T >
inline 
bool 
base_list< T >::empty() const 
{ 
	return _head._prev == head(); 
}

// function doesn't check boundaries
template < class T >
inline 
const T&
base_list< T >::front() const 
{ 
	return _head._next->_value; 
}

// function doesn't check boundaries
template < class T >
inline 
T&
base_list< T >::front() 
{ 
	return _head._next->_value; 
}

// function doesn't check boundaries
template < class T >
inline 
const T&
base_list< T >::back() const 
{ 
	return _head._prev->_value; 
}

// function doesn't check boundaries
template < class T >
inline 
T&
base_list< T >::back() 
{ 
	return _head._prev->_value; 
}

template < class T >
inline 
TYPENAME base_list< T >::const_iterator
base_list< T >::begin() const 
{ 
	return const_iterator(_head._next); 
}

template < class T >
inline 
TYPENAME base_list< T >::iterator
base_list< T >::begin() 
{ 
	return iterator(_head._next); 
}

template < class T >
inline 
TYPENAME base_list< T >::const_iterator
base_list< T >::end() const 
{ 
	return const_iterator(head()); 
}

template < class T >
inline 
TYPENAME base_list< T >::iterator
base_list< T >::end() 
{ 
	return iterator(head()); 
}

template < class T >
inline 
void
base_list< T >::clear() 
{ 
	_head._prev = _head._next = head(); 
}

template < class T >
inline 
TYPENAME base_list< T >::_node*
base_list< T >::head() 
{ 
	return static_cast< _node* >(&_head); 
}

template < class T >
inline 
TYPENAME base_list< T >::_node*
base_list< T >::head() const 
{ 
	return const_cast< TYPENAME base_list< T >::_node* >(static_cast< const TYPENAME base_list< T >::_node* >(&_head)); 
}

/////////////////////////////////////////////////
// class list uses our allocator class
// for ALL internal memory allocations
// only 4 byte size for empty list object
// only forward iterators are supported

template < class T, class A >
inline
_list< T, A >::_list() : 
	base_list< T >() 
{
}

template < class T, class A >
inline
_list< T, A >::~_list() 
{ 
	this->clear(); 
}

template < class T, class A >
inline
_list< T, A >::_list(const _list< T, A >& x) : 
	base_list< T >() 
{ 
	*this = x; 
}

// copies only pointers to head and elements
// NO memory reallocation or copy
template < class T, class A >
inline
_list< T, A >&
_list< T, A >::operator=(const _list< T, A >& x) 
{ 
	if (this != &x)
	{
		base_list< T >::clear();
		if (!x.empty()) 
		{
			this->_head._next = x._head._next;
			this->_head._prev = x._head._prev;
			// replaces head pointer
			this->_head._prev->_next = this->_head._next->_prev = this->head();
		}
	}
	return *this; 
}

template < class T, class A >
inline
void
_list< T, A >::assign(A& allocator_, const _list< T, A >& x)
{ 
	assign(allocator_, x.begin(), x.end()); 
}

template < class T, class A >
inline
void
_list< T, A >::assign(A& allocator_, TYPENAME _list< T, A >::const_iterator first, TYPENAME _list< T, A >::const_iterator last)
{ 
	this->clear(); 
	for (;first != last; ++first) 
		push_back(allocator_, *first); 
}

template < class T, class A >
inline
void
_list< T, A >::assign(A& allocator_, size_t n, const T& x)
{ 
	this->clear(); 
	for (size_t I = 0; I < n; ++I) 
		push_back(allocator_, x); 
}

// expensive operation
template < class T, class A >
inline
size_t
_list< T, A >::size() const
{ 
	size_t count = 0; 
	for (TYPENAME _list< T, A >::const_iterator I = this->begin(); I != this->end(); ++I, ++count); 
	return count; 
}

template < class T, class A >
inline
TYPENAME _list< T, A >::iterator
_list< T, A >::push_back(A& allocator_, const T& x)
{  	
	// allocates new _node 
	TYPENAME _list< T, A >::_node* n = _buynode(allocator_, x);
	if (!n)
		return this->end();

	n->_prev = this->_head._prev;
	n->_next = this->head();
	this->_head._prev->_next = n;
	this->_head._prev = n;
	return TYPENAME _list< T, A >::iterator(n);
}		

template < class T, class A >
inline
TYPENAME _list< T, A >::iterator
_list< T, A >::push_front(A& allocator_, const T& x)
{  	
	// allocates new _node
	TYPENAME _list< T, A >::_node* n = _buynode(allocator_, x);
	if (!n)
		return this->end();
	n->_prev = this->head();
	n->_next = this->_head._next;
	this->_head._next->_prev = n;
	this->_head._next = n;
	return TYPENAME _list< T, A >::iterator(n);
}

// removes first element from list
// no memory deallocation
template < class T, class A >
inline
bool
_list< T, A >::pop_front()
{  	
	if (this->empty()) 
		return false;
	this->_head._next->_next->_prev = this->head();
	this->_head._next = this->_head._next->_next;
	return true;
}

// removes first element from list
template < class T, class A >
inline
bool
_list< T, A >::pop_front(node_allocator< TYPENAME base_list< T >::_node >& allocator_)
{  	
	if (this->empty()) 
		return false;

	TYPENAME _list< T, A >::_node* f = this->_head._next;
	this->_head._next->_next->_prev = this->head();
	this->_head._next = this->_head._next->_next;
	_freenode(allocator_, f);
	return true;
}

// removes last element from list
// no memory deallocation
template < class T, class A >
inline
bool
_list< T, A >::pop_back()
{  				
	if (this->empty()) 
		return false;
	this->_head._prev->_prev->_next = this->head();
	this->_head._prev = this->_head._prev->_prev;
	return true;
}

template < class T, class A >
inline
bool
_list< T, A >::pop_back(node_allocator< TYPENAME base_list< T >::_node >& allocator_)
{  				
	if (this->empty()) 
		return false;

	TYPENAME _list< T, A >::_node* f = this->_head._prev;
	this->_head._prev->_prev->_next = this->head();
	this->_head._prev = this->_head._prev->_prev;
	_freenode(allocator_, f);

	return true;
}

template < class T, class A >
inline
TYPENAME _list< T, A >::iterator
_list< T, A >::insert(A& allocator_, TYPENAME _list< T, A >::iterator it, const T& x)
{
	TYPENAME _list< T, A >::_node* n = _buynode(allocator_, x);
	if (!n)
		return this->end();
	TYPENAME _list< T, A >::_node* i = it.node();

	n->_next = i;
	n->_prev = i->_prev;
	i->_prev = n;
	i = i->_prev;
	i->_prev->_next = i;

	return TYPENAME _list< T, A >::iterator(n);
}

template < class T, class A >
inline
void
_list< T, A >::insert(A& allocator_, TYPENAME _list< T, A >::iterator it, size_t n, const T& x)
{ 
	for (size_t I = 0; I < n; ++I) 
		it = insert(allocator_, it, x); 
}

template < class T, class A >
inline
void
_list< T, A >::insert(A& allocator_, TYPENAME _list< T, A >::iterator it, TYPENAME _list< T, A >::const_iterator first, TYPENAME _list< T, A >::const_iterator last)
{ 
	for (first; first != last; ++first) 
		it = insert(allocator_, it, *first); 
}

// removes the specified element of list
// must be defined compare operator for stored object
template < class T, class A >
inline
void
_list< T, A >::remove(const T& x)
{ 
	for (TYPENAME _list< T, A >::iterator I = this->begin(); I != this->end();) 
		if (*I == x) 
			I = remove(I); 
		else
			++I;
}

template < class T, class A >
inline
void
_list< T, A >::remove(node_allocator< TYPENAME base_list< T >::_node >& allocator_, const T& x)
{ 
	for (TYPENAME _list< T, A >::iterator I = this->begin(); I != this->end();) 
		if (*I == x) 
			I = remove(allocator_, I); 
		else
			++I;
}

// clears list
// no memory deallocation
template < class T, class A >
inline
TYPENAME _list< T, A >::iterator
_list< T, A >::erase(TYPENAME _list< T, A >::iterator iter) 
{ 
	return remove(iter); 
}

template < class T, class A >
inline
TYPENAME _list< T, A >::iterator
_list< T, A >::erase(node_allocator< TYPENAME base_list< T >::_node >& allocator_, TYPENAME _list< T, A >::iterator iter) 
{ 
	return remove(allocator_, iter); 
}

template < class T, class A >
inline
TYPENAME _list< T, A >::iterator
_list< T, A >::erase(TYPENAME _list< T, A >::iterator first, TYPENAME _list< T, A >::iterator last) 
{ 
	for (;first != last;) 
		first = remove(first); 
	return first;
}

template < class T, class A >
inline
TYPENAME _list< T, A >::iterator
_list< T, A >::erase(node_allocator< TYPENAME base_list< T >::_node >& allocator_, TYPENAME _list< T, A >::iterator first, TYPENAME _list< T, A >::iterator last) 
{ 
	for (;first != last;) 
		first = remove(allocator_, first); 
	return first;
}

template < class T, class A >
inline
TYPENAME _list< T, A >::iterator
_list< T, A >::remove(TYPENAME _list< T, A >::iterator iter)
{  	
	TYPENAME _list< T, A >::iterator ret = iter;
	++ret;
	iter.prev()->_next = iter.next();
	iter.next()->_prev = iter.prev();
	return ret;
}

template < class T, class A >
inline
TYPENAME _list< T, A >::iterator
_list< T, A >::remove(node_allocator< TYPENAME base_list< T >::_node >& allocator_, TYPENAME _list< T, A >::iterator iter)
{  	
	TYPENAME _list< T >::iterator ret = iter;
	++ret;
	iter.prev()->_next = iter.next();
	iter.next()->_prev = iter.prev();
	_freenode(allocator_, iter.node());
	return ret;
}

template < class T, class A >
inline
TYPENAME _list< T, A >::_node*
_list< T, A >::_buynode(byte_allocator& allocator_, const T& x) 
{ 
	void* ptr = allocator_.allocate(sizeof(TYPENAME _list< T, A >::_node));
	if (!ptr)	
		return 0;
	return new (ptr) TYPENAME _list< T, A >::_node(x); 
}

template < class T, class A >
inline
TYPENAME _list< T, A >::_node*
_list< T, A >::_buynode(node_allocator< TYPENAME base_list< T >::_node >& allocator_, const T& x) 
{ 
	void* ptr = allocator_.allocate();
	if (!ptr)	
		return 0;
	return new (ptr) TYPENAME _list< T, A >::_node(x); 
}

template < class T, class A >
inline 
void 
_list< T, A >::_freenode(node_allocator< TYPENAME base_list< T >::_node >& allocator_, TYPENAME _list< T, A >::_node* p)
{
	allocator_.deallocate(p);
}

/////////////////////////////////////////////////
// class list uses our allocator class
// for ALL internal memory allocations
// forward and backward iterators are supported
template < class T >
inline
list< T >::list(size_t size) : 
	base_list< T >(), _length(0), _allocator(size) 
{
}

// destructor
template < class T >
inline
list< T >::~list() 
{ 
	clear(); 
}

// copy constructor
template < class T >
inline
list< T >::list(const list< T >& x) : 
	base_list< T >(), _length(0), _allocator(x._allocator.capacity()) 
{ 
	*this = x; 
}

// copies all objects
template < class T >
inline
list< T >&
list< T >::operator=(const list< T >& x)
{
	if (this != &x)
	{
		clear();
		for (TYPENAME list< T >::const_iterator iter = x.begin(); iter != x.end(); ++iter)
			push_back(*iter);			
	}

	return *this;
}

// clears vector
// calls destructors
template < class T >
inline
void
list< T >::clear()
{ 
	for (TYPENAME list< T >::iterator I = this->begin(); I != this->end();)
	{
		TYPENAME list< T >::_node* f = I.node();
		++I;
		_freenode(f);
	}

	base_list< T >::clear();
	_allocator.clear_all();
	_length = 0;
}

template < class T >
inline
void
list< T >::assign(const list< T >& x)
{ 
	assign(x.begin(), x.end()); 
}

template < class T >
inline
void
list< T >::assign(TYPENAME list< T >::const_iterator first, TYPENAME list< T >::const_iterator last)
{ 
	clear(); 
	for (;first != last; ++first) 
		push_back(*first); 
}

template < class T >
inline
void
list< T >::assign(size_t n, const T& x)
{ 
	clear(); 
	for (size_t I = 0; I < n; ++I) 
		push_back(x); 
}

template < class T >
inline
size_t
list< T >::size() const 
{ 
	return _length; 
}

// inserts new object to the end of list
template < class T >
inline
TYPENAME 
list< T >::iterator
list< T >::push_back(const T& x)
{  	
	// allocates new _node 
	TYPENAME list< T >::_node* n = _buynode(x);
	if (!n)
		return this->end();

	n->_prev = this->_head._prev;
	n->_next = this->head();
	this->_head._prev->_next = n;
	this->_head._prev = n;
	// increment length
	++_length;
	// returns result
	return TYPENAME list< T >::iterator(n);
}	

// inserts new object to the front of the list
template < class T >
inline
TYPENAME 
list< T >::iterator
list< T >::push_front(const T& x)
{  	
	// allocates new _node
	TYPENAME list< T >::_node* n = _buynode(x);
	if (!n)
		return this->end();
	n->_prev = this->head();
	n->_next = this->_head._next;
	this->_head._next->_prev = n;
	this->_head._next = n;
	// increment length
	++_length;
	// returns result
	return TYPENAME list< T >::iterator(n);
}

// removes first element from list
// no memory deallocation
template < class T >
inline
bool
list< T >::pop_front()
{  	
	if (!_length) 
		return false;
	TYPENAME list< T >::_node* f = this->_head._next;
	this->_head._next->_next->_prev = this->head();
	this->_head._next = this->_head._next->_next;
	// decrement
	--_length;
	_freenode(f);
	return true;
}

// removes last element from list
// no memory deallocation
template < class T >
inline
bool
list< T >::pop_back()
{  				
	if (!_length) 
		return false;
	TYPENAME list< T >::_node* f = this->_head._prev;
	this->_head._prev->_prev->_next = this->head();
	this->_head._prev = this->_head._prev->_prev;
	--_length;
	_freenode(f);
	return true;
}

template < class T >
inline
TYPENAME list< T >::iterator
list< T >::insert(TYPENAME list< T >::iterator it, const T& x)
{
	TYPENAME list< T >::_node* n = _buynode(x);
	if (!n)
		return this->end();
	TYPENAME list< T >::_node* i = it.node();

	n->_next = i;
	n->_prev = i->_prev;
	i->_prev = n;
	i = i->_prev;
	i->_prev->_next = i;

	// increment length
	++_length;
	return TYPENAME list< T >::iterator(n);
}

template < class T >
inline
void
list< T >::insert(TYPENAME list< T >::iterator it, size_t n, const T& x)
{ 
	for (size_t I = 0; I < n; ++I) 
		it = insert(it, x); 
}

template < class T >
inline
void
list< T >::insert(TYPENAME list< T >::iterator it, TYPENAME list< T >::const_iterator first, TYPENAME list< T >::const_iterator last)
{ 
	for (first; first != last; ++first) 
		it = insert(it, *first); 
}

// removes the specified element of list
// must be defined compare operator for stored object
template < class T >
inline
void
list< T >::remove(const T& x)
{ 
	for (TYPENAME list< T >::iterator I = this->begin(); I != this->end();) 
		if (*I == x) 
			I = remove(I); 
		else
			++I;
}

// clears list
// no memory deallocation
template < class T >
inline
TYPENAME list< T >::iterator
list< T >::erase(TYPENAME list< T >::iterator iter) 
{ 
	return remove(iter); 
}

template < class T >
inline
TYPENAME list< T >::iterator
list< T >::erase(TYPENAME list< T >::iterator first, TYPENAME list< T >::iterator last) 
{ 
	for (;first != last;) 
		first = remove(first); 

	return first;
}

template < class T >
inline
TYPENAME list< T >::iterator
list< T >::remove(TYPENAME list< T >::iterator iter)
{  	
	iter.prev()->_next = iter.next();
	iter.next()->_prev = iter.prev();
	TYPENAME list< T >::iterator ret = iter;
	++ret;
	_freenode(iter.node());
	--_length;
	return ret;
}

template < class T >
inline
TYPENAME list< T >::_node*
list< T >::_buynode(const T& x) 
{ 
	void* ptr = _allocator.allocate();
	if (!ptr)
		return 0;
	return new (ptr) TYPENAME list< T >::_node(x); 
}

template < class T >
inline
void
list< T >::_freenode(TYPENAME list< T >::_node* p) 
{ 
	p->_value.~T(); 
	_allocator.deallocate(p); 
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_list_hpp_

