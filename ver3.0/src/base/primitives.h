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

#ifndef _terimber_primitives_h_
#define _terimber_primitives_h_

#include "allinc.h"

namespace terimber {

//////////////////////////////////////////////////////////////////////
//! \class mutex
//! \brief class supports mutex
class mutex {
	// prevents copy objects
	//! \brief copy constructor
	mutex(const mutex& x);
	//! \brief assign operator
	mutex& operator=(const mutex& x);

public:
	//! \typedef _HANDLE_
#if OS_TYPE == OS_WIN32
	typedef CRITICAL_SECTION _HANDLE_;
#else
	typedef pthread_mutex_t _HANDLE_;
#endif

	//! \brief constructor
	mutex();
	//! \brief destructor
	~mutex();
	//! \brief locks mutex
	//! NB!!! can be used in the const member function
	bool lock() const;
	//! \brief unlocks mutex
	//! NB!!! can be used in the const member function
	void unlock() const;
	//! \brief tries to locks mutex
	//! NB!!! can be used in the const member function
	bool trylock() const;
	operator _HANDLE_*() {
		return &_handle;
	}
	//! \brief returns const handle
	operator const _HANDLE_*() const {
		return &_handle;
	}
private:
	 mutable _HANDLE_ _handle;								//!< handle of mutex resource
};

//! \class mutexKeeper
//! \brief automatic mutex locker
class mutexKeeper {
public:
	//! \brief constructor
	mutexKeeper(	const mutex& mtx,						//!< mutex reference
					bool use_try = false					//!< try lock or make a blocking call
					) : _mtx(mtx) {
		_locked = use_try ? _mtx.trylock() : _mtx.lock();
	}
	//! \brief destructor
	~mutexKeeper() {
		if (_locked) {
			_mtx.unlock();
    }
	}
	//! \brief unlocks explicitly
	inline void unlock() const {
		if (_locked) {
			_mtx.unlock();
			_locked = false;
		}
	}
	//! \brief locks explicitly
	inline void lock() const {
		if (!_locked) {
			_mtx.lock();
			_locked = true;
		}
	}
	//! \brief checks the lock state
	inline operator bool() const {
		return _locked;
	}
	//! \brief checks the lock state
	inline bool operator!() const {
		return !_locked;
	}
private:
	const mutex&	_mtx;									//!< mutex reference
	mutable bool	_locked;						  //!< lock flag for explicit unlock
};


///////////////////////////////////////////////////////////////////
//! \class spinlock
//! \brief class supports mutex
class spinlock {
	// prevents copy objects
	//! \brief copy constructor
	spinlock(const spinlock& x);
	//! \brief assign operator
	spinlock& operator=(const spinlock& x);

public:

	//! \brief constructor
	spinlock();
	//! \brief destructor
	~spinlock();
	//! \brief locks spinlock
	//! NB!!! can be used in the const member function
	bool lock() const;
	//! \brief unlocks spinlock
	//! NB!!! can be used in the const member function
	void unlock() const;
	//! \brief tries to locks spinlock
	//! NB!!! can be used in the const member function
	bool trylock() const;
private:
	//! \typedef _HANDLE_
#if OS_TYPE == OS_WIN32
	typedef CRITICAL_SECTION _HANDLE_;
#else
	typedef volatile int64_t _HANDLE_;
#endif

