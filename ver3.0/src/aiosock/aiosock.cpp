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

#include "osdetect.h"

//! For VC7, and VC8
#if OS_TYPE == OS_WIN32
#if defined(_MSC_VER) && (_MSC_VER >= 1200) 
#include <winsock2.h>
#endif
#endif

//! inline hpp include
#include "base/list.hpp"
#include "aiosock/aiosock.h"
#include "base/map.hpp"
#include "base/stack.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"


//! constructor
terimber_aiosock_factory::terimber_aiosock_factory()
{
}

//! destructor
terimber_aiosock_factory::~terimber_aiosock_factory()
{
}

//! creates a new aiosock instance
terimber_aiosock*
terimber_aiosock_factory::get_aiosock(terimber_log* log, size_t capacity, size_t deactivate_time_msec)
{
	// creates a new object
	terimber::aiosock* obj = new terimber::aiosock(capacity, deactivate_time_msec);
	if (obj)
	{
		// sets the logging pointer
		obj->log_on(log);
		// activates
		obj->on();
	}

	return obj;
}


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)


// socket base error messages table
#if OS_TYPE == OS_WIN32
static exception_item aiosockMsgs[] =
{
    { WSAEACCES,				"Permission denied" },
    { WSAEADDRINUSE,			"Address already in use" },
    { WSAEADDRNOTAVAIL,			"Cannot assign requested address" },
    { WSAEAFNOSUPPORT,			"Address family not supported by protocol family" },
    { WSAEALREADY,				"Operation already in progress" },
    { WSAECONNABORTED,          "Software caused connection abort" },
    { WSAECONNREFUSED,          "Connection refused" },
    { WSAECONNRESET,            "Connection reset by peer" },
    { WSAEDESTADDRREQ,          "Destination address required" },
    { WSAEFAULT,                "Bad address" },
    { WSAEHOSTDOWN,             "Host is down" },
    { WSAEHOSTUNREACH,          "No route to host" },
    { WSAEINPROGRESS,           "Operation now in progress" },
    { WSAEINTR,                 "Interrupted function call" },
    { WSAEINVAL,                "Invalid argument" },
    { WSAEISCONN,               "Socket is already connected" },
    { WSAEMFILE,                "Too many open files" },
    { WSAEMSGSIZE,              "Message too long" },
    { WSAENETDOWN,              "Network is down" },
    { WSAENETRESET,             "Network dropped connection on reset" },
    { WSAENETUNREACH,           "Network is unreachable" },
    { WSAENOBUFS,               "No buffer space available" },
    { WSAENOPROTOOPT,           "Bad protocol option" },
    { WSAENOTCONN,              "Socket is not connected" },
    { WSAENOTSOCK,              "Socket operation on non-socket" },
    { WSAEOPNOTSUPP,            "Operation not supported" },
    { WSAEPFNOSUPPORT,          "Protocol family not supported" },
    { WSAEPROCLIM,              "Too many processes" },
    { WSAEPROTONOSUPPORT,       "Protocol not supported" },
    { WSAEPROTOTYPE,            "Protocol wrong type of socket" },
    { WSAESHUTDOWN,             "Cannot send after socket shutdown" },
    { WSAESOCKTNOSUPPORT,       "Socket type not supported" },
    { WSAETIMEDOUT,             "Connection timed out" },
    { WSAEWOULDBLOCK,           "Resource temporarily unavailable" },
    { WSAHOST_NOT_FOUND,        "Host not found" },
    { WSANOTINITIALISED,        "Successful WSAStartup not yet performed" },
    { WSANO_DATA,               "Valid name, no data record of requested type" },
    { WSANO_RECOVERY,           "This is a non-recoverable error" },
    { WSASYSNOTREADY,           "Network subsystem is unavailable" },
    { WSATRY_AGAIN,             "Non-authoritative host not found" },
    { WSAVERNOTSUPPORTED,       "WINSOCK.DLL version out of range" },
    { WSAEDISCON,               "Graceful shutdown in progress" },
    { HOST_NOT_FOUND,			"Host not found" },
    { NO_DATA,					"Valid name, no data record of requested type" },
    { NO_RECOVERY,				"This is a non-recoverable error" },
    { TRY_AGAIN,				"Non-authoritative host not found" },
    { 0,						0 }
};
#else
static exception_item aiosockMsgs[] =
{
    { EACCES,					"Permission denied" },
    { EADDRINUSE,				"Address already in use" },
    { EADDRNOTAVAIL,			"Cannot assign requested address" },
    { EAFNOSUPPORT,				"Address family not supported by protocol family" },
    { EALREADY,					"Operation already in progress" },
    { ECONNABORTED,				"Software caused connection abort" },
    { ECONNREFUSED,				"Connection refused" },
    { ECONNRESET,				"Connection reset by peer" },
    { EDESTADDRREQ,				"Destination address required" },
    { EFAULT,					"Bad address" },
    { EHOSTDOWN,				"Host is down" },
    { EHOSTUNREACH,				"No route to host" },
    { EINPROGRESS,				"Operation now in progress" },
    { EINTR,					"Interrupted function call" },
    { EINVAL,					"Invalid argument" },
    { EISCONN,					"Socket is already connected" },
    { EMFILE,					"Too many open files" },
    { EMSGSIZE,					"Message too long" },
    { ENETDOWN,					"Network is down" },
    { ENETRESET,				"Network dropped connection on reset" },
    { ENETUNREACH,				"Network is unreachable" },
    { ENOBUFS,					"No buffer space available" },
    { ENOPROTOOPT,				"Bad protocol option" },
    { ENOTCONN,					"Socket is not connected" },
    { ENOTSOCK,					"Socket operation on non-socket" },
    { EOPNOTSUPP,				"Operation not supported" },
    { EPFNOSUPPORT,				"Protocol family not supported" },
    { EPROTONOSUPPORT,			"Protocol not supported" },
    { EPROTOTYPE,				"Protocol wrong type of socket" },
    { ESHUTDOWN,				"Cannot send after socket shutdown" },
    { ESOCKTNOSUPPORT,			"Socket type not supported" },
    { ETIMEDOUT,				"Connection timed out" },
    { EWOULDBLOCK,				"Resource temporarily unavailable" },
    { 0,						0 }
};
#endif

exception_table aiosockTable(aiosockMsgs);


//! \brief initiation thread ident
const size_t aiosock_io_initiation_ident = 1;
//! \brief initiation thread stay on alert time
const size_t aiosock_io_initiation_thread_alert = 1000; // IO initiation and timeouts
//! \brief thread wait for completion Terimber Completion Port ident
const size_t aiosock_completion_io_port_ident = 2;
//! \brief waits until closure
const size_t aiosock_completion_io_port_thread_alert = INFINITE;
//! \brief working thread ident
const size_t aiosock_working_ident = 3;
//! \brief returns back to pool in 1 minute
const size_t aiosock_working_thread_alert = 60000; // working threads
 
