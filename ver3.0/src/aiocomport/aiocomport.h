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

#ifndef _terimber_aiocomport_h_
#define _terimber_aiocomport_h_

#include "base/map.h"
#include "base/list.h"
#include "base/stack.h"
#include "base/primitives.h"
#include "threadpool/threadpool.h"

#if OS_TYPE != OS_WIN32

//! define HANDLE type for non Windows OS
#ifndef HANDLE
typedef int HANDLE;
#endif

#ifndef SOCKET
typedef int SOCKET;
#endif

//! define INVALID_SOCKET constant, if any
#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET)~0)
#endif


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)


//! \struct _OVERLAPPED
//! \brief extension to support Windows overlapped 
//! and for socket 1.0 keeps accepted socket handle
typedef struct _OVERLAPPED 
{  
	SOCKET			hAccept;		//!< handle of accepted socket connection
	sockaddr_in		remoteAddress;	//!< remote address
	unsigned long	offset;			//!< offset
} OVERLAPPED;

//! \typedef LPOVERLAPPED
//! \brief pointer to LPOVERLAPPED
typedef OVERLAPPED* LPOVERLAPPED;

//! \enum en_type
//! \brief list of asynchronous actions
enum en_type
{
	TYPE_UNKNOWN = 0,									//!< unknown type
	TYPE_TCP,											//!< tcp socket type
	TYPE_UDP,											//!< udp socket type
	TYPE_FILE											//!< file type
};

/////////////////////////////////////////////////////////////////////
//! C style functions prototype

//! \brief creates completion port or associates new socket handle with already created completion port handle
//! similar to Windows CreateIoCompletionPort but supports TCP and UDP sockets
HANDLE 
CreateIoCompletionPort
(
	HANDLE sock_fd,											//!< socket handle, can be INVALID_SOCKET 
	HANDLE port_fd,											//!< completion port handle, must be 0 for new port creation
	size_t completion_key,									//!< completion key associated with socket handle, if any
	en_type type										//!< type of associated socket TCP (1) or UDP (2) or FILE (3)
);

//! \brief initiate asynchronous accept
bool 
AcceptEx
(
	HANDLE sock_listen,										//!< socket for listener 
	LPOVERLAPPED overlapped								//!< pointer to overlapped structure
);

//! \brief initiate asynchronous connect
bool
ConnectEx
(
	HANDLE sock_fd,											//!< valid socket handle
	const struct sockaddr_in* name,							//!< address/port to connect to
	LPOVERLAPPED overlapped								//!< pointer to overlapped structure
);

//! \brief initiate asynchronous recv
//! name is optional for TCP, but required for UDP sockets
bool 
WSARecv
(
	HANDLE sock_fd,											//!< valid socket handle
	void* buf,												//!< buffer to read bytes to
	size_t len,												//!< length of buffer
	LPOVERLAPPED overlapped								//!< pointer to overlapped structure
);

//! \brief initiate asynchronous recv
//! name is optional for TCP, but required for UDP sockets
bool 
WSARecvFrom
(
	HANDLE sock_fd,											//!< valid socket handle
	void* buf,												//!< buffer to read bytes to
	size_t len,												//!< length of buffer
	struct sockaddr_in* name,								//!< [out] place where to put peer address, optional == NULL for TCP sockets
	LPOVERLAPPED overlapped								//!< pointer to overlapped structure
);

//! \brief initiate asynchronous recv
//! name is optional for TCP, but required for UDP sockets
bool 
ReadFile
(
	HANDLE sock_fd,											//!< valid socket handle
	void* buf,												//!< buffer to read bytes to
	size_t len,												//!< length of buffer
	LPOVERLAPPED overlapped								//!< pointer to overlapped structure
);

//! \brief initiate asynchronous send
//! name is optional for TCP, but required for UDP sockets
bool 
WSASend
(
	HANDLE sock_fd,											//!< valid socket handle
	const void* buf,										//!< buffer to send bytes from
	size_t len,												//!< length of buffer
	LPOVERLAPPED overlapped									//!< pointer to overlapped structure
);

