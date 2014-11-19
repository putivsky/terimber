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

#include "aiocomport/aiocomport.h"

int aiocomport_compiler_blocker = 0;


#if OS_TYPE != OS_WIN32

//! include hpp inline code
#include "base/map.hpp"
#include "base/list.hpp"
#include "base/stack.hpp"
#include "base/common.hpp"
#include "base/memory.hpp"
#include "base/date.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! redefine MAX_SOCKET to max defined above
#ifndef MAX_SOCKET
#define MAX_SOCKET FD_SETSIZE
#endif

//! \brief timeout for select function
const size_t us_timeout = 1000000;
//! \brief returns working thread back to thread pool after 10 seconds inactivity
const size_t cp_working_thread_alert = 10000;

//! \brief creates singleton of completion port
//! to expose C style functions API
static aiocomport singleton;

//! \class handle_desc 
//! \brief helps to keep the current activity read/write for socket handle
class handle_desc
{
public:
	HANDLE	_handle;	//!< socket handle
	bool	_writing;	//!< flags write actions
	bool	_reading;	//!< flags read actions
};

//! implementation
//! redirects call to singleton instance

HANDLE 
CreateIoCompletionPort
(
	HANDLE sock_fd,
	HANDLE port_fd,
	size_t completion_key,
	en_type type
)
{
	return singleton.CreateIoCompletionPort(sock_fd, port_fd, completion_key, type);
}

bool 
AcceptEx
(
	HANDLE sock_listen,
	LPOVERLAPPED overlapped
)
{
	return singleton.AcceptEx(sock_listen, overlapped);
}

bool
ConnectEx
(
	HANDLE sock_fd,
	const struct sockaddr_in* name,
	LPOVERLAPPED overlapped
)
{
	return singleton.ConnectEx(sock_fd, name, overlapped);
}

bool 
WSARecv
(
	HANDLE sock_fd,
	void* buf,
	size_t len,
	LPOVERLAPPED overlapped
)
{
	return singleton.WSARecv(sock_fd, buf, len, overlapped);
}

bool 
WSARecvFrom
(
	HANDLE sock_fd,
	void* buf,
	size_t len,
	struct sockaddr_in* name,
	LPOVERLAPPED overlapped
)
{
	return singleton.WSARecvFrom(sock_fd, buf, len, name, overlapped);
}

bool 
ReadFile
(
	HANDLE sock_fd,
	void* buf,
	size_t len,
	LPOVERLAPPED overlapped
)
{
	return singleton.ReadFile(sock_fd, buf, len, overlapped);
}

bool 
WSASend
(
	HANDLE sock_fd,
	const void* buf,
	size_t len,
	LPOVERLAPPED overlapped
)
{
	return singleton.WSASend(sock_fd, buf, len, overlapped);
}

bool 
WSASendTo
(
	HANDLE sock_fd,
	const void* buf,
	size_t len,
	const struct sockaddr_in* name,
	LPOVERLAPPED overlapped
)
{
	return singleton.WSASendTo(sock_fd, buf, len, name, overlapped);
}

bool 
WriteFile
(
	HANDLE sock_fd,
	const void* buf,
	size_t len,
	LPOVERLAPPED overlapped
)
{
	return singleton.WriteFile(sock_fd, buf, len, overlapped);
}


int 
GetQueuedCompletionStatus
(
	HANDLE port_fd,
	size_t* number_bytes,
	size_t* completion_key,
	LPOVERLAPPED* overlapped,
	size_t timeout_milliseconds
)
{
	return singleton.GetQueuedCompletionStatus(port_fd, number_bytes, completion_key, overlapped, timeout_milliseconds);
}

bool 
PostQueuedCompletionStatus
(
	HANDLE port_fd,
	size_t bytes_transferred,
	size_t completion_key,
	LPOVERLAPPED overlapped
)
{
	return singleton.PostQueuedCompletionStatus(port_fd, bytes_transferred, completion_key, overlapped);
}

bool
CloseHandle
(
	HANDLE port_fd
)
{
	return singleton.CloseHandle(port_fd);
}

bool
CancelIo
(
	HANDLE sock_fd,
	LPOVERLAPPED overlapped
)
{
	return singleton.CancelIo(sock_fd, overlapped);
}

void
SetLog
(
	terimber_log* log
)
{
	singleton.SetLog(log);
}

//! \brief logging the internal state information
void
DoXRay()
{
	// call common code
	singleton.DoXRay();
}

//////////////////////////////////////////////////////////////////////////////////////
#ifndef NO_NPTL
// define signals only if real-time signals are supported
#define AIOSOCKSIGNAL (SIGRTMIN + 3)
#define AIOFILESIGNAL (SIGRTMIN + 4)

#endif

//! implementation
aiocomport::aiocomport() : 
_event_thread_id(0)
{
	struct sigaction sa;
	sa.sa_handler = SIG_DFL;
	sa.sa_flags = 0;
	// block SIGIO on a process level
	sigemptyset(&sa.sa_mask);

#ifndef NO_NPTL
	sigaddset(&sa.sa_mask, SIGIO);	
	sigaddset(&sa.sa_mask, AIOSOCKSIGNAL);	
	sigaddset(&sa.sa_mask, AIOFILESIGNAL);	
#else
//	sigaddset(&sa.sa_mask, SIGIO);		
#endif
	// block signal for the process level
	sigprocmask(SIG_BLOCK, &sa.sa_mask, 0);	
	// block signal for all threads	created
	pthread_sigmask(SIG_BLOCK, &sa.sa_mask, 0);

#ifndef NO_NPTL
	// set sigaction for SIGIO
	sigaction(SIGIO, &sa, 0);
	sigaction(AIOSOCKSIGNAL, &sa, 0);
	sigaction(AIOFILESIGNAL, &sa, 0);
#else
//	sigaction(SIGIO, &sa, 0);	
#endif
}