#if OS_TYPE == OS_WIN32
//! Windows Completion Port function prototypes
	typedef BOOL (PASCAL *PCONNECTEX)(SOCKET s,
									const struct sockaddr* name,
									int namelen,
									PVOID lpSendBuffer,
									DWORD dwSendDataLength,
									LPDWORD lpdwBytesSent,
									LPOVERLAPPED lpOverlapped);

	typedef BOOL (PASCAL *PACCEPTEX)(SOCKET sListenSocket,
									SOCKET sAcceptSocket,
									PVOID lpOutputBuffer,
									DWORD dwReceiveDataLength,
									DWORD dwLocalAddressLength,
									DWORD dwRemoteAddressLength,
									LPDWORD lpdwBytesReceived,
									LPOVERLAPPED lpOverlapped);

	typedef BOOL (PASCAL *PDISCONNECTEX)(SOCKET hSocket,
										LPOVERLAPPED lpOverlapped,
										DWORD dwFlags,
										DWORD reserved);

//! GUID for retrieving function pointers from winsock library
const GUID GUID_MSWSOCK_ACCEPTEX = {0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}};
const GUID GUID_MSWSOCK_CONNECTEX = {0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}};
const GUID GUID_MSWSOCK_DISCONNECTEX = {0x7fda2e11,0x8630,0x436f,{0xa0, 0x31, 0xf5, 0x36, 0xa6, 0xee, 0xc1, 0x57}};

#ifndef SO_UPDATE_ACCEPT_CONTEXT
#define SO_UPDATE_ACCEPT_CONTEXT    0x700B
#endif

#ifndef SO_UPDATE_CONNECT_CONTEXT
#define SO_UPDATE_CONNECT_CONTEXT   0x7010
#endif

#endif

////////////////////////////////////
aiosock_block::aiosock_block()
{ 
	clear();
}

aiosock_block::aiosock_block(const aiosock_block& x)
{
	*this = x;
}

aiosock_block& 
aiosock_block::operator=(const aiosock_block& x)
{
	if (this != &x)
	{
		memcpy(this, &x, sizeof(aiosock_block));
	}

	return *this;
}

void 
aiosock_block::clear()
{
	memset(this, 0, sizeof(aiosock_block));
}

void 
aiosock_block::settimeout(size_t timeout)
{
	if (timeout == INFINITE)
	{
#if OS_TYPE == OS_WIN32 
		_timeout = 0;
#else
		_timeout.tv_sec = _timeout.tv_usec = 0;
#endif
		_expired = 0;
	}
	else
	{
#if OS_TYPE == OS_WIN32 
		_timeout = timeout;
#else
		_timeout.tv_sec = timeout / 1000;
		_timeout.tv_usec = (timeout % 1000) * 1000 + 1;
#endif
		date now;
		_expired = (sb8_t)now + timeout;
	}
}

/////////////////////////////////////////////////////////////////////////////
//static
bool
aiosock::resolve_sock_error_code(int err, char* buf, size_t len)
{
	if (!buf || !len)
		return false;

	const char* ret = 0;
	if ((ret = aiosockTable.get_error(err)))
	{
		TERIMBER::str_template::strcpy(buf, ret, len - 1);
		buf[len] = 0;
	}
	else
	{
		os_get_error(err, buf, len);
	}

	return true;
}


aiosock::aiosock(size_t capacity, size_t deactivate_time_msec) : 
_socket_map(less< size_t >(), 64)
,_reverse_map(less< aio_sock_handle >(), 64)
,_socket_generator(64)
,_outgoing_list(64)
,_aiosock_io_handle(0)
,_thread_pool(capacity + 3, deactivate_time_msec) // 3 + (xp, completion, working) + additional working threads
,_capacity(capacity)
,_on(false)
,_flag_io_port(false)
{
}

aiosock::~aiosock()
{
	// just in case
	off();
}

bool
aiosock::on()
{
	if (_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiosock already started");
		return false;
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "starting aiosock...");

	if (_sockStartup())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not initiate socket library");
		return false;
	}

#if OS_TYPE != OS_WIN32
	TERIMBER::SetLog(this);
#endif
	_thread_pool.log_on(this);

	if (!_thread_pool.on())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not start thread pool");
		return false;
	}

		format_logging(0, __FILE__, __LINE__, en_log_info, "init completion port");
#if OS_TYPE == OS_WIN32
		_aiosock_io_handle = ::CreateIoCompletionPort((HANDLE)INVALID_SOCKET, 0, 0, 0);
#else
		_aiosock_io_handle = TERIMBER::CreateIoCompletionPort((HANDLE)INVALID_SOCKET, 0, 0, TYPE_UNKNOWN);
#endif


	if (!_aiosock_io_handle)
	{
#if OS_TYPE == OS_WIN32
		::CloseHandle((HANDLE)_aiosock_io_handle);
#else
		TERIMBER::CloseHandle(_aiosock_io_handle);
#endif
		format_logging(0, __FILE__, __LINE__, en_log_info, "can not initiate completion port");
		return false;
	}

	_flag_io_port = true;
	_thread_pool.borrow_thread(aiosock_completion_io_port_ident, 0, this, aiosock_completion_io_port_thread_alert);
	_start_io_port.wait();

	// starts working thread
	_thread_pool.borrow_thread(aiosock_working_ident, 0, this, aiosock_working_thread_alert);

	// starts ininitial thread
	_in_thread.start();
	job_task task(this, aiosock_io_initiation_ident, aiosock_io_initiation_thread_alert, 0);
	_in_thread.assign_job(task);

	_on = true;

	format_logging(0, __FILE__, __LINE__, en_log_info, "aio socket port is initialized");

	return _aiosock_io_handle != 0;
}


// uninit aio port
void
aiosock::off()
{
	if (!_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiosock already stopped");
		return;
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "stoping aiosock...");
	_in_thread.cancel_job();
	_in_thread.stop();


	format_logging(0, __FILE__, __LINE__, en_log_info, "send stop message to completion port");
#if OS_TYPE == OS_WIN32
	::PostQueuedCompletionStatus((HANDLE)_aiosock_io_handle, 0, 0, 0);
#else
	TERIMBER::PostQueuedCompletionStatus(_aiosock_io_handle, 0, 0, 0);
#endif

	_stop_io_port.wait();
	format_logging(0, __FILE__, __LINE__, en_log_info, "completion port stopped");


#if OS_TYPE == OS_WIN32
	// if I/O is running send quit message
	::CloseHandle((HANDLE)_aiosock_io_handle);
#else
	TERIMBER::CloseHandle(_aiosock_io_handle);
#endif


	_aiosock_io_handle = 0;

	format_logging(0, __FILE__, __LINE__, en_log_info, "Stoping thread pool");
	_thread_pool.revoke_client(this);
	_thread_pool.off();
	_thread_pool.log_on(0);

	format_logging(0, __FILE__, __LINE__, en_log_info, "Close all sockets");

	mutexKeeper guard(_mtx);
	for (aiosock_socket_map_iterator_t iter = _socket_map.begin(); iter != _socket_map.end(); ++iter)
	{
		iter->_incoming_list.erase(_incoming_list_allocator, iter->_incoming_list.begin(), iter->_incoming_list.end());
		_cancel_socket(iter->_handle);
		_close_socket(iter->_handle, iter->_tcp_udp);
	}


	format_logging(0, __FILE__, __LINE__, en_log_info, "cleans up resources");

	// clears
	_socket_generator.clear();
	_socket_map.clear();
	_reverse_map.clear();
	_listeners_map.clear();
	_delay_key_map.clear();

	// clears outgoing, timeouted, and initial blocks
	_clear_block_lists();

	// resets allocators
	_incoming_list_allocator.clear_extra();
	_block_allocator.clear_extra();

	// turns off logging for Terimber Completion port
#if OS_TYPE != OS_WIN32
	TERIMBER::SetLog(0);
#endif

	// uninit socket library
	_sockCleanup();

	// resets flag
	_on = false;

	format_logging(0, __FILE__, __LINE__, en_log_info, "aio socket port is uninitialized");
}