//! \brief initiate asynchronous send
//! name is optional for TCP, but required for UDP sockets
bool 
WSASendTo
(
	HANDLE sock_fd,											//!< valid socket handle
	const void* buf,										//!< buffer to send bytes from
	size_t len,												//!< length of buffer
	const struct sockaddr_in* name,							//!< [in] place where to get peer address, optional == NULL for TCP sockets
	LPOVERLAPPED overlapped									//!< pointer to overlapped structure
);

//! \brief initiate asynchronous send
//! name is optional for TCP, but required for UDP sockets
bool 
WriteFile
(
	HANDLE sock_fd,											//!< valid socket handle
	const void* buf,										//!< buffer to send bytes from
	size_t len,												//!< length of buffer
	LPOVERLAPPED overlapped									//!< pointer to overlapped structure
);

//! \brief waits for completion for specific completion port handle within a specified timeout
//! \return 0 on success, and an error code otherwise
int 
GetQueuedCompletionStatus
(
	HANDLE port_fd,											//!< valid completion port handle
	size_t* number_bytes,									//!< [out] number bytes processed 
	size_t* completion_key,									//!< [out] completion key
	LPOVERLAPPED* overlapped,								//!< [out] pointer to overlapped structure
	size_t timeout_milliseconds								//!< tiemout in milliseconds
);

//! \brief explicitly inserts completion event into specific completion port queue
//! this is a way sending a signal to the caller of GetQueuedCompletionStatus
bool 
PostQueuedCompletionStatus
(
	HANDLE port_fd,											//!< valid completion port handle
	size_t bytes_transferred,								//!< bytes processed
	size_t completion_key,									//!< completion key
	LPOVERLAPPED overlapped									//!< pointer to overlapped structure
);

//! \brief closes specific completion port
//! all associated asynchronous actions for associated sockets will be closed
bool
CloseHandle
(
	HANDLE port_fd											//!< valid completion port handle
);

//! \brief closes specific completion port
//! all associated asynchronous actions for associated sockets will be closed
bool
CancelIo
(
	HANDLE sock_fd,											//!< valid socket handle
	LPOVERLAPPED overlapped								//!< overlapped pointer
);

//! \brief set external log, if pointer is null logging is off
void
SetLog
(
	terimber_log* log										//!< pointer to external log object, can be NULL if logging is off
);

//! \brief logging the internal state information
void
DoXRay
(
);

/////////////////////////////////////////////////////////////////////

//! \class aiocomport
//! \brief implements completion port functionality on Linux/Unix systems
//! we we have only one static instance of this class