aiocomport::~aiocomport()
{
	_event_thread_id = 0;
}

HANDLE 
aiocomport::CreateIoCompletionPort
(
	HANDLE sock_fd,
	HANDLE port_fd,
	size_t completion_key,
	en_type type
)
{
	// locks port mutex
	mutexKeeper keeper(_port_mtx);
	// remembers port handles
	HANDLE handle = port_fd;
	// inits iterator
	port_attr_map_t::iterator it_port = _port_attr_map.end();

	// checks if new port requested
	if (port_fd == 0)
	{
		// creates new port
		if (_port_attr_map.empty())
		{
			// start event thread first time
			job_task task(this, 0, INFINITE, 0);
			_event_thread.start();
			_event_thread.assign_job(task);
			_ev_activate.wait();
		}

		// generates port ident
		handle = _generator.generate();

		// inserts new port into map
		port_attributes attr;
		it_port = _port_attr_map.insert(handle, attr).first;
		if (it_port == _port_attr_map.end())
		{
			// returns back port handle
			_generator.save((size_t)handle);

			// stops event thread, if any
			if (_port_attr_map.empty())
			{
				_event_thread_id = 0;

				keeper.unlock();
				_event_thread.cancel_job();
				_event_thread.stop();
				_ev_deactivate.wait();
			}

			format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
			errno = -1; // sets error
			return 0;
		}
	}
	else
	{
		// must exists
		it_port = _port_attr_map.find(port_fd);
		if (it_port == _port_attr_map.end())
		{
			format_logging(0, __FILE__, __LINE__, en_log_error, "completion port ident %d not found", port_fd);
			errno = -1; // sets error
			return 0;
		}
	}
		
	if (sock_fd == INVALID_SOCKET) // port association with invalid handle
	{
		if (port_fd != 0)
		{
			format_logging(0, __FILE__, __LINE__, en_log_error, "can not accosiate invalid socket handle %d with completion port %d", sock_fd, port_fd);
			errno = -1; // sets error
			return 0;
		}
	}
	else // socket handle is provided
	{
		// set owner
		pid_t pid = getpid();
		// makes socket non-blocked
		int oldflags =  ::fcntl(sock_fd, F_GETFL, 0);
		::fcntl(sock_fd, F_SETOWN, pid);

		if (type != TYPE_FILE)
		{
#ifndef NO_NPTL 	
			::fcntl(sock_fd, F_SETFL, oldflags | O_NONBLOCK | O_ASYNC);
			// set signal
			::fcntl(sock_fd, F_SETSIG, AIOSOCKSIGNAL);
#else
			::fcntl(sock_fd, F_SETFL, oldflags | O_NONBLOCK);
#endif
			format_logging(0, __FILE__, __LINE__, en_log_info, "socket %d will signal to thread %d", sock_fd, pid);
		}
		else
		{
			::fcntl(sock_fd, F_SETFL, oldflags | O_NONBLOCK);
			format_logging(0, __FILE__, __LINE__, en_log_info, "file %d will signal to thread %d", sock_fd, pid);
		}

		// makes association
		// checks if the association exists
		fd_port_map_t::iterator it_fd = _fd_port_map.find(sock_fd);
		if (it_fd != _fd_port_map.end())
		{
			assert(port_fd != 0);
			format_logging(0, __FILE__, __LINE__, en_log_error, "accosiation already exists for socket handle %d with completion port %d", sock_fd, port_fd);
			errno = -1; // sets error
			return 0;
		}

		// inserts into association
		fd_item item(it_port, completion_key, type);

		it_fd = _fd_port_map.insert(sock_fd, item).first;
		if (it_fd == _fd_port_map.end())
		{
			if (port_fd == 0)
			{
				_port_attr_map.erase(handle);
				_generator.save((size_t)handle);
				// stop event thread, if any
				if (_port_attr_map.empty())
				{
					_event_thread_id = 0;
				
					keeper.unlock();
					_event_thread.cancel_job();
					_event_thread.stop();
					_ev_deactivate.wait();
				}
			}

			format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
			errno = -1; // sets error
			return 0;
		}
	}

	if (sock_fd == INVALID_SOCKET)
		format_logging(0, __FILE__, __LINE__, en_log_info, "new completion port %d created", handle);
	else
		format_logging(0, __FILE__, __LINE__, en_log_info, "socket %d is associated with completion port %d", sock_fd, handle);

	errno = 0;
	return handle;
}

bool 
aiocomport::AcceptEx
(
		HANDLE sock_listen,
		LPOVERLAPPED overlapped
)
{
	// call common code
	return initiate_action(ACTION_ACCEPT, sock_listen, overlapped, 0, 0, 0);
}

bool
aiocomport::ConnectEx
(
	HANDLE sock_fd,
	const struct sockaddr_in* name,
	LPOVERLAPPED overlapped
)
{
	// call common code
	return initiate_action(ACTION_CONNECT, sock_fd, overlapped, 0, 0, name);
}

bool 
aiocomport::WSARecv
(
	HANDLE sock_fd,
	void* buf,
	size_t len,
	LPOVERLAPPED overlapped
)
{
	// calls common code
	return initiate_action(ACTION_RECV, sock_fd, overlapped, buf, len, 0);
}

bool 
aiocomport::WSARecvFrom
(
	HANDLE sock_fd,
	void* buf,
	size_t len,
	struct sockaddr_in* name,
	LPOVERLAPPED overlapped
)
{
	// calls common code
	return initiate_action(ACTION_RECV, sock_fd, overlapped, buf, len, name);
}