// virtual 
bool 
aiosock::v_has_job(size_t ident, void* data)
{
	// Completion port is closed leave return false
	if (_aiosock_io_handle == 0)
		return false;

	switch (ident)
	{
		case aiosock_completion_io_port_ident: // io port
			return _flag_io_port;
		case aiosock_io_initiation_ident:
			{
				// locks mutex
				mutexKeeper guard(_mtx);

				// first priority, inits asynchronous socket actions
				if (!_initial_list.empty())
					return true;

				// second priority, adds new accepts
				for (aiosock_listener_map_t::const_iterator iter = _listeners_map.begin(); iter != _listeners_map.end(); ++iter)
				{
					if (iter->_curr_count < iter->_max_count)
						return true;
				}

				// finally checks timeouts
				date now;
				sb8_t unow = (sb8_t)now;
				// gets what user wants and does it
				for (aiosock_socket_map_iterator_t iter_socket = _socket_map.begin(); iter_socket != _socket_map.end(); ++iter_socket)
				{
					for (aiosock_pblock_alloc_list_t::iterator iter_block = iter_socket->_incoming_list.begin(); 
																	iter_block != iter_socket->_incoming_list.end(); ++iter_block)
					{
						// gets pointer to block
						aiosock_block* block = *iter_block;

						if (block->_expired != 0 // not infinite timeout
							&& unow >= block->_expired) // already expired
						{
							return true;
						}
					}
				}

				// all blocks are up to date
				return false;
			}
		case aiosock_working_ident:
		default:
			{
				// locks mutex
				mutexKeeper guard(_mtx);
				// checks output queue
				return !_outgoing_list.empty();
			}
	}

	return false;
}

void 
aiosock::wait_for_io_completion()
{
	// notifies main thread about starting this one
	_start_io_port.set();

	while (true) // infinite loop, until GetQueuedCompletionStatus returns zero key it's a signal to leave thread
	{
		aiosock_block* ov = 0;
		size_t sock_key = os_minus_one;
		size_t num_bytes = 0;
#if OS_TYPE == OS_WIN32
		bool bRes = (TRUE == ::GetQueuedCompletionStatus((HANDLE)_aiosock_io_handle, 
														(DWORD*)&num_bytes, 
#if defined(_MSC_VER) && (_MSC_VER > 1200) 
														(ULONG_PTR*)
#else
														(DWORD*)
#endif
														&sock_key, 
														(LPOVERLAPPED*)&ov, 
														INFINITE));

		int cRes = bRes ? 0 : ::GetLastError(); 
#else
		int cRes = TERIMBER::GetQueuedCompletionStatus(_aiosock_io_handle, 
														&num_bytes, 
														&sock_key, 
														(LPOVERLAPPED*)&ov, 
														INFINITE);

		if (ov) // transfer address
		{
			ov->_address = ov->remoteAddress;
		}
#endif
		if (!sock_key)
		{
			// resets flag
			_flag_io_port = false;
			// sends signal to main thread that this thread is about to close
			_stop_io_port.set();
			// breaks while loop
			break;
		}

		// processes completed block
		complete_block(sock_key, ov, cRes, num_bytes);
	} // while
}

void 
aiosock::complete_block(size_t sock_key, aiosock_block* ov, int err, size_t processed)
{
	// locks mutex
	mutexKeeper guard(_mtx);
	// finds the socket info by sock_key
	aiosock_socket_map_iterator_t iter_sock = _socket_map.find(sock_key);

	if (iter_sock == _socket_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket key %d not found", sock_key);
		return;
	}

	// loop for all incoming blocks for this particular socket
	for (aiosock_pblock_alloc_list_t::iterator iter_block = iter_sock->_incoming_list.begin(); iter_block != iter_sock->_incoming_list.end(); ++iter_block)
	{
		// looking for specific overlapped pointer
		if (*iter_block != ov)
			continue;

		// gets block
		aiosock_block* block = *iter_block;
		// removes from incoming list 
		iter_sock->_incoming_list.erase(_incoming_list_allocator, iter_block);
		// assigns error code if any
		block->_err = err;
		// assigns processed bytes
		block->_processed = processed;
		// put block to the output queue
		_outgoing_list.push_back(block);
		// unlocks mutex
		guard.unlock();
		// wakes up working thread any from range
		if (!_capacity || !_thread_pool.borrow_from_range(aiosock_working_ident, aiosock_working_ident + _capacity, 0, this, aiosock_working_thread_alert))
			_thread_pool.borrow_thread(aiosock_working_ident, 0, this, aiosock_working_thread_alert);

		return;
	} // for

#if OS_TYPE == OS_WIN32

	// loop for all abounded blocks - block could be timeouted but still inside Completion Port, 
	// we can not destroy such a block, so we put it to the abounded blocks queue
	for (aiosock_pblock_alloc_list_t::iterator iter_abounded = _abounded_list.begin(); iter_abounded != _abounded_list.end(); ++iter_abounded)
	{
		if (*iter_abounded != ov)
			continue;

		// gets block
		aiosock_block* block = *iter_abounded;
		// removes block from abounded list
		_abounded_list.erase(iter_abounded);
		// destroys block
		_put_block(block);
		break;
	} // for

#endif
}

