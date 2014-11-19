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

#ifndef _terimber_algorithm_h_
#define _terimber_algorithm_h_

#include "base/primitives.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \namespace algorithm
//! \brief supports thread safe STL algorithms
namespace algorithm
{
	//! \brief find_if thread safe method
	template< class Cont, class Pred >
    inline 
	void 
	find_if(		Cont& cont,								//!< input container
					Pred& pr,								//!< predicate
					const mutex& mtx						//!< mutex
				);
	//! \brief for_each thread safe method
	template< class Cont, class Pred >
    inline 
	void 
	for_each(		Cont& cont,								//!< input container
					Pred& pr,								//!< predicate
					const mutex& mtx						//!< mutex
				);
	//! \brief remove_if thread safe method
	template< class Cont, class Pred >
    inline 
	void 
	remove_if(		Cont& cont,								//!< input container
					Pred& pr,								//!< predicate
					const mutex& mtx						//!< mutex
				);
	//! \brief push_back with external allocator thread safe method
	template< class Al, class Obj, class Cont >
    inline 
	void 
	push_back(		Al& al,									//!< external allocator
					Cont& cont,								//!< input container
					Obj& obj,								//!< object
					const mutex& mtx						//!< mutex
				);
	//! \brief push_back_if with external allocator and predicate thread safe method
	template< class Al, class Cont, class Pred, class Obj >
    inline 
	bool 
	push_back_if(	Al& al,									//!< external allocator
					Cont& cont,								//!< input container
					Pred& pr,								//!< predicate 
					Obj& obj,								//!< object
					const mutex& mtx						//!< mutex
				);
	//! \brief push_back thread safe method
	template< class Obj, class Cont >
    inline 
	void 
	push_back(		Cont& cont,								//!< input container
					Obj& obj,								//!< object
					const mutex& mtx						//!< mutex
				);
	//! \brief push_back_if with predicate thread safe method
	template< class Cont, class Pred, class Obj >
    inline 
	bool 
	push_back_if(	Cont& cont,								//!< input container
					Pred& pr,								//!< predicate 
					Obj& obj,								//!< object
					const mutex& mtx						//!< mutex
				);
	//! \brief remove thread safe method
	template< class Obj, class Cont >
    inline 
	void 
	remove(			Cont& cont,								//!< input container
					Obj& obj,								//!< object
					const mutex& mtx						//!< mutex
				);
	//! \brief swaps method
	template< class Obj >
    inline 
	void 
	swap(			Obj& first,								//!< first object
					Obj& second								//!< second object
				);
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_algorithm_h_

