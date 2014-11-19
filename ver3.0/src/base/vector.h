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

#ifndef _terimber_vector_h_
#define _terimber_vector_h_

#include "base/memory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class base_vector
//! \brief provides the vector functionality
template < class T >
class base_vector
{
protected:
	//! \brief constructor
	base_vector();
	//! \brief destructor
	~base_vector();
	//! \brief copy constructor
	base_vector(const base_vector& x);
public:
	//! random iterators are supported
	//! \typedef iterator
	//! \brief iterator
	typedef T* iterator;
	//! \typedef const_iterator
	//! \brief const iterator
	typedef const T* const_iterator;
	//! \brief check empty
	inline 
	bool 
	empty() const;
	//! \brief returns size of vector
	inline 
	size_t 
	size() const;
	//! \brief operator[] const
	//! vector doesn't check boundaries
	inline const T& operator[](size_t p) const;
	//! \brief operator[]
	//! vector doesn't check boundaries
	inline T& operator[](size_t p);
	//! \brief returns begin const iterator
	inline const_iterator begin() const;
	//! \brief returns begin iterator
	inline iterator begin();
	//! \brief returns end const iterator
	inline const_iterator end() const;
	//! \brief returns end iterator
	inline iterator end();
protected:
	T*			_first;										//!< stores pointer to the first element
	size_t		_length;									//!< length of vector
};

//! \class _vector
//! \brief supports vector with external allocator
//! size of empty vector is 8 byte
template < class T, class A = byte_allocator >
class _vector : public base_vector< T >
{
public:
	//! \brief constructor
	_vector< T, A >();
	//! \brief destructor
	~_vector< T, A >();
	//! \brief copy constructor
	//! copies only pointer to first element and length
	//! no memory reallocation or copy
	_vector< T, A >(const _vector< T, A >& x);
	//! \brief assign operator
	//! copies only pointer to first element and length
	//! no reallocation or copying of memory
	_vector< T, A >& operator=(const _vector< T, A >& x);
	//! \brief assign range of iterators
	inline 
	bool 
	assign(			A& allocator_,							//!< external allocator
					TYPENAME _vector< T, A >::const_iterator first, //!< first iterator
					TYPENAME _vector< T, A >::const_iterator last	//!< last iterator
					);
	//! \brief assigns n equal values
	inline 
	bool 
	assign(			A& allocator_,							//!< external allocator
					size_t n,								//!< number of elements
					const T& x = T()						//!< object 
					);
	//! \brief assigns another vector 
	//! copies values
  	inline 
	bool 
	assign(			A& allocator_,							//!< external allocator
					const _vector< T, A >& x				//!< input vector
					);
	//! \brief expands boundaries if needed
	//! must be defined as a copy constructor for template T
	inline 
	bool 
	resize(			A& allocator_,							//!< external allocator
					size_t n,								//!< new vector length
					const T& x								//!< input value
					);		
	//! \brief expands boundaries if needed
	// with zeroing additional memory, no constructors invoked
	inline 
	bool 
	resize(			A& allocator_,							//!< external allocator
					size_t n								//!< new vector length
					);

	//! \brief reduces boundaries if needed
	//! no memory deallocation, no destructors invoked
	inline 
	void 
	reduce(			size_t n								//!< new vector length
					);

	//! \brief clear vector
	// no memory deallocation
	inline 
	void 
	clear();

	//! \brief clears vector and deallocates memory on external allocator
	inline 
	void 
	clear(			array_allocator< T >& allocator_		//!< external allocator
					);

private:
	//! \brief reduces size of vector
	//! no memory deallocation
	inline 
	void 
	_reduce(		size_t n								//!< new vector size
					);
	//! \brief allocates array of objects on external byte_allocator
	inline 
	T* 
	_buynodes(		byte_allocator& allocator_,				//!< external allocator
					size_t n								//!< new vector size
					);
	//! \brief allocates array of objects on external array_allocator
	inline 
	T* 
	_buynodes(		array_allocator< T >& allocator_,		//!< external allocator
					size_t n								//!< new array size
					);
};

//! \class vector
//! \brief class supports vector with internal allocator
template < class T >
class vector : public base_vector< T >
{
public:
	//! \brief constructor
	vector< T >(	size_t capacity = os_def_size			//!< chunk size for array_allocator
					);
	//! \brief destructor
	~vector< T >();
	//! \brief copy constructor
	vector< T >(const vector< T >& x);
	//! \brief assign operator
	inline 
	vector< T >& 
	operator=(const vector< T >& x);
	//! \brief assigns range of iterators
	inline 
	bool 
	assign(			TYPENAME vector< T >::const_iterator first, //!< first iterator
					TYPENAME vector< T >::const_iterator last	//!< last iterator
					);
	//! \brief assigns n equal copies
	inline 
	bool 
	assign(			size_t n,									//!< new vector size
					const T& x = T()							//!< object
					);
	//! \brief assigns another vector
  	inline 
	bool 
	assign(			const vector< T >& x						//!< input vector
					);
	//! \brief expands boundaries if needed
	//! must be defined as a copy constructor for template T
	inline 
	bool 
	resize(			size_t n,									//!< new vector size
					const T& x = T()							//!< object
					);
	//! \brief clears vector
	inline 
	void 
	clear();

private:
	//! \brief reduces size of vector
	inline 
	void 
	_reduce(		size_t n									//!< new vector size
					);
	//! \brief allocates array of nodes
	inline 
	T* 
	_buynodes(		size_t n									//!< new vector size
					);
private:
	array_allocator< T >	_allocator;							//!< internal allocator
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_vector_h_

