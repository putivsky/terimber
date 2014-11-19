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

#ifndef _terimber_timer_h_
#define _terimber_timer_h_

#include "threadpool/thread.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class timer_callback
//! \brief implements callback for timer event firing
class timer_callback
{
public:
	//! \brief destructor
	virtual 
	~timer_callback() {}
	//! \brief periodically notifies user in a separate thread
	virtual 
	void 
	notify(			size_t ident,							//!< timer ident
					size_t interval,						//!< repeatition interval in milliseconds
					size_t multiplier						//!< multiplier coefficient for repeatition interval 
					) = 0;
};

//! \class timer
//! \brief timer
class timer : public terimber_thread_employer
{
public:
	//! \brief constructor
	timer();
	//! \brief destructor
	~timer();
	//! \brief activates the timer
	void 
	activate(		timer_callback* callback,				//!< user callback
					size_t ident,							//!< timer ident, one timer callback object can handle multiple timers
					size_t interval,						//!< repeatition interval in milliseconds
					size_t multiplier = 1,					//!< multiplier coefficient for repeatition interval
					bool reactivate = true					//!< reactivate flag (repeat firing timer events
					);
	//! \brief deactivates
	//! stops timer
	void 
	deactivate();
protected:
	//! terimber_thread_employer
	//! \brief checks job
	virtual 
	bool 
	v_has_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);
	//! \brief does job
	virtual 
	void 
	v_do_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);	
private:
	thread			_thread;								//!< thread that will fire events
	mutex			_mtx;									//!< multithreaded mutex
	size_t			_ident;									//!< timer ident
	size_t			_interval;								//!< interval of timer events in msec
	size_t			_multiplier;							//!< multiplier of timer interval
	size_t			_trigger;								//!< keep the remain interval before firing timer event
	bool			_reactivate;							//!< reactivation flag
	bool			_first;									//!< flag of first firing
	timer_callback*	_callback;								//!< keep callback object pointer
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_timer_h_
