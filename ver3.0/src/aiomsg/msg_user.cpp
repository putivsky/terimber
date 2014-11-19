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

#include "aiomsg/msg_user.h"
#include "base/map.hpp"
#include "base/list.hpp"
#include "base/memory.hpp"
#include "base/string.hpp"
#include "base/common.hpp"
#include "base/template.hpp"
#include "base/stack.hpp"
#include "base/number.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

static const char* s_err_text = "Asyncronic sending timeout occured";

// we need here to additional threads
// ident == 0 as usual for queue messages (all functionality in base class
// ident == 1 for asynchronous callbacks
// ident == 2 for asynchronous timeouts
const size_t callbacks_thread_ident = 0;
const size_t timeouts_thread_ident = 1;

msg_wait_reply::msg_wait_reply(event* event_) :
	_event(event_), _reply(0)
{
	assert(_event);
}

msg_wait_async_reply::msg_wait_async_reply(const guid_t& ident, ub8_t timeout_) :
	_ident(ident), _reply(0)
{
	date date_;
	date_ += timeout_;
	_expired = date_ ;
}

////////////////////////////////////////////////////////////////
msg_user_connection::msg_user_connection(msg_communicator* communicator_, msg_callback_notify* callback_, const conf_connection& info_, size_t additional_threads) : 
	msg_connection(communicator_, info_), _callback(callback_),
	_additional_threads(additional_threads)
{
}

// virtual 
msg_user_connection::~msg_user_connection()
{
	// clears wait reply
	mutexKeeper keeper(_mtx_wait);
	for (reply_map_t::iterator iter = _map.begin(); iter != _map.end(); ++iter)
	{
		msg_cpp* msg = iter->get_reply();
		if (msg)
			_communicator->destroy_msg(msg);

		iter->set_reply(0);
	}

	_map.clear();

	// clears wait reply async map
	mutexKeeper keeper_async(_mtx_async_wait);
	for (reply_async_map_t::iterator iter_async = _async_map.begin(); iter_async != _async_map.end(); ++iter_async)
	{
		msg_cpp* msg = iter_async->get_reply();
		_communicator->destroy_msg(msg);
	}

	_async_map.clear();

	// clears wait reply async list
	for (reply_async_list_t::iterator iter_list = _async_list.begin(); iter_list != _async_list.end(); ++iter_list)
	{
		msg_cpp* msg = iter_list->get_reply();
		_communicator->destroy_msg(msg);
	}

	_async_list.clear();
//	_counter_generator.clear();
}

// instead of ping message - let's check the timeouted asynchronous requests
// virtual 
void 
msg_user_connection::ping_notify()
{
	if (peek_async_timeouted())
		// wakes up the third thread
		_communicator->get_thread_manager().borrow_thread(timeouts_thread_ident, 0, this, stay_on_alert_time);

	set_last_activity();
}

//virtual 
void 
msg_user_connection::wakeup()
{
	if (!_additional_threads || !_communicator->get_thread_manager().borrow_from_range(queue_thread_ident, queue_thread_ident + _additional_threads, 0, this, stay_on_alert_time))
		_communicator->get_thread_manager().borrow_thread(queue_thread_ident, 0, this, stay_on_alert_time);
}

