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

#ifndef _terimber_aiogate_factory_h_
#define _terimber_aiogate_factory_h_

#include "log.h"

// A few very important notes...
// 1. aiogate guarantees that all sent blocks will be delivered in the original placed order "send"
// 2. aiogate guarantees that all received blocks will be delivered through callback "on_recv" one by one
// it's impossible to have two or more invoked callbacks "on_recv" at the same time for the same socket handle.
// 3. All methods are thread-safe.

// forward declaration
class terimber_aiogate;

//! \brief accepts mask
const ub4_t aiogate_accept_mask = 0x00000001;
//! \brief connects mask
const ub4_t aiogate_connect_mask = 0x00000002;
//! \brief sends mask
const ub4_t aiogate_send_mask = 0x00000004;
//! \brief receives mask
const ub4_t aiogate_recv_mask = 0x00000008;
//! \brief binds mask
const ub4_t aiogate_bind_mask = 0x00000010;

//! \class terimber_aiogate_buffer
//! \brief multiple buffers to send
class terimber_aiogate_buffer
{
public:
	const void*		buf;									//!< buffer pointer
	size_t			len;									//!< buffer length
};

//! \class terimber_aiogate_pin
//! \brief class abstraction for pin object
//! user has to implement the code
class terimber_aiogate_pin
{
public:
	//! \brief destructor
	virtual ~terimber_aiogate_pin() {}

	//! \brief accepts the new incoming pin
	virtual 
	void 
	on_accept(	const sockaddr_in& local,					//!< ip/port of this pin
				const sockaddr_in& remote,					//!< ip/port of remote peer
				size_t ident,								//!< unique ident for this pin will be used with the following callback
				terimber_aiogate* callback					//!< aiogate itself
				) = 0;

	//! \brief connects to the new pin
	virtual 
	void 
	on_connect(	const sockaddr_in& local,					//!< ip/port of this pin
				const sockaddr_in& remote,					//!< ip/port of remote peer
				size_t ident,								//!< unique ident for this pin will be used with the following callback
				terimber_aiogate* callback					//!< aiogate itself
				) = 0;
	
	//! \brief connect to the new UDP pin
	virtual 
	void 
	on_bind(	const sockaddr_in& local,					//!< ip/port of this pin
				size_t ident,								//!< unique ident for this pin will be used with the following callback
				terimber_aiogate* callback					//!< aiogate itself
				) = 0;

	//! \brief aiogate calls this callback when bytes have come from peer
	// \return true if it is expected of aiogate to automatically invoke recv method
	// expected_more param is just for performance optimization
	virtual 
	bool 
	on_recv(	const void* buf,							//!< array of received bytes
				size_t len,									//!< the length of array of bytes
				const sockaddr_in& peeraddr,				//!< peer address
				bool& expected_more							//!< tip for aiogate to use a big chunk of memory instead of small buffer
				) = 0;
	
	//! \brief aiogate invokes this callback only if all bytes have been sent
	virtual 
	void 
	on_send(	const sockaddr_in& peeraddr					//!< peer address
				) = 0;
	
	//! \brief aiogate invokes this function when pin connection is deactivate
	// only internal action can be taken - no aiogate calls anymore for this pin
	virtual 
	void 
	on_close(	ub4_t mask							//!< reason for closure
				) = 0;
};


//! \class terimber_aiogate_pin_factory
//! \brief class abstraction for connector
// user has to implement the code
class terimber_aiogate_pin_factory
{
public:
	//! \brief destructor
	virtual ~terimber_aiogate_pin_factory() {}

	//! \brief creates new pin
	//! aiogate will send it as a parameter
	virtual 
	terimber_aiogate_pin* 
	create(		void* arg									//!< user data for internal use
				) = 0;
	
	//! \brief destroys pin
	virtual 
	void 
	destroy(	terimber_aiogate_pin* pin					//!< pin pointer
				) = 0;
};

//! \class terimber_aiogate
//! \brief class abstraction aiogate
//! aiogate will provide this interface
class terimber_aiogate : public terimber_log_helper
{
public:
	//! \brief destructor
	virtual ~terimber_aiogate() {}

