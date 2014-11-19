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

#ifndef _terimber_stack_h_
#define _terimber_stack_h_

#include "base/memory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class base_stack
//! \brief class provides the stack functionality
template < class T >
class base_stack
{
public:
	//! forward declaration
	class _node;
	//! \class _node_
	//! \brief internal help class
	class _node_
	{
	public:
		//! \brief constructor
		_node_() : 
		  _next(0) 
		 {
		 }

		_node*	_next;										//!< pointer to the next _node
	};

	//! \class _node
	//! \brief list node
	class _node : public _node_
	{
	public:
		//! \brief constructor
		_node(		const T& x								//!< input value
					) : 
			_node_(), 
			_value(x) 
		{
		}

		T _value;											//!< object instance stored in list, must support copy constructor
	};
protected:
	//! \brief constructor
	base_stack< T >();
	//! \brief destructor
	~base_stack< T >();
	//! \brief copy constructor
	base_stack< T >(const base_stack< T >& x);
public:
	//! \brief check empty, use instead of size
	inline 
	bool 
	empty() const;
	//! \brief returns const top element
	//! function doesn't check boundaries
	inline 
	const T& 
	top() const;
	//! \brief returns top element
	//! function doesn't check boundaries
	inline T& 
	top();

////////////////
	//! forward declaration
	//! we support only one direction - forward
	class iterator;
	//! \class const_iterator
	//! \brief most of STL algorithms are supported
	class const_iterator
	{
	public:
		//! \typedef iterator_category
		typedef std::forward_iterator_tag iterator_category;
		//! \typedef size_type
		typedef size_t size_type;
		//! \typedef pointer
		typedef T* pointer;
		//! \typedef const_pointer
		typedef const T* const_pointer;
		//! \typedef reference
		typedef T& reference;
		//! \typedef const_reference
		typedef const T& const_reference;
		//! \typedef value_type
		typedef T value_type;
		//! \typedef difference_type
		typedef size_t difference_type;
		
		//! \brief constructor
		inline 
		const_iterator(_node* p								//!< pointer to node
						) : 
			_ptr(p) 
		{
		}
		//! \brief copy constructor
		inline 
		const_iterator(const iterator& x) : 
			_ptr(x._ptr) 
		{
		}
		//! \brief operator*
		inline 
		const T& 
		operator*() const 
		{ 
			return _ptr->_value; 
		}
		//! \brief operator->
		inline 
		const T* 
		operator->() const 
		{ 
			return &_ptr->_value; 
		}
		//! \brief operator++
		inline 
		const_iterator 
		operator++() 
		{ 
			_ptr = _ptr->_next; 
			return *this; 
		}
		//! \brief operator++(int)
		inline 
		const_iterator 
		operator++(int) 
		{ 
			const_iterator tmp = _ptr; 
			_ptr = _ptr->_next; 
			return tmp; 
		}
		//! \brief operator==
		inline 
		bool 
		operator==(const const_iterator& x) const 
		{ 
			return _ptr == x._ptr; 
		}
		//! \brief operator!=
		inline 
		bool 
		operator!=(const const_iterator& x) const 
		{ 
			return _ptr != x._ptr; 
		}
		//! \brief gets next node
		inline 
		_node* 
		next() const 
		{ 
			return _ptr->_next; 
		}
		//! \brief gets node pointer
		inline 
		_node* 
		node() const 
		{ 
			return _ptr; 
		}
	protected:
		_node*		_ptr;									//!< pointer to node
	};
	//! \class iterator
	//! \brief non const iterator
	class iterator : public const_iterator 
	{
	public:
		//! \brief constructor
		inline 
		iterator(	_node* p								//!< pointer to node
					) : 
			const_iterator(p) 
		{
		}
		//! \brief operator*
		inline 
		T& 
		operator*() const	
		{ 
			return this->_ptr->_value; 
		}
		//! \brief operator->
		inline 
		T* 
		operator->() const 
		{ 
			return &this->_ptr->_value; 
		}
		//! \brief operator++
		inline 
		iterator 
		operator++() 
		{ 
			this->_ptr = this->_ptr->_next; 
			return *this; 
		}
		//! \brief operator++(int)
		inline 
		iterator 
		operator++(int) 
		{ 
			iterator tmp = this->_ptr; 
			this->_ptr = this->_ptr->_next; 
			return tmp; 
		}
		//! \brief operator==
		inline 
		bool 
		operator==(const iterator& x) const 
		{ 
			return this->_ptr == x._ptr; 
		}
		//! \brief operator!=
		inline 
		bool 
		operator!=(const iterator& x) const 
		{ 
			return this->_ptr != x._ptr; 
		}
	};

