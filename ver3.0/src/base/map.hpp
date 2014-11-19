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

#ifndef _terimber_map_hpp_
#define _terimber_map_hpp_

#include "base/map.h"
#include "base/common.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

template< class T >
inline
bool
less< T >::operator()(const T& first, const T& second) const
{ 
	return first < second; 
}

////////////////////////////////////////////////////////
template < class T1, class T2 > 
pair< T1, T2 >::pair() :
	first(T1()), second(T2())
{
}

template < class T1, class T2 > 
pair< T1, T2 >::pair(const T1& v1, const T2& v2) : 
	first(v1), second(v2)
{
}

template < class T1, class T2 > 
pair< T1, T2 >::pair(const pair< T1, T2 >& x) : 
	first(x.first), second(x.second)
{
}

template < class T1, class T2 > 
inline
bool
pair< T1, T2 >::operator==(const pair< T1, T2 >& x) const
{ 
	return first == x.first && second == x.second; 
}

template < class T1, class T2 > 
inline 
bool 
pair< T1, T2 >::operator<(const pair< T1, T2 >& x) const
{ 
	return first != x.first ? first < x.first : second < x.second; 
}

template < class T1, class T2 > 
inline 
bool 
pair< T1, T2 >::operator!=(const pair< T1, T2 >& x) const
{ 
	return !(*this == x); 
}

////////////////////////////////////////////////////////////
template < class K, class T, class Pr, bool M >
//explicit 
base_map< K, T, Pr, M >::base_map(const Pr& pr) : 
	key_compare(pr), _head(), _size(0) 
{
	clear();
}

template < class K, class T, class Pr, bool M >
base_map< K, T, Pr, M >::~base_map()	
{
}

template < class K, class T, class Pr, bool M >
base_map< K, T, Pr, M >::base_map(const base_map< K, T, Pr, M >& x) : 
	key_compare(x.key_compare), _head(), _size(x._size) 
{ 
	clear(); 
}

