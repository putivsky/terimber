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

#ifndef _terimber_aiosock_factory_h_
#define _terimber_aiosock_factory_h_

#include "log.h"

//! \enum aiosock_type
//! \brief asynchronous socket actions
enum aiosock_type
{
	AIOSOCK_CONNECT,										//!< connect
	AIOSOCK_ACCEPT,											//!< accept
	AIOSOCK_SEND,											//!< send
	AIOSOCK_RECV,											//!< receive
};

//! \class terimber_aiosock_callback
//! \brief abstract interface aiosock callbacks
class terimber_aiosock_callback
{
public:
	//! \brief destructor
	virtual
	~terimber_aiosock_callback() {}
	
	//! \brief port will call function after error occured
	virtual 
	void 
	v_on_error(	size_t handle,								//!< socket handle
				int err,									//!< error code
				aiosock_type mask,							//!< reason
				void* userdata								//!< user defined data
				) = 0;
	//! \brief port will call function after successfully connecting to socket
	virtual 
	void 
	v_on_connect(	size_t handle,							//!< socket handle
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					) = 0;
	//! \brief port will call function after successfully sending buffer to socket
	virtual 
	void 
	v_on_send(		size_t handle,							//!< socket handle
					void* buf,								//!< buffer for sending
					size_t requested,						//!< requested bytes to send
					size_t processed,						//!< actually sent bytes
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					) = 0;
	//! \brief port will call function after successfully receiving buffer from socket
	virtual 
	void 
	v_on_receive(	size_t handle,							//!< socket handle
					void* buf,								//!< buffer for receiving
					size_t requested,						//!< requested bytes for receive
					size_t processed,						//!< actually received bytes
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					) = 0;
	//! \brief port will call function after successfully accepting the new incoming connection
	//! user can change the callback, by default it's an object which created a listener
	virtual 
	void 
	v_on_accept(	size_t handle,							//!< listener socket handle
					size_t handle_accepted,					//!< accepted socket handle
					terimber_aiosock_callback*& callback,	//!< callback for processing further actions
					const sockaddr_in& peeraddr,			//!< peer address
					void* userdata							//!< user defined data
					) = 0;
};

//! \class terimber_aiosock
//! \brief abstract interface for the socket port
class terimber_aiosock : public terimber_log_helper
{
public:
	//! \brief destructor
	virtual ~terimber_aiosock() {}
	//! \brief create TCP socket
	virtual 
	size_t 
	create(	terimber_aiosock_callback* callback,			//!< callback for processing asynchronous actions
			bool tcp_udp									//!< type of socket TCP/UDP
			) = 0;
	//! \brief close socket
	virtual 
	void 
	close(	size_t handle									//!< socket handle
			) = 0;
	//! \brief sends buffer to specified socket asynchronously
	virtual 
	int 
	send(	size_t handle,									//!< socket handle
			const void* buf,								//!< buffer to send
			size_t len,										//!< buffer length
			size_t timeout,									//!< timeout in milliseconds
			const sockaddr_in* toaddr,						//!< peer address, optional for TCP sockets
			void* userdata									//!< user defined data
			) = 0;
	//! \brief receives buffer of bytes from specified socket asynchronously
	virtual 
	int 
	receive(size_t handle,									//!< socket handle
			void* buf,										//!< buffer to receive
			size_t len,										//!< buffer length
			size_t timeout,									//!< timeout in milliseconds
			const sockaddr_in* fromaddr,					//!< peer address, optional for TCP sockets
			void* userdata									//!< user defined data
			) = 0;

	// TCP only methods
	//! \brief connects to the specified socket synchronously
	virtual 
	int 
	connect(size_t handle,									//!< socket handle
			const char* address,							//!< address (computer IP or DNS name)
			unsigned short port,							//!< port
			size_t timeout,									//!< timeout in milliseconds
			void* userdata									//!< user defined data
			) = 0;
	//! \brief turns the specified socket to the listening state and will automatically accept incoming connections asynchronously
	//! address can be null in case of local host
	virtual 
	int 
	listen(	size_t handle,									//!< socket handle
			unsigned short port,							//!< listening port
			size_t max_connection,							//!< max connections waiting for accept
			const char* address,							//!< listener address: IP address, DSN name, localhost
			unsigned short accept_pool,						//!< max active acceptors waiting for incoming connections
			void* userdata									//!< user defined data
			) = 0;
	//! \brief gets the peer address
	virtual 
	int 
	getpeeraddr(	size_t handle,							//!< socket handle
					sockaddr_in& addr						//!< peer address
					) = 0;
	// /TCP

	// UDP only methods
	//! \brief binds to the specified socket address
	virtual 
	int 
	bind(	size_t handle,									//!< socket handle
			const char* address,							//!< address: IP address, DSN name, localhost
			unsigned short port								//!< port
			) = 0;
	// UDP

	//! \brief gets the sock address
	virtual 
	int 
	getsockaddr(	size_t handle,							//!< socket handle
					sockaddr_in& addr						//!< this socket address
					) = 0;

	//! \brief gets error string by code
	virtual 
	bool
	get_error_description(
					int err,								//!< error code
					char* buf,								//!< [in, out] buffer
					size_t	len								//!< buffer length
					) const = 0;

	//! \brief makes the snapshot of internal state
	virtual 
	void 
	doxray() = 0;
};
 
//! \class terimber_aio_sock_access
//! \brief aiosock class factory
class terimber_aiosock_factory
{
public:
	//! \brief constructor
	terimber_aiosock_factory();
	//! \brief destructor
	~terimber_aiosock_factory();

	//! \brief creates terimber socket port object
	//! caller is responsible for destroying it
	terimber_aiosock* 
	get_aiosock(	terimber_log* log,							//!< logging pointer
					size_t capacity = 3,						//!< amount of additional threads can be opened and used simultaniously to process callbacks
					size_t deactivate_time_msec = 60000			//!< interval in milliseconds to close unused threads
				);
};

#endif // _terimber_socket_port_access_h_
