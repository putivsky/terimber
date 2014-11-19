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

#ifndef _terimber_msg_que_h_
#define _terimber_msg_que_h_

#include "base/except.h"
#include "base/primitives.h"
#include "threadpool/threadpoolfactory.h"
#include "aiomsg/msg_cpp.h"
#include "aiomsg/msg_base.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class msg_queue
//! \brief priority message queue
template < size_t P = 3, size_t C = 1024 >
class msg_queue
{
	//! \enum en_msg_queue
	enum en_msg_queue
	{ 
		PRIORITY = P,										//!< priority levels 
		CAPACITY = C										//!< max capacity
	};
public:
	//! \brief blocks queue
	inline 
	bool 
	block();
	//! \brief checks the blocking state
	inline 
	bool 
	is_block();
protected:
	//! \brief constructor
	msg_queue();
	//! \brief removes message from queue
	inline 
	bool 
	pop(			msg_cpp*& item							//!< [out] pointer to message
					);
	//! \brief checks if there is a message in a queue
	inline 
	bool 
	peek();
	//! \brief checks the top priority message in a queue
	inline 
	bool 
	touch(			size_t& top_priority					//!< [out] the top priority message in a queue
					);
	//! \brief we don't know how to wake up yet
	virtual 
	void 
	wakeup() = 0;
	//! \brief pushes incoming message to the queue
	inline 
	void 
	push(			msg_cpp* item							//!< input message
					);
	//! \brief unblocks queue
	inline 
	bool 
	unblock();

private:
	//! \brief returns the priority within the range
	static 
	inline 
	ub1_t 
	_check(			msg_cpp* item							//!< pointer to message
					);
private:
	bool					_blocked;						//!< block flag
	mutex					_mtx_queue;						//!< mutex
	list< msg_cpp* >		_queue[PRIORITY];				//!< priority queues
};

template < size_t P, size_t C >
msg_queue< P, C >::msg_queue() : 
	_blocked(false) 
{
} 
// static
template < size_t P, size_t C >
inline 
ub1_t
msg_queue< P, C >::_check(msg_cpp* item)
{ 
	return item->priority >= PRIORITY ? PRIORITY - 1 : (ub1_t)item->priority; 
} 

template < size_t P, size_t C >
inline 
void
msg_queue< P, C >::push(msg_cpp* item)
{
	// locks mutex
	mutexKeeper keeper(_mtx_queue);
	if (_blocked) // queue blocked 
		exception::_throw("Queue has been blocked");
	// gets the correspondent queue
	list< msg_cpp* >& q = _queue[_check(item)];
	if (q.size() == CAPACITY) // out of space
		exception::_throw("Queue max capacity has been reached");
	// adds item to queue
	q.push_back(item);
	// wakes up thread
	wakeup();
} 

template < size_t P, size_t C >
inline 
bool
msg_queue< P, C >::pop(msg_cpp*& item)
{
	// locks mutex
	mutexKeeper keeper(_mtx_queue);
	// loop for all queues
	for (ub1_t index = 0; index < PRIORITY; ++index)
	{
		if (!_queue[index].empty())
		{
			// gets the message pointer
			item = _queue[index].front();
			// removes message from queue
			_queue[index].pop_front();
			return true;
		}
	}
	return false;
} 

template < size_t P, size_t C >
inline 
bool
msg_queue< P, C >::peek()
{
	// locks mutex
	mutexKeeper keeper(_mtx_queue);
	// loop for all queues
	for (ub1_t index = 0; index < PRIORITY; ++index)
	{
		if (!_queue[index].empty()) 
			return true;
	}
	return false;
} 

template < size_t P, size_t C >
inline 
bool
msg_queue< P, C >::touch(size_t& top_priority)
{
	// locks mutex
	mutexKeeper keeper(_mtx_queue);
	// loop for all queues
	for (ub1_t index = 0; index < PRIORITY; ++index)
	{
		if (!_queue[index].empty()) 
		{ 
			top_priority = index; 
			return true; 
		}
	}
	return false;
}

template < size_t P, size_t C >
inline 
bool
msg_queue< P, C >::block()
{ 
	// locks mutex
	mutexKeeper keeper(_mtx_queue); 
	return !_blocked ? (_blocked = true) : false; 
} 

template < size_t P, size_t C >
inline 
bool 
msg_queue< P, C >::unblock()
{ 
	// locks mutex
	mutexKeeper keeper(_mtx_queue); 
	return _blocked ? !(_blocked = false) : false; 
}

template < size_t P, size_t C >
inline 
bool
msg_queue< P, C >::is_block()
{ 
	// locks mutex
	mutexKeeper keeper(_mtx_queue); 
	return _blocked; 
} 

// forwards declaration
class msg_communicator;
//! \class msg_queue_processor
//! \brief class inherits thread processor class
// and msg_queue class
class msg_queue_processor : public msg_base, 
							public msg_queue< 3 >, 
							public terimber_thread_employer
{
public:
	//! \brief constructor
	msg_queue_processor(msg_communicator* communicator		//!< pointer to communicator
						);
	//! \brief destructor
	~msg_queue_processor();
protected:
	//! \brief checks queue
	virtual 
	bool 
	v_has_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);

protected:
	//! \brief action on turning on
	virtual 
	void 
	v_on();
	//! \brief action on turning off
	virtual 
	void 
	v_off(); 
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_msg_que_h_

