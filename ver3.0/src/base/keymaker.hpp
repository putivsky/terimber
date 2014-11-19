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

#ifndef _terimber_keymaker_hpp_
#define _terimber_keymaker_hpp_

#include "base/keymaker.h"
#include "base/date.h"

namespace terimber {

inline
keylocker::keylocker(size_t capacity) :
	_capacity(capacity),
	_opened(true),
	_readers(0),
	_evCanOpen(true, true),
	_evCanClose(true, false),
	_evCanEnter(false, false),
  _evCanLock(false, false) {
}

// client calls
inline bool keylocker::enter(size_t timeoutMs) const {
	size_t spentTime = 0;
  // loops until we get resource or run out of time
	while (spentTime < timeoutMs) {
    // locks spinlock
	  spinlockKeeper guard(_lock);	// access to resources

    if (_opened && _readers < _capacity) {
      ++_readers;
      return true;
    }

    const bool closed = !_opened;
    const bool noVacancy = _readers >= _capacity;

    // unlocks mutex
		guard.unlock();

		// gets the current date
		date now;

    if (closed) {
			if (WAIT_OBJECT_0 != _evCanOpen.wait(timeoutMs - spentTime)) { // wait for unlock
				return false;
      }
    } else if (noVacancy) {
			if (WAIT_OBJECT_0 != _evCanEnter.wait(timeoutMs - spentTime)) { // wait for unlock
				return false;
      }
    }

		// adjusts timeoutMs
		spentTime += (size_t)date::get_difference(now);
	}

	return false;
}

inline void keylocker::leave() const {
  spinlockKeeper guard(_lock);	// access to resources
	assert(_readers > 0);
	--_readers; // decrements readers
	if (_opened) {// has client rooms and key is open
		_evCanEnter.set();
	}	else if (!_readers) { // last client left
		_evCanClose.set();
	}
}

// server calls
inline bool keylocker::lock(size_t timeoutMs) const {
	size_t spentTime = 0;
	while (spentTime < timeoutMs) { // loops until we get client resource or run out of time
    // locks spinlock
	  spinlockKeeper guard(_lock);	// access to resources
    const bool close = !_opened;
    if (_opened) { // still open
       _opened = false; // locks the door
	     _evCanOpen.reset(); // resets event
      if (!_readers) {
        break; // no readers we done
      }
	  }
	  // unlocks spinlock
		guard.unlock();
		// gets the current date
		date now;
    if (close) { // wait for server
			if (WAIT_OBJECT_0 != _evCanLock.wait(timeoutMs - spentTime)) { // wait for unlock
				return false;
      }
    } else { // wait for readers
		  if (WAIT_OBJECT_0 == _evCanClose.wait(timeoutMs - spentTime)) { // wait for readers left
        break;
      }
			// locks spinlock
			guard.lock();
			_opened = true; // unlocks the door on timeoutMs
      _evCanOpen.set(); // sets the event, lets readers enter first before...
      _evCanLock.set();
			return false;
    }
  }

	// resets signal
	_evCanClose.reset();
	return true;
}

inline void keylocker::unlock() const {
  // locks spinlock
	spinlockKeeper guard(_lock);	// access to resources
	if (_opened) {// should not happend
		return;
	}
	_opened = true; // unlocks the door
  _evCanOpen.set(); // notifies readers who are waiting for entry, if any
  _evCanLock.set(); // wake up other writters
}

inline keylockerWriter::keylockerWriter(const keylocker& locker,
                                        size_t timeoutMs) :
	_locker(locker) {
		_locked = _locker.lock(timeoutMs);
}
//
// destructor
//
inline keylockerWriter::~keylockerWriter() {
	if (_locked)
		_locker.unlock();
}
//
// checks the locked state
//
inline keylockerWriter::operator bool() const {
	return _locked;
}

inline bool keylockerWriter::operator!() const {
	return !_locked;
}

//! \brief unlocks explicitly
inline void keylockerWriter::unlock() const {
	if (_locked)
		_locker.unlock(), _locked = false;
}

//! \brief locks explicitly
inline void keylockerWriter::lock(size_t timeoutMs) const {
	if (!_locked)
		_locked = _locker.lock(timeoutMs);
}

////////////////////////////////////////////////////////

// class supports automatic entry/leave of gate
//
// constructor
//
inline keylockerReader::keylockerReader(const keylocker& locker, size_t timeoutMs) :
	_locker(locker) {
	_inside = _locker.enter(timeoutMs);
}
//
// destructor
//
inline keylockerReader::~keylockerReader() {
	if (_inside) {
		_locker.leave();
  }
}
// checks access flag

inline keylockerReader::operator bool() const {
	return _inside;
}

inline bool keylockerReader::operator!() const {
	return !_inside;
}

//! \brief unlocks explicitly
inline void keylockerReader::leave() const {
	if (_inside) {
		_locker.leave();
    _inside = false;
  }
}

//! \brief locks explicitly
inline void keylockerReader::enter(size_t timeoutMs) const{
	if (!_inside) {
		_inside = _locker.enter(timeoutMs);
  }
}

}

#endif // _terimber_keymaker_hpp_

