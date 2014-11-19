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

#ifndef _terimber_fuzzyaccess_h_
#define _terimber_fuzzyaccess_h_

#include "base/memory.h"
#include "base/list.h"
#include "fuzzy/fuzzywrapper.h"

//! \class fuzzy_matcher
//! \brief fuzzy match library interface
class fuzzy_matcher
{
public:
	//! \brief destructor
	virtual 
	~fuzzy_matcher() 
	{
	}

	// methods

	//! \brief adds a new n-gram to the internal repository (utf-8)
	//! caller can add the same n-gram many times
	//! class will support internal references count
	virtual 
	size_t 
	add(			const char* phrase,						//!< input phrase
					TERIMBER::byte_allocator& all			//!< external allocator
					) = 0;
	
	//! \brief removes the previously added ngram
	//! caller can remove the same n-gram many times unless the references count goes to zero
	virtual 
	bool 
	remove(			const char* phrase,						//!< input phrase
					TERIMBER::byte_allocator& all			//!< external allocator
					) = 0;
	//! \brief remove previously added ngram by ident
	//! caller can remove the same n-gram many times unless the references count goes to zero
	virtual 
	bool 
	remove(			size_t ident,							//!< input ident
					TERIMBER::byte_allocator& all			//!< external allocator
					) = 0;
	//! \brief does the fuzzy match
	virtual 
	bool 
	match(			ngram_quality nq,						//!< ngram quality for matching
					phonetic_quality pq,					//!< phonetic quality for matching
					const char* phrase,						//!< input phrase
					TERIMBER::byte_allocator& all,			//!< external allocator for output container
					TERIMBER::byte_allocator& tmp,			//!< external temporary allocator
					TERIMBER::_list< const char* >& suggestions //!< [out] output list of suggestions
					) const = 0;

	//! \brief does the fuzzy match
	virtual 
	bool match(		ngram_quality nq,						//!< ngram quality for matching
					phonetic_quality fq,					//!< phonetic quality for matching
					const char* phrase,						//!< input phrase
					TERIMBER::byte_allocator& all,			//!< external allocator for output container
					TERIMBER::byte_allocator& tmp,			//!< external temporary allocator
					TERIMBER::_list< size_t >& suggestions	//!< [out] output list of sugestions idents
					) const = 0;

	//! \brief clean up engine
	virtual
	void
	reset() = 0;
};

//! \class fuzzy_matcher_factory
//! \brief class factory for fuzzy matcher
class fuzzy_matcher_factory
{
public:
	//! \brief creates the fuzzy matcher intsance
	fuzzy_matcher* 
	get_fuzzy_matcher(size_t memory_usage					//!< max memory usage
					);
};

#endif //_terimber_fuzzyaccess_h_