	//! \brief returns begin const iterator
	inline 
	TYPENAME 
	base_stack< T >::const_iterator 
	begin() const;
	//! \brief returns begin iterator
	inline 
	TYPENAME 
	base_stack< T >::iterator 
	begin();
	//! \brief returns end const iterator
	inline 
	TYPENAME 
	base_stack< T >::const_iterator 
	end() const;
	//! \brief returns end iterator
	inline 
	TYPENAME 
	base_stack< T >::iterator 
	end();
	//! \brief clears stack
	inline 
	void 
	clear();

protected:
	//! \brief returns pointer to head
	inline 
	TYPENAME 
	base_stack< T >::_node* 
	head();
	//! \brief returns pointer to head (const)
	inline 
	TYPENAME base_stack< T >::_node* 
	head() const;
	
	TYPENAME base_stack< T >::_node_	_head;				//!< store only head of stack
};


//! \class _stack
//! \brief class stack uses external allocator class
//! for ALL internal memory allocations
//! only 4 byte size for empty stack object
template < class T, class A = byte_allocator >
class _stack : public base_stack< T >
{
public:
	//! \brief constructor
	_stack< T, A >();
	//! \brief destructor
	~_stack< T, A >();
	//! \brief copy constructor
	//! copies only pointers to head element
	//! NO reallocation or copy of memory
	_stack< T, A >(const _stack< T, A >& x);
	//! \brief assign operator
	//! copy only pointers to head element
	//! NO reallocation or copy of memory
	inline 
	_stack< T, A >& 
	operator=(const _stack< T, A >& x);
	//! \brief gets size - expensive operation use empty instead
	inline 
	size_t 
	size();
	//! \brief inserts new object to the top of stack
	inline 
	TYPENAME
	_stack< T, A >::iterator
	push(			A& allocator_,							//!< external allocator
					const T& x								//!< object
					);
	//! \brief removes first element from stack
	//! no memory deallocation
	inline 
	bool 
	pop();
	//! \brief removes first element from stack
	//! memory deallocation on external allocator
	inline 
	bool 
	pop(			node_allocator< TYPENAME base_stack< T >::_node >& allocator_ //!< external allocator
					);
	
protected:
	//! \brief allocates node on byte_allocator
	inline 
	TYPENAME _stack< T, A >::_node* 
	_buynode(		byte_allocator& allocator_,				//!< external allocator
					const T& x								//!< input value
					);
	//! \brief allocates node on node_allocator
	inline 
	TYPENAME 
	_stack< T, A >::_node* 
	_buynode(		node_allocator< TYPENAME base_stack< T >::_node >& allocator_,//!< external allocator
					const T& x								//!< input value
					);
	//! \brief deallocates node on node_allocator
	inline 
	void 
	_freenode(		node_allocator< TYPENAME base_stack< T >::_node >& allocator_, //!< external allocator
					TYPENAME _stack< T, A >::_node* p		//!< pointer to node
					);
};

//! \class stack
//! \brief class stack uses internal allocator class
//! for ALL internal memory allocations
//! forward iterators are supported
template < class T >
class stack : public base_stack< T >
{
public:
	//! \brief constructor
	stack< T >(		size_t size = os_def_size				//!< default node_allocator chunk
					);
	//! \brief destructor
	~stack< T >();
	//! \brief copy constructor
	stack< T >(const stack< T >& x);
	//! \brief assigns operator
	inline 
	stack< T >& operator=(const stack< T >& x);
	//! \brief returns stack size
	inline 
	size_t 
	size() const;
	//! \brief inserts new object to the front of stack
	inline 
	TYPENAME
	stack< T >::iterator
	push(			const T& x								//!< input value
					);
	//! \brief removes top element from stack
	inline 
	bool 
	pop();
	//! \brief clears stack
	//! call object destructors
	inline 
	void 
	clear();

protected:
	//! \brief allocates node on internal allocator
	inline 
	TYPENAME 
	stack< T >::_node* 
	_buynode(		const T& x								//!< input value
					);
	//! \brief deallocates node on internal allocator
	inline 
	void 
	_freenode(		TYPENAME stack< T >::_node* p			//!< pointer to node
					);

protected:
    size_t							_length;				//!< keep length
	node_allocator< TYPENAME base_stack< T >::_node > _allocator; //!< internal allocator
};

//! \class unique_key_generator
//! \brief supports the generation and reusing of the unique key sequence
//
class unique_key_generator
{
	//! \brief private copy constructor
	unique_key_generator(const unique_key_generator& x);
	//! \brief private assign operator
	unique_key_generator& operator=(const unique_key_generator& x);

public:
	//! \brief constructor
	inline 
	unique_key_generator(size_t capacity = os_def_size		//!< default capacity for stack chunks
						);
	//! \brief destructor
	inline 
	~unique_key_generator();
	//! \brief clears all internal resources
	inline 
	void 
	clear();
	//! \brief generates the new unique key or takes it from repository
	inline 
	size_t 
	generate();
	//! \brief saves the used key and store it in repository
	inline 
	void 
	save(			size_t key								//!< used key
					);
private:
	size_t				_last;								//!< keep the last key
	stack< size_t >		_rep;								//!< repository for the used keys
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_stack_h_

