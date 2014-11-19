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
#include "base/primitives.h"

namespace terimber {

#if OS_TYPE != OS_WIN32

static void calculateAbsoluteTimeByTimeout(size_t tmRelative, 
                                           timespec& tmAbsolute) {
	// sets timeout
	timeval now_;
	gettimeofday(&now_, 0);

	tmAbsolute.tv_sec = now_.tv_sec + (unsigned long)tmRelative / 1000; 
	tmAbsolute.tv_nsec = now_.tv_usec * 1000 + 
                       ((unsigned long)tmRelative % 1000) * 1000000;
				
	if (tmAbsolute.tv_nsec >= 1000000000) {
		++tmAbsolute.tv_sec;
		tmAbsolute.tv_nsec -= 1000000000;
	}
}

#endif
///////////////////////////////////////////////////////////
// constructor
mutex::mutex() { 
#if OS_TYPE == OS_WIN32
	InitializeCriticalSection(&_handle); 
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&_handle, &attr);
#endif
}

// destructor
mutex::~mutex() { 
#if OS_TYPE == OS_WIN32
	DeleteCriticalSection(&_handle);
#else
	pthread_mutex_destroy(&_handle);
#endif
}

// NB!!! can be used in the const member function
bool mutex::lock() const { 
#if OS_TYPE == OS_WIN32
	EnterCriticalSection(&_handle);
	return true;
#else
	return !pthread_mutex_lock(&_handle);
#endif
}

// unlocks mutex
// NB!!! can be used in the const member function
void mutex::unlock() const { 
#if OS_TYPE == OS_WIN32
	LeaveCriticalSection(&_handle); 
#else
	pthread_mutex_unlock(&_handle);
#endif
}

// NB!!! can be used in the const member function
bool mutex::trylock() const { 
#if OS_TYPE == OS_WIN32
	return TRUE == TryEnterCriticalSection(&_handle);
#else
	return !pthread_mutex_trylock(&_handle);
#endif
}

///////////////////////////////////////////////////////////
static const int kSpinCount = 1000;

//! \brief does atomic swap
#if OS_TYPE != OS_WIN32
bool swap(int64_t compare, int64_t value, volatile int64_t* lock) {
  int64_t prev;
  __asm__ __volatile__ (
"1: ldarx   %0,0,%2\n\
    cmpw    0,%0,%3\n\
    bne-    2f\n\
    stdcx.  %4,0,%2\n\
    bne-    1b\n\
    2:"
            : "=&r" (prev), "+m" (*lock)
            : "r" (lock), "r" (compare), "r", (value)
            : "cc");
  return compare == prev;
}

//! \brief spin one round
bool spin(volatile int64_t* lock) const {
  size_t count = kSpinCount;
  while (count > 0 && *lock != 0) {
    --count;
  }
  return *lock == 0;
}

//! \brief spin one round
void nap() const {
  const size_t kSleepTimeNs = 1000;
  struct timespec tm;
  tm.tv_sec = 0;
  tm.tv_nsec = kSleepTimeNs;
  nanosleep(&tm, NULL);
}
#endif

//! \brief constructor
spinlock::spinlock() {
#if OS_TYPE == OS_WIN32
  InitializeCriticalSectionAndSpinCount(&_handle, kSpinCount);
#else
  _handle = 0;
#endif
}

//! \brief destructor
spinlock::~spinlock() {
#if OS_TYPE == OS_WIN32
  DeleteCriticalSection(&_handle);
#else
  _handle = 0;
#endif
}

//! \brief locks spinlock
//! NB!!! can be used in the const member function
bool spinlock::lock() const {
#if OS_TYPE == OS_WIN32
  EnterCriticalSection(&_handle);
#else
  while (!swap(0, 1, &_lock)) {
    // spin CPU here
    if (!spin()) {
      nap(); // long timeout
    }
  }
#endif
  return true;
}

//! \brief unlocks spinlock
//! NB!!! can be used in the const member function
void spinlock::unlock() const {
#if OS_TYPE == OS_WIN32
	LeaveCriticalSection(&_handle); 
#else
  swap(1, 0, &_lock);
#endif
}

//! \brief tries to locks spinlock
//! NB!!! can be used in the const member function
bool spinlock::trylock() const {
#if OS_TYPE == OS_WIN32
	return TRUE == TryEnterCriticalSection(&_handle);
#else
  return swap(0, 1, &_lock);
#endif
}

///////////////////////////////////////////////////////////
// constructor creates event
event::event(bool manualReset, bool initialState) { 
#if OS_TYPE == OS_WIN32
	// creates event
	_handle = CreateEvent(0, manualReset, initialState, 0); 
#else
	_handle._cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	// inits condition thread
	pthread_cond_init(_handle._cond, 0);
	// set initial setting
	_handle._manualReset = manualReset;
	_handle._signaled = initialState;
#endif
}

// destructor
event::~event() { 
#if OS_TYPE == OS_WIN32
	// close event handle
	CloseHandle(_handle); 
#else
	// destroys condition thread
	pthread_cond_destroy(_handle._cond);
	free(_handle._cond);
#endif
}
	