// virtual 
void 
aiosock::v_do_job(size_t ident, void* data)
{
	switch (ident)
	{
		case aiosock_completion_io_port_ident: // Terimber Completion Port
			wait_for_io_completion();
			break;
		case aiosock_io_initiation_ident:
			{
				//  locks mutex
				mutexKeeper guard(_mtx);

				if (_initial_list.empty())
				{
					// initial list is empty
					// unlocks mutex
					guard.unlock();

					// processes acceptors, if any
					if (!process_accept_blocks())
					{
						// no acceptors - process timeouted blocks
						process_timeouted_blocks();
					}

					return;
				}

				// gest first initial block
				aiosock_block* block = _initial_list.front();
				// removes it from initial list
				_initial_list.pop_front();

				// processes initial  block
				if (int err = _process_block(block))
				{
					// assigns error, block does NOT initiate asynchronous action
					block->_err = err;
					// puts to the output queue
					_outgoing_list.push_back(block);
					// unlocks mutex
					guard.unlock();
					// wakes up any working thread from range
					if (!_capacity || !_thread_pool.borrow_from_range(aiosock_working_ident, aiosock_working_ident + _capacity, 0, this, aiosock_working_thread_alert))
						_thread_pool.borrow_thread(aiosock_working_ident, 0, this, aiosock_working_thread_alert);
				}
			}
			break;
		case aiosock_working_ident:
		default:
			{
				aiosock_block* block = 0;
				aio_sock_handle handle = 0;
				aio_sock_handle accept_handle = 0;
				terimber_aiosock_callback* client_obj = 0;
	
				// locks mutex
				mutexKeeper guard(_mtx);

				if (_outgoing_list.empty())
					return; // output queue is empty - nothing to do.

				// gets first block in queue
				block = _outgoing_list.front();
				// removes it from queue
				_outgoing_list.pop_front();

				// finds correspondent socket
				aiosock_socket_map_t::iterator iter_sock = _socket_map.find(block->_socket_ident);
				
				if (iter_sock == _socket_map.end())
				{
					format_logging(0, __FILE__, __LINE__, en_log_error, "socket key %d not found", block->_socket_ident);
					// block has been removed
					_put_block(block);
					return;
				}
				else
				{
					// gets client callback pointer
					client_obj = iter_sock->_client_obj;
					// gets socket handle
					handle = iter_sock->_handle;
					// increment callback invoking counter
					++iter_sock->_callback_invoking;

					// checks if it's an accept block type
					if (block->_type == AIOSOCK_ACCEPT)
					{
#if OS_TYPE == OS_WIN32
						// tries to find and get acceptor handle
						// for Windows Completion Port it has been associated with the socket map
						aiosock_socket_map_t::iterator iter_accept = _socket_map.find(block->_accept_ident);
						if (iter_accept != _socket_map.end())
							accept_handle = iter_accept->_handle; // assigns acceptor socket handle
#endif
						// finds correspondent listener
						aiosock_listener_map_t::iterator iter = _listeners_map.find(block->_socket_ident);
						if (iter != _listeners_map.end())
							--iter->_curr_count; // decrements the acceptor counter for listener 
					}
				}

				// processing
				if (block->_err)
				{
					// error occured
					// unlocks mutex
					guard.unlock();

					// if it's an acceptor block type and the acceptor ident is not zero
					if (block->_type == AIOSOCK_ACCEPT && block->_accept_ident)
					{
						// closes accpetor socket - only for Windows completion Port
						close(block->_accept_ident);
						block->_accept_ident = 0;
					}

					try
					{
						// invokes user callback for error processing
						client_obj->v_on_error(block->_socket_ident, block->_err, block->_type, block->_userdata);
					}
					catch (...)
					{
						format_logging(0, __FILE__, __LINE__, en_log_error, "v_on_error exception for socket %d", block->_socket_ident);
						assert(false);
					}
				}
				else
				{
					// no errors
					// checks block type
					switch (block->_type)
					{
						case AIOSOCK_CONNECT:
							// unlocks mutex
							guard.unlock();

#if OS_TYPE == OS_WIN32
							// for Windows Completion Port we need to update newely connected socket
							setsockopt(handle, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, 0, 0);
#endif
							try
							{
								// invokes user callback for connect
								client_obj->v_on_connect(block->_socket_ident, block->_address, block->_userdata);
							}
							catch (...)
							{
								format_logging(0, __FILE__, __LINE__, en_log_error, "v_on_connect exception for socket %d", block->_socket_ident);
								assert(false);
							}
							break;
						case AIOSOCK_ACCEPT:
#if OS_TYPE == OS_WIN32
							setsockopt(accept_handle, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&handle, sizeof(handle));
#else
							// associates new accepted socket with internal map
							block->_accept_ident = _assign_socket(block->hAccept, client_obj, true);
#endif
							// finds correspondent socket
							iter_sock = _socket_map.find(block->_accept_ident);
							if (iter_sock != _socket_map.end())
							{
								// assigns listener (default) callback
								terimber_aiosock_callback*& r_accept_callback = iter_sock->_client_obj;
								// unlocks mutex
								guard.unlock();
								try
								{
									// invokes user callback for accept - user can change callback
									client_obj->v_on_accept(block->_socket_ident, block->_accept_ident, r_accept_callback, block->_address, block->_userdata);
								}
								catch (...)
								{
									format_logging(0, __FILE__, __LINE__, en_log_error, "v_on_accept exception for socket %d", block->_socket_ident);
									assert(false);
								}
							}
							else
							{
								// unlocks mutex - socket not found
								guard.unlock();
								format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", block->_socket_ident);
							}

							// wakes up thread
							_in_thread.wakeup();
							break;
						case AIOSOCK_SEND:
							// unlocks mutex
							guard.unlock();

							try
							{
								// invokes user callback for send - user can change callback
								client_obj->v_on_send(block->_socket_ident, (void*)block->_buf, block->_len, block->_processed, block->_address, block->_userdata);
							}
							catch (...)
							{
								format_logging(0, __FILE__, __LINE__, en_log_error, "v_on_send exception for socket %d", block->_socket_ident);
								assert(false);
							}
							break;
						case AIOSOCK_RECV:
							// unlocks mutex
							guard.unlock();

							try
							{
								// invokes user callback for receive - user can change callback
								client_obj->v_on_receive(block->_socket_ident, (void*)block->_buf, block->_len, block->_processed, block->_address, block->_userdata);
							}
							catch (...)
							{
								format_logging(0, __FILE__, __LINE__, en_log_error, "v_on_receive exception for socket %d", block->_socket_ident);
								assert(false);
							}
							break;
						default:
							assert(false);

					} // switch
				} // else

				// locks mutex again
				guard.lock();
				
				// checks if we are still in a map
				// finds correspondent socket
				iter_sock = _socket_map.find(block->_socket_ident);
				
				// socket not found
				if (iter_sock == _socket_map.end())
				{
					// checks delay map - missing socket could be there
					aiosock_delay_key_t::iterator iter_delay = _delay_key_map.find(block->_socket_ident);

					if (iter_delay != _delay_key_map.end())
					{
						if (--*iter_delay <= 0) // decrement dealy counter for socket
						{
							// if counter is zero - erases it from delay map
							_delay_key_map.erase(iter_delay);
							// saves socket ident back to generator
							_socket_generator.save(block->_socket_ident);
						}
					}
				}
				else
				{
					// decrement callback counter
					--iter_sock->_callback_invoking;
				}

				// destroys block
				_put_block(block);
			} // block
			break;
	} // switch
}

// virtual 
size_t 
aiosock::create(terimber_aiosock_callback* callback, bool tcp_udp)
{
	if (!_aiosock_io_handle)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiosock is not initialized");
		return 0;
	}

// tries to create a new socket TCP/UDP
#if OS_TYPE == OS_WIN32
	aio_sock_handle handle = ::WSASocket(PF_INET, tcp_udp ? SOCK_STREAM : SOCK_DGRAM, tcp_udp ? IPPROTO_TCP : IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED);