//! \brief action on turning off
//virtual 
void 
msg_user_connection::v_off() 
{
	mutexKeeper keeper(_mtx_wait);
	for (reply_map_t::iterator iter = _map.begin(); iter != _map.end(); ++iter)
	{
		msg_cpp* msg = iter->get_reply();
		if (msg)
			_communicator->destroy_msg(msg);

		iter->set_reply(0);
	}


	msg_queue_processor::v_off();
}
// virtual 
void 
msg_user_connection::push_msg(msg_cpp* msg_)
{
	// analyzes type
	switch (msg_->_type)
	{
		case user_type_reply:
			{
				// locks mutex
				mutexKeeper keeper(_mtx_wait);
				reply_map_t::iterator iter = _map.find(msg_->_marker);
				if (iter != _map.end())
					iter->set_reply(msg_);
				else
					_communicator->destroy_msg(msg_);
			}
		break;
		case user_type_reply_async:
			{
				// locks mutex
				mutexKeeper keeper_async(_mtx_async_wait);

				reply_async_map_t::iterator iter_async = _async_map.find(msg_->_marker);
				if (iter_async != _async_map.end()) // find reply
				{
					// it can be expired message returned later, but with the same ident we are reusing now
					// checks it's datestamp
					// sets reply
					iter_async->set_reply(msg_);
					// moves wait reply from map to list
					_async_list.push_back(*iter_async);
					_async_map.erase(iter_async);
					// wakes up the second thread
					_communicator->get_thread_manager().borrow_thread(callbacks_thread_ident, 0, this, stay_on_alert_time);
				}
				else 
				{
					_communicator->destroy_msg(msg_);
					_communicator->get_thread_manager().borrow_thread(timeouts_thread_ident, 0, this, stay_on_alert_time);
				}
			}
		break;
		default:
			// incoming message
			msg_connection::push_msg(msg_);
		break;
	} // switch
}

bool 
msg_user_connection::send(bool copy, msg_cpp* msg_, msg_cpp*& reply_)
{
	if (!msg_ || msg_->_receiver == _info._address || msg_->_receiver == null_uuid)
	{
		_error = "Invalid receiver address";
		return false;
	}

	if (msg_->priority != MSG_PRIORITY_HIGH
		|| msg_->priority != MSG_PRIORITY_NORMAL)
	{
		msg_->priority = MSG_PRIORITY_NORMAL;
	}

	if (msg_->msgid & 0x80000000)
	{
		msg_->msgid &= 0x7FFFFFFF;
	}

	pool_object_keeper< event_pool_t > ekeeper(&_communicator->get_event_pool(), 0, (size_t)msg_->timeout); 
	if (!ekeeper)
	{
		_error = "no enough event resources";
		return false;
	}
    
	ekeeper->reset();

	msg_wait_reply _wait(ekeeper);

	ub8_t timeout = msg_->timeout;
	guid_t marker = uuid_gen();

	{
		mutexKeeper keeper(_mtx_wait);
		_map.insert(marker, _wait);
	}

	msg_creator creator(_communicator);
	msg_pointer_t msg__(creator);

	try
	{
		// makes a copy of the msg, if any
		msg__ = copy ? _communicator->copy_msg(msg_) : msg_;
		// sets transport info
		msg__->_sender = _info._address;
		msg__->_marker = marker;
	
		msg__->_type = user_type_send;
		// pushes message to the communicator queue
		_communicator->comm_msg(msg__);
		msg__.detach();
	}
	catch (exception& err)
	{
		// deletes message if copy was created
		if (!copy) 
			msg__.detach();

		// sets error text
		_error = err.what();
		// removes wait
		mutexKeeper keeper(_mtx_wait);
		_map.erase(marker);

		return false;
	}
	
	// waits for reply
	if (WAIT_OBJECT_0 == ekeeper->wait((size_t)timeout))
	{
		// locks mutex
		mutexKeeper keeper(_mtx_wait);
		reply_map_t::iterator iter = _map.find(marker);
		if (iter != _map.end()) // find reply
		{
			// gets reply
			reply_ = iter->get_reply();
			// erases from map
			_map.erase(iter);
			
			return true;
		}
	}
	else
	{
		// removes wait
		mutexKeeper keeper(_mtx_wait);
		_map.erase(marker);
	}

	// times out case
	_error = "Timeout occured";

	try
	{
		reply_ = _communicator->construct_msg(_error.length());

		reply_->msgid = MSG_ERROR_ID;
		memcpy(reply_->get_body(), (const char*)_error, _error.length());
	}
	catch (exception&)
	{
	}

	return true;
}

