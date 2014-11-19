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

#ifndef _terimber_template_hpp_
#define _terimber_template_hpp_

#include "base/template.h"
#include "base/keymaker.h"
#include "alg/algorith.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// constructor
template < class C >
pool< C >::pool(C& creator, size_t pool_size) :
_creator(creator), _locker(pool_size ? pool_size : 1), _busy_objects(256), _free_objects(256)
{
}
// destructor
template < class C >
pool< C >::~pool() 
{ 
	clear(); 
}

// returns object 
template < class C >
inline
TYPENAME pool< C >::TYPE* 
pool< C >::loan_object(const ARG& arg, size_t timeout)
{
	// wait for availability
	if (_locker.enter(timeout))
	{
		TYPE* obj = 0;
		// locks mutex
		mutexKeeper guard(_mtx);

		if (!_free_objects.empty()) // has some free objects
		{
			// gets front the most recent object
			const pool_entry& x  = _free_objects.front();
			// assign object
			obj = x._obj;
			// adds to busy list
			_busy_objects.push_back(x);
			// removes from free list
			_free_objects.pop_front();
		}
		else // needs to create a new object
		{
			if ((obj = _creator.create(arg))) // create successfully
			{
				pool_entry entry(obj);
				_busy_objects.push_back(entry); // store object in pool
			}
			else // can't create a new object
			{
				_locker.leave(); // leaves gate
				// returns null pointer
				return 0;
			}
		}

		// unlocks mutex
		guard.unlock();
		// activates object
		_creator.activate(obj, arg);
		// returns result
		return obj;
	}

	// no objects are available
	return 0;
}

// returns object back to pool
template < class C >
inline
void
pool< C >::return_object(TYPE* obj, const ARG& arg)
{
	if (!obj)
		return;

	// locks mutex
	mutexKeeper guard(_mtx);

	// removes from busy
	for (TYPENAME list_pool_entry_t::iterator it_busy = _busy_objects.begin(); it_busy != _busy_objects.end(); ++it_busy)
	{
		if (it_busy->_obj == obj) // found
		{
			date now;
			it_busy->_rest = now;
			_creator.back(obj, arg);
			_free_objects.push_front(*it_busy);
			_busy_objects.erase(it_busy);
			guard.unlock();
			_locker.leave(); // releases locker
			return;
		}
	}

	assert(false); // tries to return object that is not in pool
}

// clears pool
template < class C >
inline
void
pool< C >::clear(const ARG& arg)
{
	keylockerWriter keeper(_locker, 
#ifdef _DEBUG
		10000
#else
		3000
#endif
		);

	assert(keeper);

	// locks mutex
	mutexKeeper guard(_mtx);

	while (!_busy_objects.empty())
	{
		_creator.destroy(_busy_objects.front()._obj, arg);
		_busy_objects.pop_front();
	}

	while (!_free_objects.empty())
	{
		_creator.destroy(_free_objects.front()._obj, arg);
		_free_objects.pop_front();
	}
}

// deactivates objects in pool
template < class C >
inline
void
pool< C >::deactivate(size_t maxrest, const ARG& arg)
{ 
	// locks mutex
	mutexKeeper guard(_mtx);

	date now;
	sb8_t deadline = now;
	deadline -= maxrest;

	for (TYPENAME list_pool_entry_t::iterator it_free = _free_objects.begin(); it_free != _free_objects.end(); ++it_free)
	{
		if (!it_free->_rest || it_free->_rest > deadline)
			continue;

		it_free->_rest = 0;
		_creator.deactivate(it_free->_obj, arg);
	}
}

// purges objects in pool
template < class C >
inline
void
pool< C >::purge(size_t maxrest, const ARG& arg)
{ 
	// locks mutex
	mutexKeeper guard(_mtx);

	date now;
	sb8_t deadline = now;
	deadline -= maxrest;

	for (TYPENAME list_pool_entry_t::iterator it_free = _free_objects.begin(); it_free != _free_objects.end();)
	{
		if (it_free->_rest > deadline)
			 ++it_free;
		else
		{
			_creator.destroy(it_free->_obj, arg);
			it_free = _free_objects.erase(it_free);
		}
	}
}