#else
	aio_sock_handle handle = ::socket(PF_INET, tcp_udp ? SOCK_STREAM : SOCK_DGRAM, tcp_udp ? IPPROTO_TCP : IPPROTO_UDP);
#endif
	if (handle == INVALID_SOCKET)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not create socket");
		return 0;
	}

	// locks mutex
	mutexKeeper guard(_mtx);
	// associates socket handle with internal map
	return _assign_socket(handle, callback, tcp_udp);
}

// virtual 
void 
aiosock::close(size_t ident)
{
	if (!_aiosock_io_handle)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiosock is not initialized");
		return;
	}

	// locks mutex
	mutexKeeper guard(_mtx);
	// tries to find socket
	aiosock_socket_map_iterator_t iter = _socket_map.find(ident);
	if (iter == _socket_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", ident);
		return;
	}

	size_t handle = iter->_handle;
	bool tcp_udp = iter->_tcp_udp;
	bool delay_key = false;

	// erases all incoming blocks
	iter->_incoming_list.erase(_incoming_list_allocator, iter->_incoming_list.begin(), iter->_incoming_list.end());
	// checks if callbacks are not invoking for this socket
	if (iter->_callback_invoking)
	{
		delay_key = true;
		aiosock_delay_key_t::iterator iter_delay = _delay_key_map.find(ident);
		if (iter_delay != _delay_key_map.end())
			*iter_delay += iter->_callback_invoking;
		else
			_delay_key_map.insert(ident, iter->_callback_invoking);
	}

	_reverse_map.erase(iter->_handle);
	_socket_map.erase(iter);

	// locks initial block and removes the closed blocks
	for (aiosock_pblock_list_t::iterator in_iter = _initial_list.begin(); in_iter != _initial_list.end();)
	{
		aiosock_block* block = *in_iter;
		if (block->_socket_ident == ident)
		{
			_put_block(block);
			in_iter = _initial_list.erase(in_iter);
		}
		else
			++in_iter;
	}

	for (aiosock_pblock_list_t::iterator out_iter = _outgoing_list.begin(); out_iter != _outgoing_list.end();)
	{
		aiosock_block* block = *out_iter;
		if (block->_socket_ident == ident)
		{
			_put_block(block);
			out_iter = _outgoing_list.erase(out_iter);
		}
		else
			++out_iter;
	}

	_cancel_socket(handle);

#if OS_TYPE == OS_WIN32
	// before erasing, moves all blocks to timeouted
	for (aiosock_pblock_list_t::iterator iter_list = iter->_incoming_list.begin(); iter_list != iter->_incoming_list.end();)
	{			
		_abounded_list.push_back(*iter_list);
		iter_list = iter->_incoming_list.erase(_incoming_list_allocator, iter_list);
	}
#else
	// before erasing, moves all blocks to timeouted
	for (aiosock_pblock_list_t::iterator iter_list = iter->_incoming_list.begin(); iter_list != iter->_incoming_list.end();)
	{			
		aiosock_block* block = *iter_list;
		_put_block(block);
		iter_list = iter->_incoming_list.erase(_incoming_list_allocator, iter_list);
	}

#endif

	guard.unlock();

	_close_socket(handle, tcp_udp);

	guard.lock();

	aiosock_listener_map_t::iterator iter_listener = _listeners_map.find(ident);

	if (iter_listener != _listeners_map.end())
		_listeners_map.erase(iter_listener);

	if (!delay_key)
		// returns handle back to generator
		_socket_generator.save(ident);

	format_logging(0, __FILE__, __LINE__, en_log_info, "socket handle %u is closed", handle);
}

// binds to the specified socket address
// virtual 
int
aiosock::bind(size_t ident, const char* address, unsigned short port)
{
	sockaddr_in ipAddr;
	if (!resolve_socket_address(address, port, ipAddr))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not resolve socket %d, address %s, port %hu", ident, address, port);
		return -1;
	}

	aio_sock_handle handle = find_socket_handle(ident);

	if (handle == INVALID_SOCKET)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", ident);
		return -1;
	}

	// binds
	if (::bind(handle, (struct sockaddr*)&ipAddr, sizeof(struct sockaddr_in)))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not bind socket %d, address %s, port %hu", ident, address, port);
		return -1;
	}

	return 0;
}

// sends buffer to specified socket asynchronously
// virtual 
int 
aiosock::send(size_t ident, const void* buf, size_t len, size_t timeout, const sockaddr_in* toaddr, void* userdata)
{
	// locks mutex
	mutexKeeper guard(_mtx);
	// creates block
	aiosock_block* block = _get_block();
	// sets timeout
	block->settimeout(timeout);
	// assigns type
	block->_type = AIOSOCK_SEND;
	// assigns user data
	block->_userdata = userdata;
	// assigns buffer
	block->_buf = (char*)buf;
	// assigns buffer length
	block->_len = len;
	// activates block
	return _activate_block(ident, block, toaddr);
}


// receives buffer of bytes from specified socket asynchronously
// virtual 
int 
aiosock::receive(size_t ident, void* buf, size_t len, size_t timeout, const sockaddr_in* fromaddr, void* userdata)
{
	// locks mutex
	mutexKeeper guard(_mtx);
	// creates block
	aiosock_block* block = _get_block();
	// sets timeout
	block->settimeout(timeout);
	// assigns type
	block->_type = AIOSOCK_RECV;
	// assigns user data
	block->_userdata = userdata;
	// assigns buffer
	block->_buf = (char*)buf;
	// assigns buffer length
	block->_len = len;
	// activates block
	return _activate_block(ident, block, fromaddr);
}

// connects to the specified socket synchronously
// virtual 
int
aiosock::connect(size_t ident, const char* address, unsigned short port, size_t timeout, void* userdata)
{
	sockaddr_in ipAddr;
	if (!resolve_socket_address(address, port, ipAddr))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not resolve socket, address %s, port %hu", address, port);
		return -1;
	}
    
	// creates block
	mutexKeeper guard(_mtx);
	// creates block
	aiosock_block* block = _get_block();
	// sets timeout
	block->settimeout(timeout);
	// set address
	block->_address = ipAddr;
	// assigns type
	block->_type = AIOSOCK_CONNECT;
	// assigns user data
	block->_userdata = userdata;
	// activates block
	return _activate_block(ident, block, 0);
}

// turns the specified socket to other listening state and will automatically accept incoming connections asynchronously
// virtual 
int
aiosock::listen(size_t ident, unsigned short port, size_t max_connection, const char* address, unsigned short accept_pool, void* userdata)
{
	// resolves address
	sockaddr_in ipAddr;
	if (!resolve_socket_address(address, port, ipAddr))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not resolve socket %d, address %s, port %hu", ident, address, port);
		return -1;
	}

	// finds socket
	aio_sock_handle handle = find_socket_handle(ident);

	if (handle == INVALID_SOCKET)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", ident);
		return -1;
	}

	// binds
	if (::bind(handle, (const sockaddr*)&ipAddr, sizeof(struct sockaddr_in)))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not bind socket %d, address %s, port %hu", ident, address, port);
		return -1;
	}

	// starts to listen
	if (::listen(handle, (int)max_connection))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not start listener %d, address %s, port %hu", ident, address, port);
		return -1;
	}

	// locks mutex
	mutexKeeper guard(_mtx);
	// finds listener
	aiosock_listener_map_t::iterator iter_listener = _listeners_map.find(ident);
	if (iter_listener == _listeners_map.end())
	{
		// inserts new listener
		listener_info dummy(0, accept_pool, userdata);
		_listeners_map.insert(ident, dummy);
	}

	// unlocks
	guard.unlock();

	// wakes up initial thread
	_in_thread.wakeup();

	format_logging(0, __FILE__, __LINE__, en_log_info, "listener started, address %s, port %hu", address, port);
	return 0;
}

