/*
 * The Software License
 * =================================================================================
 * Copyright (c) 2003-2009 The Terimber Corporation. All rights reserved.
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

#ifndef _terimber_number_h_
#define _terimber_number_h_

#include "base/memory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class room_array
template < class T >
class room_array
{
	//! \enum room_size
	enum room_size { S = sizeof(T) };

public:
	//! \brief constructor
	room_array< T >(size_t size,							//!< size
					byte_allocator* allocator_ = 0			//!< external allocator
					);
	//! \brief copy constructor
	room_array< T >(const room_array< T >& src);
	//! \brief assigns operator
	room_array< T >& operator=(const room_array< T >& src);
	//! \brief destructor
	~room_array< T >();
	//! \brief operator >> moves right n bytes 
	inline 
	room_array< T >& 
	operator>>(		size_t shift							//!< shift
					);
	//! \brief oparator << moves left n bytes
	inline 
	room_array< T >& 
	operator<<(		size_t shift							//!< shift
					);
	//! \brief resizes internal buffer if required
	inline 
	room_array< T >& 
	resize(			size_t size,							//!< resize buffer
					bool clean = false						//!< preserve previous data or not
					);
	//! \brief allocates additional space if any preserving internal data
	inline 
	room_array< T >& 
	reserve(		size_t size								//!< new size
					);
	//! \brief swaps bytes between arrays
	inline 
	void 
	swap(			room_array& src							//!< input array
					);
	//! \brief operator T*
	inline 
	operator T*() 
	{ 
		return _buffer; 
	}
	//! \brief operator const T*
	inline operator const T*() const 
	{ 
		return _buffer; 
	}
	//! \brief retruns the size of buffer
	inline 
	size_t 
	size() const;
	//! \brief fills out internal buffer
	inline 
	room_array< T >& 
	fill(			const T& src,							//!< fill out buffer with src repeating n times
					size_t len								//!< count
					);
	//! \brief copies to internal buffer
	inline 
	room_array< T >& 
	copy(			const T* src,							//!< external buffer			
					size_t len								//!< buffer length
					);
	//! \brief copies from internal buffer
	inline 
	void 
	back(			T* src,									//!< [out] preallocated external buffer
					size_t len								//!< external buffer length
					) const;
	//! \brief clears internal buffer
	inline 
	room_array< T >& 
	clear();
	//! \brief returns pointer to allocator
	inline 
	byte_allocator* 
	get_allocator();
private:
	byte_allocator*	_allocator;								//!< pointer to external allocator
	T*				_buffer;								//!< internal buffer
	size_t			_size;									//!< buffer size
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_number_h_

