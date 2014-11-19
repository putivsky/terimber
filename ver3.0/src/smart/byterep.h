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

#ifndef _terimber_byterep_h_
#define _terimber_byterep_h_

#include "base/memory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class byte_tank
//! \brief class supports byte memory management
//! allocated block will have 4 hidden leading bytes 
//! 1st byte = backet number [4-12], 2nd - byte = offset/state [0-255], 3rd-4th bytes = page number [0-64K], 
//! for allocation out of backet

template < unsigned char N >
class byte_tank
{
	//! \typedef rooms_type_t
	//! \brief 64 bits
	typedef ub8_t rooms_type_t;	
public:
	//! \enum BACKET
	//! \brief backet constants
	enum BACKET 
	{ 
		LENGTH = (1<<N) + 4,								//!< length of backet chunk
		ROOMS = 8*sizeof(rooms_type_t)						//!< availbale chunks
	};

	//! \brief constructor
	byte_tank(		byte_allocator& all						//!< external allocator
					);

	//! \brief checks if the backet is full
	inline 
	bool 
	full() const;
	//! \brief checks if the backet is empty
	inline 
	bool 
	empty() const ;
	//! \brief gets available chunk
	inline 
	char* 
	get_chunk();
	//! \brief returns back chunk
	inline 
	void 
	put_chunk(		char* ptr								//!< pointer to the chunk
					);
	//! \brief ready to reuse chunks 
	inline 
	void 
	reset();

private:	
	char*			_head;									//!< memory pointer
	rooms_type_t	_states;								//!< bitset, bit = 1 - busy, bit = 0 free
};

//! \class byte_backet
//! \brief paged backet of chunks
template < unsigned char N, ub4_t M >
class byte_backet
{
public:
	//! \brief constructor
	byte_backet();
	//! \brief destructor
	~byte_backet();
	//! \brief allocates chunk
	inline 
	char* 
	allocate();
	//! \brief deallocates (mark for reusing) chunk
	inline 
	void 
	deallocate(		char* ptr								//!< pointer to chunk
					);
	//! \brief ready to reuse memory
	inline 
	void 
	reset();
	//! \brief releases back to OS unused memory
	inline 
	void 
	release();

private:
	byte_allocator		_all;								//!< internal allocator
	byte_tank< N >*		_pages[M];							//!< array of pages
	ub4_t				_page;								//!< page counter
};

//! \class byte_repository
//! \brief byte repository class 
//! for allocation memory by fixed chunks
class byte_repository
{
	//! class must be created dynamically due to the size
	friend class byte_repository_factory;
public:
	//! \brief allocates len bytes
	inline 
	char* 
	allocate(		size_t len								//!< size
					);
	//! \brief deallocates (reuse) memory
	inline 
	void 
	deallocate(		char* ptr								//!< memory pointer
					);

	//! \brief release memory back to OS
	inline
	void
	reset();

private:
	//! \brief constructor
	byte_repository() 
	{
	}

	// page = (1<<N + 4)*64*M
	byte_backet< 4, 64*1024 >	_backet4;					//!< page = (16 + 4)*64*64K ~ 64M
	byte_backet< 5, 32*1024 >	_backet5;					//!< page = (32 + 4)*64*32K ~ 64M
	byte_backet< 6, 16*1024 >	_backet6;					//!< page = (64 + 4)*64*16K ~ 64M
	byte_backet< 7, 8*1024 >	_backet7;					//!< page = (128 + 4)*64*8K ~ 64M
	byte_backet< 8, 4*1024 >	_backet8;					//!< page = (256 + 4)*64*4K ~ 64M
	byte_backet< 9, 2*1024 >	_backet9;					//!< page = (512 + 4)*64*2K ~ 64M
	byte_backet< 10, 1*1024 >	_backet10;					//!< page = (1024 + 4)*64*1K ~ 64M
	byte_backet< 11, 512 >		_backet11;					//!< page = (2048 + 4)*64*512 ~ 64M
	byte_backet< 12, 256 >		_backet12;					//!< page = (4096 + 4)*64*256 ~ 64M
	byte_backet< 13, 128 >		_backet13;					//!< page = (8192 + 4)*64*128 ~ 64M
};

//! \class byte_repository_factory
//! \brief class factory to create byte_repository
class byte_repository_factory
{
public:
	//! \brief constructor
	byte_repository_factory();
	//! \brief destructor
	~byte_repository_factory();
	//! \brief checks if the repository is allocated
	inline 
	bool 
	is_ready() const;
	//! \brief returns the pointer to the repository
	inline 
	byte_repository* operator->();
private:
	byte_repository*	_repository;						//!< repository pointer
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_byterep_h_