// gets the peer address
//virtual 
int
aiosock::getpeeraddr(size_t ident, sockaddr_in& addr)
{
	// finds socket
	aio_sock_handle handle = find_socket_handle(ident);

	if (handle == INVALID_SOCKET)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", ident);
		return -1;
	}

#if OS_TYPE == OS_WIN32
	int 
#else
	socklen_t
#endif
	len = sizeof(sockaddr_in);
	return ::getpeername(handle, (sockaddr*)&addr, &len);
}

// gets the sock address
//virtual 
int
aiosock::getsockaddr(size_t ident, sockaddr_in& addr)
{
	// finds socket
	aio_sock_handle handle = find_socket_handle(ident);

	if (handle == INVALID_SOCKET)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", ident);
		return -1;
	}

#if OS_TYPE == OS_WIN32
	int 
#else
	socklen_t
#endif
	len = sizeof(sockaddr_in);
	return ::getsockname(handle, (sockaddr*)&addr, &len);
}

// makes a snapshot of the internal state
// virtual 
void
aiosock::doxray()
{
	mutexKeeper guard(_mtx);

	size_t socks = _socket_map.size(), 
		listeners = _listeners_map.size(),
		delay_actions = _delay_key_map.size(),   
		initiated_actions = _initial_list.size(),
		completed_actions = _outgoing_list.size(),
		abounded_actions = 
#if OS_TYPE == OS_WIN32
		_abounded_list.size();
#else
		0;
#endif

	guard.unlock();

	format_logging(0, __FILE__, __LINE__, en_log_xray, "<aiosock socks=\"%d\" listeners = \"%d\" delayed=\"%d\" initiated=\"%d\" completed=\"%d\" abounded=\"%d\" />",
		socks, listeners, delay_actions, initiated_actions, completed_actions, abounded_actions);

#if OS_TYPE != OS_WIN32
	TERIMBER::DoXRay();
#endif
	_thread_pool.doxray();
}

size_t 
aiosock::_assign_socket(aio_sock_handle handle, terimber_aiosock_callback* callback, bool tcp_udp)
{
	// creates a new map entry
	aiosock_socket new_socket(tcp_udp, handle, callback);

	// generates a new ident
	size_t ident = _socket_generator.generate();

	// inserts into map
	aiosock_socket_map_iterator_t iter_sock = _socket_map.insert(ident, new_socket).first;
	if (iter_sock == _socket_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		_close_socket(new_socket._handle, tcp_udp);
		_socket_generator.save(ident);
		return 0;
	}

	// inserts into reverse map
	aiosock_reverse_map_iterator_t iter_reverse = _reverse_map.insert(handle, iter_sock).first;
	if (iter_reverse == _reverse_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		_close_socket(new_socket._handle, tcp_udp);
		_socket_map.erase(iter_sock);
		_socket_generator.save(ident);
		return 0;
	}


	if (tcp_udp) // sets options for TCP
	{
		linger lingerStruct;
		//lingerStruct.l_onoff = 1; // brutal
		lingerStruct.l_onoff = 0; // graceful       
		lingerStruct.l_linger = 0;
		::setsockopt(handle, SOL_SOCKET, SO_LINGER, (const char*)&lingerStruct, sizeof(lingerStruct));
	
		int noDelay = 1;
		::setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&noDelay, sizeof(noDelay));

		int keepAlive = 1;
		::setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepAlive, sizeof(keepAlive));
	}
	
	// sets option for all types
	int reUse = 1;
	::setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (const char*)&reUse, sizeof(reUse));

#if OS_TYPE == OS_WIN32
	// associates TCP socket with Windows Completion Port
	if (!::CreateIoCompletionPort((HANDLE)new_socket._handle, 
								(HANDLE)_aiosock_io_handle, 
								(DWORD)ident, 
								3))
#else
	// associates socket with Terimber completion port, if it's not TCP and not Windows XP
	if (!TERIMBER::CreateIoCompletionPort(new_socket._handle, 
								_aiosock_io_handle, 
								ident, 
								tcp_udp ? TYPE_TCP : TYPE_UDP))

#endif
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not assign socket handle to completion port");
		_close_socket(new_socket._handle, true);
		_reverse_map.erase(iter_reverse);
		_socket_map.erase(iter_sock);
		_socket_generator.save(ident);
		return 0;
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "assign socekt handle %u, ident %u is open", new_socket._handle, ident);

	return ident;
}

void 
aiosock::_close_socket(aio_sock_handle handle, bool tcp_udp)
{
	static char rbuf[512];

	// shuts down send first
#if OS_TYPE == OS_WIN32
	if (tcp_udp)
		::shutdown(handle, SD_SEND);
#else
	if (tcp_udp)
		::shutdown(handle, SHUT_WR);
#endif

	if (tcp_udp)
	{
		// if TCP tries to receive the rest of bytes
		int res = 0;

		do
		{
		  fd_set recv_set;
		  FD_ZERO(&recv_set); 
		  FD_SET(handle, &recv_set);

		  struct timeval timeout_val = {0, 1000};
			
      res = ::select((int)handle + 1, &recv_set, 0, 0, &timeout_val);
		
			if (res)
			{
				res = ::recv(handle, rbuf,  512, 0);
				if (res  > 0 && res < 512)
					break;
			}
		}
		while (res > 0);
	}

	// shuts down receive
#if OS_TYPE == OS_WIN32
	if (tcp_udp)
		::shutdown(handle, SD_RECEIVE);
	// closes socket
	::closesocket(handle);
#else
	if (tcp_udp)
		::shutdown(handle, SHUT_RD);
	// closes socket
	::close(handle);
#endif	

	format_logging(0, __FILE__, __LINE__, en_log_info, "socket closed, handle: %d", handle);
}

void 
aiosock::_cancel_socket(aio_sock_handle handle)
{
	// clears all tickets
#if OS_TYPE == OS_WIN32
	::CancelIo((HANDLE)handle);
#else
	TERIMBER::CancelIo(handle, 0);
#endif
}

#if OS_TYPE != OS_WIN32

//! \brief cancels asynchronous operation
void 
aiosock::_cancel_aio(		aio_sock_handle handle,				//!< file handle
					LPOVERLAPPED overlapped				//!< overlapped block
					)
{
	TERIMBER::CancelIo(handle, overlapped);
}

#endif

