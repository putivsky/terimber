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

#ifndef _terimber_common_h_
#define _terimber_common_h_

#include "base/string.h"
#include "base/template.h"
#include "base/vector.h"
#include "base/date.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \enum numeric_radix
//! \brief radix enum
enum numeric_radix
{
	RADIX2 = 2,												//!< base 2 - binary
	RADIX8 = 8,												//!< base 2 - octet
	RADIX10 = 10,											//!< base 10 - decimal
	RADIX16 = 16											//!< base 16 - heximal
};

//! \brief checks ptr on null and throw exception
inline 
void* 
check_pointer(		void* ptr								//!< pointer to allocated memory
					);
//! \brief transforms byte to lower case size_t
inline 
size_t 
get8bits(			unsigned char x							//!< input byte
					);
//! \brief transforms 2 byte word to lower case size_t
inline 
size_t 
get16bits(			const unsigned char* x					//!< input pointer to array of bytes
					);
//! \brief produces hash
template < class T >
inline 
size_t 
do_hash(			const T* x,							//!< input pointer to array
					size_t len = os_minus_one				//!< the length of input array
					);
//! \brief makes a copy of string using allocator memory
template < class T >
inline 
T* 
copy_string(		const T* x,							//!< input string
					byte_allocator& _allocator,				//!< external allocator
					size_t length = os_minus_one			//!< the length of input string
					);
//! \brief converts heximal string to preallocated binary array 
template < class T >
inline 
bool 
hex_to_binary(		ub1_t* dest,							//!< [out] array of bytes
					const T* x,								//!< input string
					size_t len = os_minus_one				//!< the length of input string
					);
//! \brief converts one radix symbol to byte
template < class T >
inline 
bool 
symbol_to_byte(		ub1_t &dest,							//!< [out] byte
					T x,									//!< input char
					numeric_radix radix_ = RADIX16			//!< radix
					);
//! \brief converts radix symbols to byte
template < class T >
inline 
bool 
hex_to_byte(		ub1_t &dest,							//!< [out] byte
					const T* x,							//!< input string
					numeric_radix radix_ = RADIX16			//!< radix
					);
//! \brief converts binary array to preallocated heximal string
template < class T >
inline 
void 
binary_to_hex(		T* dest,								//!< [out] preallocated string
					const ub1_t* x						//!< input array of bytes - internal format
					);
//! \brief converts byte to preallocated heximal string
template < class T >
inline 
void 
byte_to_hex(		T* dest,								//!< [out] preallocated string
					ub1_t x								//!< input byte
					);
//! \brief converts byte to symbol
template < class T >
inline 
void 
byte_to_symbol(		T& dest,								//!< [out] char
					ub1_t x								//!< input byte
					);
//! \brief converts guid to string - no dashes or curly brackets, just heximal 16 bytes representation
template < class T >
inline 
T* 
guid_to_string(		T* dest,								//!< [out] preallocated string
					const guid_t& x						//!< guid
					);
//! \brief converts string to guid - no dashes or curly brackets, just heximal 16 bytes representation
template < class T >
inline 
bool 
string_to_guid(		guid_t& dest,							//!< [out] guid
					const T* x							//!< input string
					);
//! \brief compares the two values where comparision operators are defined
template < class T >
inline 
int 
simple_compare(		const T& v1,							//!< first value
					const T& v2								//!< second value
					);
//! \brief binary compare 
template < class T >
inline 
int 
memory_compare(		const T& v1,							//!< first value
					const T& v2								//!< second value
					);
//! \brief compares Terimber binaries - internal format
inline 
int 
memory_compare_binary(const ub1_t* v1,						//!< first value
					const ub1_t* v2							//!< second value
					);
//! \brief compares simple values by pointers
template < class T >
inline 
int 
simple_compare_ptr(	const T* v1,							//!< first value
					const T* v2								//!< second value
					);
//! \brief strings using external function
template < class T, class F >
inline 
int 
string_compare(		const T& v1,							//!< first value
					const T& v2,							//!< second value
					F fn									//!< comaprision function
					);
//! \brief produces hash value for terimber_xml_value enum
inline 
size_t 
do_hash(			vt_types type,							//!< value type
					const terimber_xml_value& x			//!< input value
		);
