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

#ifndef _terimber_map_h_
#define _terimber_map_h_

#include "base/memory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class less 
template< class T >
class less
{
public:
	//! \brief functor for operator<
	inline 
	bool 
	operator()(		const T& first,							//!< first argument
					const T& second							//!< second argument
					) const;
};

//! \class pair
//! \brief no dependences from STL containers
//! stores a pair of values
template < class T1, class T2 > 
class pair
{
public:
	//! \brief default constructor
	pair< T1, T2 >();
	//! \brief constructor
	pair< T1, T2 >(	const T1& v1,							//!< first argument
					const T2& v2							//!< second argument
					);
	//! \brief copy constructor
	pair< T1, T2 >(const pair< T1, T2 >& x);
	//! \brief equal operator
	inline bool operator==(const pair< T1, T2 >& x) const;
	//! \brief less operator
	inline bool operator<(const pair< T1, T2 >& x) const;
	//! \brief non-equal operator
	inline bool operator!=(const pair< T1, T2 >& x) const;

	T1 first;												//!< the first stored value
	T2 second;												//!< the second stored value
};

//! \class base_map
//! \brief implements binary tree
template < class K, class T, class Pr = less< K >, bool M = false >
class base_map 
{
public:
	//! forward declaration
	class iterator;
    class const_iterator;

	//! friends
	friend class const_iterator;
	friend class iterator;

	//! \typedef redblack
	//! \brief red/black flag
	typedef bool redblack;

//! \brief read is true
#define c_red true
//! \brief black is false
#define c_black false

	//! \typedef nodetype
	//! \brief head/leaf
	typedef bool nodetype;

//! \brief head is true
#define t_head true
//! \brief leaf is false
#define t_leaf false

	//! forward declaration
	class _node;
	//! \class _node_
	//! \brief base node
	class _node_
	{
	public:
		//! \brief constructor
		_node_() : 
			_left(0), 
			_parent(0), 
			_right(0), 
			_color(c_red), 
			_type(t_leaf) 
		{
		}
		
		_node*		_left;									//!< pointer to left node
		_node*		_parent;								//!< pointer to parent node
		_node*		_right;									//!< pointer to right node
		redblack	_color;									//!< node color
		nodetype	_type;									//!< node type
	};
	//! \class _node
	//! \brief node with template key and value
	class _node : public _node_
	{
	public:
		//! \brief constructor
		_node() : 
			_node_() 
		{
		}
		
		K			_key;									//!< key
		T			_value;									//!< value
		//! \brief finds the max node
		inline 
		static 
		_node* 
		_max(		_node* p								//!< pointer to node
					) 
		{ 
			while (p->_right->_type != t_head) 
				p = p->_right; 
			return p; 
		}
		//! \brief finds the min node
        inline 
		static 
		_node* 
		_min(_node* p) 
		{ 
			while (p->_left->_type != t_head) 
				p = p->_left; 
			return p; 
		}
	};

protected:
	//! \brief constructor
	explicit 
	base_map< K, T, Pr, M >(const Pr& pr = Pr()				//!< predicate
					);
	//! \brief copy constructor
    base_map< K, T, Pr, M >(const base_map< K, T, Pr, M >& x);
	//! \brief destructor
	~base_map< K, T, Pr, M >();

public:
	//! publish iterators
	//! \class const_iterator
	//! \brief const iterator	
	class const_iterator
	{
	public:
		//! STL types
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