aio_sock_handle 
aiosock::find_socket_handle(size_t ident)
{
	// locks mutex
	mutexKeeper guard(_mtx);
	// finds socket
	aiosock_socket_map_iterator_t iter = _socket_map.find(ident);
	return (iter != _socket_map.end()) ? iter->_handle : INVALID_SOCKET;
}

bool
aiosock::resolve_socket_address(const char* address, unsigned short port, sockaddr_in& addr)
{
	in_addr ipAddr;
	memset(&addr, 0, sizeof(sockaddr_in));

	if (address) // remote socket
	{
		bool askDNS = false;
		for (const char* s = address; *s ; ++s)
		{
			if (!isdigit( *s ) && *s != '.') // if a domain name was specified
			{
				askDNS = true;
				break;
			}
		}

		if (askDNS)
		{
			struct hostent* host = gethostbyname(address);
			if (!host)
				return false;

			memcpy(&ipAddr, host->h_addr, host->h_length);
		}
		else
		{
			ipAddr.s_addr = inet_addr(address);
			if (ipAddr.s_addr == INADDR_NONE)
				return false;
		}
	}
	else
		ipAddr.s_addr = htonl(INADDR_LOOPBACK);
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = ipAddr;

	return true;
}

int 
aiosock::_process_block(aiosock_block* block)
{
	if (!_aiosock_io_handle)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiosock is not initialized");
		return -1;
	}

	aiosock_socket_map_iterator_t iter_sock = _socket_map.find(block->_socket_ident);
	if (iter_sock == _socket_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", block->_socket_ident);
		return -1;
	}

	aio_sock_handle handle = iter_sock->_handle;
	terimber_aiosock_callback* client = iter_sock->_client_obj;

	// puts block into the incoming list
	iter_sock->_incoming_list.push_back(_incoming_list_allocator, block);

	//  assume that we took the top block for processing
	// but didn't remove it
	int res = 0;
	switch (block->_type)
	{
		case AIOSOCK_CONNECT:
			res = _process_connect(handle, block);
#if OS_TYPE == OS_WIN32
#else
			if (res == EINPROGRESS)
				res = EWOULDBLOCK;
#endif
			break;
		case AIOSOCK_ACCEPT:
			// assigns socket here
			res = _process_accept(handle, client, block);
			break;
		case AIOSOCK_SEND:
			res = _process_send(handle, block, iter_sock->_tcp_udp);
			break;
		case AIOSOCK_RECV:
			res = _process_recv(handle, block, iter_sock->_tcp_udp);
			break;
	} // switch

	// removes from incoming queue if
	// - in-place success
	// - occured error differs from pending operation
	// - occured error differs from winsock2 operation

	if (!res
		|| res 
#if OS_TYPE == OS_WIN32
		&& (
			res != ERROR_IO_PENDING
			&& res != WSAEWOULDBLOCK
		)
#else	// for Linux
		&& res != EWOULDBLOCK
#endif
	)
	{
		// removes from incoming list
		iter_sock->_incoming_list.pop_back(_incoming_list_allocator);

		// populates outgoing queue here
		block->_err = res;

		// inserts block into outgoing list 
		if (!res)
		{
			// inserts into output queue
			_outgoing_list.push_back(block);

			// wakes up any working thread from range
			if (!_capacity || !_thread_pool.borrow_from_range(aiosock_working_ident, aiosock_working_ident + _capacity, 0, this, aiosock_working_thread_alert))
				_thread_pool.borrow_thread(aiosock_working_ident, 0, this, aiosock_working_thread_alert);

			return 0;
		}
		else // deletes block
		{
			// returns error
			return res;
		}
	}
	else // this is a pending operation
	{
		block->_err = 0;
		block->_processed = 0;
		return 0;
	}
}

// inserts block into incoming queue for processing in a separate thread
int 
aiosock::_activate_block(size_t ident, aiosock_block* block, const sockaddr_in* paddr)
{
	// finds socket
	aiosock_socket_map_iterator_t iter_sock = _socket_map.find(ident);
	if (iter_sock == _socket_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", ident);
		_put_block(block);
		return -1;
	}

	// assigns socket handle
	aio_sock_handle handle = iter_sock->_handle;
	// assigns user callback
	terimber_aiosock_callback* client = iter_sock->_client_obj;

	// assigns socket ident
	block->_socket_ident = ident;

	// assigns address
	if (!iter_sock->_tcp_udp)
	{
		assert(paddr != 0);
		block->_address = *paddr;
	}
	
	// puts block into the initial list
	_initial_list.push_back(block);

	// wakes up initial thread
	_in_thread.wakeup();

	return 0;
}

int
aiosock::_process_connect(aio_sock_handle handle, aiosock_block* block)
{
	// sets send timeout
	::setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, (const char*)&block->_timeout, sizeof(block->_timeout));
#if OS_TYPE == OS_WIN32 
	// loads accept function pointer here
	GUID dummy_connect = GUID_MSWSOCK_CONNECTEX;
	PCONNECTEX _ConnectEx = 0;
	DWORD dwBytes = 0;

	sockaddr_in ipAddr;
	memset(&ipAddr, 0, sizeof(sockaddr_in));
	ipAddr.sin_family = AF_INET;
	ipAddr.sin_port = htons(0);

	if (SOCKET_ERROR == ::WSAIoctl(handle, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&dummy_connect, sizeof(dummy_connect), &_ConnectEx, sizeof(_ConnectEx), &dwBytes, 0, 0)
		|| SOCKET_ERROR == ::bind(handle, (const struct sockaddr*)&ipAddr, sizeof(struct sockaddr_in))
		)
	{
		return (block->_err = ::GetLastError());
	}
	else
	{
		return (*_ConnectEx)(handle, 
					(const struct sockaddr*)&block->_address, 
					sizeof(struct sockaddr_in), 
					0,
					0,
					0, 
					block) ?
		ERROR_IO_PENDING : 
		(block->_err = ::GetLastError());
	}

#else
	return TERIMBER::ConnectEx(handle, 
					&block->_address,
					block) ?
		EWOULDBLOCK : 
		(block->_err = errno);
#endif
}

int
aiosock::_process_accept(aio_sock_handle handle, terimber_aiosock_callback* client, aiosock_block* block)
{
#if OS_TYPE == OS_WIN32 
	// creates socket
	aio_sock_handle accept_handle = ::socket(AF_INET, SOCK_STREAM, 0);
	if (accept_handle == INVALID_SOCKET)
		return (block->_err = ::WSAGetLastError());

	block->_accept_ident = _assign_socket(accept_handle, client, true);
	
	if (!block->_accept_ident)
		return WSAEINVAL;

	GUID dummy_accept = GUID_MSWSOCK_ACCEPTEX;
	PACCEPTEX _AcceptEx = 0;
	DWORD dwBytes = 0;

	// loads accept here
	if (SOCKET_ERROR == ::WSAIoctl(handle, SIO_GET_EXTENSION_FUNCTION_POINTER,
				&dummy_accept, sizeof(dummy_accept), &_AcceptEx, sizeof(_AcceptEx), &dwBytes, 0, 0))
	{
		close(block->_accept_ident);
		return (block->_err = ::GetLastError());
	}
	else
	{
		if ((*_AcceptEx)(handle, 
					accept_handle, 
					block->_accept_buf, 
					0, 
					sizeof(struct sockaddr_in) + 16, 
					sizeof(struct sockaddr_in) + 16, 
					(LPDWORD)&block->_processed, 
					block))
		{
			return ERROR_IO_PENDING;
		}
		else
		{
			if (ERROR_IO_PENDING != (block->_err = ::GetLastError()))
				close(block->_accept_ident);

			return block->_err;
		}
	}
#else
	return TERIMBER::AcceptEx(handle,
						block) ?
		EWOULDBLOCK : 
		(block->_err = errno);
#endif
}

