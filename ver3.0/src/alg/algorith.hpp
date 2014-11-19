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

#ifndef _terimber_algorithm_hpp_
#define _terimber_algorithm_hpp_

#include "alg/algorith.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// class supports thread safe STL algorithms
namespace algorithm
{
	template< class Cont, class Pred >
    inline void find_if(Cont& cont, Pred& pr, const mutex& mtx)
	{	
		mutexKeeper keeper(mtx);
		TYPENAME Cont::iterator first = cont.begin(), last = cont.end();
		for (; first != last; ++first)
			if (pr(*first))
				break;
	}
	
	template< class Cont, class Pred >
    inline void for_each(Cont& cont, Pred& pr, const mutex& mtx)
	{ 
		mutexKeeper keeper(mtx); 
		TYPENAME Cont::iterator first = cont.begin(), last = cont.end();

		for (; first != last; ++first)
			pr(*first);
	}

	template< class Cont, class Pred >
    inline void remove_if(Cont& cont, Pred& pr, const mutex& mtx)
	{ 
		mutexKeeper keeper(mtx); 
		TYPENAME Cont::iterator first = cont.begin(), last = cont.end();

		for (; first != last; ++first)
			if (pr(*first))
			{
				cont.erase(first);
                break;
			}
	}

	template< class Al, class Obj, class Cont >
    inline void push_back(Al& al, Cont& cont, Obj& obj, const mutex& mtx)
	{ 
		mutexKeeper keeper(mtx); 
		cont.push_back(al, obj);
	}

	template< class Al, class Cont, class Pred, class Obj >
    inline bool push_back_if(Al& al, Cont& cont, Pred& pr, Obj& obj, const mutex& mtx)
	{ 
		mutexKeeper keeper(mtx); 
		TYPENAME Cont::iterator first = cont.begin(), last = cont.end();

		for (; first != last; ++first)
			if (pr(*first))
				return false;

		cont.push_back(al, obj);
		return true;
	}

	template< class Obj, class Cont >
    inline void push_back(Cont& cont, Obj& obj, const mutex& mtx)
	{ 
		mutexKeeper keeper(mtx); 
		cont.push_back(obj);
	}

	template< class Cont, class Pred, class Obj >
    inline bool push_back_if(Cont& cont, Pred& pr, Obj& obj, const mutex& mtx)
	{ 
		mutexKeeper keeper(mtx); 
		TYPENAME Cont::iterator first = cont.begin(), last = cont.end();

		for (; first != last; ++first)
			if (pr(*first))
				return false;

		cont.push_back(obj);
		return true;
	}

	template< class Obj, class Cont >
    inline void remove(Cont& cont, Obj& obj, const mutex& mtx)
	{ 
		mutexKeeper keeper(mtx); 
		cont.remove(obj);
	}

	template< class Obj >
    inline void swap(Obj& first, Obj& second)
	{
		Obj temp(first);
		first = second;
		second = temp;
	}
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_algorithm_hpp_