guid_t 
msg_user_connection::send_async(bool copy, msg_cpp* msg_)
{
	if (!msg_ || msg_->_receiver == _info._address|| msg_->_receiver == null_uuid)
	{
		_error = "Invalid receiver address";
		return null_uuid;
	}

	if (msg_->priority != MSG_PRIORITY_HIGH
		|| msg_->priority != MSG_PRIORITY_NORMAL)
	{
		msg_->priority = MSG_PRIORITY_NORMAL;
	}

	if (msg_->msgid & 0x80000000)
	{
		msg_->msgid &= 0x7FFFFFFF;
	}


	guid_t marker = uuid_gen();

	msg_wait_async_reply _wait(marker, msg_->timeout);

	msg_creator creator(_communicator);
	msg_pointer_t msg__(creator);

	{
		mutexKeeper keeper(_mtx_async_wait);
		_async_map.insert(marker, _wait);
	}


	try
	{
		// makes a copy of the message, if any
		msg__ = copy ? _communicator->copy_msg(msg_) : msg_;
		// set transport info
		msg__->_sender = _info._address;
		msg__->_marker = marker;

		msg__->_type = user_type_send_async;
		// pushes message to the communicator queue
		_communicator->comm_msg(msg__);
		msg__.detach();
	}
	catch (exception& err)
	{
		// deletes message if copy was created
		if (!copy) 
			msg__.detach();

		// sets error text
		_error = err.what();

		return null_uuid;
	}
	
	return marker;
}

bool 
msg_user_connection::post(bool copy, msg_cpp* msg_)
{
	if (!msg_ || msg_->_receiver == _info._address || msg_->_receiver == null_uuid)
	{
		_error = "Invalid receiver address";
		return false;
	}

	if (msg_->priority != MSG_PRIORITY_HIGH
		|| msg_->priority != MSG_PRIORITY_NORMAL)
	{
		msg_->priority = MSG_PRIORITY_NORMAL;
	}

	if (msg_->msgid & 0x80000000)
	{
		msg_->msgid &= 0x7FFFFFFF;
	}

	msg_creator creator(_communicator);
	msg_pointer_t msg__(creator);

	try
	{
		// makes a copy of the message, if any
		msg__ = copy ? _communicator->copy_msg(msg_) : msg_;
		// sets transport info
		msg__->_sender = _info._address;

		msg__->_type = user_type_post;
		// pushes message to the communicator queue
		_communicator->comm_msg(msg__);
		msg__.detach();
	}
	catch (exception& err)
	{
		// deletes message if copy was created
		if (!copy) 
			msg__.detach();
		// sets error text
		_error = err.what();
		return false;
	}

	return true;
}

// virtual 
bool 
msg_user_connection::v_has_job(size_t ident, void* user_data)
{
	if (!msg_base::is_on())
		return false;

	switch (ident)
	{
		case callbacks_thread_ident:
			return peek_async(); // async reply messages
		case timeouts_thread_ident: // timeouted asynchronous calls
			return peek_async_timeouted(); // there are timeouts
		case queue_thread_ident:  
		default:
			return msg_queue_processor::v_has_job(ident, user_data); // there are timeouts
	}
}
// virtual 
void 
msg_user_connection::v_do_job(size_t ident, void* user_data)
{
	switch (ident)
	{
		case callbacks_thread_ident:
			{
				msg_creator creator(_communicator);
				msg_cpp*				msg_ = 0;
				guid_t					marker;
				if (pop_async(msg_, marker))
				{
					msg_pointer_t msg(creator);
					msg = msg_;

					if (_callback) // asynch call
					{
						try 
						{ 
							if (_callback->async_callback(msg, marker)) 
								msg.detach(); 
						} // who knows what user can throw?
						catch (...) 
						{
						}// and what are we suppose to do here?				
					} // if callback
				}
			}
			break;
		case timeouts_thread_ident:
			{
				guid_t					marker;
				while (pop_async_timeouted(marker))// checks async time expired
				{
					if (_callback) // asynch call
					{
						msg_creator creator(_communicator);
						msg_pointer_t err(creator, 0);
						msg_pack::make_error_msg(err, s_err_text);
						//err->_receiver = err->_destination = err->_sender = err->_initiator = _info._address;
						err->_receiver = err->_sender = _info._address;
						try 
						{ 
							if (_callback->async_callback(err, marker)) 
								err.detach(); 
						} // who knows what the user can throw?
						catch (...) 
						{
						}// and what are we suppose to do here?	
					}			
				}
			}
			break;
		case queue_thread_ident: // incoming messages
		default:
			process_income_message();
	} // switch
}

