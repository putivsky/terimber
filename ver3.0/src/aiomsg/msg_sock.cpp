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

#include "aiomsg/msg_sock.h"

#include "base/list.hpp"
#include "base/map.hpp"
#include "base/memory.hpp"
#include "base/string.hpp"
#include "base/template.hpp"
#include "base/except.h"
#include "base/number.hpp"
#include "base/common.hpp"

#include <oserror.h>

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

extern exception_table msgMsgTable;
extern exception_table aiosockTable;

// anylizes the socket status
void sockStatus(aiosock& port, int status_)
{
	if (status_) // throws exception from socket error table
	{
		int code = 0;
		char err[1024];
		err[0] = 0;
		port.get_error_description( 
#if OS_TYPE == OS_WIN32
		(code = WSAGetLastError()),
#else
		(code = errno),
#endif
		err, sizeof(err) - 1);
		exception::_throw(err);
	}
}

////////////////////////////////////////////////////////////////
msg_sock_connection::msg_sock_connection(msg_communicator* communicator_, const conf_connection& info_) :
	msg_connection(communicator_, info_), 
	_msg_send(0),
	_offset_send(0),
	_msg_recv(0),
	_size_recv(0),
	_offset_recv(0)
{  
	_handle = _communicator->get_aiosock().create(this, true);
}

msg_sock_connection::msg_sock_connection(msg_communicator* communicator_, size_t handle, const conf_listener& linfo, const conf_connection& info) :
	msg_connection(communicator_, linfo, info), 
	_msg_send(0),
	_offset_send(0),
	_msg_recv(0),
	_size_recv(0),
	_offset_recv(0)
{  
	_handle = handle;
}

msg_sock_connection::~msg_sock_connection()
{
	// closes socket
	if (_handle)
	{
		_communicator->get_aiosock().close(_handle);
		_handle = 0;
	}

	if (_msg_send)
	{
		_communicator->destroy_msg(_msg_send);
		_msg_send = 0;
	}
	
	if (_msg_recv)
	{
		_communicator->destroy_msg(_msg_recv);
		_msg_recv = 0;
		_size_recv = 0;
	}
}

// port will call function after error occured
// virtual 
void 
msg_sock_connection::v_on_error(size_t handle, int err, aiosock_type mask, void* userdata)
{
	assert(handle == _handle);
	// shuts down connection
	_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "shutdown conection, error: %x, mask: %d", err, mask);
	_communicator->shutdown_connection(this);
}

// port will call function after successfully connecting to socket
// virtual 
void 
msg_sock_connection::v_on_connect(size_t handle, const sockaddr_in& peeraddr, void* userdata)
{
	assert(handle == _handle);
	// connection established - initiates handshake
	msg_creator creator(_communicator);
	msg_pointer_t msg(creator);

	try
	{
		msg = prepare_handshake_msg();
		// sends handshake message
		push(msg);
		msg.detach();

		int err = _communicator->get_aiosock().receive(_handle, &_size_recv, sizeof(ub4_t), INFINITE, 0, 0);
		sockStatus(_communicator->get_aiosock(), err);
	}
	catch (exception& x)
	{
		_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "shutdown conection, error: %s", x.what());
		_communicator->shutdown_connection(this);
	}
}

// port will call function after successfully sending buffer to socket
// virtual 
void 
msg_sock_connection::v_on_send(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& peeraddr, void* userdata)
{
	assert(handle == _handle);
	// checks if we send all bytes
	if (requested != processed)
	{
		assert(requested < processed);
		_offset_send += (ub4_t)processed;
		int err = _communicator->get_aiosock().send(_handle, _msg_send->get_block() + _offset_send, requested - processed, (size_t)_msg_send->timeout, 0, 0);

		try
		{
			sockStatus(_communicator->get_aiosock(), err);
		}
		catch (...)
		{
			// shuts down connection
			_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "shutdown conection, error: %x", err);
			_communicator->shutdown_connection(this);
		}
	}
	else
	{
		// destroys message and wakes up processing thread
		_communicator->destroy_msg(_msg_send);
		_msg_send = 0;
		_offset_send = 0;
		wakeup();
	}
}