bool 
aiocomport::ReadFile
(
	HANDLE sock_fd,
	void* buf,
	size_t len,
	LPOVERLAPPED overlapped
)
{
	// calls common code
	return initiate_action(ACTION_READ, sock_fd, overlapped, buf, len, 0);
}

bool 
aiocomport::WSASend
(
	HANDLE sock_fd,
	const void* buf,
	size_t len,
	LPOVERLAPPED overlapped
)
{
	// calls common code
	return initiate_action(ACTION_SEND, sock_fd, overlapped, buf, len, 0);
}

bool 
aiocomport::WSASendTo
(
	HANDLE sock_fd,
	const void* buf,
	size_t len,
	const struct sockaddr_in* name,
	LPOVERLAPPED overlapped
)
{
	// calls common code
	return initiate_action(ACTION_SEND, sock_fd, overlapped, buf, len, name);
}

bool 
aiocomport::WriteFile
(
	HANDLE sock_fd,
	const void* buf,
	size_t len,
	LPOVERLAPPED overlapped
)
{
	// calls common code
	return initiate_action(ACTION_WRITE, sock_fd, overlapped, buf, len, 0);
}

bool 
aiocomport::initiate_action(en_action action, HANDLE sock_id, LPOVERLAPPED overlapped, const void* buf, size_t len, const sockaddr_in* name)
{
	// checks overlapped pointer
	if (!overlapped)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "overlapped structure is not specified");
		errno = -1; // sets error
		return false;
	}

	// locks mutex
	mutexKeeper keeper(_port_mtx);

	// checks if listener socket associates with completion port
	fd_port_map_t::iterator it_fd = _fd_port_map.find(sock_id);
	if (it_fd == _fd_port_map.end())
	{	
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", sock_id);
		errno = -1; // sets error
		return false;
	}

	// initiates queue item
	queue_item item;
	item._action = action;		// what to do
	item.aio_fildes = sock_id;				// socket handle
	item._key = it_fd->_completion_key;	// completion key
	item._overlapped = overlapped;		// pointer to overlapped structure
	item.aio_buf = (void*)buf;				// buffer
	item.aio_nbytes = len;					// buffer length
	item.aio_offset = overlapped->offset;	// offset for files
	overlapped->hAccept = (SOCKET)INVALID_SOCKET; // assign invalid value
	if (name)
		overlapped->remoteAddress = *name;

	int ret = 0;
	SOCKET hAccept = (SOCKET)INVALID_SOCKET;
	socklen_t alen = sizeof(sockaddr_in);

	// adds block to the incoming list
	queue_container_iterator_t iter = it_fd->_initial_container.push_back(_queue_allocator, item); 

	if (iter == it_fd->_initial_container.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory for socket %d", sock_id);
		errno = -1;
		return false;
	}


	switch (action)
	{
		case ACTION_CONNECT:
			// checks if the name is provided for TCP socket
			if (!name)
			{
				it_fd->_initial_container.erase(_queue_allocator, iter);
				format_logging(0, __FILE__, __LINE__, en_log_error, "connection address for socket %d is not specified", sock_id);
				errno = -1; // sets error
				return false;
			}
			// tries to connect
			ret = ::connect(item.aio_fildes, (const sockaddr*)name, alen);
			break;
		case ACTION_ACCEPT:
			// tries to accept now
			hAccept = ::accept(item.aio_fildes, (sockaddr*)&overlapped->remoteAddress, &alen);
			ret = (hAccept == INVALID_SOCKET) ? -1 : 0;
			break;
		case ACTION_RECV:
			// checks if the name is provided for UDP socket
			if (it_fd->_type == TYPE_UDP && !name)
			{
				it_fd->_initial_container.erase(_queue_allocator, iter);
				format_logging(0, __FILE__, __LINE__, en_log_error, "peer address for UDP socket %d is required", sock_id);
				errno = -1; // sets error
				return false;
			}

			// checks buffer pointer
			if (!buf || !len)
			{
				it_fd->_initial_container.erase(_queue_allocator, iter);
				format_logging(0, __FILE__, __LINE__, en_log_error, "null or empty buffer for socket %d found", sock_id);
				errno = -1; // sets error
				return false;
			}

			// tries to recv now
			ret = (it_fd->_type == TYPE_TCP) ?	::recv(item.aio_fildes, (char*)item.aio_buf, item.aio_nbytes, 0) :
										::recvfrom(item.aio_fildes, (char*)item.aio_buf, item.aio_nbytes, 0, (sockaddr*)&overlapped->remoteAddress, &alen);
			break;
		case ACTION_SEND:
			// checks if the name is provided for UDP socket
			if (it_fd->_type == TYPE_UDP && !name)
			{
				it_fd->_initial_container.erase(_queue_allocator, iter);
				format_logging(0, __FILE__, __LINE__, en_log_error, "peer address for UDP socket %d is required", sock_id);
				errno = -1; // sets error
				return false;
			}

			// checks buffer pointer
			if (!buf || !len)
			{
				it_fd->_initial_container.erase(_queue_allocator, iter);
				format_logging(0, __FILE__, __LINE__, en_log_error, "null or empty buffer for socket %d found", sock_id);
				errno = -1; // sets error
				return false;
			}

			// tries to send now
			ret = (it_fd->_type == TYPE_TCP) ?	::send(item.aio_fildes, (const char*)item.aio_buf, (int)item.aio_nbytes, MSG_NOSIGNAL) :
										::sendto(item.aio_fildes, (const char*)item.aio_buf, (int)item.aio_nbytes, 0, (const sockaddr*)&overlapped->remoteAddress, alen);
			break;
		case ACTION_READ:
			{
#ifndef NO_NPTL
				iter->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
  				iter->aio_sigevent.sigev_signo = AIOFILESIGNAL;
#else
				iter->aio_sigevent.sigev_notify = SIGEV_NONE;
  				iter->aio_sigevent.sigev_signo = 0;
#endif
				iter->aio_sigevent.sigev_value.sival_ptr = &*iter;
				ret = aio_read(&*iter);
			}
			break;
		case ACTION_WRITE:
			{
#ifndef NO_NPTL
				iter->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
  				iter->aio_sigevent.sigev_signo = AIOFILESIGNAL;
#else
				iter->aio_sigevent.sigev_notify = SIGEV_NONE;
  				iter->aio_sigevent.sigev_signo = 0;
#endif
  				iter->aio_sigevent.sigev_value.sival_ptr = &*iter;
				ret = aio_write(&*iter);
			}
			break;
		default:
			assert(false);
			return false;
	}

	if (ret >= 0)
	{
		if (it_fd->_type != TYPE_FILE)
		{
			switch (action)
			{
				case ACTION_CONNECT:
					break;
				case ACTION_ACCEPT:
					// assigns accept handle
					iter->_overlapped->hAccept = hAccept;
					break;
				case ACTION_RECV:
					// assigns processed byte
					iter->_processed = ret;
					break;
				case ACTION_SEND:
					// assigns processed byte
					iter->_processed = ret;
					break;
				default:
					assert(false);
					return false;
			}

			// puts to the output queue
			it_fd->_port_iter->_completion_container.push_back(_queue_allocator, *iter);
			// remove from initial container
			it_fd->_initial_container.erase(_queue_allocator, iter); 
			// signals to the queue event
			it_fd->_port_iter->_queue_event.set();
			return true;
		}
	}
	else if (
		errno != EWOULDBLOCK && errno != EINPROGRESS
		)
	{		
		it_fd->_initial_container.erase(_queue_allocator, iter);
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not initiate action for socket %d, error %d", sock_id, errno);
		return false;
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "asynchronous action is initiated - atcion %d", action);
	// asynchronous action is initiated