		//! \brief default constructor
		inline 
		const_iterator() : 
			_ptr(0) 
		{
		}
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
		//! \brief const operator*
		inline 
		const T& 
		operator*() const 
		{ 
			return _ptr->_value; 
		}
		//! \brief const operator->
		inline 
		const T* 
		operator->() const 
		{ 
			return &_ptr->_value; 
		}
		//! \brief operator++
		inline 
		const_iterator& 
		operator++() 
		{ 
			_inc(); 
			return *this; 
		}
		//! \brief operator++(int)
		inline 
		const_iterator 
		operator++(int) 
		{ 
			const_iterator tmp = *this; 
			_inc(); 
			return tmp; 
		}
		//! \brief operator--
		inline 
		const_iterator& operator--() 
		{ 
			_dec(); 
			return *this; 
		}
		//! \brief operator--(int)
		inline 
		const_iterator 
		operator--(int) 
		{ 
			const_iterator tmp = *this; 
			_dec(); 
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
		//! \brief returns const key
		inline 
		const 
		K& key() const 
		{ 
			return _ptr->_key; 
		}
		//! \brief moves the iterator to previous element
		inline void _dec()
		{	
			if (_ptr->_type == t_head)
				_ptr = _ptr->_right;
			else if (_ptr->_left->_type != t_head)
				_ptr = _node::_max(_ptr->_left);
			else
			{ 
				_node* p; 
				while ((p = _ptr->_parent)->_type != t_head && _ptr == p->_left)
					_ptr = p;
				if (p->_type != t_head)
					_ptr = p;
			}
		}
		//! \brief moves the iterator to next element
		inline void _inc()
		{
			if (_ptr->_type == t_head)
				//; ???
				_ptr = _ptr->_left;
			else if (_ptr->_right->_type != t_head)
				_ptr = _node::_min(_ptr->_right);
			else
			{
				_node* p;
				while ((p = _ptr->_parent)->_type != t_head && _ptr == p->_right)
					_ptr = p;

				_ptr = p; 
			}
		}
		//! \brief returns the pointer to the left of node
		inline 
		_node* 
		left() const 
		{ 
			return _ptr->_left; 
		}
		//! \brief returns the pointer to the right of node
		inline 
		_node* 
		right() const 
		{ 
			return _ptr->_right; 
		}
		//! \brief returns the pointer to the parent of node
		inline 
		_node* 
		parent() const 
		{ 
			return _ptr->_parent; 
		}
		//! \brief returns the pointer to the node
		inline 
		_node* node() const 
		{ 
			return _ptr; 
		}
    protected:
		_node*			_ptr;								//!< pointer to the node
	};
	
