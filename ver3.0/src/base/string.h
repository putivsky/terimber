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

#ifndef _terimber_string_h_
#define _terimber_string_h_

#include "base/memory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class base_string
//! \brief supports base string functionality
template < class T > 
class base_string
{
	//! \enum en_base_string_size
	//! \brief calculates char size
	enum 
	en_base_string_size 
	{
		S = sizeof(T)										//!< char size
	};
public:
	//! \brief default constructor
	base_string< T >(byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief destructor
	~base_string< T >();
	//! \brief constructor from another string
	base_string< T >(const T* x,							//!< input string
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief copy constructor
	base_string< T >(const base_string< T >& x);
	//! \brief gets string pointer
	//! makes it inline, Microsoft can't compile this operator outside header file
	inline 
	operator const T*() const 
	{ 
		return _handle; 
	}
	//! \brief assign operator
	base_string< T >& operator=(const base_string< T >& x);
	//! \brief assign operator from string
	base_string< T >& 
	operator=(		const T* x				//!< input string
					);
	//! \brief operator==
	inline 
	bool 
	operator==(const base_string< T >& x) const;
	//! \brief operator!=
	inline 
	bool 
	operator!=(const base_string< T >& x) const;
	//! \brief operator<
	inline 
	bool 
	operator<(const base_string< T >& x) const;
	//! \brief operator<=
	inline 
	bool 
	operator<=(const base_string< T >& x) const;
	//! \brief operator>
	inline 
	bool 
	operator>(const base_string< T >& x) const;
	//! \brief operator>=
	inline 
	bool 
	operator>=(const base_string< T >& x) const;
	//! \brief operator+= 
	inline 
	base_string< T >& 
	operator+=(const base_string< T >& x);
	//! compare operator for input pointers
	//! \brief operator==
	inline 
	bool 
	operator==(		const T* x							//!< input string
					) const;
	//! \brief operator!=
	inline 
	bool 
	operator!=(		const T* x							//!< input string
					) const;
	//! \brief operator<
	inline 
	bool 
	operator<(		const T* x							//!< input string
					) const;
	//! \brief operator<=
	inline 
	bool 
	operator<=(		const T* x							//!< input string
					) const;
	//! \brief operator>
	inline 
	bool 
	operator>(		const T* x							//!< input string
					) const;
	//! \brief operator>=
	inline 
	bool 
	operator>=(		const T* x							//!< input string
					) const;
	//! \brief compare function
	inline 
	int 
	compare(		const T* x,							//!< input string
					size_t size = os_minus_one				//!< string length
					) const;
	//! \brief operator+= for another string
	inline 
	base_string< T >& 
	operator+=(		const T* x							//!< input string
					);
	//! \brief appends string
	inline 
	base_string< T >& 
	append(			const T* x,							//!< input string
					size_t size = os_minus_one				//!< string length
					);
	//! \brief assign string
	//
	inline 
	base_string< T >& 
	assign(			const T* x,							//!< input string
					size_t size = os_minus_one				//!< string length
					);
	//! \brief returns string length
	inline 
	size_t 
	length() const;
	//! \brief reserves memory, be very careful and write no more than allocated chars
	inline 
	T* 
	reserve(		size_t size								//!< string length
					);
	//! \brief gets external allocator specified in constructor, if any
	inline 
	byte_allocator* 
	get_allocator();
private:
	//! \brief compare function
	inline 
	int 
	_compare(		const T* x,							//!< input string
					size_t len = os_minus_one				//!< string length
					) const;
    //! \brief allocates string if need & assign
	inline 
	void 
	_assign(		const T* x,							//!< input string
					size_t len = os_minus_one				//!< string length
					);
	//! \brief appends new string to assigned one previously
	inline 
	void 
	_add(			const T* x,							//!< input string
					size_t len = os_minus_one				//!< string length
					);
	//! \brief allocates string
	inline 
	void 
	_create();
	//! \brief destroys string
	inline 
	void 
	_destroy();
	
private:
	byte_allocator*		_allocator;							//!< external allocator
	T*					_handle;							//!< string buffer		
	size_t				_length;							//!< string length
};

//! \typedef string_t
//! \brief narrow string - usually utf-8
typedef base_string< char >		string_t;
//! \typedef wstring_t
//! \brief wide string - usually utf-16
typedef base_string< wchar_t >	wstring_t;

//! \namespace str_template
//! \brief string utilities
namespace str_template
{
//! \brief template function for string length calculation
template < class T > 
inline 
size_t 
strlen(				const T* x							//!< input string
					);
//! \brief template function for string coping
template < class T > 
inline T* 
strcpy(				T* dest,								//!< destrination buffer
					const T* x,							//!< input string
					size_t len = os_minus_one				//!< string length
					);
//! \brief template function for string comparision
template < class T > 
inline 
int 
strcmp(				const T* dest,							//!< first string
					const T* x,							//!< second string
					size_t len = os_minus_one				//!< number chars to compare
					);
//! \brief template function for string comparision without case narrow string
inline 
int 
strnocasecmp(		const char* dest,						//!< first string
					const char* x,						//!< second string
					size_t len								//!< number chars to compare
					);
//! \brief template function for string comparision without case wide string
inline 
int 
strnocasecmp(		const wchar_t* dest,					//!< first string
					const wchar_t* x,						//!< second string
					size_t len								//!< number chars to compare
					);
//! \brief converts narrow string format to itself
inline 
const char* 
strformat(			char* buf,								//!< [out] buffer
					const char* format						//!< input format
					);
//! \brief converts narrow string format to wide format string
inline 
const 
wchar_t* 
strformat(			wchar_t* buf,							//!< [out] buffer
					const char* format						//!< input format
					);

//! \brief scans only one parameter narrow char
inline 
int 
strscan(			const char* buf,						//!< scan buffer
					size_t len,								//!< buffer length
					const char* format,						//!< format string
					void* res								//!< pointer to argument memory
					);
//! \brief scans only one parameter wide char
inline 
int 
strscan(			const wchar_t* buf,						//!< scan buffer
					size_t len,								//!< buffer length
					const wchar_t* format,					//!< format string
					void* res								//!< pointer to argument memory
					);
//! \brief prints to memory formatted string narrow string
inline 
int 
strprint(			char* buf,								//!< [out] buffer
					size_t len,								//!< buffer length
					const char* format,						//!< format string
					...										//!< input arguments
					);
//! \brief prints to memory formatted string wide string
inline 
int 
strprint(			wchar_t* buf,							//!< [out] buffer
					size_t len,								//!< buffer length
					const wchar_t* format,					//!< format string
					...										//!< input arguments
					);
//! \brief gets the length required for utf-8 to utf-16 conversion
inline 
size_t 
multibyte_to_unicode_len(const char* x,					//!< input string
					size_t len = os_minus_one				//!< string length
					);
//! \brief gets the length required for utf-16 to utf-8 conversion
inline 
size_t 
unicode_to_multibyte_len(const wchar_t* x,				//!< input string
					size_t len = os_minus_one				//!< string length
					);
//! \brief converts utf-8 to utf-16 using pre-allocated buffer
inline 
const wchar_t* 
multibyte_to_unicode(wchar_t* dest,							//!< [out] buffer
					 size_t dest_len,						//!< buffer length
					 const char* x,						//!< input string
					 size_t src_len = os_minus_one			//!< input string length
					 );
//! \brief converts utf-8 to utf-16 using wstring_t class
inline 
const wchar_t* 
multibyte_to_unicode(wstring_t& dest,						//!< [out] string object
					 const char* x,						//!< input string
					 size_t len = os_minus_one				//!< input string length
					 );
//! \brief converts utf-8 to utf-16 using external allocator
inline 
const wchar_t* 
multibyte_to_unicode(byte_allocator& _allocator,			//!< external allocator
					 const char* x,						//!< input string
					 size_t len = os_minus_one				//!< input string length
					 );
//! \brief converts utf-16 to utf-8 using pre-allocated buffer
inline 
const char* 
unicode_to_multibyte(char* dest,							//!< [out] buffer
					 size_t dest_len,						//!< buffer length
					 const wchar_t* x,					//!< input string
					 size_t src_len = os_minus_one			//!< input string length
					 );
//! \brief converts utf-16 to utf-8 using string_t class
inline 
const char* 
unicode_to_multibyte(string_t& dest,						//!< [out] string object
					 const wchar_t* x,					//!< input string
					 size_t len = os_minus_one				//!< input string length
					 );
//! \brief converts utf-16 to utf-8 using external allocator
inline 
const char* 
unicode_to_multibyte(byte_allocator& _allocator,			//!< external allocator
					 const wchar_t* x,					//!< input string
					 size_t len = os_minus_one				//!< input string length
					 );

}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_string_h_

