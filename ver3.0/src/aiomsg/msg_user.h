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

#ifndef _terimber_msg_user_h_
#define _terimber_msg_user_h_

#include "aiomsg/msg_conn.h"
#include "aiomsg/msg_lsnr.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class msg_wait_reply
//! \brief implements the message reply keeper for synchronous send
class msg_wait_reply
{
public:
	//! \brief constructor
	msg_wait_reply(	event* event_							//!< event pointer
					);
	//! \brief destructor
	~msg_wait_reply() 
	{
	}
	
	//! \brief sets the incoming reply
	//! assigns pointer and fire event
	inline 
	void 
	set_reply(		msg_cpp* msg_							//!< message pointer
					) 
	{ 
		// assigns reply pointer
		_reply = msg_; 
		// sets signal
		_event->set(); 
	}

	//! \brief returns stored reply and clear the stored pointer
	inline 
	msg_cpp* 
	get_reply()
	{
		// remembers reply message pointer
		msg_cpp* retVal = _reply; 
		// reset 
		_reply = 0;
		// return
		return retVal; 
	}
private:
	event*				_event;								//!< keeps the pointer to the event object
	msg_cpp*			_reply;								//!< keeps the pointer to the reply message
};

//! \class msg_wait_async_reply
//! \brief implements the message reply keeper for asynchronous send
class msg_wait_async_reply
{
public:
	//! \brief constructor
	msg_wait_async_reply(const guid_t& ident,				//!< unique message marker
						ub8_t timeout_						//!< timeout in milliseconds
						);
	//! \brief destructor
	~msg_wait_async_reply() 
	{
	}
	//! \brief sets incoming reply
	inline 
	void 
	set_reply(		msg_cpp* msg_							//!< message pointer
					) 
	{ 
		_reply = msg_; 
	}
	//! \brief gets stored reply
	inline 
	msg_cpp* 
	get_reply() 
	{ 
			// remembers reply message pointer
		msg_cpp* retVal = _reply; 
		// reset 
		_reply = 0;
		// return
		return retVal; 
	}
	//! \brief gets stored reply + ident
	inline 
	msg_cpp* 
	get_reply_data(	guid_t& ident							//!< [out] message ident
					)
	{ 
		// remembers reply message pointer
		msg_cpp* retVal = _reply; 
		// reset 
		_reply = 0;
		// assign ident
		ident = _ident; 
		// return
		return retVal; 
	}
	//! \brief checks expiration date
	inline 
	bool 
	is_expired(sb8_t now) const 
	{ 
		return _expired < now;	
	}

private:
	guid_t					_ident;							//!< keep ident
	msg_cpp*				_reply;							//!< keep the pointer to reply message
	sb8_t					_expired;						//!< keep date expired
};

//! \class msg_user_connection
//! \brief implements user connection - the input/output channel to the business logic world
class msg_user_connection : public msg_connection
{
	// typedefs
	//! \typedef reply_map_t
	//! \brief maps guid to synchronous replies
	typedef map< guid_t, msg_wait_reply >		reply_map_t;
	//! \typedef reply_async_map_t
	//! \brief maps guid to asynchronous replies
	typedef map< guid_t, msg_wait_async_reply > reply_async_map_t;
	//! \typedef reply_async_list_t
	//! \brief list of received asynchronous replies
	typedef list< msg_wait_async_reply >		reply_async_list_t;
public:
	//! \brief constructor
	msg_user_connection(msg_communicator* communicator,		//!< communicator pointer
					msg_callback_notify* callback,			//!< user callback
					const conf_connection& info,			//!< connection info
					size_t additional_threads = 0			//!< additional threads
					);
	//! \brief destructor
	virtual ~msg_user_connection();
	//! \brief overrides pushing message to the queue
	//! to avoid the dead blocking situation
	virtual 
	void 
	push_msg(		msg_cpp* msg							//!< message pointer
					);
	//! \brief establishes the user type connection
	static 
	msg_user_connection* 
	connect(		msg_communicator* communicator,			//!< communicator pointer
					msg_callback_notify* callback,			//!< user callback
					const conf_connection& info,			//!< connection info
					size_t additional_threads				//!< additional threads
					);
	//! \brief sends message synchronically
	bool 
	send(			bool copy,								//!< flag make a copy of input message
					msg_cpp* msg,							//!< input message
					msg_cpp*& reply							//!< reply pointer
					);
	//! \brief sends message asynchronousally
	guid_t 
	send_async(		bool copy,								//!< flag make a copy of input message
					msg_cpp* msg							//!< input message
					);
	//! \brief posts message
	bool 
	post(			bool copy,								//!< flag make a copy of input message
					msg_cpp* msg							//!< input message
					);
	//! \brief returns the last error
	inline 
	const char* 
	get_last_error() const 
	{ 
		return _error; 
	}
protected:
	//! \brief picks asynchronous wait list
	inline 
	bool 
	peek_async() const 
	{ 
		mutexKeeper keeper(_mtx_async_wait); 
		return !_async_list.empty(); 
	}
	//! \brief pops the top message from asynchronous wait list
	bool 
	pop_async(		msg_cpp*& msg,							//!< [out] message pointer
					guid_t& ident							//!< [out] message ident
					);
	//! \brief checks if there is asynchronous reply
	bool 
	peek_async();
	//! \brief removes timeouted asynchronous replies
	bool 
	pop_async_timeouted(guid_t& ident						//!< [out] message ident
					);
	//! \brief checks if there is asynchronous tiemouted reply
	bool 
	peek_async_timeouted();

	//! \brief inserts ping message
	virtual 
	void 
	ping_notify();
protected:
	//! overrides the base functionality
	//! \brief check job
	virtual 
	bool 
	v_has_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);
	//! \brief does the job
	virtual 
	void 
	v_do_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);
	//! \brief overrides the base functionality
	virtual 
	void 
	wakeup();

	//! \brief action on turning off
	virtual 
	void 
	v_off();
private:
	//! \brief processes incoming messages
	void 
	process_income_message();
private:
	string_t				_error;							//!< last error description
	msg_callback_notify*	_callback;						//!< user callback function
	mutex					_mtx_wait;						//!< mutex for waiting list
	mutex					_mtx_async_wait;				//!< mutex for asynchronous waiting list
	reply_map_t				_map;							//!< map: marker -> wait reply object
	reply_async_map_t		_async_map;						//!< map: marker -> async wait reply object
	reply_async_list_t		_async_list;					//!< list: async wait reply objects
															//! when async reply incoming the waiting object is moved from the map to the list
	size_t					_additional_threads;			//! additional treads
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_msg_comm_h_

