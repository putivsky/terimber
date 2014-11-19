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

#ifndef _terimber_memory_h_
#define _terimber_memory_h_

#include "allinc.h"
#include "base/proto.h"
#include <stdint.h>

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// all classes that support management of memory
// for objects with a fixed or dynamic size
// must be used only in one thread
// NO multithread protection

//! \brief rounding on external mask
#define ALIGNED_MASK_SIZEOF(s, m) ((s+(m-1))&~(m-1))
//! \brief macros for rounding memory object size
#define ALIGNED_SIZEOF(s) ALIGNED_MASK_SIZEOF(s, sizeof(void*))

//! \class mem_chunk
//! \brief class is used for a linked list of memory chunks
class mem_chunk
{
public:
  enum mem_chunk_alignment 
  {
    MMC_ALIGN = sizeof(void*)
  };

  size_t			  _chunk_size;					  //!< the size of chunk
	mem_chunk*		_next_chunk;						//!< pointer to next memory chunk
	uint8_t	      _mem[MMC_ALIGN];	      //!< pointer to allocated memory
};

//! \class byte_allocator
//! \brief high performance class for allocation
class byte_allocator
{
	//! \brief private copy constructor
	byte_allocator(const byte_allocator& x);
	//! \brief private assign operator
	byte_allocator& operator=(const byte_allocator& x);
public:
	//! \brief constructor
	byte_allocator(	size_t capacity = os_def_size			//!< chunk capacity
					);
	//! \brief destructor
	~byte_allocator();
	//! \brief clears all memory
	void 
	clear_all(		bool secure = false						//!< rewrite memory before release
					);
	//! \brief clears only extra chunks
	void 
	clear_extra(	bool secure = false						//!< rewrite memory before release
					);
	//! \brief resets allocator for reusing the memory again
	inline 
	void 
	reset(			bool secure = false						//!< rewrite memory before release
					);
	//! \brief allocates required array of bytes
	inline 
	void* 
	allocate(		size_t size								//!< size if bytes
					);

	//! \brief does nothing
	// doesn't support deallocation by pointer
	// everything or nothing
	inline 
	void 
	deallocate(		void*
					);
	//! \brief returns the capacity of chunk
	inline 
	size_t 
	capacity() const;
	//! \brief returns the count of chunks
	inline 
	size_t 
	count() const;
private:
	//! \brief allocates new chunk of requested size
	void* 
	new_chunk(		size_t size								//!< size in bytes
					);
	//! \brief tries to find the next chunk in a linked list with requested size
	void* 
	next_chunk(		size_t size								//!< size in bytes
					);
protected:
	const size_t	 _capacity;								//!< chunk default capacity
	size_t			   _count;									//!< current count of allocated chunks of memory
  uint8_t*       _free_pos;								//!< start position of available memory in current chunk
	mem_chunk*		 _start_chunk;						//!< start chunk
	mem_chunk*		 _using_chunk;						//!< current chunk
};

//! \class chunk_stack
//! \brief keeps free chunks of memory for reusing
class chunk_stack
{
public:
	//! \brief constructor
	inline 
	chunk_stack();
	//! \brief clears stack
	inline 
	void 
	clear();
	//! \brief pushes new chunk
	inline 
	void 
	push(			size_t* chunk							//!< pointer to new chunk
					);
	//! \brief removes the head chunk
	inline 
	size_t* 
	pop();
	//! \brief returns the head chunk
	inline 
	size_t* 
	top();
	//! \brief returns the head chunk
	inline 
	bool 
	empty();

private:
	size_t* _head;											//!< pointer to the head chunk
};

