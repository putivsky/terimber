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

#include "threadpool/thread.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

///////////////////////////////////////////////////////////
// static 
#if OS_TYPE == OS_WIN32
unsigned int __stdcall 
#else
void* 
#endif
thread::start_thread(void* data)
{
	// copies this pointer
	thread* _this = (thread*)data;
	// changes state
	_this->change_state(THREAD_RUNNING);
	// signals about the start thread;
	_this->_ev_start.set();
	// does the working
	while (_this->sleep())
	{
		// goes to do the job
		while (_this->execute());
	}

	// need set close thread event to signal state
	// to notify parent thread about exiting from thread
	_this->_ev_end.set();
	// leave thread
	return 0;
}

thread::thread() :
	_job_task(0, 0, INFINITE, 0),
	_state(THREAD_CLOSE),
	_handle(0)
{
}
	
thread::~thread() 
{
	if (_handle)
		stop();
}

thread_state 
thread::get_state() const
{
	// locks mutex
	mutexKeeper keeper(_mtx);
	return _state;
}

bool 
thread::change_state(thread_state new_state)
{
	// locks mutex
	mutexKeeper keeper(_mtx);
	// possible state transitions
	/*
	CLOSE -> STARTING
	STARTING -> SLEEPING
	SLEEPING -> RUNNING
	SLEEPING -> STOPPING
	RUNNING -> SLEEPING
	RUNNING -> STOPPING
	STOPPING -> CLOSE
	*/

	/*
							 V-----------------|
	CLOSE -> STARTING -> SLEEPING |-> RUNNING -|
		^					|		
		|					|
		STOPPING <----------|
	*/
	switch (_state)
	{
		case THREAD_CLOSE:
			return (new_state == THREAD_STARTING) ? (_state = new_state, true) : false;
		case THREAD_STARTING:
			return (new_state == THREAD_RUNNING 
					|| new_state == THREAD_CLOSE
					) ? (_state = new_state, true) : false;
		case THREAD_SLEEPING:
			return (new_state == THREAD_RUNNING 
					|| new_state == THREAD_STOPPING
					) ? (_state = new_state, true) : false;
		case THREAD_RUNNING:
			return (new_state == THREAD_SLEEPING 
					|| new_state == THREAD_STOPPING
					) ? (_state = new_state, true) : false;
		case THREAD_STOPPING:
			return (new_state == THREAD_CLOSE) ? (_state = new_state, true) : false;
		default:
			assert(false);
	}

	return false;
}

bool 
thread::inside_thread() const
{
	// analyzes the thread handle
#if OS_TYPE == OS_WIN32
	return ::GetCurrentThread() == _handle;
#else
	return pthread_self() == _handle;
#endif
}

bool 
thread::start()
{
	// analyzes the thread handle
	if (inside_thread())
		return false;

	// checks the state
	// starts only the stop thread
	if (!change_state(THREAD_STARTING))
		return false;
	// nonsignal event
	_ev_start.reset();
	_ev_end.reset();
	// creates thread
#if OS_TYPE == OS_WIN32
	// dummy thread ID
	unsigned int threadID = 0;
	if (!(_handle = (void*)_beginthreadex(0, 0, start_thread, this, 0, &threadID)))
	{
		change_state(THREAD_CLOSE);
		return false;
	}

#else
	if (pthread_create(&_handle, 0, start_thread, this))
		return false;
#endif

	// waits for start
	_ev_start.wait();
	return true;
}

bool 
thread::assign_job(const job_task& job_task)
{
	// analyzes thread handle
	if (inside_thread())
		return false;

	// checks state
	mutexKeeper keeper(_mtx_job);
	switch (_state)
	{
		case THREAD_SLEEPING:
		case THREAD_RUNNING:
			_job_task = job_task;
			break;
		default:
			return false;
	}

	keeper.unlock();
	// wakes up the thread
	_ev_wakeup.set();
	return true;
}

bool 
thread::cancel_job()
{	
	// analyzes thread handle
	if (inside_thread())
		return false;

	// checks state
	mutexKeeper keeper(_mtx_job);
	switch (_state)
	{
		case THREAD_SLEEPING:
		case THREAD_RUNNING:
			_job_task.clear();
			break;
		default:
			return false;
	}

	keeper.unlock();

	// wakes up the thread
	_ev_wakeup.set();
	return true;
}

bool 
thread::stop()
{
	// analyzes the thread handle
	if (inside_thread())
		return false;

	// checks the state
	if (!change_state(THREAD_STOPPING))
		return false;

	// wakes up the thread
	_ev_wakeup.set();
	// waits for the thread to end
	_ev_end.wait();
#if OS_TYPE == OS_WIN32
	// waits for the windows thread ending
	if (WAIT_TIMEOUT == WaitForSingleObject(_handle, 1000))
		// troubles while close thread gracefully
		TerminateThread(_handle, 0);

	// closes handle couse we started with beginthreadex function
	CloseHandle(_handle);

#else
	if (pthread_detach(_handle) != 0)
		pthread_cancel(_handle);
#endif

	_handle = 0;

	// sets the state to closed
	if (!change_state(THREAD_CLOSE))
	{
		assert(false);
		return false;
	}

	return true;
}

void 
thread::wakeup() const
{
	_ev_wakeup.set();
}

bool 
thread::execute()
{
	mutexKeeper keeper(_mtx_job);
	if (_state != THREAD_RUNNING // check thread state
		|| !_job_task._employer // checks pointer
		)
		return false;

	// copy task
	job_task task = _job_task;
	// unlocks mutex
	keeper.unlock();


	try
	{
		if (!task._employer->v_has_job(task._ident, task._user_data)) // check job
			return false;

		task._employer->v_do_job(task._ident, task._user_data);
	}
	catch (...)
	{
		// well, well ...
		// employer has thrown exception - bad guy
		// but we are still alive and will continue to work
		assert(false);
	}

	return true;
}

bool 
thread::sleep()
{
	// checks stop 
	if (!change_state(THREAD_SLEEPING))
		return false;

	// locks job resources
	mutexKeeper keeper(_mtx_job);
	size_t wait_timeout = _job_task._timeout;
	keeper.unlock();

	// waits for the wake up signal
	_ev_wakeup.wait(wait_timeout);

	// sets the state
	return change_state(THREAD_RUNNING);
}

//////////////////////////////////
// static 
thread* 
thread_creator::create(const job_task& task)
{
	thread* obj = new thread;
	if (obj)
		obj->start(); // start thread
	return obj;
}

//static 
void 
thread_creator::activate(thread* obj, const job_task& task)
{
	// thread can be closed by the deactivation process
	if (obj->get_state() == THREAD_CLOSE)
		obj->start();
	obj->assign_job(task);
}

//static 
void 
thread_creator::back(thread* obj, const job_task&)
{
	obj->cancel_job(); 
}

// static 
void 
thread_creator::destroy(thread* obj, const job_task& task)
{
	deactivate(obj, task);
	delete obj;
}

// static 
void 
thread_creator::deactivate(thread* obj, const job_task& task)
{
	back(obj, task);
	obj->stop();
}


#pragma pack()
END_TERIMBER_NAMESPACE