#ifdef NO_NPTL	
	_ev_wakeup.set();
#endif	
	return true;
}


int
aiocomport::GetQueuedCompletionStatus
(
	HANDLE port_fd,
	size_t* number_bytes,
	size_t* completion_key,
	LPOVERLAPPED* overlapped,
	size_t timeout_milliseconds
)
{
	int ret = -1;

	// locks mutex
	mutexKeeper keeper(_port_mtx);

	// finds port attributes
	port_attr_map_t::iterator it_port = _port_attr_map.find(port_fd);
	if (it_port == _port_attr_map.end() // not found
		|| it_port->_go_to_exit // signals to exit
		)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "completion port %d does not exist", port_fd);
		return ret;
	}

	// increment loop counter preventing this entry from removal
	++it_port->_loop_lock;

	// gets reference to port attributes
	//port_attributes& r_attr = *it_port;

	// unlocks mutex
	keeper.unlock();

	// sets timeout in miiliseconds
	int timeout = (timeout_milliseconds == INFINITE) ? INFINITE : timeout_milliseconds;
	// gets current date/time
	date now;
	event* queue_event = 0;

	do
	{
		// adjusts timeout here
		if (timeout != INFINITE)
		{
			timeout -= (int)date::get_difference(now);

			if (timeout < 0) // run out of time
			{
				format_logging(0, __FILE__, __LINE__, en_log_error, "timeout occured for port queue %d", port_fd);
				errno = -1;
				break;
			}
		}

		// locks mutex
		keeper.lock();

		// finds port attributes
		it_port = _port_attr_map.find(port_fd);
		if (it_port == _port_attr_map.end() // not found
			|| it_port->_go_to_exit // signals to exit
			)
		{
			format_logging(0, __FILE__, __LINE__, en_log_error, "completion port %d does not exist", port_fd);
			return ret;
		}

		// checks if we got something in output queue
		if (!it_port->_completion_container.empty())
		{
			// gets top and leave
			const queue_item& item = it_port->_completion_container.front();
			// assigns processed bytes if any
			if (number_bytes)
				*number_bytes = item._processed;
			// sets completion key if any
			if (completion_key)
				*completion_key = item._key;
			// sets overlapped pointer if any
			if (overlapped)
				*overlapped = item._overlapped;

			// sets return error code
			ret = item._error;

			// removes queue item returning memory to queue allocator
			it_port->_completion_container.pop_front(_queue_allocator);

			// breaks the loop
			format_logging(0, __FILE__, __LINE__, en_log_paranoid, "queue item processed for port %d: socket %d, key %d, bytes %d, error %d", port_fd, item.aio_fildes, item._key, item._processed, item._error);

			// unlock
			keeper.unlock();

			break;
		}

		queue_event = &it_port->_queue_event;

		keeper.unlock();

	}
	while (WAIT_OBJECT_0 == queue_event->wait(timeout)); // wait for signal

	// locks mutex again
	keeper.lock();

	// tries to find completion port attributes again
	it_port = _port_attr_map.find(port_fd);
	if (it_port == _port_attr_map.end()) // already removed 
	{	
		return ret;
	}

	// decrement loop counter 
	--it_port->_loop_lock;
	if (it_port->_loop_lock == 0 // this is the last thread processing output queue for specified completion port
		&& it_port->_go_to_exit // signals to exit
		)
	{
		it_port->_exit_event.set(); // signal specific port can be removed
	}

	return ret;
}


