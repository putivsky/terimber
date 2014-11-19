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

#ifndef _terimber_keymaker_h_
#define _terimber_keymaker_h_

#include "base/primitives.h"

namespace terimber {

//! \class keylocker
//! \brief class supports the locking on demand functionality
class keylocker {
public:
	//! \brief constructor
	keylocker(size_t maxReaders = ~0);//!< max locker readers
	//! client functions
	//! \brief read-only lock
	bool enter(size_t timeoutMs) const;
	//! \brief read-only unlock
	void leave() const;
	//! server functions
	//! \brief writes lock
	bool lock(size_t timeoutMs) const;
	//! \brief writes unlock
	void unlock() const;
private:
	const size_t	 _maxReaders;	  //!< max readers allowed
	mutable bool	 _opened;				//!< flag if door is open for readers
	mutable size_t _readers;			//!< number readers entered
	event			     _evCanOpen;		//!< signal to waiting readers - locker is open
	event			     _evCanClose;		//!< signal to writer - all readers left locker
	event			     _evCanEnter;		//!< signal to waiting readers - one reader left
	event			     _evCanLock;	  //!< signal to waiting writers - door is unlocked
	spinlock	     _lock;         //!< access to resources
};

//! \class keylockerWriter
//! \brief automatic locker keeper
class keylockerWriter
{
public:
	//! \brief constructor
	keylockerWriter(const keylocker& locker,		//!< keylocker object
							size_t timeoutMs = INFINITE)		//!< timeout in milliseconds
    : _locker(locker) {
		_locked = _locker.lock(timeoutMs);
  }	
  //! \brief destructor
	~keylockerWriter() {
	  if (_locked) {
		  _locker.unlock();
    }
  }
	//! \brief checks the locked state
	operator bool() const {
	  return _locked;
  }
	//! \brief checks the locked state
	bool operator!() const {
	  return !_locked;
  }
	//! \brief unlocks explicitly 
	void unlock() const  {
	  if (_locked) {
		  _locker.unlock();
      _locked = false;
    }
  }
	//! \brief locks explicitly
	void lock(size_t timeoutMs = INFINITE) const {
	  if (!_locked) {
		  _locked = _locker.lock(timeoutMs);
    }
  }
private:
	mutable bool		_locked;							//!< flag door is locked
	const keylocker&	_locker;							//!< keylocker object
};

//! \class keylockerReader
//! \brief class supports automatic entry/leave of gate
class keylockerReader
{
public:
	//! \brief constructor
	keylockerReader(const keylocker& locker,		//!< keylocker object
							    size_t timeoutMs = INFINITE) :
	  _locker(locker) {
	  _inside = _locker.enter(timeoutMs);
  }
	//! \brief destructor
	~keylockerReader() {
  	if (_inside) {
	  	_locker.leave();
    }
  }
	//! \brief checks access flag
	operator bool() const {
	  return _inside;
  }
	//! \brief checks access flag
	bool operator!() const{
	  return !_inside;
  }
	//! \brief unlocks explicitly 
	void leave() const {
	  if (_inside) {
		  _locker.leave();
      _inside = false;
    }
  }
	//! \brief locks explicitly
	void enter(size_t timeoutMs = INFINITE) const {
	  if (!_inside) {
		  _inside = _locker.enter(timeoutMs);
  }
}
private:
	mutable bool		_inside;							//!< flag client entered
	const keylocker&	_locker;							//!< keylocker object
};

}

#endif // _terimber_keymaker_h_

