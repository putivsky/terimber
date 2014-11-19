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

#ifndef _terimber_fuzzyphonetic_h_
#define _terimber_fuzzyphonetic_h_

#include "fuzzy/fuzzyimpl.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \namespace fuzzyphonetic 
//! \brief fuzzy match library implementation
namespace fuzzyphonetic
{
	//! \brief converts the char to upper case
	inline 
	char 
	to_upper(		char ch									//!< input char
					);
	//! \brief converts the char to lower case
	inline 
	char 
	to_lower(		char ch									//!< input char
					);

	//! \brief converts a single word to the phonetic key
	//! memory is allocated on an external allocator
	metaphone_key 
	convert_to_metaphone(const char* word,					//!< input word
					size_t len,								//!< word length
					byte_allocator& all						//!< external allocator
					);
	//! \brief converts phrase to reflection
	void 
	convert_to_reflection(const char* word,					//!< input word
					size_t len,								//!< word length
					byte_allocator& all,					//!< external allocator
					reflection_key& reflection				//!< [out] reflection
					);
	//! \brief calculates the matrix distance
	template< class T >
	inline
	size_t 
	metaphone_distance(const T* ax,							//!< first input array
					size_t x,								//!< first input array length
					const T* ay,							//!< second input array
					size_t y,								//!< second input array length
					byte_allocator& tmp,					//!< external temporary allocator
					size_t max_penalty						//!< max penalty, just a performance tip
					);
	//! \brief finds the distance between the two metaphone keys
	inline
	size_t 
	find_metaphone_distance(const metaphone_key& x,			//!< first key 
					const metaphone_key& y,					//!< second key
					byte_allocator& tmp,					//!<  external temporary allocator
					size_t max_penalty						//!< max penalty, just a performance tip
					);
	//! \brief finds the distance between the two reflection keys
	inline
	size_t 
	find_reflection_distance(const reflection_key& x,		//!< first key 
					const reflection_key& y,				//!< second key
					byte_allocator& tmp,					//!<  external temporary allocator
					size_t max_penalty						//!< max penalty, just a performance tip
					);
	//! \brief finds the distance between the two words
	inline
	size_t 
	find_word_distance(const char* x,						//!< first word
					size_t xlen,							//!< first word length
					const char* y,							//!< second word
					size_t ylen,							//!< second word length
					byte_allocator& tmp,					//!<  external temporary allocator
					size_t max_penalty						//!< max penalty, just a performance tip
					);
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif //_terimber_fuzzymetaphoneic_h_
