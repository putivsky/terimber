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

#ifndef _terimber_aiosock_h_
#define _terimber_aiosock_h_

#include "base/primitives.h"
#include "base/stack.h"
#include "base/list.h"
#include "base/map.h"
#include "threadpool/threadpool.h"
#include "threadpool/thread.h"
#include "aiosock/aiosockfactory.h"
#include "aiocomport/aiocomport.h"

BEGIN_TERIMBER_NAMESPACE

#pragma pack(4)

//! \class aiosock_block
//! \brief defines the asynchronous control block for WIN platform
class aiosock_block : public OVERLAPPED
{
public:
	//! \brief default constructor
	aiosock_block();
	//! \brief copy constructor
	aiosock_block(const aiosock_block& x);
	//! \brief assign operator
	aiosock_block& operator=(const aiosock_block& x);

	//! \brief clears resources
	void 
	clear();
	//! \brief sets timeout and expiration date
	void
	settimeout(	size_t timeout								//!< timeout in milliseconds
				);

public:
	aiosock_type				_type;						//!< action type
	char*						_buf;						//!< buffer send/receive
	size_t						_len;						//!< buffer length
	size_t						_socket_ident;				//!< socket ident
	size_t						_processed;					//!< processed data size
	int							_err;						//!< error result
	void*						_userdata;					//!< user defined data
	size_t						_accept_ident;				//!< accepted socket ident
	size_t						_flags;						//!< flags for keeping
	ub1_t						_accept_buf[2*(sizeof(struct sockaddr_in) + 16)]; //!< memory for accept addresses	
	sockaddr_in					_address;					//!< address to connect/recv/send
#if OS_TYPE == OS_WIN32
	size_t						
#else
	timeval
#endif
					_timeout;								//!< timeout in milliseconds
	sb8_t			_expired;								//!< expiration date
};

//! \typedef aio_sock_handle
//! \brief defines handle type
typedef SOCKET aio_sock_handle;

