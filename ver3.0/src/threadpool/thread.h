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

#ifndef _terimber_thread_h_
#define _terimber_thread_h_

#include "base/template.h"
#include "threadpool/threadpoolfactory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class job_task
//! \brief class to send task to client thread
class job_task
{
public:
	//! \brief default constructor
	job_task() :
		_employer(0), 
		_ident(0), 
		_timeout(INFINITE), 
		_user_data(0) 
	{
	}
	//! \brief constructor
	job_task(		terimber_thread_employer* employer,		//!< user callback
					size_t ident,							//!< thread ident
					size_t timeout,							//!< timeout for next v_has_job call if the first one returns false
					void* user_data							//!< user defined data
					) :
		_employer(employer), 
		_ident(ident), 
		_timeout(timeout), 
		_user_data(user_data) 
	{
	}
	//! \brief copy constructor
	job_task(const job_task& x) 
	{ 
		*this = x; 
	}
	//! \brief assign operator
	job_task& operator=(const job_task& x)
	{
		if (this != &x)
		{
			_employer = x._employer;
			_ident = x._ident; 
			_timeout = x._timeout;
			_user_data = x._user_data;
		}
		return *this;
	}
	//! \brief clears job
	void clear()
	{
		_employer = 0;
		_ident = 0; 
		_timeout = INFINITE;
		_user_data = 0;
	}

	terimber_thread_employer*	_employer;					//!< pointer to the class which will be doing the real job
	size_t						_ident;						//!< ident, employer might to differ the different task inside do_job function 
	size_t						_timeout;					//!< timeout
	void*						_user_data;					//!< additional data for employer class
};

//! \enum thread_state
//! \brief thread states
enum thread_state
{
	THREAD_CLOSE,											//!< thread is not running at all
	THREAD_STARTING,										//!< thread is starting
	THREAD_SLEEPING,										//!< thread is running and sleeping for waikup 
	THREAD_RUNNING,											//!< thread is running and doing job
	THREAD_STOPPING											//!< thread is stopping
};

//! \class thread
//! \brief safe thread class
class thread
{	
	//! \brief thread run function
	static
#if OS_TYPE == OS_WIN32
	unsigned int __stdcall
#else
	void*
#endif
	 start_thread(	void* data								//!< thread argument
					);

	//! \brief checks if the API call has been made inside this thread
	//! for instance, start, stop, assign_job, and cancel_job function calls from the same thread are ignored
	bool 
	inside_thread() const;
	//! prevents the copying of objects
	//! \brief copy constructor
	thread(const thread& x);
	//! \brief assign operator
	thread& operator=(const thread& x);
public:
	//! \brief constructor
	thread();
	//! \brief destructor
	~thread();
	//! \brief returns current state - thread safe
	thread_state get_state() const; 
	//! \brief starts thread, return false if thread already started
	bool 
	start();
	//! \brief assigns new employer - thread safe, returns false if pointer is null 
	//! or thread is not running
	//! user has to call wakeup function to activate thread
	bool 
	assign_job(		const job_task& job_task				//!< job task
					);
	//! \brief cancels current job - thread safe, returns false if thread is not running
	bool 
	cancel_job();
	//! \brief stops thread, returns false if thread already stopped
	bool 
	stop();
	//! \brief signals thread to wakeup, if thread is in waiting state
	void 
	wakeup() const;
private:
	//! \brief changes current state
	bool 
	change_state(	thread_state new_state					//!< new thread state
					);
	//! \brief makes a copy of task and call employer function
	bool 
	execute();
	//! \brief waits for either a new job (return true) or for the work to finish (return false)
	bool 
	sleep();
private:
	job_task		_job_task;								//!< keep pointer to employer object
	mutex			_mtx;									//!< mutex for thread safe access to thread state
	event			_ev_start;								//!< event is activated inside thread and deactivated automatically
	event			_ev_wakeup;								//!< event is activated by caller and deactivated automatically
	event			_ev_end;								//!< event is activated inside thread just before exit and deactivated automatically
	mutex			_mtx_job;								//!< mutex for job task access - sharing access to emplyer object
	thread_state	_state;									//!< keeps thread state
#if OS_TYPE == OS_WIN32
	HANDLE
#else
	pthread_t
#endif
	_handle;												//!< keep thread resources
};

//! \class thread_creator
//! \brief creator for pool
class thread_creator : public proto_creator< thread_creator, thread, job_task >
{
public:
	//! \brief creates a new thread object
	static
	thread* 
	create(			const job_task& task					//!< create new thread object
					);
	//! \brief starts thread and assign task if any
	static 
	void 
	activate(		thread* obj,							//!< thread object
					const job_task& task					//!< task
					);
	//! \brief cancels task if any
	static 
	void 
	back(			thread* obj,							//!< thread object
					const job_task& task					//!< task
					);
	//! \brief closes thread and destroy thread object
	static 
	void 
	destroy(		thread* obj,							//!< thread object
					const job_task& task					//!< task
					);
	//! \brief stops thread
	static
	void 
	deactivate(		thread* obj,							//!< thread object
					const job_task& task					//!< task
					);
};

//! \typedef thread_pool_t
//! \brief thread pool type
typedef pool< thread_creator > thread_pool_t;

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_thread_h_
