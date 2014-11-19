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

#ifndef _terimber_proto_h_
#define _terimber_proto_h_

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class proto_creator
//! \brief base proto class for creators
template < class C, class T, class A >
class proto_creator
{
public:
	//! \typedef TYPE
	//! \brief object type
	typedef T TYPE;
	//! \typedef ARG
	//! \brief argument type
	typedef A ARG;
	//! \brief returns static constructor
	static 
	C& 
	static_constructor()
	{
		return s_constructor;
	}
	//! \brief returns static defualt argument
	static 
	const A& 
	get_default_arg()
	{
		return s_def_arg;
	}
	//! \brief returns static default timeout
	static 
	size_t 
	get_default_timeout()
	{
		return 1000; // 1 second
	}
	//! \brief creates object
	static
	T* 
	create(			const A&
					)
	{
		return new T();
	}
	//! \brief activates object
	//! nothing to do
	static 
	void 
	activate(		T*, 
					const A&
					) 
	{
	}
	//! \brief finds object - all objects are equal
	static 
	bool 
	find(			T*, 
					const A&
					)
	{
		return true;
	}
	//! \brief backing object - nothing to do
	static 
	void 
	back(			T*, 
					const A&
					) 
	{
	}
	//! \brief destroys object
	static 
	void 
	destroy(		T* obj,									//!< object pointer
					const A&
					)
	{
		delete obj;
	}

	//! \brief deactivates object - nothing to do
	static 
	void 
	deactivate(		T*, 
					const A&
					) 
	{
	}

private:
	static A s_def_arg;										//!< default static argument
	static C s_constructor;									//!< default static constructor
};

//! static definitions
template < class C, class T, class A >
C proto_creator< C, T, A >::s_constructor;

template < class C, class T, class A >
A proto_creator< C, T, A >::s_def_arg;


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_proto_h_