bool 
aiocomport::PostQueuedCompletionStatus
(
	HANDLE port_fd,
	size_t bytes_transferred,
	size_t completion_key,
	LPOVERLAPPED overlapped
)
{
	// locks mutex
	mutexKeeper keeper(_port_mtx);

	// finds completion port attributes
	port_attr_map_t::iterator it_port = _port_attr_map.find(port_fd);
	if (it_port == _port_attr_map.end()) // not found
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "completion port %d not found", port_fd);
		return false;
	}

	// initiates queue item
	queue_item item;
	item._action = ACTION_USERDATA;			// what to do
	item.aio_fildes = (int)INVALID_SOCKET;				// socket handle
	item._key = completion_key;				// completion key
	item._overlapped = overlapped;		// pointer to overlapped structure
	item._processed = bytes_transferred;

	// puts to the output queue
	it_port->_completion_container.push_back(_queue_allocator, item); 
	// signals to queue event
	it_port->_queue_event.set();

	format_logging(0, __FILE__, __LINE__, en_log_paranoid, "user data action initiated for port %d, key %d, bytes %d", port_fd, completion_key, bytes_transferred);
	return true;
}

bool
aiocomport::CloseHandle
(
	HANDLE hObject
)
{
	// locks mutex
	mutexKeeper keeper(_port_mtx);

	// finds completion port attributes
	port_attr_map_t::iterator it_port = _port_attr_map.find(hObject);
	if (it_port == _port_attr_map.end()) // not found
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "completion port %d not found", hObject);
		return false;
	}

	// if flag is already set, leave function
	if (it_port->_go_to_exit)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "another thread is closing completion port %d", hObject);
		return false;
	}
	else // sets exit flag
		it_port->_go_to_exit = true;

	
	if (it_port->_loop_lock) // some queue is still in use
	{
		// notifies queue event
		it_port->_queue_event.set();

		// makes a copy
		event& r_event = it_port->_exit_event;
		// unlocks port mutex and wait 3 seconds 
		keeper.unlock();
		// waits to give a chance queue to exit a loop
		r_event.wait(3000);
		// locks mutex again
		keeper.lock();
	}

	// finds completion port attributes again
	it_port = _port_attr_map.find(hObject);
	if (it_port != _port_attr_map.end())
	{
		// removes all completion items
		for (queue_container_t::iterator it_comp  = it_port->_completion_container.begin(); it_comp != it_port->_completion_container.end();)
			it_comp = it_port->_completion_container.erase(_queue_allocator, it_comp);
		
		// removes all socket associations
		for (fd_port_map_t::iterator it_fd = _fd_port_map.begin(); it_fd != _fd_port_map.end();)
		{
			if (it_fd->_port_iter.key() == hObject)
			{
				// returns all initial items to allocator
				for (queue_container_t::iterator it_item = it_fd->_initial_container.begin(); it_item != it_fd->_initial_container.end();)
				{
					if (it_fd->_type == TYPE_FILE) // only for files
						aio_cancel(hObject, &*it_item);

					it_item = it_fd->_initial_container.erase(_queue_allocator, it_item);
				}

				// removes socket
				it_fd = _fd_port_map.erase(it_fd);
			}
			else
				++it_fd;
		}

		// erases port atributes
		_port_attr_map.erase(it_port);

		// if there are no more attributes, does cleans up
		if (_port_attr_map.empty())
		{
			// resets queue allocator
			_queue_allocator.reset();
			// stop event thread
			_event_thread_id = 0;

			keeper.unlock();
			_event_thread.cancel_job();
			_event_thread.stop();
			_ev_deactivate.wait();
		}
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "completion port %d is closed", hObject);
	return true;
}

bool
aiocomport::CancelIo
(
	HANDLE sock_fd,
	LPOVERLAPPED overlapped
)
{
	// locks mutex
	mutexKeeper keeper(_port_mtx);

	// checks if the socket associate with completion port
	fd_port_map_t::iterator it_fd = _fd_port_map.find(sock_fd);
	if (it_fd == _fd_port_map.end()) // not found
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "socket %d not found", sock_fd);
		return false;
	}

	// cleans up initial container
	for (queue_container_t::iterator it_item = it_fd->_initial_container.begin(); it_item != it_fd->_initial_container.end();)
	{
		// either overlapped not specified (any) or it should match
		if (overlapped != 0 && overlapped != it_item->_overlapped)
		{
			++it_item;
			continue;
		}

		if (it_fd->_type == TYPE_FILE) // only for files
			aio_cancel(sock_fd, &*it_item);

		// removes from initial queue
		it_item = it_fd->_initial_container.erase(_queue_allocator, it_item);
	}


	// cleans up complete queue
	for (queue_container_t::iterator it_queue = it_fd->_port_iter->_completion_container.begin(); it_queue != it_fd->_port_iter->_completion_container.begin();)
	{
		if (it_queue->aio_fildes != sock_fd || overlapped != 0 && overlapped != it_queue->_overlapped)
		{
			++it_queue;
			continue;
		}

		it_queue =  it_fd->_port_iter->_completion_container.erase(_queue_allocator, it_queue);
	}

	
	// removes socket
	if (!overlapped)
	{
		_fd_port_map.erase(it_fd);
		format_logging(0, __FILE__, __LINE__, en_log_info, "socket %d is removed from completion port", sock_fd);
	}

	return true;
}

void
aiocomport::SetLog
(
	terimber_log* log
)
{
	// call helper method
	log_on(log);
}