int
aiosock::_process_send(aio_sock_handle handle, aiosock_block* block, bool tcp_udp)
{
	// sends data as non-blocking 
#if OS_TYPE == OS_WIN32
	if (tcp_udp)
	{
		// even operation will complete without pending
		// WriteFile will notify IO Complition Port
		return ::WriteFile((HANDLE)handle, 
						block->_buf, 
						(DWORD)block->_len, 
						(LPDWORD)&block->_processed, 
						block) ?
			ERROR_IO_PENDING : 
			(block->_err = ::GetLastError());
	}
	else
	{
		WSABUF buf;
		buf.buf = block->_buf;
		buf.len = (u_long)block->_len;
		return ::WSASendTo(handle, &buf, 1, (LPDWORD)&block->_processed, 0, (const sockaddr *)&block->_address, sizeof(block->_address), block, 0) ?
			ERROR_IO_PENDING :
			(block->_err = ::GetLastError());
	}
#else
	return ((tcp_udp) ? TERIMBER::WSASend(handle, block->_buf, block->_len, block)
			: TERIMBER::WSASendTo(handle, block->_buf, block->_len, &block->_address, block)) ?
		EWOULDBLOCK : 
		(block->_err = errno);
#endif
}

int 
aiosock::_process_recv(aio_sock_handle handle, aiosock_block* block, bool tcp_udp)
{
#if OS_TYPE == OS_WIN32 
	// even operation will complete without pending
	// ReadFile will notify IO Complition Port
	if (tcp_udp)
	{
		return ::ReadFile((HANDLE)handle, 
						block->_buf, 
						(DWORD)block->_len, 
						(LPDWORD)&block->_processed, 
						block) ?
			ERROR_IO_PENDING : 
			(block->_err = ::GetLastError());
	}
	else
	{
		WSABUF buf;
		buf.buf = block->_buf;
		buf.len = (u_long)block->_len;
		// use this buffer for long-life memory
		block->_accept_ident = sizeof(sockaddr_in);
		block->_flags = 0; //PJ_IOQUEUE_ALWAYS_ASYNC;
		return ::WSARecvFrom(handle, &buf, 1, (LPDWORD)&block->_processed, (LPDWORD)&block->_flags, (sockaddr *)&block->_address, (int*)&block->_accept_ident, block, 0) ?
			ERROR_IO_PENDING :
			(block->_err = ::GetLastError());
	}
#else
	return ((tcp_udp) ? TERIMBER::WSARecv(handle, block->_buf, block->_len, block) 
			: TERIMBER::WSARecvFrom(handle, block->_buf, block->_len, &block->_address, block)) ?
			EWOULDBLOCK : 
			(block->_err = errno);

#endif
}

void 
aiosock::process_timeouted_blocks()
{
	// gets current date
	date now;
	sb8_t unow = (sb8_t)now;

	// locks mutex
	mutexKeeper guard(_mtx);
	// gets what user wants and does it
	for (aiosock_socket_map_iterator_t iter_socket = _socket_map.begin(); iter_socket != _socket_map.end(); ++iter_socket)
	{
		size_t socket_key = iter_socket.key();
		for (aiosock_pblock_alloc_list_t::iterator iter_block = iter_socket->_incoming_list.begin(); 
														iter_block != iter_socket->_incoming_list.end();)
		{
			// gest pointer to block
			aiosock_block* block = *iter_block;

			if (block->_expired == 0	// infinite timeout
				|| unow > block->_expired) // not expired yet
			{
				++iter_block;
				continue;
			}

			// gets socket information
			aio_sock_handle handle = iter_socket->_handle;
			terimber_aiosock_callback* client_obj = iter_socket->_client_obj;

			// erases block from list
			iter_block = iter_socket->_incoming_list.erase(_incoming_list_allocator, iter_block);

			// sets callback counter
			++iter_socket->_callback_invoking;
			// unlocks mutex
			guard.unlock();

			format_logging(0, __FILE__, __LINE__, en_log_error, "timeouted %s action for socket %d", 
				block->_type == AIOSOCK_CONNECT ? "connect" : 
					(block->_type == AIOSOCK_ACCEPT ? "accept" : 
						(block->_type == AIOSOCK_RECV ? "recv" : "send")),
				socket_key);

			// calls error callback
			try
			{
				client_obj->v_on_error(block->_socket_ident, 
#if OS_TYPE == OS_WIN32
					WSAETIMEDOUT
#else							
					ETIMEDOUT
#endif
					, block->_type
					, block->_userdata);
			}
			catch (...)
			{
				assert(false);
			}

			// resets callback counter
			guard.lock();
			
			// finds correspondent socket
			aiosock_socket_map_t::iterator iter_find = _socket_map.find(block->_socket_ident);
			
			if (iter_find == _socket_map.end())
			{
				// checks delay map
				aiosock_delay_key_t::iterator iter_delay = _delay_key_map.find(block->_socket_ident);

				if (iter_delay != _delay_key_map.end())
				{
					assert(*iter_delay > 0);

					if (--*iter_delay <= 0)
					{
						_delay_key_map.erase(iter_delay);
						_socket_generator.save(block->_socket_ident);
					}
				}
				// else - socket port is about to close - nothing to do
			}
			else
			{
				// decrement callback counter
				assert(iter_find->_callback_invoking > 0);
				--iter_find->_callback_invoking;
			}


#if OS_TYPE == OS_WIN32
			// puts block to abounded list
			_abounded_list.push_back(block);
#else
			_cancel_aio(handle, block);
			_put_block(block);
#endif
			// one at the time 
			return;
		} // for block
	} // for socket
}

bool 
aiosock::process_accept_blocks()
{
	// locks mutex
	mutexKeeper guard(_mtx);

	// loop for all listeners
	for (aiosock_listener_map_t::iterator iter = _listeners_map.begin(); iter != _listeners_map.end(); ++iter)
	{
		// current counts are good
		if (iter->_curr_count >= iter->_max_count)
			continue;

		// creates block
		aiosock_block* block = _get_block();
		// sets timeout infinite
		block->settimeout(INFINITE);
		// sets type
		block->_type = AIOSOCK_ACCEPT;
		// sets user data
		block->_userdata = iter->_userdata;
		// activates block
		if (!_activate_block(iter.key(), block, 0))
		{
			// increment acceptor counter
			++iter->_curr_count;
			return true;
		}
	}

	return false;
}

#pragma pack()
END_TERIMBER_NAMESPACE