template < class C >
inline
void
pool< C >::get_stats(size_t& free_objects, size_t& busy_objects) const
{
	// locks mutex
	mutexKeeper guard(_mtx);
	free_objects = _free_objects.size();
	busy_objects = _busy_objects.size();
}

//////////////////////////////////////////////////////////////////
// template class supports smart allocation and deallocation of the pointer to the objects
// creator must provide two function T* create(size_t) & destroy(T*)
// constructor
// object internally created
template < class C >
smart_pointer< C >::smart_pointer(C& crt, const ARG& n) : 
	_crt(crt) 
{ 
	_ptr = crt.create(n); 
}

// constructor
// object will later be assigned explicitly
template < class C >
smart_pointer< C >::smart_pointer(C& crt) : 
	_crt(crt), _ptr(0) 
{
}

// destructor
template < class C >
smart_pointer< C >::~smart_pointer() 
{ 
	clear(); 
}

// assign operator
template < class C >
inline 
smart_pointer< C >&
smart_pointer< C >::operator=(const TYPE* x) 
{ 
	clear(); 
	_ptr = (TYPENAME smart_pointer< C >::TYPE*)x; 
	return *this; 
}
	
template < class C >
inline 
TYPENAME smart_pointer< C >::TYPE*
smart_pointer< C >::operator->() 
{ 
	return _ptr; 
}

template < class C >
inline 
const TYPENAME smart_pointer< C >::TYPE*
smart_pointer< C >::operator->() const 
{ 
	return _ptr; 
}

// access operator to the address of pointer to object
// interface support
template < class C >
inline 
TYPENAME smart_pointer< C >::TYPE**
smart_pointer< C >::operator&() 
{ 
	return &_ptr; 
}

// checks object
template < class C >
inline 
bool
smart_pointer< C >::operator!() const 
{ 
	return !_ptr; 
}

// destroys object
template < class C >
inline 
void
smart_pointer< C >::clear() 
{ 
	if (_ptr)
	{
		_crt.destroy(_ptr); 
		_ptr = 0; 
	}
}

template < class C >
inline 
TYPENAME smart_pointer< C >::TYPE*
smart_pointer< C >::detach() 
{ 
	TYPENAME smart_pointer< C >::TYPE* ptr = _ptr; 
	_ptr = 0; 
	return ptr; 
}

// attaches new object and frees old object, if specified
template < class C >
inline 
void
smart_pointer< C >::attach(TYPE* obj, bool free) 
{ 
	free ? clear() : detach(); _ptr = obj; 
}

/////////////////////////////////////////////////////
template < class P >
pool_object_keeper< P >::pool_object_keeper(P* pool_, TYPE* obj) : 
	_pool(pool_), _obj(obj) 
{
}

template < class P >
pool_object_keeper< P >::pool_object_keeper(P* pool_, const ARG& arg, size_t timeout) : 
	_pool(pool_)
{ 
	_obj = _pool ? _pool->loan_object(arg, timeout) : 0; 
}

template < class P >
pool_object_keeper< P >::~pool_object_keeper() 
{ 
	if (_pool && _obj) 
		_pool->return_object(_obj); 
}

template < class P >
inline
TYPENAME pool_object_keeper< P >::TYPE*
pool_object_keeper< P >::operator->() 
{ 
	return _obj; 
}

template < class P >
inline
const TYPENAME pool_object_keeper< P >::TYPE*
pool_object_keeper< P >::operator->() const 
{ 
	return _obj; 
}

template < class P >
inline
bool
pool_object_keeper< P >::operator!() const 
{ 
	return !_obj; 
}


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_template_hpp_

