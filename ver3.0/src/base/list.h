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

#ifndef _terimber_list_h_
#define _terimber_list_h_

#include "base/memory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class base_list
//! \brief base class for list family
template < class T >
class base_list
{
	//! \brief copy constructor
	base_list< T >(const base_list< T >& x);
	//! \brief assign operator
	base_list< T >& operator=(const base_list< T >& x);

public:
	//! forward declaration
	class _node;
	//! \class _node_
	//! \brief internal help class for head node
	class _node_
	{
	public:
		//! \brief constructor
		_node_() : 
			_next(0), 
			_prev(0) 
		{
		}

		_node*	_next;										//!< pointer to the next _node
		_node*	_prev;										//!< pointer to the prev _node
	};

	//! \class _node
	//! \brief node with object
	class _node : public _node_
	{
	public:
		//! \brief constructor
		_node(const T& x) : 
			_node_(), 
			_value(x) 
		{
		}
		
		T		_value;										//!< object instance stored in list, must support copy constructor
	};

protected:

	//! \brief  constructor
	inline 
	base_list< T >();
	//! \brief  destructor
	inline 
	~base_list< T >();

public:
	//! \brief  checks empty, uses instead of size
	inline 
	bool 
	empty() const;
	//! \brief gets first const object
	//! function doesn't check boundaries
	inline 
	const T& 
	front() const;
	//! \brief get first object
	//! function doesn't check boundaries
	inline 
	T& 
	front();
	//! \brief get last const object
	//! function doesn't check boundaries
	inline 
	const T& 
	back() const;
	//! \brief get last object
	//! function doesn't check boundaries
	inline 
	T& 
	back();

////////////////
	//! there are iterator definitions
	//! forward declaration
	class iterator;
	//! \class const_iterator
	//! \brief const interator
	class const_iterator
	{
	public:
		//! \typedef iterator_category
		typedef std::bidirectional_iterator_tag iterator_category;
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
		const_iterator(_node* p						//!< pointer to node
						) : _ptr(p) {}
		//! \brief constructor
		inline 
		const_iterator(const iterator& x				//!< non const interator
						) : _ptr(x._ptr) {}
		//! \brief operator *
		inline 
		const T& 
		operator*() const 
		{ 
			return _ptr->_value; 
		}
		//! \brief operator ->
		inline 
		const T* 
		operator->() const 
		{ 
			return &(_ptr->_value); 
		}
		//! \brief operator ++()
		inline 
		const_iterator 
		operator++() 
		{ 
			_ptr = _ptr->_next; 
			return *this; 
		}
		//! \brief operator ++(int)
		inline 
		const_iterator 
		operator++(int) 
		{ 
			const_iterator tmp = _ptr; 
			_ptr = _ptr->_next; 
			return tmp; 
		}
		//! \brief operator --()
		inline 
		const_iterator 
		operator--() 
		{ 
			_ptr = _ptr->_prev; 
			return *this; 
		}
		//! \brief operator --(int)
		inline 
		const_iterator 
		operator--(int) 
		{ 
			const_iterator tmp = _ptr; 
			_ptr = _ptr->_prev; 
			return tmp; 
		}
		//! \brief equal operator
		inline 
		bool 
		operator==(const const_iterator& x) const 
		{ 
			return _ptr == x._ptr; 
		}
		//! \brief non equal operator
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
		//! \brief  gets previous node
		inline 
		_node* 
		prev() const 
		{ 
			return _ptr->_prev; 
		}
		//! \brief  gets current node
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
	//! non const iterator
	class iterator : public const_iterator 
	{
	public:
		//! \brief constructor
		inline 
		iterator(	_node* p									//!< pointer to node
					) : const_iterator(p) 
		{
		}
		//! \brief operator *
		inline 
		T& 
		operator*() const	
		{ 
			return this->_ptr->_value; 
		}
		//! \brief operator ->
		inline 
		T* 
		operator->() const 
		{ 
			return &(this->_ptr->_value); 
		}
		//! \brief operator ++()
		inline 
		iterator 
		operator++() 
		{ 
			this->_ptr = this->_ptr->_next; 
			return *this; 
		}
		//! \brief operator ++(int)
		inline 
		iterator 
		operator++(int) 
		{ 
			iterator tmp = this->_ptr; 
			this->_ptr = this->_ptr->_next; 
			return tmp; 
		}
		//! \brief operator --()
		inline 
		iterator 
		operator--() 
		{ 
			this->_ptr = this->_ptr->_prev; 
			return *this; 
		}
		//! \brief operator --(int)
		inline 
		iterator 
		operator--(int) 
		{ 
			iterator tmp = this->_ptr; 
			this->_ptr = this->_ptr->_prev; 
			return tmp; 
		}
		//! \brief equal operator
		inline 
		bool 
		operator==(const iterator& x) const 
		{ 
			return this->_ptr == x._ptr; 
		}
		//! \brief non equal operator 
		inline 
		bool 
		operator!=(const iterator& x) const 
		{ 
			return this->_ptr != x._ptr;
		}
	};