//! \class aiosock
//! \brief  expands windows IO Completion Port idea to Linux
class aiosock : public terimber_thread_employer, 
				public terimber_aiosock
{
	//! \typedef aiosock_pblock_allocator_t
	//! \brief node allocator for list of blocks
	typedef node_allocator< base_list< aiosock_block* >::_node >		aiosock_pblock_allocator_t;
	//! \typedef aiosock_pblock_alloc_list_t
	//! \brief list of block pointers with external allocator
	typedef _list< aiosock_block*, aiosock_pblock_allocator_t >			aiosock_pblock_alloc_list_t;
	//! \typedef aiosock_pblock_list_t
	//! \brief list of block pointers with internal allocator
	typedef list< aiosock_block* >										aiosock_pblock_list_t;
	//! \typedef aiosock_block_allocator_t
	//! \brief node allocator - block factory
	typedef node_allocator< aiosock_block >								aiosock_block_allocator_t;

	//! \class listener_info
	//! \brief listener information
	class listener_info
	{
	public:
		//! \brief constructor
		listener_info(	size_t curr_count,					//!< current active acceptors
						size_t max_count,					//!< max active acceptors
						void* userdata						//!< user defined data
						) :
			_curr_count(curr_count), 
			_max_count(max_count), 
			_userdata(userdata) 
		{
		}
		size_t		_curr_count;							//!< current initiated acceptors
		size_t		_max_count;								//!< max allowed acceptors
		void*		_userdata;								//!< user defined data
	};

	//! \typedef aiosock_listener_map_t
	//! \brief maps listener ident to listener information
	typedef map< size_t, listener_info >							aiosock_listener_map_t;

	//! \class aiosock_socket 
	//! \brief keeps socket information including initiated asynchronous actions blocks
	class aiosock_socket
	{
	public:
		//! \brief constructor
		aiosock_socket(bool tcp_udp,						//!< socket type TCP/UDP
						aio_sock_handle handle,				//!< handle
						terimber_aiosock_callback* callback //!< callback

			) : _handle(handle), _client_obj(callback), _callback_invoking(0), _tcp_udp(tcp_udp)  
		{
		}
		//! \brief copies constructor
		aiosock_socket(const aiosock_socket& x) : _handle(x._handle), _client_obj(x._client_obj), _callback_invoking(x._callback_invoking), _tcp_udp(x._tcp_udp)  {}
		//! \brief destructor
		~aiosock_socket()
		{
			assert(_incoming_list.empty());
		}


		
		aio_sock_handle				_handle;				//!< socket handle
		terimber_aiosock_callback*	_client_obj;			//!< pointer to the object for callback notofication
		aiosock_pblock_alloc_list_t	_incoming_list;			//!< keeps incoming asynchronous requests
		size_t						_callback_invoking;		//!< counter of the callbacks invoking
		bool						_tcp_udp;				//!< type of socket TCP/UDP
	};

	//! \typedef aiosock_socket_map_t
	//! \brief maps socket ident yo socket info
	typedef map< size_t, aiosock_socket >									aiosock_socket_map_t;
	//! \typedef aiosock_socket_map_iterator_t
	//! \brief aiosock_socket_map iterator
	typedef map< size_t, aiosock_socket >::iterator							aiosock_socket_map_iterator_t;
	//! \typedef aiosock_reverse_map_t
	//! \brief reverses map socket handle to socket map interator
	typedef map< aio_sock_handle, aiosock_socket_map_iterator_t >			aiosock_reverse_map_t;
	//! \typedef aiosock_reverse_map_iterator_t
	//! \brief aiosock_reverse_map iterator
	typedef map< aio_sock_handle, aiosock_socket_map_iterator_t >::iterator	aiosock_reverse_map_iterator_t;
	//! \typedef aiosock_delay_key_t
	//! \brief maps socket ident to delay counter
	typedef map< size_t, size_t >											aiosock_delay_key_t;

	//! \brief clears all block lists outgoing, abounded, initial 
	void 
	_clear_block_lists()
	{
		if (!_outgoing_list.empty())
		{
			for (aiosock_pblock_list_t::iterator out_iter = _outgoing_list.begin(); out_iter != _outgoing_list.end(); ++out_iter)
			{
				aiosock_block* block = *out_iter;
				_put_block(block);
			}

			_outgoing_list.clear();
		}

#if OS_TYPE == OS_WIN32

		if (!_abounded_list.empty())
		{
			for (aiosock_pblock_list_t::iterator tm_iter = _abounded_list.begin(); tm_iter != _abounded_list.end(); ++tm_iter)
			{
				aiosock_block* block = *tm_iter;
				_put_block(block);
			}

			_abounded_list.clear();
		}

#endif
		if (!_initial_list.empty())
		{
			for (aiosock_pblock_list_t::iterator in_iter = _initial_list.begin(); in_iter != _initial_list.end(); ++in_iter)
			{
				aiosock_block* block = *in_iter;
				_put_block(block);
			}

			_initial_list.clear();
		}
	}

	//! \brief gets new blocks
	inline 
	aiosock_block* 
	_get_block()
	{
		aiosock_block* ptr = _block_allocator.allocate();
		if (ptr)
		{
			new(ptr) aiosock_block();
		}

		return ptr;
	}

	//! \brief returns used block back
	inline 
	void 
	_put_block(aiosock_block* block)
	{
		block->~aiosock_block();
		_block_allocator.deallocate(block);
	}


public:
	//! \brief constructor
	aiosock(size_t capacity,								//!< additional threads for processing asynchronous completion callbacks
			size_t deactivate_time_msec						//!< timeout in milliseconds to despose unused threads
			);
	//! \brief destructor
	~aiosock();

	//! \brief activates
	bool 
	on();
	//! \brief deactivates
	void 
	off();

	//! \brief creates socket
	virtual 
	size_t 
	create(	terimber_aiosock_callback* callback,			//!< callback function
			bool tcp_udp									//!< socket type TCP/UDP
			);
	//! \brief closes socket
	virtual 
	void 
	close(	size_t ident									//!< socket ident
			);
	//! \brief sends buffer to specified socket asynchronously
	virtual 
	int 
	send(	size_t ident,									//!< socket ident
			const void* buf,								//!< buffer to send
			size_t len,										//!< buffer length
			size_t timeout,									//!< timeout in milliseconds
			const sockaddr_in* toaddr,						//!< peer address, optional for TCP sockets
			void* userdata									//!< user defined data
			);
	//! \brief receives buffer of bytes from specified socket asynchronously
	virtual 
	int 
	receive(size_t ident,									//!< socket ident
			void* buf,										//!< buffer to receive
			size_t len,										//!< buffer length
			size_t timeout,									//!< timeout in milliseconds
			const sockaddr_in* fromaddr,					//!< peer address, optional for TCP sockets
			void* userdata									//!< user defined data
			);
	//! \brief connects to the specified socket synchronously
	virtual 
	int 
	connect(size_t ident,									//!< socket ident
			const char* address,							//!< peer address: IP, DNS name, localhost
			unsigned short port,							//!< peer port
			size_t timeout,									//!< timeout in milliseconds
			void* userdata									//!< user defined data
			);
	//! \brief turns the specified socket to ***othe*** listening state and will accept automatically the incoming connections asynchronously
	virtual 
	int 
	listen(	size_t ident,									//!< socket ident
			unsigned short port,							//!< listener port
			size_t max_connection,							//!< max waiting connections to accept
			const char* address,							//!< listener address: IP, DNS name, localhost
			unsigned short accept_pool,						//!< max initiated acceptors
			void* userdata									//!< user defined data
			);

	//! \brief binds UDP socket to address
	virtual 
	int 
	bind(	size_t ident,									//!< socket ident
			const char* address,							//!< address: IP, DNS name, localhost
			unsigned short port								//!< port
			);

	//! \brief gets the peer address
	virtual 
	int 
	getpeeraddr(size_t ident,								//!< socket ident
				sockaddr_in& addr							//!< peer address
				);		
	//! \brief gets the sock address
	virtual 
	int 
	getsockaddr(size_t ident,								//!< socket ident 
				sockaddr_in& addr							//!< this socket address
				);

	//! \brief gets error string by code
	virtual 
	bool
	get_error_description(
					int err,								//!< error code
					char* buf,								//!< [in, out] buffer
					size_t	len								//!< buffer length
				) const
	{
		return resolve_sock_error_code(err, buf, len);
	}

	//! \brief makes the snapshot of internal state
	virtual 
	void 
	doxray();

	//! \brief
	static
	bool
	resolve_sock_error_code(
					int err,								//!< error code
					char* buf,								//!< [in, out] buffer
					size_t	len								//!< buffer length
					);
protected:
	//! \brief detects if there is any job to do
	virtual 
	bool 
	v_has_job(	size_t ident,								//!< thread ident
				void* data									//!< user defined data
				);
	//! \brief executes actual job in separate thread
	virtual 
	void 
	v_do_job(	size_t ident,								//!< thread ident
				void* data									//!< user defined data
				);

private:
	//! \brief resolves address into IP
	bool 
	resolve_socket_address(const char* address,				//!< socket address as string
							unsigned short port,			//!< socket port
							sockaddr_in& addr				//!< socket address as struct
							);

	//! \brief thread-safe function to find socket handle by socket ident
	aio_sock_handle 
	find_socket_handle(	size_t ident						//!< socket ident
						);

	//! \brief processes timeouted blocks
	void 
	process_timeouted_blocks();

	//! \brief initiates new accepts
	bool 
	process_accept_blocks();

	//! \brief completes asynchronous action 
	void 
	complete_block(		size_t sock_key,					//!< socket ident
						aiosock_block* ov,					//!< asynchronous block pointer
						int err,							//!< error code, if any
						size_t processed					//!< bytes processed
						);

	//! \brief inserts block into incoming queue for processing in a separate thread
	int 
	_activate_block(	size_t ident,						//!< socket ident
						aiosock_block* block,				//!< asynchronous block pointer
						const sockaddr_in* pddr				//!< peer address, optional for TCP sockets
						);

	//! \brief cancels asynchronous operations
	void 
	_cancel_socket(		aio_sock_handle handle				//!< socket handle
						);

#if OS_TYPE != OS_WIN32

	//! \brief cancels asynchronous operation
	void 
	_cancel_aio(		aio_sock_handle handle,				//!< socket handle
					OVERLAPPED* overlapped				//!< overlapped block
					);
#endif

	//! \brief associates socket handle with internal structures
	size_t 
	_assign_socket(		aio_sock_handle handle,				//!< socket handle
						terimber_aiosock_callback* callback, //!< callback for processing completion actions
						bool tcp_udp						//!< socket type TCP/UDP
						);

	//! \brief closes socket
	void 
	_close_socket(		aio_sock_handle handle,				//!< socket handle
						bool tcp_udp						//!< socket type TCP/UDP
						);


	//! \brief starts actual IO processing
	int 
	_process_block(		aiosock_block* block				//!< pointer to block
						);

	// specific function for IO blocks processing
	//! \brief processing connect
	int 
	_process_connect(	aio_sock_handle handle,				//!< socket handle
						aiosock_block* block				//!< pointer to block
						);
	//! \brief processing accept
	int 
	_process_accept(	aio_sock_handle handle,				//!< socket handle
						terimber_aiosock_callback* client, //!< callback for processing completion actions
						aiosock_block* block				//!< pointer to block
						);
	//! \brief processing send
	int 
	_process_send(		aio_sock_handle handle,				//!< socket handle
						aiosock_block* block,				//!< pointer to block
						bool tcp_udp						//!< socket type TCP/UDP
						);
	//! \brief processing receive
	int 
	_process_recv(		aio_sock_handle handle,				//!< socket handle
						aiosock_block* block,				//!< pointer to block
						bool tcp_udp						//!< socket type TCP/UDP
						);

	//! \brief waits for completion actions in separate thread Terimber Completion Port
	void 
	wait_for_io_completion();

public:
	 
	mutex							_mtx;					//!< multithreaded access to socket map
	aiosock_socket_map_t			_socket_map;			//!< socket map
	aiosock_reverse_map_t			_reverse_map;			//!< reverse socket map
	aiosock_listener_map_t			_listeners_map;			//!< listeners map
	aiosock_delay_key_t				_delay_key_map;			//!< map of the keys which can not be returned back to generator right now
	unique_key_generator			_socket_generator;		//!< generates socket idents
	aiosock_pblock_allocator_t		_incoming_list_allocator;//!< external allocator for list of pointers to blocks
	aiosock_block_allocator_t		_block_allocator;		//!< block allocator
	aiosock_pblock_list_t			_initial_list;			//!< keeps initial processing reuqests
	aiosock_pblock_list_t			_outgoing_list;			//!< keeps processed asynchronous requests

#if OS_TYPE == OS_WIN32
	aiosock_pblock_list_t			_abounded_list;			//!< keeps abounded asynchronous requests for Complition Port only
#endif

private:
	HANDLE							_aiosock_io_handle; 	//!< this Terimber port handle
	threadpool						_thread_pool;			//!< thread pool
	size_t							_capacity;				//!< max thread pool capacity
	thread							_in_thread;				//!< housekeeping thread - process initatial and timeouted blocks
	static bool						_port_init;				//!< initialize once
	bool							_on;					//!< activation flag
	bool							_flag_io_port;			//!< signals that the Terimber Completion Port is running
	event							_start_io_port;			//!< signals that the thread is waiting for completion actions - Terimber Completion Port
	event							_stop_io_port;			//!< signals that the thread stopped for completion actions - Terimber Completion Port
};



#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_aioport_h_