	//! \brief initiates listening on ip/port pin
	//! returns unique identificator for listener
	//! caller invokes function only once - aiogate will accept incoming connection automatically
	//! listen --> accept --> factory->create() --> pin->on_accept() -|
	//!				|________________________________________________|
	//! until deaf function called
	//! all failed accepts ignored
	virtual 
	size_t 
	listen(		const char* address,						//!< server ip or name, can be null - localhost will be used
				unsigned short port,						//!< listening port
				size_t max_connection,						//!< max waited connections
				unsigned short buffered_acceptors,			//!< how many acceptors will be waiting for incoming pins
				terimber_aiogate_pin_factory* factory,		//!< factory, which knows how to create pin
				void* arg									//!< user defined argument - will be used for terimber_aiogate_pin_factory::create method as an input argument
															//! this is a way how to separate different sources of incoming socket connections
				) = 0; 

	//! \brief stops listener with specified ident 
	virtual 
	void 
	deaf(		size_t ident								//!< listener ident
				) = 0;

	//! \brief initiate connection
	//! returns ident if asynchronous connection process has been initiated
	//! when asynchronous connection completed (failed or succeeded)
	//! connect --> factory->create() --> (pin->on_connect() OR pin->on_close())
	virtual 
	size_t 
	connect(	const char* remote,							//!< remote host ip or name
				unsigned short rport,						//!< remote port
				const char* local,							//!< local host ip or name - optional can be null
				unsigned short lport,						//!< local port - optional can be zero
				size_t timeout,								//!< timeout in milliseconds 
				terimber_aiogate_pin_factory* factory,		//!< factory, which knows how to create pin
				void* arg									//!< user defined argument - will be used for terimber_aiogate_pin_factory::create method as an input argument
															//! this is a way to separate different destinations of outgoing socket connections
				) = 0;

	// UDP only methods
	//! \brief binds to the specified socket address
	virtual 
	size_t 
	bind(		const char* address,						//!< local host ip or name 
				unsigned short port,						//!< local port
				terimber_aiogate_pin_factory* factory,		//!< factory, which knows how to create pin
				void* arg									//!< user defined argument - will be used for terimber_aiogate_pin_factory::create method as an input argument
				) = 0;
	// /UDP

	//! \brief sends buf bytes asynchronously
	//! caller does NOT need to keep a valid pointer to the buffer until the asynchronous operation is be completed
	//! aiogate will make a copy of sending bytes.
	virtual 
	bool 
	send(		size_t ident,								//!< unique pin identificator
				const void* buf,							//!< buffer to send
				size_t len,									//!< length of buffer
				const sockaddr_in* toaddr					//!< peer address, optional for UDP only
				) = 0;

	//! \brief same as above but can take the array of buffers
	virtual 
	bool 
	send_bulk(	size_t ident,								//!< unique pin identificator
				const terimber_aiogate_buffer* bulk,		//!< buffers to send
				size_t count,								//!< length of bulk
				const sockaddr_in* toaddr					//!< peer address, optional for UDP only
				) = 0;

	//! \brief initiates receive process, 
	//! either use the big buffer or just a small one
	//! in order to save the memory usage for a unknown waiting time 
	virtual 
	bool 
	recv(		size_t ident,								//!< unique pin identificator
				bool expect_delivery,						//!< memory usage advisor 
				const sockaddr_in* toaddr					//!< peer address, optional for UDP only
				) = 0; 
	//! \brief closes connection pin
	virtual 
	bool 
	close(		size_t ident								//!< unique pin identificator
				) = 0;
	//! \brief sets send timeout
	virtual 
	bool 
	set_send_timeout(size_t ident,							//!< unique pin identificator
				size_t timeout								//!< timeout in milliseconds
				) = 0;  

	//! \brief sets recv timeout
	virtual 
	bool 
	set_recv_timeout(size_t ident,							//!< unique pin identificator
				size_t timeout								//!< timeout in milliseconds
				) = 0;

	//! \brief makes the snapshot of internal state
	virtual 
	void 
	doxray() = 0;
};

//! \class terimber_aiogate_factory
//! \brief class factory for aiogate objects
class terimber_aiogate_factory
{
public:
	//! \brief constructor
	terimber_aiogate_factory();

	//! \brief caller is responsible for deleting the aiogate object
	//! and until that keep alive all terimber_aiogate_pin_factory objects
	//! addional_working_threads - by default aiogate will use only one working thread to invoke user callbacks
	//! caller can specify more additional threads and be ready to process simultaneous callbacks 
	terimber_aiogate* 
	get_terimber_aiogate(terimber_log* log,					//!< logging
					size_t addional_working_threads = 0,	//!< by default aiogate will use only one working thread to invoke user callbacks
					size_t max_mem_usage = 1024*1024*16		//!< max memory usage by default 16MB
					);
};

#endif // _terimber_aiogate_access_h_