	//! \class iterator
	//! \brief non const iterator
	class iterator : public const_iterator 
	{
	public:
		//! \brief default constructor
		inline 
		iterator() 
		{}
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
		iterator& 
		operator++() 
		{ 
			this->_inc(); 
			return *this; 
		}
		//! \brief operator++(int)
		inline 
		iterator 
		operator++(int) 
		{ 
			iterator tmp = *this; 
			this->_inc(); 
			return tmp; 
		}
		//! \brief operator--
		inline 
		iterator& 
		operator--() 
		{ 
			this->_dec(); 
			return *this; 
		}
		//! \brief operator--(int)
		inline 
		iterator 
		operator--(int) 
		{ 
			iterator tmp = *this; 
			this->_dec(); 
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
	
	//! \brief returns predicate
	inline 
	const Pr& 
	comp() const;
	//! \brief begins iterator
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::iterator 
	begin();
	//! \brief const begin iterator
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::const_iterator 
	begin() const;
	//! \brief ends iterator
	inline
	TYPENAME 
	base_map< K, T, Pr, M >::iterator 
	end();
	//! \brief const end iterator
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::const_iterator 
	end() const;
	//! \brief returns size of map
	inline 
	size_t 
	size() const;
	//! \brief checks if map is empty
	inline 
	bool 
	empty() const;
	//! \brief finds iterator by key
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::iterator 
	find(			const K& k								//!< key
					);
	//! \brief finds const iterator by key
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::const_iterator 
	find(			const K& k								//!< key
					) const;
	//! \typedef pairii_t
	//! \brief range of iterators
	typedef pair< iterator, iterator >				pairii_t;
	//! \typedef paircc_t
	//! \brief const range of iterators
	typedef pair< const_iterator, const_iterator >	paircc_t;
	//! \brief finds lower bound by key
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::iterator 
	lower_bound(	const K& k								//!< key
					);
	//! \brief finds const lower bound by key
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::const_iterator 
	lower_bound(	const K& k								//!< key
					) const;
	//! \brief finds upper bound by key
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::iterator 
	upper_bound(	const K& k								//!< key
					);
	//! \brief finds const upper bound by key
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::const_iterator 
	upper_bound(	const K& k								//!< key
					) const;
	//! \brief finds equal range by key
	inline 
	pairii_t 
	equal_range(	const K& k								//!< key
					);
	//! \brief finds const equal range by key
	inline 
	paircc_t 
	equal_range(	const K& k								//!< key
					) const;
	//! \brief clears map
	inline 
	void 
	clear();

	//! \typedef pairib_t
	//! \brief iterator and boolean flag
	typedef pair< iterator, bool >					pairib_t;

protected:
	//! \brief inserts new node
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::iterator 
	_insert(		bool left,								//!< left flag
					TYPENAME base_map< K, T, Pr, M >::_node* w, //!< target node
					TYPENAME base_map< K, T, Pr, M >::_node* n, //!< new node
					const K& k,								//!< key
					const T& v								//!< value
					);
	//! \brief erases node
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::iterator 
	_erase(			iterator w,								//!< iterator for erase
					TYPENAME base_map< K, T, Pr, M >::_node*& e //!< [out] deleted node
					);
	//! \brief finds lower bound node by key
	inline 
	TYPENAME base_map< K, T, Pr, M >::_node* 
	_lbound(		const K& k								//!< key
					) const;
	//! \brief finds upper bound node by key
   	inline 
	TYPENAME base_map< K, T, Pr, M >::_node* 
	_ubound(		const K& k								//!< key
					) const;
	//! \brief gets the left most node
    inline 
	TYPENAME 
	base_map< K, T, Pr, M >::_node*& 
	lmost();
	//! \brief gets the left most node (const)
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::_node*& 
	lmost() const;
	//! \brief gets the right most node
    inline 
	TYPENAME 
	base_map< K, T, Pr, M >::_node*& 
	rmost();
    //! \brief gets the right most node (const_
    inline 
	TYPENAME 
	base_map< K, T, Pr, M >::_node*& 
	rmost() const;
    //! \brief gets the root node
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::_node*& 
	root();
    //! \brief gets the root node (const)
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::_node*& 
	root() const;
    inline 
	TYPENAME 
	base_map< K, T, Pr, M >::_node* 
	head();
	//! \brief gets the head node (const)
	inline 
	TYPENAME 
	base_map< K, T, Pr, M >::_node* 
	head() const;
	//! \brief left rotate node
	inline 
	void 
	_lrotate(		TYPENAME base_map< K, T, Pr, M >::_node* x //!< pointer to node
					);
	//! \brief right rotate node
	inline 
	void 
	_rrotate(		TYPENAME base_map< K, T, Pr, M >::_node* x //!< pointer to node
					);
	//! \brief constructs value on pre-allocated memory
	inline 
	void 
	_consval(		T* p,									//!< pointer to pre-allocated memory
					const T& v								//!< value
					);
	//! \brief constructs key on pre-allocated memory
	inline 
	void 
	_conskey(		K* p,									//!< pointer to pre-allocated memory
					const K& k								//!< key
					);
	//! \brief destroys value on pre-allocated memory
	inline 
	void 
	_destval(		T* p									//!< pointer to pre-allocated memory
					);
	//! \brief destroys key on pre-allocated memory
	inline 
	void 
	_destkey(		K* p									//!<  pointer to pre-allocated memory
					);

	
	Pr				key_compare;							//!< predicate
	TYPENAME base_map< K, T, Pr, M >::_node_	_head;		//!< head
	size_t			_size;									//!< size
};

//////////////////////////////////////////////////////////////
//! \class _map
//! \brief all memory allocations are on the external allocators
template < class K, class T, class A = byte_allocator, class Pr = less< K >, bool M = false >
class _map : public base_map< K, T, Pr, M >
{
	//! \brief private assign operator
	_map< K, T, A, Pr, M >& operator=(const _map< K, T, A, Pr, M >& x);
public:
	//! \brief constructor
	explicit 
	_map< K, T, A, Pr, M >(const Pr& pr = Pr()				//!< predicate
					);
	//! \brief copy constructor
	_map< K, T, A, Pr, M >(const _map< K, T, A, Pr, M >& x //!< input
							);
	//! \brief destructor
	~_map< K, T, A, Pr, M >();
	//! \brief assigns external map
	//! external allocator is required
	inline 
	void 
	assign(			A& allocator_,							//!< external allocator
					const _map< K, T, A, Pr, M >& x		//!< input map
					);
	//! \brief inserts key/value
	//! external allocator required
	/*
	inline 
	TYPENAME 
	_map< K, T, A, Pr, M >::iterator 
	insert(			A& allocator_,							//!< external allocator
					const K& k,								//!< key
					const T& v								//!< value
					);
	*/
	inline 
	TYPENAME 
	_map< K, T, A, Pr, M >::pairib_t 
	insert(			A& allocator_,							//!< external allocator
					const K& k,								//!< key
					const T& v								//!< value
					);

