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

#ifndef _terimber_threadpool_factory_h_
#define _terimber_threadpool_factory_h_

#include "log.h"

//! \class terimber_thread_employer
//! \brief abstract interface for user of thread manager
//! base job employer class
//! employer is responsible for synchronization
//! while job is requested and processed
class terimber_thread_employer
{
public:
	//! \brief destructor
	virtual ~terimber_thread_employer() 
	{
	}
	//! the real implementation depends on what to do
	//! leave for future implementation
	//! \brief checks job
	virtual 
	bool 
	v_has_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					) = 0;
	//! \brief does job
	virtual 
	void 
	v_do_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					) = 0;
};

//! \class terimber_threadpool
//! \brief abstract interface for the thread pool
class terimber_threadpool : public terimber_log_helper
{
public:
	//! \brief destructor
	virtual ~terimber_threadpool() 
	{
	}
	//! \brief user can call function even inside v_do_real_job function - see above
	virtual 
	bool 
	borrow_thread(	size_t ident,							//!< thread ident will be used as input parameter for client thread functions
					void* data,								//!< user defined data will be used as input parameter for client thread functions
					terimber_thread_employer* client,		//!< user callback
					size_t stay_on_alert_time_msec			//!< time in milliseconds until unused thread will go back to the pool
					) = 0;
	//! \brief user can ask for a little help and wake up any available thread in the provided range of idents
	virtual 
	bool 
	borrow_from_range(size_t from,							//!< from ident
					size_t to,								//!< to ident
					void* data,								//!< user defined data will be used as input parameter for client thread functions
					terimber_thread_employer* client,		//!< user callback
					size_t stay_on_alert_time_msec			//!< time in milliseconds until unused thread will go back to the pool
					) = 0;
	//! \brief sometimes we need to stop calling client functions immediately
	//! however the client function calles already in progress should be completed
	virtual 
	void 
	revoke_client(	terimber_thread_employer* client		//!< user callback
					) = 0;
	//! \brief do xray
	virtual 
	void 
	doxray() = 0;
};

//! \class terimber_threadpool_factory
//! \brief thread pool factory
//! creates thread pools
//! the caller is responsible for destroying the thread pool object
//! but NEVER do it inside client thread functions (v_has_job or v_do_job)
class terimber_threadpool_factory
{
public:
	//! \brief constructor
	terimber_threadpool_factory();
	//! \brief destructor
	~terimber_threadpool_factory();

	//! \brief creates terimber thread pool object
	//! caller is responsible for destroying it
	terimber_threadpool* 
	get_thread_pool(terimber_log* log,						//!< logging pointer
					size_t capacity,						//!< how many threads can be opened and used simultaniously
					size_t deactivate_time_msec				//!< deactivate_time_msec is interval in milliseconds (minimum 100 msec) 
															//!< after that all unused threads returned to the pool will be closed
					);
};


#endif