	 mutable _HANDLE_ _handle;
};

//! \class spinlockKeeper
//! \brief automatic mutex locker
class spinlockKeeper {
public:
	//! \brief constructor
	spinlockKeeper(const spinlock& slock,		//!< spinlock reference
					       bool use_try = false)//!< try lock or make a blocking call
	  : _slock(slock) {
		_locked = use_try ? _slock.trylock() : _slock.lock();
	}
	//! \brief destructor
	~spinlockKeeper() {
		if (_locked) {
			_slock.unlock();
    }
	}
	//! \brief unlocks explicitly
	inline void unlock() const {
		if (_locked) {
			_slock.unlock();
			_locked = false;
		}
	}
	//! \brief locks explicitly
	inline void lock() const {
		if (!_locked) {
			_slock.lock();
			_locked = true;
		}
	}
	//! \brief checks the lock state
	inline operator bool() const {
		return _locked;
	}
	//! \brief checks the lock state
	inline bool operator!() const {
		return !_locked;
	}
private:
	const spinlock&	_slock;							//!< spinlock reference
	mutable bool	  _locked;						//!< lock flag for explicit unlock
};


/////////////////////////////////////////////////////////////////
//! \class event
//! \brief class supports event
class event {
	//! prevents the copying of objects
	//! \brief copy constructor
	event(const event& x);
	//! \brief assign operator
	event& operator=(const event& x);
public:

	//! \typedef _HANDLE_
#if OS_TYPE == OS_WIN32
	typedef HANDLE _HANDLE_;
#else
	typedef class HANDLE {
	public:
		mutex				      _mtx;							//!< mutex
		pthread_cond_t*   _cond;						//!< condition thread
		bool				      _manualReset;		  //!< reset flag
		bool				      _signaled;        //!< signal flag
	} *_HANDLE_;
#endif

	//! \brief constructor
	event(bool manualReset = false,			//!< manual reset
			  bool signalState = false);					//!< initial state
	//! \brief destructor
	~event();
	//! \brief signal event
	//! NB!!! can be used in the const member function
	void set() const;
	//! \brief nonsignal event
	//! NB!!! can be used in the const member function
	void reset() const;
	//! \brief returns handle
	operator _HANDLE_() const {
		return
#if OS_TYPE == OS_WIN32
		_handle;
#else
		&_handle;
#endif
	}

	//! \brief waits for the signal state
	//! NB!!! can be used in the const member function
	size_t wait(size_t timeoutMs = INFINITE) const;
private:
	mutable HANDLE _handle;									//!< event handler
};

//! \class semaphore
//! \brief class supports semaphores
class semaphore {
	//! prevents the copying of objects
	//! \brief copy constructor
	semaphore(const semaphore& x);
	//! \brief assign operator
	semaphore& operator=(const semaphore& x);

public:
	//! \typedef _HANDLE_
#if OS_TYPE == OS_WIN32
	typedef HANDLE _HANDLE_;
#else
	typedef class HANDLE {
	public:
		mutex				      _mtx;						//!< mutex
		pthread_cond_t*		_cond;					//!< condition thread
		size_t				    _initialCount;	//!< initial locks
		size_t				    _maxCount;			//!< max locks
	} *_HANDLE_;
#endif

	//! \brief constructor
	semaphore(size_t initialCount = 1,  //!< initial locks
				    size_t maxCount = 1);		  //!< max locks
	//! \brief destructor
	~semaphore();
	//! \brief releases semaphore
	//! NB!!! can be used in the const member function
	bool release(size_t count = 1) const;	//!< unlock counts
	//! \brief waitd for signal state
	//! NB!!! can be used in the const member function
	size_t wait(size_t timeoutMs = INFINITE) const;

private:
	mutable HANDLE _handle;									//!< keep semaphore handle
};

//! \class semaphoreKeeper
//! \brief automatic semaphore keeper
class semaphoreKeeper {
public:
	//! \brief constructor
	semaphoreKeeper(const semaphore& sema,				//!< semaphore reference
						      size_t timeoutMs = INFINITE)	//!< timeout in milliseconds
	  : _sema(sema) {
		_acquired = WAIT_OBJECT_0 == _sema.wait(timeoutMs);
	}
	//! \brief destructor
	~semaphoreKeeper() {
		if (_acquired) {
			_sema.release();
    }
	}
	//! \brief checks the access state
	inline operator bool() const {
		return _acquired;
	}
	//! \brief operator!
	inline bool operator!() const {
		return !_acquired;
	}

private:
	const semaphore&	_sema;								//!< semaphore reference
	bool				      _acquired;						//!< result of accessing
};

}

#endif