	//! \brief erases node by iterator
	//! erase for byte_allocator, which can not deallocate random items
	inline 
	TYPENAME _map< K, T, A, Pr, M >::iterator 
	erase(			TYPENAME _map< K, T, A, Pr, M >::iterator p //!< iterator
					);
	//! \brief erases node by iterator
	//! node_allocator can deallocate random items
	inline 
	TYPENAME 
	_map< K, T, A, Pr, M >::iterator 
	erase(			node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& allocator_, //!< external allocator
					TYPENAME _map< K, T, A, Pr, M >::iterator p	//!< iterator
					);

	//! \brief erases node by key, no memory deallocation
	inline 
	void 
	erase(			const K& k								//!< key
					);
	//! \brief erases node by key with memory deallocation on external allocator
	inline 
	void 
	erase(			node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& allocator_, //!< external allocator
					const K& k								//!< key
					);
	//! \brief erases the range of iterators no memory deallocation
	inline 
	TYPENAME 
	_map< K, T, A, Pr, M >::iterator 
	erase(			TYPENAME _map< K, T, A, Pr, M >::iterator first, //!< first iterator
					TYPENAME _map< K, T, A, Pr, M >::iterator last //!< last iterator
					);
	//! \brief erases the range of iterators with memory deallocaton on external allocator
	inline 
	TYPENAME _map< K, T, A, Pr, M >::iterator 
	erase(			node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& allocator_, //!< external allocator
					TYPENAME _map< K, T, A, Pr, M >::iterator first, //!< first iterator
					TYPENAME _map< K, T, A, Pr, M >::iterator last //!< last iterator
					);
protected:
	//! \brief removes node, no memory deallocation
	inline 
	void 
	_erase(			TYPENAME _map< K, T, A, Pr, M >::_node* x //!< pointer to node
					);
	//! \brief removes node with memory deallocation on external allocator
	inline 
	void 
	_erase(			node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& allocator_, //!< external allocator
					TYPENAME _map< K, T, A, Pr, M >::_node* x //!< pointer to node
					);
	//! \brief creates new node on external allocator and insert it
	inline 
	TYPENAME 
	_map< K, T, A, Pr, M >::iterator 
	_insert(		A& _allocator,							//!< external allocator
					bool left,								//!< left flag
					TYPENAME _map< K, T, A, Pr, M >::_node* w, //!< target node
					const K& k,								//!< key
					const T& v								//!< value
					);
	//! \brief creates new node on external allocator (byte_allocator)
	inline 
	TYPENAME 
	_map< K, T, A, Pr, M >::_node* 
	_buynode(		byte_allocator& _allocator,				//!< external allocator	
					TYPENAME  _map< K, T, A, Pr, M >::_node* p, //!< parent node
					TYPENAME _map< K, T, A, Pr, M >::redblack c, //!< node color
					TYPENAME _map< K, T, A, Pr, M >::nodetype t //!< node type
					);
	//! \brief creates new node on external allocator (node_allocator)
	inline 
	TYPENAME 
	_map< K, T, A, Pr, M >::_node* 
	_buynode(		node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& _allocator, //!< external allocator	
					TYPENAME _map< K, T, A, Pr, M >::_node* p, //!< parent node
					TYPENAME _map< K, T, A, Pr, M >::redblack c, //!< node color
					TYPENAME _map< K, T, A, Pr, M >::nodetype t //!< node type
					);
	//! \brief destroys node and returns memory to external allocator
	inline 
	void 
	_freenode(		node_allocator< TYPENAME base_map< K, T, Pr, M >::_node >& _allocator, //!< external allocator
					TYPENAME _map< K, T, A, Pr, M >::_node* s //!< pointer to node
					);
};

/////////////////////
//! \class map
//! \brief impelments map with internal node allocator
template < class K, class T, class Pr = less< K >, bool M = false >
class map : public base_map< K, T, Pr, M >
{
public:
	//! \brief constructor
	explicit 
	map< K, T, Pr, M >(	const Pr& pr = Pr(),				//!< predicate
					size_t size = os_def_size				//!< number of element on one page of allocator
					);
	//! \brief copy constructor
    map< K, T, Pr, M >(const map< K, T, Pr, M >& x);
	//! \brief destructor
	~map< K, T, Pr, M >();
	//! \brief assign operator
	map< K, T, Pr, M >& operator=(const map< K, T, Pr, M >& x);
	//! \brief inserts new element key/value
/*
	inline 
	TYPENAME map< K, T, Pr, M >::iterator 
	insert(			const K& k,								//!< key
					const T& v								//!< value
					);
*/
	inline 
	TYPENAME map< K, T, Pr, M >::pairib_t 
	insert(			const K& k,								//!< key
					const T& v								//!< value
					);
	//! \brief erases element by iterator
	inline 
	TYPENAME map< K, T, Pr, M >::iterator 
	erase(			TYPENAME map< K, T, Pr, M >::iterator p //!< iterator
					);
	//! \brief erases element by key
	inline 
	void 
	erase(			const K& k								//!< key
					);
	//! \brief clears map
	inline 
	void 
	clear();

protected:
	//! \brief the range of ierators
	inline 
	TYPENAME map< K, T, Pr, M >::iterator 
	erase(			TYPENAME map< K, T, Pr, M >::iterator first, //!< first
					TYPENAME map< K, T, Pr, M >::iterator last //!< last
					);
	//! \brief erases a node
	inline 
	void 
	_erase(			TYPENAME map< K, T, Pr, M >::_node* x	//!< pointer to node
					);
	//! \brief inserts a new node
	inline 
	TYPENAME 
	map< K, T, Pr, M >::iterator 
	_insert(		bool left,								//!< left flag 
					TYPENAME map< K, T, Pr, M >::_node* w,	//!< target node
					const K& k,								//!< key
					const T& v								//!< value
					);
	//! \brief create a new node on internal allocator
	inline 
	TYPENAME 
	map< K, T, Pr, M >::_node* 
	_buynode(		TYPENAME map< K, T, Pr, M >::_node* p,	//!< parent node
					TYPENAME map< K, T, Pr, M >::redblack c, //!< node color
					TYPENAME map< K, T, Pr, M >::nodetype t //!< node type
					);
	//! \brief destroys node
	inline 
	void 
	_freenode(		TYPENAME map< K, T, Pr, M >::_node* s	//!< pointer to node
					);
	
	node_allocator< TYPENAME base_map< K, T, Pr, M >::_node > _allocator; //!< internal node allocator
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_map_h_