	//! \brief gets begin const iterator
	inline TYPENAME base_list< T >::const_iterator begin() const;
	//! \brief gets begin iterator
	inline TYPENAME base_list< T >::iterator begin();
	//! \brief gets const end interator
	inline TYPENAME base_list< T >::const_iterator end() const;
	//! \brief ends iterator
	inline TYPENAME base_list< T >::iterator end();
	//! \brief clears list wihtout memory deallocation
	inline 
	void 
	clear();
protected:
	//! \brief returns pointer to head node
	inline 
	TYPENAME base_list< T >::_node* 
	head();
	//! \brief returns pointer to head node (const function)
	inline 
	TYPENAME base_list< T >::_node* 
	head() const;

	TYPENAME base_list< T >::_node_		_head;				//!< store only head of list
};

/////////////////////////////////////////////////
//! \class _list
//! \brief class _list uses external allocator class
//! for ALL memory allocations
//! only 4 byte size for empty list object
//! only forward and backward iterators are supported
template < class T, class A = byte_allocator >
class _list : public base_list< T >
{
public:
	//! \brief constructor
	inline 
	_list< T, A >();
	//! \brief destructor
	inline 
	~_list< T, A >();
	//! \brief copy constructor
	//! copies only pointers to head and to elements
	//! NO memory reallocation or copy
	inline 
	_list< T, A >(const _list< T, A >& x);
	//! \brief assign operator
	//! copies only pointers to head and to elements
	//! NO memory reallocation or copy
	inline 
	_list< T, A >& operator=(const _list< T, A >& x);
	//! \brief make a full copy
	inline 
	void 
	assign(			A& allocator_,							//!< external allocator
					const _list< T, A >& x				//!< input
					);
	//! \brief makes a full copy
	inline 
	void 
	assign(			A& allocator_,							//!< external allocator		
					TYPENAME _list< T, A >::const_iterator first, //!< first iterator
					TYPENAME _list< T, A >::const_iterator last	  //!< last iterator
				);
	//! \brief adds n elements to list
	inline 
	void 
	assign(			A& allocator_,							//!< external allocator	
					size_t n,								//!< number of elements
					const T& x = T()						//!< object
				);
	//! \brief gets list size
	//! expensive operation
	inline 
	size_t 
	size() const;
	//! \brief inserts new object to the end of list
	inline 
	TYPENAME _list< T, A >::iterator 
	push_back(		A& allocator_,							//!< external allocator
					const T& x								//!< object
				);
	//! \brief inserts new object to the front of list
	inline 
	TYPENAME _list< T, A >::iterator
	push_front(		A& allocator_,							//!< external allocator
					const T& x								//!< object
				);
	//! \brief removes first element from list
	//! no memory deallocation
	inline 
	bool 
	pop_front();
	//! \brief removes first element from list
	inline 
	bool 
	pop_front(		node_allocator< TYPENAME base_list< T >::_node >& allocator_	//!< external allocator
					);
	//! no memory deallocation
	inline 
	bool 
	pop_back();
	//! \brief removes last element from list
	inline 
	bool 
	pop_back(		node_allocator< TYPENAME base_list< T >::_node >& allocator_	//!< external allocator
					);
	//! \brief inserts object before provided iterator
	inline 
	TYPENAME _list< T, A >::iterator									
	insert(			A& allocator_,							//!< external allocator
					TYPENAME _list< T, A >::iterator it,	//!< before iterator
					const T& x = T()						//!< object
					);
	//! \brief inserts n objects before provided iterator
    inline 
	void 
	insert(			A& allocator_,							//!< external allocator
					TYPENAME _list< T, A >::iterator it,	//!< before iterator
					size_t n,								//!< number of objects
					const T& x								//!< object
					);
	//! \brief inserts the list of objects before provided iterator
    inline 
	void 
	insert(			A& allocator_,							//!< external allocator
					TYPENAME _list< T, A >::iterator it,	//!< before iterator
					TYPENAME _list< T, A >::const_iterator first,	//!< begin iterator			
					TYPENAME _list< T, A >::const_iterator last		//!< end iterator
					);
	//! \brief removes the specified element of list without memory deallocation
	//! must be defined compare operator for stored object
	inline 
	void 
	remove(			const T& x								//!< object
					);
	//! \brief removes the specified element of list with memory deallocation
	//! must be defined compare operator for stored object
	inline 
	void 
	remove(			node_allocator< TYPENAME base_list< T >::_node >& allocator_,	//!< external allocator
					const T& x								//!< object to remove
					);
	//! \brief removes list object by iterator
	//! no memory deallocation
	inline 
	TYPENAME _list< T, A >::iterator 
	erase(			TYPENAME _list< T, A >::iterator iter	//!< erase iterator 
					);
	//! \brief removes list object by iterator
	//! with memory deallocation
	inline 
	TYPENAME _list< T, A >::iterator 
	erase(			node_allocator< TYPENAME base_list< T >::_node >& allocator_,	//!< external allocator
					TYPENAME _list< T, A >::iterator iter	//!< erase iterator
					);
	//! \brief removes list objects by iterator interval
	//! no memory deallocation
	inline 
	TYPENAME _list< T, A >::iterator 
	erase(			TYPENAME _list< T, A >::iterator first,	//!< first iterator
					TYPENAME _list< T, A >::iterator last	//!< last iterator
					);
	//! \brief removes list objects by iterator interval
	//! with memory deallocation
	inline 
	TYPENAME _list< T, A >::iterator 
	erase(			node_allocator< TYPENAME base_list< T >::_node >& allocator_,	//!< external allocator
					TYPENAME _list< T, A >::iterator first,	//!< first iterator
					TYPENAME _list< T, A >::iterator last	//!< last iterator
					);
private:
	//! \brief removes object by iterator interval
	//! no memory deallocation
	inline 
	TYPENAME _list< T, A >::iterator 
	remove(			TYPENAME _list< T, A >::iterator iter	//!< remove iterator
					);
	//! \brief removes object by iterator interval
	//! with memory deallocation
	inline 
	TYPENAME _list< T, A >::iterator 
	remove(			node_allocator< TYPENAME base_list< T >::_node >& allocator_,	//!< external allocator
					TYPENAME _list< T, A >::iterator iter	//!< remove iterator
					);
	//! \brief allocates node object on byte_allocator
	inline 
	TYPENAME _list< T, A >::_node* 
	_buynode(		byte_allocator& allocator_,				//!< external allocator
					const T& x								//!< object
					);
	//! \brief allocates node object on node_allocator
	inline 
	TYPENAME _list< T, A >::_node* 
	_buynode(		node_allocator< TYPENAME base_list< T >::_node >& allocator_,	//!< external allocator
					const T& x								//!< object
					);
	//! \brief deallocates node object on node_allocator
	inline 
	void 
	_freenode(		node_allocator< TYPENAME base_list< T >::_node >& allocator_,	//!< external allocator
					TYPENAME _list< T, A >::_node* p		//!< pointer to node
					);
};

/////////////////////////////////////////////////
//! \class list
//! class list uses internal allocator class
//! for ALL internal memory allocations
//! forward and backward iterators are supported
template < class T >
class list : public base_list< T >
{
public:
	//! \brief constructor
	list< T >(	size_t size = os_def_size					//!< default element count - memory optimization for allocator pages
				);
	//! \brief destructor
	~list< T >();
	//! \brief copy constructor
	list< T >(const list< T >& x);
	//! \brief assign operator copy all objects
	list< T >& operator=(const list< T >& x);
	//! \brief clear list
	//! call destructors of objects
	inline 
	void 
	clear();
	//! \brief assigns new list calling copy constructors
	inline 
	void 
	assign(			const list< T >& x					//!< input list
					);
	//! \brief assigns new list (from : to) calling copy constructors
	inline 
	void 
	assign(			TYPENAME  list< T >::const_iterator first,	//!< first iterator
					TYPENAME list< T >::const_iterator last		//!< last iterator
					);
	//! \brief assigns new list (n elements) calling copy constructor
	inline 
	void 
	assign(			size_t n,								//!< number of elements
					const T& x = T()						//!< object
					);
	//! \brief returns size of list
	inline 
	size_t 
	size() const;
	//! \brief inserts new object to the end of list
	inline 
	TYPENAME list< T >::iterator
	push_back(		const T& x								//!< object
					);
	//! \brief inserts new object to the front of list
	inline 
	TYPENAME list< T >::iterator
	push_front(		const T& x								//!< object
					);
	//! \brief removes first element from list
	//! calling object destructor
	inline 
	bool 
	pop_front();
	//! \brief removes last element from list
	//! calling object destructor
	inline 
	bool 
	pop_back();
	//! \brief inserts new element before specified iterator 
	//! calling copy constructor
	TYPENAME list< T >::iterator 
	insert(			TYPENAME list< T >::iterator it,		//!< before iterator
					const T& x = T()						//!< object
					);
	//! \brief assigns n new elements before specified iterator 
	// calling copy constructor
    void 
	insert(			TYPENAME list< T >::iterator it,		//!< before iterator
					size_t n,								//!< number of elements
					const T& x								//!< object
					);
	//! \brief assigns [from : to[ new elements before specified iterator 
	//! calling copy constructor
    void 
	insert(			TYPENAME list< T >::iterator it,		//!< before iterator
					TYPENAME list< T >::const_iterator first,	//!< first iterator
					TYPENAME list< T >::const_iterator last	//!< last iterator
					);
	//! \brief removes the specified element of list
	//! must be defined compare operator for stored object
	inline 
	void 
	remove(			const T& x								//!< object
					);
	//! \brief removes specified element
	//! calling destructors
	//! returns the next iterator
	inline 
	TYPENAME list< T >::iterator 
	erase(			TYPENAME list< T >::iterator iter		//!< erases iterator
					);
	//! \brief removes specified elements [from : to[
	//! calling destructors
	//! returns the next iterator
	inline 
	TYPENAME list< T >::iterator 
	erase(			TYPENAME list< T >::iterator first,		//!< first iterator
					TYPENAME list< T >::iterator last		//!< last iterator
					);
private:
	//! \brief removes the specified iterator
	inline 
	TYPENAME list< T >::iterator 
	remove(			TYPENAME list< T >::iterator iter		//!< removes iterator
					);
protected:
	//! \brief allocates memory for new node and call copy constructor for element
	inline 
	TYPENAME list< T >::_node* 
	_buynode(		const T& x								//!< obect
					);
	//! \brief deallocates memory calling destructor for element
	inline 
	void 
	_freenode(		TYPENAME list< T >::_node* p			//!< pointer to node
					);
protected:
	size_t												_length;	//!< keeps the length of list
	node_allocator< TYPENAME base_list< T >::_node >	_allocator; //!< internal allocator
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_list_h_