//! \class rep_allocator
//! \brief class support "deallocation" actually memory reusing
class rep_allocator : public byte_allocator
{
	//! \brief private copy constructor
	rep_allocator(const rep_allocator& x);
	//! \brief private assign operator
	rep_allocator operator=(const rep_allocator& x);
protected:
	//! \brief constructor
	rep_allocator(	size_t capacity = os_def_size			//!< default capacity
					);
	//! \brief destructor
	~rep_allocator();

public:
	//! \brief clears all chunks
	inline 
	void 
	clear_all(		bool secure = false						//!< rewrite memory before release
					);
	//! \brief clears extra chunks
	inline 
	void 
	clear_extra(	bool secure = false						//!< rewrite memory before release
					);
	//! \brief resets allocator for memory reusing
	inline 
	void 
	reset(			bool secure = false						//!< rewrite memory before release
					);
	//! \brief pushes pointer to object back to repository stack
	inline 
	void 
	deallocate(		void* p									//!< pointer to memory
					);
protected:
	chunk_stack	_rep;										//!< repository stack of chunks
};

//! \class node_allocator
//! \brief high performance template class for allocation
//! one object at one time
template < class T >
class node_allocator : public rep_allocator
{
public:
	//! \brief constructor
	node_allocator< T >(size_t capacity = os_def_size		//!< default capacity
						);
	//! \brief allocates one object
	inline 
	T* 
	allocate();
};

//! \class array_allocator
//! \brief template class for allocation of the array of objects
//! it seems like this class covers the functionality of the previous one
//! but the previous class (node_allocator) has better performance
template < class T >
class array_allocator : public rep_allocator
{
public:
	//! \brief constructor
	array_allocator< T >(size_t capacity = os_def_size		//!< default capacity
					);
	//! \brief destructor
	~array_allocator< T >();
	//! \brief allocates array of n elements
	inline 
	T* 
	allocate(		size_t n								//!< number of elements
					);

private:
	//! \brief finds the array of requested length in repository
	inline
	T* 
	pop(			size_t n								//!< number of elements
					);
};

//! \class byte_allocator_creator
//! \brief creator for byte_allocator
class byte_allocator_creator : public proto_creator< byte_allocator_creator, byte_allocator, size_t >
{
public: 
	//! \brief creates object
	static 
	byte_allocator* 
	create(			size_t size								//!< default capacity
					);
	//! \brief finds allocator with specified or bigger chunk
	static 
	bool 
	find(			byte_allocator* obj,					//!< object
					size_t size								//!< chunk size
					);
	//! \brief clears extra chunks allocator
	static 
	void 
	back(			byte_allocator* obj,					//!< object
					size_t
					);
	//! \brief clears allocator
	static 
	void 
	deactivate(		byte_allocator* obj,					//!< object
					size_t
					);
};

//! \class node_allocator_creator
//! \brief creator for node_allocator
template < class T >
class node_allocator_creator : public proto_creator< node_allocator_creator< T >, node_allocator< T >, size_t >
{
public:
	//! \brief creates node_allocator
	static 
	node_allocator< T >* 
	create(			size_t size								//!< default chunk capacity
					);
	//! \brief finds node_allocator with the same or bigger chunk capacity
	static
	bool 
	find(			node_allocator< T >* obj,				//!< object
					size_t size								//!< default chunk capacity
					);
	//! \brief clears extra
	static
	void 
	back(			node_allocator< T >* obj,				//!< object
					size_t
					);
	//! \brief clears allocator
	static
	void 
	deactivate(		node_allocator< T >* obj,				//!< object
					size_t
					);
};

//! \class array_allocator_creator
//! \brief creator for array_allocator
template < class T >
class array_allocator_creator : public proto_creator< array_allocator_creator< T >, array_allocator< T >, size_t >
{
public:
	//! \brief creates array_allocator
	static 
	array_allocator< T >* 
	create(			size_t size								//!< default chunk capacity
					);
	//! \brief finds allocator with the same or bigger chunk capacity
	static
	bool 
	find(			array_allocator< T >* obj,				//!< object
					size_t size								//!< chunk capacity
					);
	//! \brief clears extra
	static
	void 
	back(			array_allocator< T >* obj,				//!< object
					size_t
					);
	//! \brief clears
	static
	void 
	deactivate(		array_allocator< T >* obj,				//!< object
					size_t
					);
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_memory_h_

