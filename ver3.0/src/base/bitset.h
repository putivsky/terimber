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

#ifndef _terimber_bitset_h_
#define _terimber_bitset_h_

#include "base/vector.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class base_bitset
//! \brief provides the bit functionality
class base_bitset
{
	//! \enum en_bitset
	enum en_bitset
	{
		bs_bits_in_byte = 8,								//!< bits in byte
		bs_offset = 3,										//!< power of 2 for bits in bytes
		bs_mask = bs_bits_in_byte - 1						//!< mask for one byte
	};

public:

	//! \brief sets bit value by index
	inline 
	bool 
	set(			size_t index,							//!< index 
					bool value								//!< value
					);
	//! \brief returns bit value by index
	inline 
	bool 
	get(			size_t index							//!< index
					) const;
	//! \brief finds first zero bit 
	inline 
	bool 
	find_first_free(size_t& index							//!< index
					) const;

	//! \brief checks if set is empty
	inline
	bool 
	empty() const;
	//! \brief equal operator
	inline
	bool 
	operator==(const base_bitset& x) const;
	//! \brief non equal operator
	inline
	bool 
	operator!=(const base_bitset& x) const;
	//! \brief operator |=
	inline
	base_bitset& 
	operator|=(const base_bitset& x);
	//! \brief operator &=
	inline
	base_bitset&
	operator&=(const base_bitset& x);
	//! \brief sets all bits to zero
	inline
	void
	reset();
	//! \brief operator<
	inline
	bool operator<(const base_bitset& x) const;

protected:
	//! \brief constructor
	inline 
	base_bitset(	size_t capacity							//!< capacity
					);
	//! \brief returns the min bytes required to accomodate bits
	static 
	inline 
	size_t 
	get_byte_len(	size_t bitlen							//!< bit length
					);
	//! \brief returns required offset in bytes for index										);
	static 
	inline 
	size_t 
	offset(			size_t index							//!< index
					);
	//! \brief returns the mask for reminder of bits by index
	static 
	inline 
	unsigned char 
	mask(			size_t index							//!< index
					);

protected:
	size_t						_capacity;					//!< capacity - max bits stored in set
	unsigned char*				_bits;						//!< array of bytes
};

//! \class _bitset
//! \brief bitset with external allocator
class _bitset : public base_bitset
{
public:
	//! \brief default constructor
	inline 
	_bitset();
	//! \brief constructor
	inline 
	_bitset(		TERIMBER::byte_allocator& all,			//!< external allocator
					size_t capacity							//!< capacity
					);
	//! \brief assigns another set with the same length
	inline
	_bitset& 
	operator=(const _bitset& x);
	//! \brief assigns another set with external allocator
	inline
	_bitset& 
	assign(			byte_allocator& all,					//!< external allocator
					const _bitset& x						//!< source
					);
	//! \brief changes capacity
	inline
	_bitset&
	resize(			byte_allocator& all,					//!< external allocator
					size_t capacity							//!< capacity
					);
	//! \brief sets capacity to zero
	inline
	void
	clear();
};

//! \class bitset
class bitset : public base_bitset
{
public:
	//! \brief default constructor
	inline 
	bitset();

	//! \brief constructor
	inline 
	bitset(			size_t capacity							//!< capacity
					);
	//! \brief destructor
	inline 
	~bitset();
	//! \brief assigns another set with the same length
	inline
	bitset& operator=(const bitset& x);
	inline
	//! \brief changes capacity
	bitset&
	resize(			size_t capacity							//!< capacity
					);
	//! \brief sets capacity to zero
	inline
	void
	clear();
};


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_stack_h_