void 
msg_user_connection::process_income_message()
{
	msg_creator creator(_communicator);
	msg_cpp* msg_ = 0;

	if (pop(msg_))
	{
		msg_pointer_t msg(creator);
		msg = msg_;

		if (_additional_threads && peek()) // additional messages
		{
			if (!_additional_threads || !_communicator->get_thread_manager().borrow_from_range(queue_thread_ident,  queue_thread_ident + _additional_threads, 0, this, stay_on_alert_time))
				_communicator->get_thread_manager().borrow_thread(queue_thread_ident, 0, this, stay_on_alert_time);	
		}

		// checks callback
		if (_callback)
		{
			try
			{
				// sender is waiting for a reply
				if (msg->_type == user_type_send || msg->_type == user_type_send_async)
				{
					msg_pointer_t reply(creator);
					reply = _communicator->construct_msg(0);

					// preserves timestamp, timeout, receiver
					sb8_t old_timestamp = msg->_timestamp;
					ub8_t old_timeout = msg->timeout;
					guid_t old_sender = msg_->_sender;

					msg_pack::make_reply_msg(msg, reply);

					if (_callback->incoming_callback(msg, reply)) 
						msg.detach();
					

					// restores timestamp, timeout, receiver
					reply->_timestamp = old_timestamp;
					reply->timeout = old_timeout;
					reply->_receiver = old_sender;

					
					_communicator->comm_msg(reply);
					reply.detach();
				}
				else if (msg->_type == user_type_post)
				{
					if (_callback->incoming_callback(msg, 0)) 
						msg.detach();
				}
				else
					assert(false);
			}
			catch (exception&) 
			{
			}
		} // if callback

	} // if pop
}

// pops the top message from asynchronous wait list
bool 
msg_user_connection::pop_async(msg_cpp*& msg_, guid_t& ident)
{
	mutexKeeper keeper(_mtx_async_wait);
	if (_async_list.empty()) return false;
	msg_ = _async_list.front().get_reply_data(ident);
	_async_list.pop_front();
	return true;
}

bool 
msg_user_connection::peek_async()
{
	mutexKeeper keeper(_mtx_async_wait);
	return !_async_list.empty();
}

bool 
msg_user_connection::pop_async_timeouted(guid_t& ident)
{
	mutexKeeper keeper(_mtx_async_wait);
	date now;
	for (reply_async_map_t::iterator iter = _async_map.begin(); iter != _async_map.end(); ++iter)
		if (iter->is_expired(now))
		{
			iter->get_reply_data(ident);
			_async_map.erase(iter);
			return true;
		}

	return false;
}

bool 
msg_user_connection::peek_async_timeouted()
{
	mutexKeeper keeper(_mtx_async_wait);
	date now;
	for (reply_async_map_t::iterator iter = _async_map.begin(); iter != _async_map.end(); ++iter)
		if (iter->is_expired(now))
			return true;

	return false;
}

// static 
msg_user_connection* 
msg_user_connection::connect(msg_communicator* communicator_, msg_callback_notify* callback_, const conf_connection& info_, size_t additional_thread)
{
	// creates the new user connection
	msg_user_connection* connection = new msg_user_connection(communicator_, callback_, info_, additional_thread);
	connection->_state = CONN_STATE_CONNECTED;

	try
	{
		// adds connection to the list
		communicator_->add_connection(connection);
	}
	catch (exception& x)
	{
		delete connection;
		throw x;
	}

	// return connection
	return connection;
}

#pragma pack()
END_TERIMBER_NAMESPACE
