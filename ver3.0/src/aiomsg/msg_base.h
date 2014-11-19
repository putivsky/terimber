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

#ifndef _terimber_msg_base_h_
#define _terimber_msg_base_h_

#include "base/primitives.h"


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// forward declaration
class msg_communicator;

//! \class msg_base
// class implements the base functionality for thread activity
class msg_base
{
public:
	//! \brief constructor
	msg_base(msg_communicator* communicator_				//!< communicator pointer
			);
	//! \brief destructor
	virtual ~msg_base();
	//! implements the base activate/deactivate functions
	//! \brief turns on
	void 
	on();
	//! \brief turns off
	void 
	off();
	//! \brief returns the current state
	inline 
	bool 
	is_on() const 
	{ 
		mutexKeeper keeper(_mtx_res); 
		return _on; 
	}
protected:
	//! \brief sets the current state
	inline 
	void 
	set_on(bool on) 
	{ 
		mutexKeeper keeper(_mtx_res); 
		_on = on; 
	}

	//! \brief action on turning on
	virtual 
	void 
	v_on() 
	{
		set_on(true);
	}
	//! \brief action on turning off
	virtual 
	void 
	v_off() 
	{
		set_on(false);
	}
protected:
	msg_communicator*		_communicator;					//!< pointer to communicator object
	mutex					_mtx_res;						//!< mutex for internal resources
	bool					_on;							//!< on/off flag
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_msg_base_h_