//! \brief compares the two terimber_xml_value values of the same type with additional restrictions 
inline 
int 
compare_value(		vt_types type,							//!< value type
					const terimber_xml_value& first,		//!< first value
					const terimber_xml_value& second,		//!< second value
					bool use_hash,							//!< using hash for string
					bool case_insensitive					//!< compare strings low case
					);
//! \brief copies terimber_xml_value using external allocator
inline 
terimber_xml_value 
copy_value(			vt_types type,							//!< value type
					const terimber_xml_value& x,			//!< input value
					byte_allocator& allocator_				//!< external allocator
					);
//! \brief converts string representation to terimber_xml_value using external allocator
terimber_xml_value 
parse_value(		vt_types type,							//!< value type
					const char* x,						//!< input string
					size_t len_ = os_minus_one,				//!< input string length
					byte_allocator* allocator_ = 0			//!< external allocator
					);
//! \brief converts terimber_xml_value to string representation using external allocator
const char* 
persist_value(		vt_types type,							//!< value type
					const terimber_xml_value& x,			//!< input value
					byte_allocator* allocator_				//!< external allocator
					);

//! \typedef mem_pool_t
//! \brief memory pool of allocators
typedef pool< byte_allocator_creator > mem_pool_t;
//! \brief equal operator
inline 
bool 
operator==(			const guid_t& first,					//!< first value
					const guid_t& second					//!< second value
					);
//! \brief non equal operator
inline 
bool 
operator!=(			const guid_t& first,					//!< first value
					const guid_t& second					//!< second value
		   );
//! \brief less operator
inline 
bool 
operator<(			const guid_t& first,					//!< first value
					const guid_t& second					//!< second value
		  );
//! \brief greater operator
inline 
bool 
operator>(			const guid_t& first,					//!< first value
					const guid_t& second					//!< second value
		  );

// when we can't estimate in advance how many bytes 
// will be persisted from input stream into 'name', 'value' or 
// something else that looks like one block of sequence of byte
// then we can use paged buffer class
//! \typedef paged_vector_t
//! \brief vector of bytes
typedef _vector< ub1_t >		paged_vector_t;
//! \typedef paged_store_t
//! \brief list of pages
typedef _list< paged_vector_t >	paged_store_t;

//! \class paged_buffer
//! \brief high-performance resizable buffer of bytes
class paged_buffer
{
public:
	//! \brief constructor
	paged_buffer(	byte_allocator& data_allocator,			//!< allocator for internal data pages 
					byte_allocator& tmp_allocator,			//!< allocator for temporary needs 
					size_t page_size						//!< page size
					);
	~paged_buffer();
	
	//! \brief pushes symbol to buffer
	inline 
	paged_buffer& 
	operator<<(		ub1_t symbol							//!< input symbol
					);
	//! \brief pushes string to buffer
	inline 
	paged_buffer& 
	operator<<(		const char* x							//!< input string
					);
	//! \brief pushes bytes to buffer
	inline 
	paged_buffer& 
	append(			const ub1_t* x,						//!< input byte buffer
					size_t size								//!< size of buffer
					);

	//! \brief persists paged blocks into one permanent block
	inline 
	const char* 
	persist();

	//! \brief persists paged blocks into one permanent block
	inline 
	const ub1_t* 
	persist(		size_t& size_							//!< [out] the size of buffer
					);

	//! \brief persists paged blocks into one permanent block (internal allocator)
	//! if internally only one block has been allocated
	//! then class dosn't make a copy - it simply returns the pointer
	inline 
	void 
	reset();

	//! \brief calculates the size of persisted chars
	inline 
	size_t 
	size() const;

	//! \brief shares memory resource for a while
	inline 
	byte_allocator& 
	get_tmp_allocator();
	
	//! \brief returns page size
	inline 
	size_t 
	get_page_size() const;

private:
	//! \brief adds page
	void 
	add_page();
	//! \brief persists paged blocks into one permanent block
	const ub1_t* 
	_persist(		size_t& size_							//!< [out] size of the buffer
					);
	//! \brief  persists paged blocks into null terminated string
	const char*
	_persist();

private:
	byte_allocator&				_data_allocator;			//!< internal allocator for depot
	byte_allocator&				_tmp_allocator;				//!< internal allocator for data
	paged_store_t				_buffer;					//!< paged buffer
	size_t						_size;						//!< size of the one page of buffer
	size_t						_pos;						//!< current position inside the page
	ub1_t*						_ptr;						//!< pointer to current page
	ub1_t*						_primary;					//!< head of the pages
};


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_common_h_