void
aiocomport::DoXRay()
{
	// locks mutex
	mutexKeeper keeper(_port_mtx); // port mutex

	size_t connect_initiated = 0, accept_initiated = 0, send_initiated = 0, recv_initiated = 0;
	size_t connect_completed = 0, accept_completed = 0, send_completed = 0, recv_completed = 0;

	// loop for all sockets ready for IO
	for (fd_port_map_t::iterator it_fd = _fd_port_map.begin(); it_fd != _fd_port_map.end(); ++it_fd)
	{
		queue_container_t::iterator it_item = it_fd->_initial_container.begin();
		for (; it_item != it_fd->_initial_container.end(); ++it_item)
		{
			switch (it_item->_action)
			{
				case ACTION_CONNECT:
					++connect_initiated;
					break;
				case ACTION_SEND:
				case ACTION_WRITE:
					++send_initiated;
					break;
				case ACTION_ACCEPT:
					++accept_initiated;
					break;
				case ACTION_RECV:
				case ACTION_READ:
					++recv_initiated;
					break;
				default:
					break;
			}
		}

		it_item = it_fd->_port_iter->_completion_container.begin();
		for (; it_item != it_fd->_port_iter->_completion_container.end(); ++it_item)
		{
			switch (it_item->_action)
			{
				case ACTION_CONNECT:
					++connect_completed;
					break;
				case ACTION_SEND:
				case ACTION_WRITE:
					++send_completed;
					break;
				case ACTION_ACCEPT:
					++accept_completed;
					break;
				case ACTION_RECV:
				case ACTION_READ:
					++recv_completed;
					break;
				default:
					break;
			}
		}
	}

	keeper.unlock();

	format_logging(0, __FILE__, __LINE__, en_log_xray, "<aiocomport><initial connect=\"%d\" accept=\"%d\" send=\"%d\" recv=\"%d\" /><completed connect=\"%d\" accept=\"%d\" send=\"%d\" recv=\"%d\" /></aiocomport>",
		connect_initiated, accept_initiated, send_initiated, recv_initiated,
		connect_completed, accept_completed, send_completed, recv_completed);
}


//! \brief aio callback function
void 
aiocomport::func_sock_signal(int fd, int si_code)
{
	// locks mutex
	mutexKeeper keeper(_port_mtx); // port mutex

	// finds the correspondent socket desc
	fd_port_map_t::iterator it_fd = _fd_port_map.find(fd);
	if (it_fd == _fd_port_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "func_signal: can not find socket %d", fd);
		return; // socket is gone
	}

	
	switch (si_code)
	{
		case POLL_IN: // read
			{		
				size_t accept_waited = 0, recv_waited = 0;
				for (queue_container_t::iterator it_item = it_fd->_initial_container.begin(); it_item != it_fd->_initial_container.end();)
				{
					switch (it_item->_action)
					{
						case ACTION_ACCEPT:
							{
								it_item->_processed = 0;
								socklen_t len = sizeof(sockaddr_in);

								// does real accept
								it_item->_overlapped->hAccept = ::accept(it_item->aio_fildes, (sockaddr*)&it_item->_overlapped->remoteAddress, &len);
								if (INVALID_SOCKET != it_item->_overlapped->hAccept)
								{
									// success
									it_item->_error = 0;
								}
								else if (
									(it_item->_error = errno) == EWOULDBLOCK
										)
								{
									break; // again would block
								}
	
								// moves to the output queue
								it_fd->_port_iter->_completion_container.push_back(_queue_allocator, *it_item);

								// removes from initial queue
								it_item = it_fd->_initial_container.erase(_queue_allocator, it_item);

								++accept_waited;

								// try one more accept
								continue;
							}
							break;
						case ACTION_RECV:
							{
								socklen_t alen = sizeof(sockaddr_in), len = it_item->aio_nbytes;

								// does real read
								int ret = (it_fd->_type == TYPE_TCP) ?	::recv(it_item->aio_fildes, (char*)it_item->aio_buf, len, 0) :
																::recvfrom(it_item->aio_fildes, (char*)it_item->aio_buf, len, 0, (sockaddr*)&it_item->_overlapped->remoteAddress, &alen);

								if (ret < 0)
								{
									it_item->_processed = 0;
									if (
										(it_item->_error = errno) == EWOULDBLOCK || errno == EAGAIN// || errno == ECONNREFUSED
										)
									{
										it_item->_error = 0;
										break; // would block again
									}
								}
								else
								{
									it_item->_processed = ret;
									it_item->_error = 0;
								}
								
								// moves to the output queue
								it_fd->_port_iter->_completion_container.push_back(_queue_allocator, *it_item);

								// removes from initial queue
								it_item = it_fd->_initial_container.erase(_queue_allocator, it_item);

								++recv_waited;
								// it could be more read blocks waiting
								continue;
							}
							break;
						default:
							 ++it_item; // some write blocks
							continue;
					} // switch

					break;
				} // for

				if (accept_waited + recv_waited)
				{
					format_logging(0, __FILE__, __LINE__, en_log_info, "socket select command detected %d accept and %d recv", accept_waited, recv_waited);
					// wake up queue
					it_fd->_port_iter->_queue_event.set();
				}


			}
			break;
		case POLL_OUT: // write
			{
				size_t connect_waited = 0, send_waited = 0;


				for (queue_container_t::iterator it_item = it_fd->_initial_container.begin(); it_item != it_fd->_initial_container.end();)
				{
					switch (it_item->_action)
					{
						case ACTION_CONNECT:
							{
								// real connect completed
								it_item->_processed = 0;
								it_item->_error = 0;
								
								// moves to the output queue
								it_fd->_port_iter->_completion_container.push_back(_queue_allocator, *it_item);
								// removes from initial queue
								it_item = it_fd->_initial_container.erase(_queue_allocator, it_item);

								++connect_waited;
							}
							break;
						case ACTION_SEND:
							{
								// does real read
								int ret = (it_fd->_type == TYPE_TCP) ?	::send(it_item->aio_fildes, (const char*)it_item->aio_buf, (int)it_item->aio_nbytes, 0) :
																::sendto(it_item->aio_fildes, (const char*)it_item->aio_buf, (int)it_item->aio_nbytes, 0, (const sockaddr*)&it_item->_overlapped->remoteAddress, sizeof(sockaddr_in));

								if (ret < 0)
								{
									it_item->_processed = 0;
									if (
										(it_item->_error = errno) == EWOULDBLOCK || errno == EAGAIN
										)
									{
										it_item->_error = 0;
										break; // would block again
									}
								}
								else
								{
									it_item->_processed = ret;
									it_item->_error = 0;
								}
								
								// moves to the output queue
								it_fd->_port_iter->_completion_container.push_back(_queue_allocator, *it_item);

								// removes from initial queue
								it_item = it_fd->_initial_container.erase(_queue_allocator, it_item);

								++send_waited;
								// it could be more write blocks waiting
								continue;
							}
							break;
						default:
							 ++it_item; // some read blocks
							 continue;
					} // switches

					break;
				} // for

				if (connect_waited + send_waited)
				{
					format_logging(0, __FILE__, __LINE__, en_log_info, "socket select command detected %d connect and %d send", connect_waited, send_waited);
					// wakes up queue
					it_fd->_port_iter->_queue_event.set();
				}
			}	
			break;
		case POLL_ERR:
		case POLL_HUP: // error
			{
				size_t error_waited= 0;
				// extracts error code from socket
				int err;

				socklen_t errlen = sizeof(err);
				getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&err, &errlen);

				// scans all initial action for this socket and reports error
				for (queue_container_t::iterator it_item = it_fd->_initial_container.begin(); it_item != it_fd->_initial_container.end();)
				{
					it_item->_processed = 0;
					it_item->_error = err;
					// moves to the output queue
					it_fd->_port_iter->_completion_container.push_back(_queue_allocator, *it_item);
					// removes from initial queue
					it_item = it_fd->_initial_container.erase(_queue_allocator, it_item);


					++error_waited;
				}

				if (error_waited)
				{
					format_logging(0, __FILE__, __LINE__, en_log_info, "socket select command detected %d errors", error_waited);
					// wakes up queue
					it_fd->_port_iter->_queue_event.set();
				}
			} // if

			break;
		default:
			format_logging(0, __FILE__, __LINE__, en_log_error, "unexpected si_code %d detected", si_code);
			return; // we should not expect be there
	}
}