class aiocomport :	public terimber_thread_employer,
					public terimber_log_helper
{
public:

	//! \enum en_action
	//! \brief list of asynchronous actions
	enum en_action
	{
		ACTION_UNKNOWN = 0,									//!< unknown action
		ACTION_USERDATA,									//!< userdata action
		ACTION_CONNECT,										//!< connect action
		ACTION_ACCEPT,										//!< accept action
		ACTION_RECV,										//!< receive action
		ACTION_SEND,										//!< send action
		ACTION_READ,										//!< read action
		ACTION_WRITE										//!< write action
	};
private:
	//! \class queue_item
	//! \brief completion queue item
	class queue_item : public aiocb
	{
	public:
		//! \brief constructor
		queue_item()
		{
			memset(this, 0, sizeof(queue_item));
			aio_fildes = (int)INVALID_SOCKET;
		}  

		en_action		_action;							//!< what to do
		size_t			_error;								//!< error occured, set errno
		size_t			_processed;							//!< byte processed
		size_t			_key;								//!< completion key
		LPOVERLAPPED	_overlapped;						//!< pointer to overlapped structure
	};

	//! \typedef queue_allocator_t
	//! \brief node allocator for queue items
	typedef node_allocator< _list< queue_item >::_node >	queue_allocator_t;
	//! \typedef queue_container_t
	//! \brief list of queue items
	typedef _list< queue_item, queue_allocator_t >			queue_container_t;
	//! \typedef queue_container_iterator_t
	//! \brief iterator for list of queue items
	typedef queue_container_t::iterator						queue_container_iterator_t;
	
	//! \class port_attributes
	//! \brief completion port instance attributes
	class port_attributes
	{
		//! \brief hidden copy operator
		port_attributes& operator=(const port_attributes& x);
	public:
		//! \brief default constuctor
		port_attributes() : _go_to_exit(false), _loop_lock(false)
		{
		}

		//! \brief copy constructor 
		port_attributes(const port_attributes& x) : _go_to_exit(x._go_to_exit), _loop_lock(x._loop_lock)
		{
		}

		bool							_go_to_exit;		//!< flag for exit
		size_t							_loop_lock;			//!< lock counter
		event							_queue_event;		//!< calling thread can wait on this event for completion actions
		event							_exit_event;		//!< calling thread can wait on this event on before exit
		queue_container_t				_completion_container;	//!< queue of the completed actions
	};

	//! \typedef port_attr_map_t
	//! \brief maps completion port handle to it's attributes
	typedef map< HANDLE, port_attributes >			port_attr_map_t;
	//! \typedef port_attr_map_iterator_t
	//! \brief iterator for port_attr_map_t
	typedef port_attr_map_t::iterator				port_attr_map_iterator_t;

	//! \class fd_item
	//! \brief association of socket and completion port
	class fd_item
	{
	public:
		//! \brief constructor
		fd_item(port_attr_map_iterator_t port_iter, size_t completion_key, unsigned long int type) : 
			_port_iter(port_iter),
			_completion_key(completion_key), 
			_type((en_type)type)
		{
		}

		queue_container_t				_initial_container;	//!< initiated asynchronous actions
		port_attr_map_iterator_t		_port_iter;			//!< iterator to the associated completion port
		size_t							_completion_key;	//!< completion key
		en_type							_type;				//!< type of item TCP or UDP socket or FILE
	};

	//! \typedef fd_port_map_t
	//! maps socket to port associated completion port attributes
	typedef map< HANDLE, fd_item >					fd_port_map_t;


public:
	//! constructor
	aiocomport();
	//! destructor
	~aiocomport();

	//! \brief create new completion port 
	//! or associated socket handle with already existed completion port
	HANDLE 
	CreateIoCompletionPort
	(
		HANDLE sock_fd,										//!< socket handle, can be INVALID_SOCKET 
		HANDLE port_fd,										//!< completion port handle, must 0 for new port creation
		size_t completion_key,								//!< completion key associated with socket handle if any
		en_type type										//!< type of associated socket TCP (1) or UDP (2) or FILE (3)
	);

	//! \brief initiate asynchronous accept
	bool 
	AcceptEx
	(
		HANDLE sock_listen,									//!< socket for listener 
		LPOVERLAPPED overlapped								//!< pointer to overlapped structure
	);

	//! \brief initiate asynchronous connect
	bool
	ConnectEx
	(
		HANDLE sock_fd,										//!< valid socket handle
		const struct sockaddr_in* name,						//!< address/port to connect ot
		LPOVERLAPPED overlapped								//!< pointer to overlapped structure
	);

	//! \brief initiate asynchronous recv
	//! name is optional for TCP, but required for UDP sockets
	bool 
	WSARecv
	(
		HANDLE sock_fd,										//!< valid socket handle
		void* buf,											//!< buffer to read bytes to
		size_t len,											//!< length of buffer
		LPOVERLAPPED overlapped								//!< pointer to overlapped structure
	);

	//! \brief initiate asynchronous recv
	//! name is optional for TCP, but required for UDP sockets
	bool 
	WSARecvFrom
	(
		HANDLE sock_fd,										//!< valid socket handle
		void* buf,											//!< buffer to read bytes to
		size_t len,											//!< length of buffer
		struct sockaddr_in* name,							//!< [out] place where to put peer address, optional == NULL for TCP sockets
		LPOVERLAPPED overlapped								//!< pointer to overlapped structure
	);


	//! \brief initiate asynchronous recv
	bool 
	ReadFile
	(
		HANDLE sock_fd,										//!< valid socket handle
		void* buf,											//!< buffer to read bytes to
		size_t len,											//!< length of buffer
		LPOVERLAPPED overlapped								//!< pointer to overlapped structure
	);

	//! \brief initiate asynchronous send
	//! name is optional for TCP, but required for UDP sockets
	bool 
	WSASend
	(
		HANDLE sock_fd,										//!< valid socket handle
		const void* buf,									//!< buffer to send bytes from
		size_t len,											//!< length of buffer
		LPOVERLAPPED overlapped								//!< pointer to overlapped structure
	);

	//! \brief initiate asynchronous send
	//! name is optional for TCP, but required for UDP sockets
	bool 
	WSASendTo
	(
		HANDLE sock_fd,										//!< valid socket handle
		const void* buf,									//!< buffer to send bytes from
		size_t len,											//!< length of buffer
		const struct sockaddr_in* name,						//!< [in] place where to get peer address, optional == NULL for TCP sockets
		LPOVERLAPPED overlapped								//!< pointer to overlapped structure
	);

	//! \brief initiate asynchronous send
	bool 
	WriteFile
	(
		HANDLE sock_fd,										//!< valid socket handle
		const void* buf,									//!< buffer to rend bytes from
		size_t len,											//!< length of buffer
		LPOVERLAPPED overlapped								//!< pointer to overlapped structure
	);

	//! \brief waits for completion for specific completion port handle within specified timeout
	//! \return 0 on success, and error code otherwise
	int 
	GetQueuedCompletionStatus
	(
		HANDLE port_fd,										//!< valid completion port handle
		size_t* number_bytes,								//!< [out] number bytes processed 
		size_t* completion_key,								//!< [out] completion key
		LPOVERLAPPED* overlapped,							//!< [out] pointer to overlapped structure
		size_t timeout_milliseconds							//!< tiemout in milliseconds
	);

	//! \brief explicitly inserts completion event into specific completion port queue
	//! this is a way sending a signal to the caller of GetQueuedCompletionStatus
	bool 
	PostQueuedCompletionStatus
	(
		HANDLE port_fd,										//!< valid completion port handle
		size_t bytes_transferred,							//!< bytes processed
		size_t completion_key,								//!< completion key
		LPOVERLAPPED overlapped								//!< pointer to overlapped structure
	);

	//! \brief closes specific completion port
	//! all associated asynchronous action for associated sockets will be abounded
	bool
	CloseHandle
	(
		HANDLE port_fd										//!< valid completion port handle
	);

	//! \brief closes specific completion port
	//! all associated asynchronous action for associated sockets will be abounded
	bool
	CancelIo
	(
		HANDLE sock_fd,										//!< valid socket handle
		LPOVERLAPPED overlapped								//!< overlapped pointer
	);

	//! \brief turns logging on or off if poiner is null
	void
	SetLog
	(
		terimber_log* log									//!< pointer to external log object, can be NULL if logging is off
	);

	//! \brief logging the internal state information
	void
	DoXRay
	(
	);

	// -- MAC OS does not have file descriptor in siginfo_t structure - send it explicitly
	//! \brief aio socket callback function
	void 
	func_sock_signal(int fd, int si_code);

	//! \brief aio callback function
	void 
	func_file_signal(int fd, void* ptr);

protected:
	//! \brief until function returns true - follow function will be called immediately
	//! once it returns false, it won't be called until next loan_thread call - see below
	virtual 
	bool 
	v_has_job(	size_t ident,								//!< thread ident
				void* data									//!< user defined data
				);
	//! \brief real job should be done inside this function called in a separate thread
	//! it is up to the client what to do
	virtual 
	void 
	v_do_job(	size_t ident,								//!< thread ident
				void* data									//!< user defined data
				);

private:
	//! \brief initiated asynchronous action
	bool 
	initiate_action(	en_action action,					//!< action
						HANDLE sock_id,						//!< valid socket handle
						LPOVERLAPPED overlapped,			//!< pointer to overlapped
						const void* buf,					//!< pointer to buffer, can be null for accept or connect
						size_t len,							//!< length of buffer, can be null for accept or connect
						const sockaddr_in* name				//!< pointer to socket address can be null for TCP or accept
						);
private:
	mutex						_port_mtx;					//!< port mutex
	port_attr_map_t				_port_attr_map;				//!< port attributes map
	fd_port_map_t				_fd_port_map;				//!< socket to iterator map

	unique_key_generator		_generator;					//!< generates port id
	size_t						_capacity;					//!< thread pool capacity

	queue_allocator_t			_queue_allocator;			//!< queue allocator

	thread						_event_thread;				//!< thread for asynchronous events
	volatile pthread_t			_event_thread_id;			//!< event thread id
	event						_ev_activate;				//!< activate event thread
	event						_ev_deactivate;				//!< deactivate event thread
#ifdef NO_NPTL
	event						_ev_wakeup;					//!< wake up event
#endif	
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif

#endif // _terimber_completionport_h_

