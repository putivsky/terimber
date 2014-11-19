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

#ifndef _terimber_fuzzywrapper_h_
#define _terimber_fuzzywrapper_h_

//! \enum ngram_quality
//! \brief ngram quality
enum ngram_quality
{
	nq_high = 0,											//!< tries to find the n-gram suggestions matching all words in user input
	nq_normal,												//!< tries to find the partial (subset of matched words) match
	nq_low													//!< tries to find any the partial (subset of matched words) match
};

//! \enum phonetic_quality
//! \brief phonetic quality
enum phonetic_quality
{
	pq_high = 0,											//!< only EXACT phonetic match AND SMALL DISTANCE between the user input and the suggestions
	pq_normal,												//!< PARTIAL phonetic match AND SMALL DISTANCE between the user input and the suggestions
	pq_low													//!< PARTIAL phonetic match AND BIGGER DISTANCE between the user input and the suggestions
};

//! \class linked_result
//! \brief class supports NON Terimber allocation
//! not so efficient, but does not require the base library source code
class linked_result
{
public:
	//! \brief constructor
	linked_result() : 
		_next(0), 
		_str(0)
	{
	}

	//! \brief destructor
	~linked_result()
	{
		if (_next)
			delete _next;

		if (_str)
			delete [] _str;
	}

public:

	linked_result*		_next;								//!< next element in linked list
	char*				_str;								//!< string pointer
};

//! \class fuzzy_wrapper
//! \brief fuzzy wrapper library interface
//! it's safe to call methods in multithreaded program
//! add/remove methods will be executed one by one; "write" blocking mode
//! match method can be executed in multiple threads at the same time; "read" non-blocking mode 
class fuzzy_wrapper
{
public:
	//! \brief destructor
	virtual 
	~fuzzy_wrapper() 
	{
	}

	// methods

	//! \brief adds a new n-gram to the internal repository (utf-8)
	//! caller can add the same n-gram many times
	//! class will support internal references count
	virtual 
	bool 
	add(			const char* phrase						//!< input phrase
					) = 0;
	
	//! \brief removes the previously added ngram
	//! caller can remove the same n-gram many times unless references count goes to zero
	virtual 
	bool 
	remove(			const char* phrase						//!< input phrase
					) = 0;

	//! \brief does the fuzzy match
	virtual 
	bool 
	match(			ngram_quality nq,						//!< ngram quality
					phonetic_quality fq,					//!< phonetic quality
					const char* phrase, 
					linked_result& suggestions
					) const = 0;

};

//! \class fuzzy_wrapper_factory
//! \brief class factory for fuzzy wrapper
class fuzzy_wrapper_factory
{
public:
	fuzzy_wrapper* 
	get_fuzzy_wrapper(size_t memory_usage					//!< max memory usage
					);
};

#endif //_terimber_fuzzywrapper_h_
