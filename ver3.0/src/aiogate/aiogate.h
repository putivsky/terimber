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

#ifndef _terimber_aiogate_h_
#define _terimber_aiogate_h_

#include "aiosock/aiosock.h"
#include "base/map.h"
#include "base/list.h"
#include "aiogate/aiogatefactory.h"
#include "threadpool/thread.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \enum aiogate_chunk
//! \brief this is a default memory chunk
enum aiogate_chunk { BUFFER_CHUNK = 1024*64 };

//! \class fixed_size_buffer
//! \brief high performance chunks linked list
class fixed_size_buffer
{
public:
	//! \brief constructor
	fixed_size_buffer() : 
		_begin(0), 
		_end(0), 
		_next(0) 
	{
	}

	ub1_t				_ptr[BUFFER_CHUNK];					//!< points to the chunk of BUFFER_CHUNK length
	size_t				_begin;								//!< begining bytes offset
	size_t				_end;								//!< ending bytes offset

	fixed_size_buffer*	_next;								//!< next chunk in a chain 
};

//! class aiogate
//! \brief abstraction for callback
class aiogate :	public terimber_aiogate,					//!< star gate abstract class interface
					public terimber_aiosock_callback,		//!< callbacks for socket port
					public terimber_thread_employer			//!< callbacks for thread that closes pins
{
	//! class listener_info
	//! \brief keeps listener information
	class listener_info
	{
	public:
		//! \brief constructor
		listener_info(	terimber_aiogate_pin_factory* factory,	//!< pointer to pin factory 
						void* arg							//!< user defined argument
						) : 
		  _factory(factory), _arg(arg) 
		{
		}
		
		//! \brief copies constructor
		listener_info(const listener_info& x) 
			: _factory(x._factory), _arg(x._arg) 
		{
		}

		terimber_aiogate_pin_factory*	_factory;			//!< factory, which knows how to create pin
		void*							_arg;				//!< user defined argument
	};

	//! class pin_info_minimum
	//! \brief min pin information
	class pin_info_minimum
	{
	public:
		//! \brief default constructor
		pin_info_minimum() : 
			_pin(0), 
			_rbuf(0), 
			_shead(0), 
			_factory(0) 
		{}

		//! \brief copies constrcutor
		pin_info_minimum(const pin_info_minimum& x) : 
			_pin(x._pin), 
			_rbuf(x._rbuf), 
			_shead(x._shead), 
			_factory(x._factory) 
		{
		}

		terimber_aiogate_pin*			_pin;				//!< pointer to pin object
		fixed_size_buffer*				_rbuf;				//!< pointer to the head of linked list of buffers for receiving 
		fixed_size_buffer*				_shead;				//!< pointer to the head of linked loist of buffers for sending
		terimber_aiogate_pin_factory*	_factory;			//!< pointer to factory, which knows how to create pin
	};
	
	//! \class pin_info
	//! \brief pin info with information about pin status
	class pin_info : public pin_info_minimum
	{
	public:
		//! \brief default constructor
		pin_info() : 
			pin_info_minimum(),
			_stail(0),
			_send_timeout(INFINITE), 
			_recv_timeout(INFINITE),
			_in_progress_mask(0x0), 
			_callback_invoking_mask(0x0), 
			_still_alive(true),
			_tcp_udp(true)
		{}

		//! \brief copies constructor
		pin_info(const pin_info& x) : 
			pin_info_minimum(x),
			_stail(x._stail),
			_send_timeout(x._send_timeout), 
			_recv_timeout(x._recv_timeout),
			_in_progress_mask(x._in_progress_mask), 
			_callback_invoking_mask(x._callback_invoking_mask), 
			_still_alive(x._still_alive),
			_tcp_udp(x._tcp_udp)
		{
		}

		ub8_t					_leader;					//!< a small buffer for waiting for incoming bytes, optimization for huge number of pins
		fixed_size_buffer*		_stail;						//!< pointer to the tail of linked list of buffers for sending
		size_t					_send_timeout;				//!< sends timeout
		size_t					_recv_timeout;				//!< receives timeout
		ub4_t					_in_progress_mask;			//!< bit mask for currently activities
		ub4_t					_callback_invoking_mask;	//!< bit mask for invoked callbacks
		bool					_still_alive;				//!< flag if pin is still alive
		bool					_tcp_udp;					//!< tpc or udp oriented pin
	};

	//! \class  pin_info_extra
	//! \brief pin information required for proper pin destruction
	class pin_info_extra : public pin_info_minimum
	{
	public:
		//! \brief constructor
		pin_info_extra(	const pin_info_minimum& x,			//!< minimum pin info
						size_t ident,						//!< pin ident
						size_t mask,						//!< reason why pin has been closed
						bool invoke_callback				//!< flag if user callback should be invoked upon pin closure
						) : 
			pin_info_minimum(x),
			_ident(ident), 
			_mask(mask), 
			_invoke_callback(invoke_callback)
		{
		}

		//! \brief copies constructor
		pin_info_extra(const pin_info_extra& x) : 
			pin_info_minimum(x),
			_ident(x._ident), 
			_mask(x._mask), 
			_invoke_callback(x._invoke_callback)
		{
		}

		size_t							_ident;				//!< pin ident
		size_t							_mask;				//!< reason why pin has been closed
		bool							_invoke_callback;	//!< flag if user callback should be invoked upon pin closure
	};

	//! \class udp_header
	//! \brief keeps information for udp package
	class udp_header
	{
	public:
		sockaddr_in						_addr;				//!< receiver address - can be different for each package
		ub4_t							_payload;			//!< the next bytes at the same page, that are actually UDP package bytes
	};

	//! \typedef listener_map_t
	//! \brief maps listener ident to listener information
	typedef map< size_t, listener_info >		listener_map_t;
	//! \typedef pin_map_t
	//! \brief maps pin ident to pin information
	typedef map< size_t, pin_info >				pin_map_t;
	//! \typedef chunk_allocator_t
	//! \brief node allocator for linked list buffers
	typedef node_allocator< fixed_size_buffer >	chunk_allocator_t;
	//! \typedef close_list_t
	//! \brief lists pins prepeared for destruction
	typedef list< pin_info_extra >				close_list_t;
public:
	//! \brief consrtuctor
	aiogate(size_t addional_working_threads,				//!< additional working thread for processing user callbacks
			size_t mem_usage								//!< max memory usage in bytes
			);

	//! \brief destructor
	virtual ~aiogate();

public:
	// terimber_socket_callback methods
	//! \brief port will call function after error occured
	virtual 
	void 
	v_on_error(	size_t handle,								//!< socket ident
				int err,									//!< error code
				aiosock_type mask,							//!< type of action when error occurred
				void* userdata								//!< user defined data
				);
	//! \brief port will call function after successfully connecting to socket
	virtual
	void 
	v_on_connect(size_t handle,								//!< socket ident
				const sockaddr_in& peeraddr,				//!< peer address
				void* userdata								//!< user defined data
				);
	//! \brief port will call function after successfully sending buffer to socket
	virtual 
	void 
	v_on_send(	size_t handle,								//!< socket handle
				void* buf,									//!< buffer to send bytes
				size_t requested,							//!< bytes requested for sending
				size_t processed,							//!< bytes actually sent
				const sockaddr_in& toaddr,					//!< peer address
				void* userdata								//!< user defined data
				);
	//! \brief port will call function after successfully receiving buffer from socket
	virtual 
	void 
	v_on_receive(size_t handle,								//!< socket ident
				void* buf,									//!< buffer to receive bytes
				size_t requested,							//!< bytes requested for receive
				size_t processed,							//!< bytes actually received
				const sockaddr_in& toaddr,					//!< peer address
				void* userdata								//!< user defined data
				);
	//! \brief port will call function after successfully accepting the new incoming connection
	//! user can change the callback, by default it's an object which created a listener
	virtual 
	void 
	v_on_accept(size_t handle,								//!< listener socket ident
				size_t handle_accepted,						//!< accepted new incoming socket connection ident
				terimber_aiosock_callback*& callback,		//!< callback for processing further asynchronous actions
				const sockaddr_in& peeraddr,				//!< peer address
				void* userdata								//!< user defined data
				);

// terimber_aiogate methods

	//! \brief starts listening TCP port on specified address
	virtual 
	size_t 
	listen(		const char* address,						//!< server ip or name, can be null - localhost will be used
				unsigned short port,						//!< listening port
				size_t max_connection,						//!< max waited connections
				unsigned short buffered_acceptors,			//!< how many acceptor will be waiting for incoming pins
				terimber_aiogate_pin_factory* factory,		//!< factory, which knows how to create pin
				void* arg									//!< user defined argument - will be used for terimber_aiogate_pin_factory::create method as an input argument
				);

	//! \brief stops listener with specified ident 
	virtual 
	void 
	deaf(		size_t ident								//!< listener socket ident
				);

	//! \brief initiates connection
	//! \return ident if asynchronous connection process has been initiated
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
				);

	// UDP only methods
	//! \brief bind to the specified socket address
	virtual 
	size_t 
	bind(		const char* address,						//!< local host ip or name 
				unsigned short port,						//!< local port
				terimber_aiogate_pin_factory* factory,		//!< factory, which knows how to create pin
				void* arg									//!< user defined argument - will be used for terimber_aiogate_pin_factory::create method as an input argument
				);
	// UDP

	//! \brief sends buf bytes asynchronously
	virtual 
	bool 
	send(		size_t ident,								//!< unique pin identificator
				const void* buf,							//!< buffer to send
				size_t len,									//!< length of buffer
				const sockaddr_in* toaddr					//!< peer address, optional - only for UDP
				); 

	//! \brief sends bulk of buffers asynchronously
	virtual 
	bool 
	send_bulk(	size_t ident,								//!< unique pin identificator
				const terimber_aiogate_buffer* bulk,		//!< buffers to send
				size_t count,								//!< length of bulk
				const sockaddr_in* toaddr					//!< peer address, optional - only for UDP
				);

	//! \brief initiates receive process, 
	//! either use big buffer or just a small one 
	//! in order to save the memory usage for a unknown waiting time 
	virtual 
	bool 
	recv(		size_t ident,								//!< unique identificator
				bool expect_delivery,						//!< memory usage advisor 
				const sockaddr_in* toaddr					//!< peer address optional - only for UDP
				); 
	//! \brief close connection pin
	virtual 
	bool 
	close(		size_t ident								//!< unique pin identificator
				);

	//! \brief set send timeout
	virtual 
	bool 
	set_send_timeout(size_t ident,							//!< unique pin identificator
				size_t timeout								//!< timeout in milliseconds
				);

	//! \brief sets recv timeout
	virtual 
	bool 
	set_recv_timeout(size_t ident,							//!< unique pin identificator
				size_t timeout								//!< timeout in milliseconds
				); 

	//! \brief makes the snapshot of internal state
	virtual 
	void 
	doxray();

	//! \brief activates aiogate
	bool 
	on();
	//! \brief deactivates aiogate
	void 
	off();