// port will call function after successfully receiving buffer from socket
// virtual 
void 
msg_sock_connection::v_on_receive(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& peeraddr, void* userdata)
{
	assert(handle == _handle);
	// checks if we received the 4 leading bytes
	if (!_msg_recv) // first step
	{
		try
		{
			assert(requested == sizeof(ub4_t));
			assert(buf == &_size_recv);

			if (buf != &_size_recv || requested != sizeof(ub4_t) || processed != sizeof(ub4_t)) // must be only 4 bytes
				exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &aiosockTable);

			size_t sizeh = ntohl(_size_recv);
			int body_size = (int)sizeh - (int)msg_cpp::block_size(0);
			if (body_size < 0)
				exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);


			// creates message and sets correct block size
			_msg_recv = _communicator->construct_msg(body_size);

			_offset_recv = sizeof(ub4_t);
			// receives the rest of message block
			int err = _communicator->get_aiosock().receive(_handle, _msg_recv->get_block() + _offset_recv, sizeh - sizeof(ub4_t), msg_default_timeout, 0, 0);
			sockStatus(_communicator->get_aiosock(), err);
		}
		catch (exception& x)
		{
			// shuts down connection
			_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "shutdown conection, error: %s", x.what());
			_communicator->shutdown_connection(this);
		}
	}
	else if (requested != processed)
	{
		assert(requested > processed);
		_offset_recv += (ub4_t)processed;
		int err = _communicator->get_aiosock().receive(_handle, _msg_recv->get_block() + _offset_recv, requested - processed, msg_default_timeout, 0, 0);
		try
		{
			sockStatus(_communicator->get_aiosock(), err);
		}
		catch (...)
		{
			// shuts down connection
			_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "shutdown conection, error: %x", err);
			_communicator->shutdown_connection(this);
		}
	}
	else
	{
		try
		{
			// unpacks message
			_msg_recv->unpack_msg(get_crypt_key());

			process_incoming_message(_msg_recv);
			_msg_recv = 0;
			_offset_recv = 0;

			// starts new receive
			int err = _communicator->get_aiosock().receive(_handle, &_size_recv, sizeof(ub4_t), INFINITE, 0, 0);
			sockStatus(_communicator->get_aiosock(), err);
		}
		catch (exception& x)
		{
			_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "shutdown conection, error: %s", x.what());
			_communicator->shutdown_connection(this);
		}	
	}
}

// port will call function after successfully accepting the new incoming connection
// virtual 
void 
msg_sock_connection::v_on_accept(size_t handle, size_t handle_accepted, terimber_aiosock_callback*& callback, const sockaddr_in& peeraddr, void* userdata)
{
	assert(false);
}

// virtual
bool 
msg_sock_connection::v_has_job(size_t ident, void* user_data)
{
	size_t top_priority = 0;

	switch (ident)
	{
		case queue_thread_ident: // outgoing message
			return (msg_connection::v_has_job(ident, user_data) // thread is on
					&& _msg_send == 0
					&& (_state == CONN_STATE_CONNECTED // handshake is completed
						|| ((_state == CONN_STATE_HANDSHAKE_INITIATOR // initiates handshake
							|| _state == CONN_STATE_HANDSHAKE_RECEIVER) // replies to handshake
							&& touch(top_priority)
							&& top_priority == MSG_PRIORITY_SYSTEM
							)
						)
					); // we are not in a process of sending
		default:
			assert(false);
	}

	return false;
}

// virtual 
void 
msg_sock_connection::v_do_job(size_t ident, void* user_data)
{
	msg_creator creator(_communicator);
 
	switch (ident)
	{
		case queue_thread_ident: // sends
		{
			assert(_msg_send == 0);
			if (!pop(_msg_send))
			{
				assert(false);
				return;
			}

			// sends message
			try
			{
				// resets session id for use type
				if (_msg_send->_type != handshake_type)
					_msg_send->_sessionid = _info._session;

				// tries to pack message
				// uses crypt only for user type
				_msg_send->pack_msg((_msg_send->_type & user_type_mask) ? get_crypt_key() : 0);
				int err = _communicator->get_aiosock().send(_handle, _msg_send->get_block(), ntohl(*(ub4_t*)_msg_send->get_block()), (size_t)_msg_send->timeout, 0, 0);
				sockStatus(_communicator->get_aiosock(), err);
	
				// sets last time activity
				set_last_activity();
			}
			catch (exception& err)
			{
				if (_msg_send->_type == user_type_send || _msg_send->_type == user_type_send_async)
				{
					try
					{
						// constructs reply message
						msg_pointer_t reply(creator, 0);
						// sets error text
						msg_pack::make_reply_msg(_msg_send, reply);
						msg_pack::make_error_msg(reply, err.what());
						// pushes message to the communicator queue
						_communicator->comm_msg(reply);
						reply.detach();
					}
					catch (exception&)
					{
					}
					catch (...)
					{
						assert(false);
					}
				}
					
				// shuts down connection
				_communicator->format_logging(0, __FILE__, __LINE__, en_log_info, "shutdown conection, error: %s", err.what());
				_communicator->shutdown_connection(this);
			}
			catch (...)
			{
				assert(false);
			}
		}
		break;
		default:
			assert(false);
	} // switch
}