// signal event
// NB!!! can be used in the const member function
void event::set() const { 
#if OS_TYPE == OS_WIN32
	// sets event to the signal state
	SetEvent(_handle); 
#else
	// locks mutex	
	mutexKeeper keeper(_handle._mtx);

	_handle._manualReset ?  // manual reset
		pthread_cond_broadcast(_handle._cond) : // Unblock everyone waiting
		pthread_cond_signal(_handle._cond);	// Unblock just one waiter

	// sets state, if any
	_handle._signaled = true;
#endif
}
// nonsignal event
// NB!!! can be used in the const member function
void event::reset() const { 
#if OS_TYPE == OS_WIN32
	// sets event to the non signal state
	ResetEvent(_handle); 
#else
	mutexKeeper keeper(_handle._mtx);
	_handle._signaled = false;
#endif
}

// waits for signal state
// NB!!! can be used in the const member function
size_t event::wait(size_t timeoutMs) const { 
#if OS_TYPE == OS_WIN32
	// waits for signal
	return WaitForSingleObject(_handle, (DWORD)timeoutMs); 
#else
	// If the event is not in the signaled state, we will have to wait
	mutexKeeper keeper(_handle._mtx);

	size_t ret = 0;  // If we don't do anything it's because it's free already

	if (!_handle._signaled) { // if already in a signal state just return
		if (!timeoutMs) {// no time for waiting
			return WAIT_TIMEOUT;
    } else {
			timespec timeout;
			if (INFINITE != timeoutMs) {
				// sets timeout
				calculateAbsoluteTimeByTimeout(timeoutMs, timeout);
			}

			// waits until condition thread returns control
			do {
				ret = (INFINITE == timeoutMs ? pthread_cond_wait(_handle._cond, _handle._mtx) : 
								pthread_cond_timedwait(_handle._cond, _handle._mtx, &timeout));
			} while (!ret && !_handle._signaled);
		}
	}

	// adjusts signaled member
	switch (ret) {
		case 0: // success
			if (!_handle._manual_reset) {
				_handle._signaled = false;
      }
			return WAIT_OBJECT_0;
		case ETIMEDOUT:
		default:
			return WAIT_TIMEOUT;
	}
#endif
}
	
///////////////////////////////////////////////////////////
// constructor create semaphore
semaphore::semaphore(size_t initialCount, size_t maxCount) { 
	// checks reasonable params
	assert(maxCount != 0);
	assert(initialCount <= maxCount);

#if OS_TYPE == OS_WIN32
	// creates semaphore
	_handle = CreateSemaphore(0, (LONG)initialCount, (LONG)maxCount, 0);
#else
	_handle._cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	// creates condition thread
	pthread_cond_init(_handle._cond, 0);
	_handle._max_count = maxCount;
	_handle._init_count = initialCount;
#endif
}

// destructor
semaphore::~semaphore() { 
#if OS_TYPE == OS_WIN32
	// closes semaphore handle
	CloseHandle(_handle); 
#else
	// destroys condition thread
	pthread_cond_destroy(_handle._cond);
	free(_handle._cond);
#endif
}

// releases semaphore
// NB!!! can be used in the const member function
bool semaphore::release(size_t count) const { 
	if (!count) {// nothing to do
		return false;
  }

#if OS_TYPE == OS_WIN32
	// releases semaphore
	return TRUE == ReleaseSemaphore(_handle, (LONG)count, 0); 
#else
	// locks mutex
	mutexKeeper keeper(_handle._mtx);

	if (_handle._initialCount >= _handle._maxCount) {
		return false;
  }
	// increment semaphore counter
	if (_handle._initialCount + count > _handle._maxCount) {
		_handle._initialCount = _handle._maxCount;
  } else {
		_handle._initialCount += count;
  }

	// sets condition thread
	count == 1 ?
		pthread_cond_signal(_handle._cond) :
		pthread_cond_broadcast(_handle._cond);

	return true;
#endif
}

// waits for signal state
// NB!!! can be used in the const member function
size_t semaphore::wait(size_t timeoutMs) const { 
#if OS_TYPE == OS_WIN32
	// waits for semaphore event
	return WaitForSingleObject(_handle, (DWORD)timeoutMs); 
#else
	mutexKeeper keeper(_handle._mtx);

	size_t ret = 0; // If we don't do anything it's because it's available already

	if (!_handle._initialCount) { // if we can acquire resource
		if (!timeoutMs) {
			return WAIT_TIMEOUT;
    } else {
			timespec timeout;
			if (INFINITE != timeoutMs) {
				// sets timeout
				calculateAbsoluteTimeByTimeout(timeoutMs, timeout);
			}

			// waits until condition thread returns control
			do {
				ret = (INFINITE == timeoutMs ? pthread_cond_wait(_handle._cond, _handle._mtx) : 
								pthread_cond_timedwait(_handle._cond, _handle._mtx, &timeout));
			} while (!ret && !_handle._initialCount);
		}
	}

	// adjusts count member
	switch (ret) {
		case 0: // success
			--_handle._initialCount;
			return WAIT_OBJECT_0;
		case ETIMEDOUT:
		default:
			return WAIT_TIMEOUT;
	}
#endif
}


}
