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
#include "aiomsg/msg_conn.h"
#include "aiomsg/msg_lsnr.h"
#include "base/list.hpp"
#include "base/template.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/number.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

extern exception_table msgMsgTable;

msg_connection::msg_connection(msg_communicator* communicator_, const conf_listener& linfo,	const conf_connection& info_) : 
	msg_queue_processor(communicator_), 
	_info(info_), 
	_linfo(linfo),
	_state(CONN_STATE_HANDSHAKE_RECEIVER),
	_rsa(rsa_key_size, info_._support_crypt)
{
	if (_info._support_crypt)
	{
		_communicator->get_msg_key(_info._crypt_private, _info._crypt_external, _info._session);
	}
}

msg_connection::msg_connection(msg_communicator* communicator_, const conf_connection& info_) : 
	msg_queue_processor(communicator_), 
	_info(info_), 
	_linfo(),
	_state(CONN_STATE_HANDSHAKE_INITIATOR),
	_rsa(rsa_key_size, info_._support_crypt)
{
	if (_info._support_crypt)
	{
		_communicator->get_msg_key(_info._crypt_private, _info._crypt_external, _info._session);
	}
}

// virtual 
msg_connection::~msg_connection()
{
	// clears message queue
	msg_cpp* msg = 0;
	while (pop(msg))
	{
		// destroys message
		_communicator->destroy_msg(msg);
	}
}

// virtual 
void msg_connection::ping_notify()
{
	// sets ping message
	msg_cpp* msg = 0;
	try
	{
		msg_creator creator(_communicator);
		msg_pointer_t msg_(creator);

		// constructs ping message
		msg_ = _communicator->construct_ping();
		// sets transport info
		msg_->_receiver = _info._address;
		// pushes message to the queue
		push_msg(msg_);
		msg_.detach();
		// sets new activity time
		set_last_activity();
	}
	catch (exception&)
	{
	}
	catch (...)
	{
		assert(false);
	}
}

// virtual 
void 
msg_connection::push_msg(msg_cpp* msg_) 
{ 
	msg_->_sessionid = _info._session; 
	push(msg_); 
}
	
//virtual 
void 
msg_connection::wakeup()
{
	_communicator->get_thread_manager().borrow_thread(queue_thread_ident, 0, this, stay_on_alert_time);
}

msg_cpp* 
msg_connection::prepare_handshake_msg()
{
	msg_cpp* msg = _communicator->construct_handshake(get_rsa());
	// set destination transport info
	msg->_receiver = _info._address;
	// set session
	_info._session = msg->_sessionid;

	return msg;
}

msg_cpp* 
msg_connection::prepare_handshake_reply(msg_cpp* msg)
{
	// saves old address
//	guid_t old_address = _info._address;
	// checks acceptence and assigns new address here
	msg_listener::accept_address(msg->_sender, _linfo, _info);
	// remembers session id
	_info._session = msg->_sessionid;
	// reset address
	_info._address = msg->_sender;
	// prepare reply message
	msg_cpp* reply = _communicator->reply_handshake(msg, _info._support_crypt ? &_info._crypt_private : 0);
	// sets state
	_state = CONN_STATE_CONNECTED;
	// sets timeout
	reply->timeout = msg->timeout;

	return reply;
}

void 
msg_connection::validate_handshake_reply(msg_cpp* msg)
{
	// we have the handshake reply
	_communicator->check_handshake(_info._session, msg, get_rsa(), _info._crypt_private);
	_info._address = msg->_sender;
	_state = CONN_STATE_CONNECTED;
}

void
msg_connection::process_incoming_message(msg_cpp* msg)
{
	// checks what we have
	// 1. incoming handshake
	// 2. handshake reply
	// 3. ping
	// 4  user message/reply

	switch (msg->_type)
	{
		case handshake_type:
			// checks if we are in the handshake state
			switch (_state)
			{
				case CONN_STATE_HANDSHAKE_INITIATOR:
					// we have the handshake reply
					_communicator->check_handshake(_info._session, msg, get_rsa(), _info._crypt_private);
					_info._address = msg->_sender;
					_state = CONN_STATE_CONNECTED;
					wakeup(); // just sends another message
					break;
				case CONN_STATE_HANDSHAKE_RECEIVER:
					{
						// we have an incoming handshake
						msg_creator creator(_communicator);
						msg_pointer_t reply(creator);
						// saves old address
						guid_t old_address = _info._address;
						// checks acceptence and assigns new address here
						msg_listener::accept_address(msg->_sender, _linfo, _info);
						// remembers session id
						_info._session = msg->_sessionid;
						// reassigns address
						_communicator->change_connection_address(old_address, this);
 						reply = _communicator->reply_handshake(msg, _info._support_crypt ? &_info._crypt_private : 0);
						// sets timeout
						reply->timeout = msg->timeout;

						// sets state
						_state = CONN_STATE_CONNECTED;
						push(reply);
						reply.detach();
					}
					break;
				default:
					exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);
			}

			// destroys message
			_communicator->destroy_msg(msg);

			break;
		case system_type:
			// checks session and ping msgid
			if (_state == CONN_STATE_CONNECTED && msg->_sessionid == _info._session && msg->msgid == msg_id_ping)
				_communicator->destroy_msg(msg);
			else
				exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);
			break;
		case user_type_send:
		case user_type_send_async:
		case user_type_post:
		case user_type_reply:
		case user_type_reply_async:
			// checks session
			if (_state != CONN_STATE_CONNECTED || msg->_sessionid != _info._session)
				exception::_throw(MSG_RESULT_INVALID_SESSION, &msgMsgTable);

			_communicator->comm_msg(msg);
			break;
		default:
			exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);
	}

	set_last_activity();
}

#pragma pack()
END_TERIMBER_NAMESPACE
