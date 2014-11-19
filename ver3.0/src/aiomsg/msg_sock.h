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

#ifndef _terimber_msg_sock_h_
#define _terimber_msg_sock_h_

#include "aiomsg/msg_conn.h"
#include "aiomsg/msg_lsnr.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class msg_sock_connection
//! \brief class implements the Socket connection
class msg_sock_connection :	public msg_connection, 
							public terimber_aiosock_callback
{
protected:
	//! \brief constructor for initiator
	msg_sock_connection(msg_communicator* communicator,		//!< communicator pointer
						const conf_connection& info			//!< connection info
						);
	//! \brief constructor for receiver
	msg_sock_connection(msg_communicator* communicator,		//!< communicator pointer
						size_t handle,						//!< socket handle
						const conf_listener& linfo,			//!< listener info
						const conf_connection& cinfo		//!< connection info
						);

public:
	//! \brief destructor
	virtual ~msg_sock_connection();


	//! \brief constructor for initiator
	//! establishes the new connection to the peer
	static 
	msg_sock_connection* 
	connect(		msg_communicator* communicator,			//!< communicator pointer
					const conf_connection& info			//!< connection info
					);
	//! \brief constructor for receiver
	//! handshake process with the peer
	static 
	void 
	accept(			size_t handle,							//!< socket handle
					msg_communicator* communicator,			//!< communicator pointer 
					const conf_listener& info,				//!< listener info
					terimber_aiosock_callback*& callback	//!< aiosock callback
					);

protected:
	//! \brief overrides the base functionality
	virtual 
	void 
	v_do_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);
	//! \brief thread callback
	virtual 
	bool 
	v_has_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);

	//! \brief aio callbacks
	//! port will call function after error occured
	virtual 
	void 
	v_on_error(		size_t handle,							//!< socket handle
					int err,								//!< error code
					aiosock_type mask,						//!< reason
					void* userdata							//!< user defined data
					);
	//! \brief port will call function after successfully connecting to socket
	virtual 
	void 
	v_on_connect(	size_t handle,							//!< socket handle
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					);
	// port will call function after successfully sending buffer to socket
	virtual 
	void 
	v_on_send(		size_t handle,							//!< socket handle
					void* buf,								//!< buffer
					size_t requested,						//!< requested bytes to send
					size_t processed,						//!< actually processed bytes
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					);
	//! \brief port will call function after successfully receiving buffer from socket
	virtual 
	void 
	v_on_receive(	size_t handle,							//!< socket handle
					void* buf,								//!< buffer
					size_t requested,						//!< requested bytes to receive
					size_t processed,						//!< actually processed bytes 
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					);
	//! \brief port will call function after successfully accepting the new incoming connection
	//! user can change the callback, by default it's an object which created a listener
	virtual 
	void 
	v_on_accept(	size_t handle,							//!< listener handle
					size_t handle_accepted,					//!< accepted socket handle
					terimber_aiosock_callback*& callback,	//!< aiosock callback
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					);

protected:
	msg_cpp*		_msg_send;								//!< messages buffer for send processes
	ub4_t			_offset_send;							//!< offset of sent message buffer
	msg_cpp*		_msg_recv;								//!< messages buffer for receive processes
	ub4_t			_size_recv;								//!< size of message should be received
	ub4_t			_offset_recv;							//!< offset of received message buffer
	size_t			_handle;								//!< socket handle
};

//! \class msg_sock_listener
//! \brief class implements the socket listener
//! that supports both types of connection
class msg_sock_listener : public msg_listener, 
							public terimber_aiosock_callback
{
public:
	//! \brief costructor
	msg_sock_listener(msg_communicator* communicator,		//!< communicator pointer
						const conf_listener& info			//!< listener info
						);

	//! \brief destructor
	virtual ~msg_sock_listener();
	//! \brief returns the sock listener type
	virtual 
	transport_type 
	get_type() const 
	{ 
		return sock; 
	}

protected:
	//! overrides the activate/deactivate functions
	//! \brief action on turn on
	virtual 
	void 
	v_on();
	//! \brief action on turn off
	virtual 
	void 
	v_off();

	//! aio callbacks
	//! \brief port will call function after error occured
	virtual 
	void 
	v_on_error(		size_t handle,							//!< socket handle
					int err,								//!< error code
					aiosock_type mask,						//!< reason
					void* userdata							//!< user defined data
					);
	//! \brief port will call function after successfully connecting to socket
	virtual 
	void 
	v_on_connect(	size_t handle,							//!< socket handle
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					);
	//! \brief port will call function after successfully sending buffer to socket
	virtual 
	void 
	v_on_send(		size_t handle,							//!< socket handle
					void* buf,								//!< buffer
					size_t requested,						//!< requested bytes to send
					size_t processed,						//!< actually porcessed bytes
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					);
	//! \brief port will call function after successfully receiving buffer from socket
	virtual 
	void 
	v_on_receive(	size_t handle,							//!< socket handle
					void* buf,								//!< buffer
					size_t requested,						//!< requested bytes to receive
					size_t processed,						//!< actually processed bytes
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					);
	//! \brief port will call function after successfully accepting the new incoming connection
	//! user can change the callback, by default it's an object which created a listener
	virtual 
	void 
	v_on_accept(	size_t handle,							//!< listener handle
					size_t handle_accepted,					//!< accepted socket handle
					terimber_aiosock_callback*& callback,	//!< aiosock callback
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					);

private:
	size_t			_handle;								//!< socket handle
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_msg_sock_h_