protected:
	// terimber_thread_employer methods
	//! \brief checks if any activity is required
	virtual 
	bool 
	v_has_job(	size_t ident,								//!< thread ident
				void* user_data								//!< user defined data
				);
	//! \brief do real pin activity
	virtual 
	void 
	v_do_job(	size_t ident,								//!< thread ident
				void* user_data								//!< user defined data
				);

private:
	//! \brief locks pin while callback invoking
	void 
	lock_pin(	mutexKeeper& mtx,							//!< mutex
				pin_info& info,								//!< pin info reference
				ub4_t mask									//!< action mask
				);

	//! \brief unlocks pin
	bool 
	unlock_pin(mutexKeeper& mtx,							//! mutex
				size_t ident,								//! pin ident
				ub4_t mask,									//! action mask
				bool unmask_in_progress						//! flag tells should function unmask ot not 
				);

	//! \brief initiates pin closure 
	void 
	initiate_close(size_t ident,							//!< pin ident
				size_t mask,								//!< reason why pin is about to close
				bool invoke_callback						//!< flag should user callback be invoked upon pin closure
				);
	//! \brief actual pin closure in a separate thread
	void 
	final_close(pin_info_extra& info						//!< pin info
				);
private:
	bool					_on;							//!< on/off flag
	mutex					_pin_mtx;						//!< pin mutex
	listener_map_t			_listener_map;					//!< listener map
	pin_map_t				_pin_map;						//!< pin map
	aiosock					_pin_port;						//!< aiosock port
	chunk_allocator_t		_pin_allocator;					//!< memory chunks allocator
	close_list_t			_pin_list;						//!< list of pins prepared for final closure
	thread					_pin_thread;					//!< housekeeping thread
}; 


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_aiogate_h_