// static 
msg_sock_connection* 
msg_sock_connection::connect(msg_communicator* communicator_, const conf_connection& info_)
{
	// creates object
	msg_sock_connection* connection = new msg_sock_connection(communicator_, info_);

	if (!connection)
		exception::_throw(MSG_RESULT_NOTMEMORY, &msgMsgTable);

	// checks handle
	if (!connection->_handle)
	{
		delete connection;
#if OS_TYPE == OS_WIN32
		exception::_throw(WSAEFAULT, &aiosockTable);
#else
		exception::_throw(EFAULT, &aiosockTable);
#endif
	}

	// connects asynchronously
	int err = 0;

	try
	{
		if (err = communicator_->get_aiosock().connect(connection->_handle, info_._network, info_._port, INFINITE, 0))
		{
			delete connection;
			sockStatus(communicator_->get_aiosock(), err);
		}

		communicator_->add_connection(connection);
	}
	catch (exception& x)
	{
		delete connection;
		throw x;
	}

	return connection;
}

// static 
void
msg_sock_connection::accept(size_t handle, msg_communicator* communicator_, const conf_listener& info_, terimber_aiosock_callback*& callback)
{
	conf_connection atom;
	atom._address = uuid_gen(); // fake guid
	// creates object
	msg_sock_connection* connection = new msg_sock_connection(communicator_, handle, info_, atom);

	if (!connection)
	{
		communicator_->get_aiosock().close(handle);
		exception::_throw(MSG_RESULT_NOTMEMORY, &msgMsgTable);
	}

	callback = connection;
	// turns on
	try
	{

		// performs handshake as receiver
		// initiates receiving
		int err = communicator_->get_aiosock().receive(connection->_handle, &connection->_size_recv, sizeof(ub4_t), INFINITE, 0, 0);
		sockStatus(communicator_->get_aiosock(), err);
		connection->v_on();
		communicator_->add_connection(connection);
	}
	catch (exception& err)
	{
		delete connection;
		throw err;
	}
}
////////////////////////////////////////////////////////
msg_sock_listener::msg_sock_listener(msg_communicator* communicator_, const conf_listener& info_) :
	msg_listener(communicator_, info_), _handle(0)
{
}

// virtual 
msg_sock_listener::~msg_sock_listener()
{
	if (_handle)
	{
		_communicator->get_aiosock().close(_handle);
		_handle = 0;
	}
}

// virtual 
void 
msg_sock_listener::v_on()
{
	if (is_on())
		return;

	_handle = _communicator->get_aiosock().create(this, true);

	if (!_handle)
		exception::_throw("Can't create the socket handler");

	int err = _communicator->get_aiosock().listen(_handle, _info._port, _info._connections, _info._network, 64, 0);
	sockStatus(_communicator->get_aiosock(), err);

	msg_base::v_on();
}

// virtual 
void 
msg_sock_listener::v_off()
{
	if (!is_on())
		return;

	if (_handle)
	{
		_communicator->get_aiosock().close(_handle);
		_handle = 0;
	}

	msg_base::v_off();
}

// port will call function after error occured
// virtual 
void 
msg_sock_listener::v_on_error(size_t handle, int err, aiosock_type mask, void* userdata)
{
	assert(handle == _handle);
}

// port will call function after successfully connecting to socket
// virtual 
void 
msg_sock_listener::v_on_connect(size_t handle, const sockaddr_in& peeraddr, void* userdata)
{
	assert(false);
}

// port will call function after successfully sending buffer to socket
// virtual 
void 
msg_sock_listener::v_on_send(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& peeraddr, void* userdata)
{
	assert(false);
}

// port will call function after successfully receiving buffer from socket
// virtual 
void 
msg_sock_listener::v_on_receive(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& peeraddr, void* userdata)
{
	assert(false);
}

// port will call function after successfully accepting the new incoming connection
// virtual 
void 
msg_sock_listener::v_on_accept(size_t handle, size_t handle_accepted, terimber_aiosock_callback*& callback, const sockaddr_in& peeraddr, void* userdata)
{
	assert(handle == _handle);
	try
	{
		// tries to create connection
		msg_sock_connection::accept(handle_accepted, _communicator, _info, callback);
	}
	catch (exception&)
	{
	}
	catch (...)
	{
		assert(false);
	}
}


#pragma pack()
END_TERIMBER_NAMESPACE