//! \brief aio callback function
void 
aiocomport::func_file_signal(int fd, void* ptr)
{
	// locks mutex
	mutexKeeper keeper(_port_mtx); // port mutex

	// finds the correspondent socket desc
	fd_port_map_t::iterator it_fd = _fd_port_map.find(fd);
	if (it_fd == _fd_port_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "func_signal: can not find socket %d", fd);
		return; // socket is gone
	}

	// cast info to block
	queue_item* qitem = static_cast< queue_item* >(ptr);
		
	for (queue_container_t::iterator it_item = it_fd->_initial_container.begin(); it_item != it_fd->_initial_container.end(); ++it_item)
	{
		if (qitem != &*it_item)
			continue;

		switch (it_item->_action)
		{
			case ACTION_READ:
			{
				int ret = aio_error (&*it_item);
				if (ret)
				{
					it_item->_processed = 0;
					if ((it_item->_error = ret) == EINPROGRESS)
					{
						it_item->_error = 0;
						break;
					}
				}
				else
				{
					it_item->_processed = aio_return(&*it_item);
					it_item->_error = 0;
				}

				// moves to the output queue
				it_fd->_port_iter->_completion_container.push_back(_queue_allocator, *it_item);

				// removes from initial queue
				it_item = it_fd->_initial_container.erase(_queue_allocator, it_item);

				format_logging(0, __FILE__, __LINE__, en_log_info, "file detected read command");
				it_fd->_port_iter->_queue_event.set();
			}
			break;
			case ACTION_WRITE:
			{
				int ret = aio_error (&*it_item);
				if (ret)
				{
					it_item->_processed = 0;
					if ((it_item->_error = ret) == EINPROGRESS)
					{
						it_item->_error = 0;
						break;
					}
				}
				else
				{
					it_item->_processed = aio_return(&*it_item);
					it_item->_error = 0;
				}
				
				// moves to the output queue
				it_fd->_port_iter->_completion_container.push_back(_queue_allocator, *it_item);

				// removes from initial queue
				it_item = it_fd->_initial_container.erase(_queue_allocator, it_item);

				format_logging(0, __FILE__, __LINE__, en_log_info, "file detected write command");
				it_fd->_port_iter->_queue_event.set();
			}
			break;
			default:
			break;
		}

		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// until function returns true - follow function will be called immediately
// once it returns false, it won't be called until next loan_thread call - see below
// virtual 
bool 
aiocomport::v_has_job(size_t ident, void* data)
{
	// locks mutex
	format_logging(0, __FILE__, __LINE__, en_log_info, "v_has_job: event_thread_id = %d", _event_thread_id);
	return _event_thread_id == 0;
}

#define quant 16
class desc_bag
{
public:	
	desc_bag() :
		_desc(0), _rptr(0), _wptr(0), _mask(0)
	{
	}
	
	HANDLE	_desc;
	aiocb*	_rptr;
	aiocb*	_wptr;
	int		_mask;
};
// real job should be done inside this function, called in separate thread
// what to do is up to client
// virtual 
void 
aiocomport::v_do_job(size_t ident, void* data)
{
	// set thread ident
	_event_thread_id = pthread_self();

	_ev_activate.set();

	format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: before while event_thread_id = %d", _event_thread_id);

#ifndef NO_NPTL
	int signo;
	siginfo_t info;

	// set timeout
	timespec timeouts;
	timeouts.tv_sec=1;
	timeouts.tv_nsec=0;

	// set signal set
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, AIOFILESIGNAL);
	sigaddset(&ss, SIGIO);
	sigaddset(&ss, AIOSOCKSIGNAL);

		
	while (_event_thread_id != 0)
	{
		if ((signo = sigtimedwait(&ss, &info, &timeouts)) > 0)
		{
			if (signo == AIOSOCKSIGNAL)
				func_sock_signal(info.si_fd, info.si_code);
			else if (signo == AIOFILESIGNAL)
				func_file_signal(info.si_fd, info.si_value.sival_ptr);				
			else if (signo == SIGIO)
			{
				// we have missed asynchronous socket event, let check if we have a valid socket handle
				format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: RT queue overflow event_thread_id = %d", _event_thread_id);

				if (info.si_fd) // try invoke func_signal
				{
					func_sock_signal(info.si_fd, info.si_code);
				}
			}	
		}
	}
#else
	fd_set s_read, s_write, s_err;
	// set timeout
	timeval timeoutv;

	TERIMBER::byte_allocator	active_desc_allocator(1024*64);

	while (_event_thread_id != 0)
	{
		active_desc_allocator.clear_extra();

		TERIMBER::_list< desc_bag > active_desc_container;

		FD_ZERO(&s_read);
		FD_ZERO(&s_write);
		FD_ZERO(&s_err);

		// locks mutex
		mutexKeeper keeper(_port_mtx); // port mutex

		int fdcount = _fd_port_map.size();
		SOCKET maxdesc = 0;
		
		// look up the socket descriptors
		for (fd_port_map_t::iterator it_fd = _fd_port_map.begin(); it_fd != _fd_port_map.end(); ++it_fd)
		{	
			desc_bag bag;
			bag._desc = it_fd.key();
	
			for (queue_container_t::iterator it_item = it_fd->_initial_container.begin(); it_item != it_fd->_initial_container.end(); ++it_item)
			{
				int m = (1 << it_item->_action);
				if (bag._mask & m)
					continue;
				
				bag._mask |= m;

				switch (it_item->_action)
				{
					case ACTION_ACCEPT:
					case ACTION_RECV:
						format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: socket %d need to accept or read", bag._desc);
						FD_SET(bag._desc, &s_read);
						break;
					case ACTION_CONNECT:
					case ACTION_SEND:
						format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: socket %d need to connect or send", bag._desc);
						FD_SET(bag._desc, &s_write);
						break;
					case ACTION_READ:
						format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: file %d need to read", bag._desc);
						FD_SET(bag._desc, &s_read);
						bag._rptr = &*it_item;
						break;
					case ACTION_WRITE:
						format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: file %d need to write", bag._desc);
						FD_SET(bag._desc, &s_write);
						bag._wptr = &*it_item;
						break;
				}
			}
			
			if (!it_fd->_initial_container.empty())
			{
				FD_SET(maxdesc = bag._desc, &s_err);
				active_desc_container.push_back(active_desc_allocator, bag);
			}
		}

		keeper.unlock();

		if (maxdesc)
		{
			// do select
			timeoutv.tv_sec=0;
			timeoutv.tv_usec=quant*1000; // in msec

			if (select(maxdesc + 1, &s_read, &s_write, &s_err, &timeoutv) > 0)
			{
				for (TERIMBER::_list< desc_bag >::const_iterator it  = active_desc_container.begin(); it != active_desc_container.end(); ++it)
				{
					// get socket
					HANDLE fd = it->_desc;
					
					// set event code POLL_IN, POLL_OUT, POLL_ERR
					
					if (FD_ISSET(fd, &s_read))
					{
						if (it->_mask & ((1 << ACTION_ACCEPT) | (1 << ACTION_RECV))) // socket
						{
							format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: socket %d signal for recv or accept", fd);
							func_sock_signal(fd, POLL_IN);
						}
						else if (it->_mask & (1 << ACTION_READ)) // file
						{
							format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: file %d signal for read", fd);
							func_file_signal(fd, it->_rptr);
							// no need to check error
							continue;
						}
					}
					
					if (FD_ISSET(fd, &s_write))
					{
						if (it->_mask & ((1 << ACTION_CONNECT) | (1 << ACTION_SEND))) // socket
						{
							format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: socket %d signal for send or connect", fd);
							func_sock_signal(fd, POLL_OUT);
						}
						else if (it->_mask & (1 << ACTION_WRITE)) // file
						{
							format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: file %d signal for write", fd);
							func_file_signal(fd, it->_wptr);
							// no need to check error
							continue;
						}
					}

					if (FD_ISSET(fd, &s_err))
					{
						if (it->_mask & ((1 << ACTION_ACCEPT) | (1 << ACTION_CONNECT) | (1 << ACTION_SEND) | (1 << ACTION_RECV))) // socket
						{
							format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: socket %d signal for error", fd);
							func_sock_signal(fd, POLL_ERR);			
						}
						else if (it->_mask & (1 << ACTION_READ)) // read file
						{
							format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: file %d signal for error on read", fd);
							func_file_signal(fd, it->_rptr);
						}
						else if (it->_mask & (1 << ACTION_WRITE)) // write file
						{
							format_logging(0, __FILE__, __LINE__, en_log_info, "v_do_job: file %d signal for error on write", fd);
							func_file_signal(fd, it->_wptr);
						}						
					}
				}
			}
		}
		else
		{
			// no action to be taken
			_ev_wakeup.wait(1000);
//			usleep(fdcount ? 1 : 1000); // 10 microseconds sleep
		}
	}
#endif
	_ev_deactivate.set();
}


#pragma pack()
END_TERIMBER_NAMESPACE

#endif