template < class K, class T, class Pr, bool M >
inline 
const Pr&
base_map< K, T, Pr, M >::comp() const
{ 
	return key_compare; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::iterator
base_map< K, T, Pr, M >::begin() 
{ 
	return iterator(_head._left); 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::const_iterator
base_map< K, T, Pr, M >::begin() const 
{ 
	return const_iterator(head()->_left); 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::iterator
base_map< K, T, Pr, M >::end() 
{ 
	return iterator(head()); 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::const_iterator
base_map< K, T, Pr, M >::end() const 
{ 
	return const_iterator(head()); 
}

template < class K, class T, class Pr, bool M >
inline 
size_t
base_map< K, T, Pr, M >::size() const 
{ 
	return _size; 
}

template < class K, class T, class Pr, bool M >
inline 
bool
base_map< K, T, Pr, M >::empty() const 
{ 
	return !_size; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::iterator
base_map< K, T, Pr, M >::find(const K& k)
{
	TYPENAME base_map< K, T, Pr, M >::iterator p = lower_bound(k);
	return p == end() || key_compare(k, p.key()) ? end() : p; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::const_iterator
base_map< K, T, Pr, M >::find(const K& k) const
{
	TYPENAME base_map< K, T, Pr, M >::const_iterator p = lower_bound(k);
	return p == end() || key_compare(k, p.key()) ? end() : p; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::iterator
base_map< K, T, Pr, M >::lower_bound(const K& k) 
{ 
	return iterator(_lbound(k)); 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::const_iterator
base_map< K, T, Pr, M >::lower_bound(const K& k) const 
{ 
	return const_iterator(_lbound(k)); 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::iterator
base_map< K, T, Pr, M >::upper_bound(const K& k) 
{ 
	return iterator(_ubound(k)); 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::const_iterator
base_map< K, T, Pr, M >::upper_bound(const K& k) const 
{ 
	return iterator(_ubound(k)); 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::pairii_t
base_map< K, T, Pr, M >::equal_range(const K& k) 
{ 
	return TYPENAME base_map< K, T, Pr, M >::pairii_t(lower_bound(k), upper_bound(k)); 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::paircc_t
base_map< K, T, Pr, M >::equal_range(const K& k) const 
{ 
	return TYPENAME base_map< K, T, Pr, M >::paircc_t(lower_bound(k), upper_bound(k)); 
}

template < class K, class T, class Pr, bool M >
inline 
void
base_map< K, T, Pr, M >::clear()
{ 
	_size = 0;
	_head._color = c_black;
	_head._type = t_head;
	_head._parent = _head._left = _head._right = head();
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::iterator
base_map< K, T, Pr, M >::_insert(bool left, TYPENAME base_map< K, T, Pr, M >::_node* w, TYPENAME base_map< K, T, Pr, M >::_node* n, const K& k, const T& v)
{
	n->_left = n->_right = head();
	_consval(&n->_value, v);
	_conskey(&n->_key, k);

	++_size;
	if (w == head())
	{
		_head._parent = n;
		_head._left = n;
		_head._right = n;
	}
	else if (left)
	{
		w->_left = n;
		if (w == _head._left)
			_head._left = n;
	}
	else
	{
		w->_right = n;
		if (w == _head._right)
			_head._right = n;
	}

	for (_node* p = n; p->_parent->_color == c_red; )
		if (p->_parent == p->_parent->_parent->_left)
		{
			w = p->_parent->_parent->_right;
			if (w->_color == c_red)
			{
				p->_parent->_color = c_black;
				w->_color = c_black;
				p->_parent->_parent->_color = c_red;
				p = p->_parent->_parent;
			}
			else
			{
				if (p == p->_parent->_right)
				{
					p = p->_parent;
					_lrotate(p);
				}
				p->_parent->_color = c_black;
				p->_parent->_parent->_color = c_red;
				_rrotate(p->_parent->_parent);
			}
		}
		else
		{
			w = p->_parent->_parent->_left;
			if (w->_color == c_red)
			{
				p->_parent->_color = c_black;
				w->_color = c_black;
				p->_parent->_parent->_color = c_red;
				p = p->_parent->_parent;
			}
			else
			{
				if (p == p->_parent->_left)
				{
					p = p->_parent;
					_rrotate(p);
				}
				p->_parent->_color = c_black;
				p->_parent->_parent->_color = c_red;
				_lrotate(p->_parent->_parent);
			}
		}

	_head._parent->_color = c_black;
	return TYPENAME base_map< K, T, Pr, M >::iterator(n);
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::iterator
base_map< K, T, Pr, M >::_erase(TYPENAME base_map< K, T, Pr, M >::iterator w, TYPENAME base_map< K, T, Pr, M >::_node*& e)
{
	if (!_size)
		return this->end();

	TYPENAME base_map< K, T, Pr, M >::_node* x, *px = 0;
	TYPENAME base_map< K, T, Pr, M >::_node* p = e = (w++).node();
	if (p->_left->_type == t_head)
		x = p->_right;
	else if (p->_right->_type == t_head)
		x = p->_left;
	else
		p = w.node(), x = p->_right;

	if (p == e)
	{
		px = e->_parent;
		if (x->_type != t_head)
			x->_parent = px;

		if (_head._parent == e)
			_head._parent = x;
		else if (px->_left == e)
			px->_left = x;
		else
			px->_right = x;

		if (_head._left == e)
			_head._left = (x->_type == t_head ? px : _node::_min(x));

		if (_head._right == e)
			_head._right = (x->_type == t_head ? px : _node::_max(x)); 
	}
	else
	{
		e->_left->_parent = p;
		p->_left = e->_left;
		if (p == e->_right)
			px = p;
		else
		{
			px = p->_parent;
			if (x->_type != t_head)
				x->_parent = px;
            px->_left = x;
			p->_right = e->_right;
			e->_right->_parent = p; 
		}

		if (_head._parent == e)
			_head._parent = p;
		else if (e->_parent->_left == e)
			e->_parent->_left = p;
		else
			e->_parent->_right = p;
        p->_parent = e->_parent;
		redblack c = p->_color;
		p->_color = e->_color;
		e->_color = c;
	}

	if (e->_color == c_black)
	{	
		for (;x != _head._parent && x->_color == c_black; px = x->_parent)
			if (x == px->_left)
			{
				p = px->_right;
				if (p->_color == c_red)
				{
					p->_color = c_black;
					px->_color = c_red;
					_lrotate(px);
					p = px->_right; 
				}
				if (p->_left->_color == c_black && p->_right->_color == c_black)
				{
					p->_color = c_red;
					x = px; 
				}
				else
                {
					if (p->_right->_color == c_black)
					{
						p->_left->_color = c_black;
						p->_color = c_red;
						_rrotate(p);
						p = px->_right; 
					}
					p->_color = px->_color;
					px->_color = c_black;
					p->_right->_color = c_black;
					_lrotate(px);
					break; 
				}
			}
			else
			{
				p = px->_left;
				if (p->_color == c_red)
				{
					p->_color = c_black;
					px->_color = c_red;
					_rrotate(px);
					p = px->_left; 
				}
				if (p->_right->_color == c_black && p->_left->_color == c_black)
				{
					p->_color = c_red;
					x = px; 
				}
				else
				{
					if (p->_left->_color == c_black)
					{
						p->_right->_color = c_black;
						p->_color = c_red;
						_lrotate(p);
						p = px->_left; 
					}
					p->_color = px->_color;
					px->_color = c_black;
					p->_left->_color = c_black;
					_rrotate(px);
					break; 
				}
			}
			x->_color = c_black; 
	}
	
	--_size;
	return w; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*
base_map< K, T, Pr, M >::_lbound(const K& k) const
{
	TYPENAME base_map< K, T, Pr, M >::_node* p = _head._parent;
	TYPENAME base_map< K, T, Pr, M >::_node* r = head();
	while (p->_type != t_head)
	if (key_compare(p->_key, k))
		p = p->_right;
	else
		r = p, p = p->_left;
	return r; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*
base_map< K, T, Pr, M >::_ubound(const K& k) const
{
	TYPENAME base_map< K, T, Pr, M >::_node* p = _head._parent;
	TYPENAME base_map< K, T, Pr, M >::_node* r = head();
	while (p->_type != t_head)
		if (key_compare(k, p->_key))
			r = p, p = p->_left;
		else
			p = p->_right;
	return r; 
}
 
template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*&
base_map< K, T, Pr, M >::lmost() 
{ 
	return _head._left; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*&
base_map< K, T, Pr, M >::lmost() const 
{ 
	return head()->_left; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*&
base_map< K, T, Pr, M >::rmost()
{ 
	return _head._right; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*&
base_map< K, T, Pr, M >::rmost() const 
{ 
	return head()->_right; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*&
base_map< K, T, Pr, M >::root() 
{ 
	return _head._parent; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*&
base_map< K, T, Pr, M >::root() const 
{ 
	return head()->_parent; 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*
base_map< K, T, Pr, M >::head() 
{ 
	return static_cast< TYPENAME base_map< K, T, Pr, M >::_node* >(&_head); 
}

template < class K, class T, class Pr, bool M >
inline 
TYPENAME base_map< K, T, Pr, M >::_node*
base_map< K, T, Pr, M >::head() const 
{ 
	return const_cast< TYPENAME base_map< K, T, Pr, M >::_node* >(static_cast< const TYPENAME base_map< K, T, Pr, M >::_node* >(&_head)); 
}

template < class K, class T, class Pr, bool M >
inline 
void
base_map< K, T, Pr, M >::_lrotate(TYPENAME base_map< K, T, Pr, M >::_node* w)
{
	TYPENAME base_map< K, T, Pr, M >::_node* p = w->_right;
	w->_right = p->_left;
	if (p->_left->_type != t_head)
		p->_left->_parent = w;
	p->_parent = w->_parent;
	if (w == _head._parent)
		_head._parent = p;
	else if (w == w->_parent->_left)
		w->_parent->_left = p;
	else
		w->_parent->_right = p;
	p->_left = w;
	w->_parent = p; 
}

template < class K, class T, class Pr, bool M >
inline 
void
base_map< K, T, Pr, M >::_rrotate(TYPENAME base_map< K, T, Pr, M >::_node* w)
{
	TYPENAME base_map< K, T, Pr, M >::_node* p = w->_left;
	w->_left = p->_right;
	if (p->_right->_type != t_head)
		p->_right->_parent = w;
	p->_parent = w->_parent;
	if (w == _head._parent)
		_head._parent = p;
	else if (w == w->_parent->_right)
		w->_parent->_right = p;
	else
		w->_parent->_left = p;
	p->_right = w;
	w->_parent = p; 
}

template < class K, class T, class Pr, bool M >
inline 
void
base_map< K, T, Pr, M >::_consval(T* p, const T& v) 
{ 
	new(p) T(v); 
}

template < class K, class T, class Pr, bool M >
inline 
void
base_map< K, T, Pr, M >::_conskey(K* p, const K& k) 
{ 
	new(p) K(k); 
}

template < class K, class T, class Pr, bool M >
inline 
void
base_map< K, T, Pr, M >::_destval(T* p) 
{ 
	p->~T(); 
}

template < class K, class T, class Pr, bool M >
inline 
void
base_map< K, T, Pr, M >::_destkey(K* p) 
{ 
	p->~K(); 
}

//////////////////////////////////////////////////////////////
template < class K, class T, class A, class Pr, bool M >
_map< K, T, A, Pr, M >::_map(const Pr& pr) : 
	base_map<K, T, Pr, M>(pr) 
{ 
	base_map< K, T, Pr, M >::clear(); 
}

template < class K, class T, class A, class Pr, bool M >
_map< K, T, A, Pr, M >::~_map() 
{
} 

template < class K, class T, class A, class Pr, bool M >
_map< K, T, A, Pr, M >::_map(const _map< K, T, A, Pr, M >& x) : 
	base_map<K, T, Pr, M>(x) 
{
}

template < class K, class T, class A, class Pr, bool M >
inline
void
_map< K, T, A, Pr, M >::assign(A& allocator_, const _map< K, T, A, Pr, M >& x)
{ 
	base_map< K, T, Pr, M >::clear(); 
	for (TYPENAME _map< K, T, A, Pr, M >::const_iterator I = x.begin(); I != x.end(); ++I) 
		insert(allocator_, I.key(), *I); 
}

/*
template < class K, class T, class A, class Pr, bool M >
inline
TYPENAME _map< K, T, A, Pr, M >::iterator
_map< K, T, A, Pr, M >::insert(A& allocator_, const K& k, const T& v)
{ 
	TYPENAME _map< K, T, A, Pr, M >::_node* x = this->_head._parent;
	TYPENAME _map< K, T, A, Pr, M >::_node* w = this->head();

	bool left = true;
	while (x->_type != t_head)
	{
		w = x;
		left = key_compare(k, x->_key);
		x = left ? x->_left : x->_right; 
	}

	if (M) return _insert(allocator_, left, w, k, v);
	
	TYPENAME base_map< K, T, Pr, M >::iterator p =  TYPENAME base_map< K, T, Pr, M >::iterator(w);

	if (!left) 
		;
	else if (p == this->begin())
		return _insert(allocator_, left, w, k, v);
	else
		--p;

    if (key_compare(p.key(), k))
		return _insert(allocator_, left, w, k, v);
	
	return p; 
}
*/
template < class K, class T, class A, class Pr, bool M >
inline
TYPENAME 
_map< K, T, A, Pr, M >::pairib_t
_map< K, T, A, Pr, M >::insert(A& allocator_, const K& k, const T& v)
{ 
	TYPENAME _map< K, T, A, Pr, M >::_node* x = this->_head._parent;
	TYPENAME _map< K, T, A, Pr, M >::_node* w = this->head();

	bool left = true;
	while (x->_type != t_head)
	{
		w = x;
		left = key_compare(k, x->_key);
		x = left ? x->_left : x->_right; 
	}

	if (M) return TYPENAME _map< K, T, A, Pr, M >::pairib_t(_insert(allocator_, left, w, k, v), true);
	
	TYPENAME base_map< K, T, Pr, M >::iterator p =  TYPENAME base_map< K, T, Pr, M >::iterator(w);

	if (!left) 
		;
	else if (p == this->begin())
		return TYPENAME _map< K, T, A, Pr, M >::pairib_t(_insert(allocator_, left, w, k, v), true);
	else
		--p;

    if (key_compare(p.key(), k))
		return TYPENAME _map< K, T, A, Pr, M >::pairib_t(_insert(allocator_, left, w, k, v), true);
	
	return TYPENAME _map< K, T, A, Pr, M >::pairib_t(p, false); 
}

template < class K, class T, class A, class Pr, bool M >
inline
TYPENAME 
_map< K, T, A, Pr, M >::iterator
_map< K, T, A, Pr, M >::erase(TYPENAME _map< K, T, A, Pr, M >::iterator p)
{
	TYPENAME _map< K, T, A, Pr, M >::_node* e = 0;
	return base_map<K, T, Pr, M>::_erase(p, e);
}

template < class K, class T, class A, class Pr, bool M >
inline 
TYPENAME 
_map< K, T, A, Pr, M >::iterator 
_map< K, T, A, Pr, M >::erase(node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& allocator_, TYPENAME _map< K, T, A, Pr, M >::iterator p)
{
	TYPENAME _map< K, T, A, Pr, M >::_node* e = 0;
	TYPENAME _map< K, T, A, Pr, M >::iterator r = base_map<K, T, Pr, M>::_erase(p, e);
	if (e)
	{
		_destval(&e->_value);
		_destkey(&e->_key);
		_freenode(allocator_, e);
	}

	return r;
}

template < class K, class T, class A, class Pr, bool M >
inline
void
_map< K, T, A, Pr, M >::erase(const K& x)
{
	if (!this->_size) 
		return;
	TYPENAME _map< K, T, A, Pr, M >::pairii_t p = equal_range(x);
	erase(p.first, p.second);
}

template < class K, class T, class A, class Pr, bool M >
inline
void
_map< K, T, A, Pr, M >::erase(node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& allocator_, const K& x)
{
	if (!this->_size) 
		return;
	TYPENAME _map< K, T, A, Pr, M >::pairii_t p = equal_range(x);
	erase(allocator_, p.first, p.second);
}

template < class K, class T, class A, class Pr, bool M >
inline
TYPENAME _map< K, T, A, Pr, M >::iterator
_map< K, T, A, Pr, M >::erase(TYPENAME _map< K, T, A, Pr, M >::iterator first, TYPENAME _map< K, T, A, Pr, M >::iterator last)
{
	if (first == this->begin() && last == this->end())
	{
		_erase(this->_head._parent);
		base_map<K, T, Pr, M>::clear();
		return this->begin(); 
	}
	else
	{
		while (first != last) erase(first++);
		return first; 
	}
}

template < class K, class T, class A, class Pr, bool M >
inline
TYPENAME _map< K, T, A, Pr, M >::iterator
_map< K, T, A, Pr, M >::erase(node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& allocator_, TYPENAME _map< K, T, A, Pr, M >::iterator first, TYPENAME _map< K, T, A, Pr, M >::iterator last)
{
	if (first == this->begin() && last == this->end())
	{
		_erase(allocator_, this->_head._parent);
		base_map<K, T, Pr, M>::clear();
		return this->begin(); 
	}
	else
	{
		while (first != last) erase(allocator_, first++);
		return first; 
	}
}

template < class K, class T, class A, class Pr, bool M >
inline
void
_map< K, T, A, Pr, M >::_erase(TYPENAME _map< K, T, A, Pr, M >::_node* e)
{ 
	for (TYPENAME _map< K, T, A, Pr, M >::_node* p = e; p->_type != t_head; e = p)
	{
		_erase(p->_right);
		p = p->_left; 
	}
}

template < class K, class T, class A, class Pr, bool M >
inline
void
_map< K, T, A, Pr, M >::_erase(node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& allocator_, TYPENAME _map< K, T, A, Pr, M >::_node* e)
{ 
	for (TYPENAME _map< K, T, A, Pr, M >::_node* p = e; p->_type != t_head; e = p)
	{
		_erase(allocator_, p->_right);
		p = p->_left; 
	}
}

template < class K, class T, class A, class Pr, bool M >
inline
TYPENAME _map< K, T, A, Pr, M >::iterator
_map< K, T, A, Pr, M >::_insert(A& _allocator, bool left, TYPENAME _map< K, T, A, Pr, M >::_node* w, const K& k, const T& v)
{ 
	TYPENAME _map< K, T, A, Pr, M >::_node* n = _buynode(_allocator, w, c_red, t_leaf); 
	if (!n)
		return this->end();
	return base_map<K, T, Pr, M>::_insert(left, w, n, k, v); 
}

template < class K, class T, class A, class Pr, bool M >
inline
TYPENAME _map< K, T, A, Pr, M >::_node*
_map< K, T, A, Pr, M >::_buynode(byte_allocator& allocator_, TYPENAME _map< K, T, A, Pr, M >::_node* p, TYPENAME _map< K, T, A, Pr, M >::redblack c, TYPENAME _map< K, T, A, Pr, M >::nodetype t)
{
	void* ptr = allocator_.allocate(sizeof(TYPENAME _map< K, T, A, Pr, M >::_node));
	if (!ptr)	
		return 0;

	TYPENAME _map< K, T, A, Pr, M >::_node* s = (TYPENAME _map< K, T, A, Pr, M >::_node*)ptr;
	s->_parent = p;
	s->_color = c;
	s->_type = t;
	s->_right = s->_left = this->head();
	return s; 
}

template < class K, class T, class A, class Pr, bool M >
inline
TYPENAME _map< K, T, A, Pr, M >::_node*
_map< K, T, A, Pr, M >::_buynode(node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& allocator_, TYPENAME _map< K, T, A, Pr, M >::_node* p, TYPENAME _map< K, T, A, Pr, M >::redblack c, TYPENAME _map< K, T, A, Pr, M >::nodetype t)
{
	void* ptr = allocator_.allocate();
	if (!ptr)	
		return 0;
	TYPENAME _map< K, T, A, Pr, M >::_node* s = (TYPENAME _map< K, T, A, Pr, M >::_node*)ptr;
	s->_parent = p;
	s->_color = c;
	s->_right = s->_left = this->head();
	s->_type = t;
	return s; 
}

template < class K, class T, class A, class Pr, bool M >
inline 
void 
_map< K, T, A, Pr, M >::_freenode(node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& _allocator, TYPENAME _map< K, T, A, Pr, M >::_node* s)
{
	_allocator.deallocate(s); 
}


/////////////////////
template < class K, class T, class Pr, bool M >
map< K, T, Pr, M >::map(const Pr& pr, size_t size) : 
	base_map<K, T, Pr, M>(pr), _allocator(size) 
{
}

template < class K, class T, class Pr, bool M >
map< K, T, Pr, M >::~map() 
{
}

template < class K, class T, class Pr, bool M >
map< K, T, Pr, M >::map(const map< K, T, Pr, M >& x) : 
	base_map<K, T, Pr, M>(x), _allocator(x._allocator.capacity()) 
{ 
	*this = x; 
}

template < class K, class T, class Pr, bool M >
inline
map< K, T, Pr, M >&
map< K, T, Pr, M >::operator=(const map< K, T, Pr, M >& x)
{
	if (&x != this)
	{
		this->clear();
		this->key_compare = x.key_compare;
		// copy
		for (TYPENAME map< K, T, Pr, M >::const_iterator I = x.begin(); I != x.end(); ++I)
			insert(I.key(), *I);
	}

	return *this;
}

/*
template < class K, class T, class Pr, bool M >
inline
TYPENAME map< K, T, Pr, M >::iterator
map< K, T, Pr, M >::insert(const K& k, const T& v)
{ 
	TYPENAME map< K, T, Pr, M >::_node* x = this->_head._parent;
	TYPENAME map< K, T, Pr, M >::_node* w = this->head();

	bool left = true;
	while (x->_type != t_head)
	{
		w = x;
		left = key_compare(k, x->_key);
		x = left ? x->_left : x->_right; 
	} 

	if (M) return _insert(left, w, k, v);
	
	TYPENAME base_map< K, T, Pr, M >::iterator p =  TYPENAME base_map< K, T, Pr, M >::iterator(w);

	if (!left) 
		;
	else if (p == this->begin())
		return _insert(true, w, k, v);
	else
		--p;

    if (key_compare(p.key(), k))
		return _insert(left, w, k, v);
	
	return p; 
}
*/

template < class K, class T, class Pr, bool M >
inline
TYPENAME 
map< K, T, Pr, M >::pairib_t
map< K, T, Pr, M >::insert(const K& k, const T& v)
{ 
	TYPENAME map< K, T, Pr, M >::_node* x = this->_head._parent;
	TYPENAME map< K, T, Pr, M >::_node* w = this->head();

	bool left = true;
	while (x->_type != t_head)
	{
		w = x;
		left = key_compare(k, x->_key);
		x = left ? x->_left : x->_right; 
	} 

	if (M) return TYPENAME map< K, T, Pr, M >::pairib_t(_insert(left, w, k, v), true);
	
	TYPENAME base_map< K, T, Pr, M >::iterator p =  TYPENAME base_map< K, T, Pr, M >::iterator(w);

	if (!left) 
		;
	else if (p == this->begin())
		return TYPENAME map< K, T, Pr, M >::pairib_t(_insert(true, w, k, v), true);
	else
		--p;

    if (key_compare(p.key(), k))
		return TYPENAME map< K, T, Pr, M >::pairib_t(_insert(left, w, k, v), true);
	
	return TYPENAME map< K, T, Pr, M >::pairib_t(p, false); 
}

template < class K, class T, class Pr, bool M >
inline
TYPENAME map< K, T, Pr, M >::iterator
map< K, T, Pr, M >::erase(TYPENAME map< K, T, Pr, M >::iterator p)
{
	TYPENAME map< K, T, Pr, M >::_node* y = 0;
	TYPENAME map< K, T, Pr, M >::iterator r = base_map<K, T, Pr, M>::_erase(p, y);
	if (y)
	{
		_destval(&y->_value);
		_destkey(&y->_key);
		_freenode(y);
	}

	return r;
}

template < class K, class T, class Pr, bool M >
inline
void
map< K, T, Pr, M >::erase(const K& x)
{
	if (!this->_size)
		return;

	TYPENAME map< K, T, Pr, M >::pairii_t p = equal_range(x);
	erase(p.first, p.second);
}

// NO memory deallocation
// calls only destructors
template < class K, class T, class Pr, bool M >
inline
void
map< K, T, Pr, M >::clear()
{ 
	if (!this->_size)
	{
		_allocator.clear_all();
		return;
	}

	erase(this->begin(), this->end());
	// clear iterator here
	base_map<K, T, Pr, M>::clear();
	_allocator.clear_all();
}

template < class K, class T, class Pr, bool M >
inline
TYPENAME map< K, T, Pr, M >::iterator
map< K, T, Pr, M >::erase(TYPENAME map< K, T, Pr, M >::iterator first, TYPENAME map< K, T, Pr, M >::iterator last)
{
	if (first == this->begin() && last == this->end())
	{
		_erase(this->_head._parent);
		base_map<K, T, Pr, M>::clear();
		return this->begin(); 
	}
	else
	{
		while (first != last) erase(first++);
		return first; 
	}
}

template < class K, class T, class Pr, bool M >
inline
void
map< K, T, Pr, M >::_erase(TYPENAME map< K, T, Pr, M >::_node* x)
{
	for (TYPENAME map< K, T, Pr, M >::_node* y = x; y->_type != t_head; x = y)
	{
		_erase(y->_right);
		y = y->_left;
		_destval(&x->_value);
		_destkey(&x->_key);
        _freenode(x); 
	}
}

template < class K, class T, class Pr, bool M >
inline
TYPENAME map< K, T, Pr, M >::iterator
map< K, T, Pr, M >::_insert(bool left, TYPENAME map< K, T, Pr, M >::_node* w, const K& k, const T& v)
{
	TYPENAME map< K, T, Pr, M >::_node* n = _buynode(w, c_red, t_leaf);
	if (!n)
		return this->end();

	return base_map<K, T, Pr, M>::_insert(left, w, n, k, v);
}
    
template < class K, class T, class Pr, bool M >
inline
TYPENAME map< K, T, Pr, M >::_node*
map< K, T, Pr, M >::_buynode(TYPENAME map< K, T, Pr, M >::_node* p, TYPENAME map< K, T, Pr, M >::redblack c, TYPENAME map< K, T, Pr, M >::nodetype t)
{
	TYPENAME map< K, T, Pr, M >::_node* s = _allocator.allocate();
	if (!s)
		return 0;
	s->_parent = p;
	s->_color = c;
	s->_right = s->_left = this->head();
	s->_type = t;
	return s; 
}

template < class K, class T, class Pr, bool M >
inline
void
map< K, T, Pr, M >::_freenode(TYPENAME map< K, T, Pr, M >::_node* s) 
{ 
	_allocator.deallocate(s); 
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_map_hpp_

