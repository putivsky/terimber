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

#ifndef _terimber_aiofile_h_
#define _terimber_aiofile_h_

#include "base/primitives.h"
#include "base/stack.h"
#include "base/list.h"
#include "base/map.h"
#include "threadpool/threadpool.h"
#include "threadpool/thread.h"
#include "aiofile/aiofilefactory.h"
#include "aiocomport/aiocomport.h"

BEGIN_TERIMBER_NAMESPACE

#pragma pack(4)

//! \class aiofile_block
//! \brief defines the asynchronous control block for WIN platform
class aiofile_block : public OVERLAPPED
{
public:
	//! \brief default constructor
	aiofile_block();
	//! \brief copy constructor
	aiofile_block(const aiofile_block& x);
	//! \brief assign operator
	aiofile_block& operator=(const aiofile_block& x);

	//! \brief clears resources
	void 
	clear();
	//! \brief sets timeout and expiration date
	void
	set_timeout(	size_t timeout								//!< timeout in milliseconds
				);

public:
	aiofile_type				_type;						//!< action type
	char*						_buf;						//!< buffer send/receive
	size_t						_len;						//!< buffer length
	size_t						_file_ident;				//!< file ident
	size_t						_processed;					//!< processed data size
	int							_err;						//!< error result
	void*						_userdata;					//!< user defined data
#if OS_TYPE == OS_WIN32
	size_t						
#else
	timeval
#endif
					_timeout;								//!< timeout in milliseconds
	sb8_t			_expired;								//!< expiration date
};

//! \typedef aio_file_handle
//! \brief defines handle type
typedef HANDLE aio_file_handle;

//! \class aiofile
//! \brief  expands windows IO Completion Port idea to Linux
class aiofile : public terimber_thread_employer, 
				public terimber_aiofile
{
	//! \typedef aiofile_pblock_allocator_t
	//! \brief node allocator for list of blocks
	typedef node_allocator< base_list< aiofile_block* >::_node >		aiofile_pblock_allocator_t;
	//! \typedef aiofile_pblock_alloc_list_t
	//! \brief list of block pointers with external allocator
	typedef _list< aiofile_block*, aiofile_pblock_allocator_t >			aiofile_pblock_alloc_list_t;
	//! \typedef aiofile_pblock_list_t
	//! \brief list of block pointers with internal allocator
	typedef list< aiofile_block* >										aiofile_pblock_list_t;
	//! \typedef aiofile_block_allocator_t
	//! \brief node allocator - block factory
	typedef node_allocator< aiofile_block >								aiofile_block_allocator_t;

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

	//! \typedef aiofile_listener_map_t
	//! \brief maps listener ident to listener information
	typedef map< size_t, listener_info >							aiofile_listener_map_t;

	//! \class aiofile_file 
	//! \brief keeps file information including initiated asynchronous actions blocks
	class aiofile_file
	{
	public:
		//! \brief constructor
		aiofile_file(	aio_file_handle handle,				//!< handle
						terimber_aiofile_callback* callback //!< callback

			) : _handle(handle), _client_obj(callback), _callback_invoking(0) 
		{
		}
		//! \brief copies constructor
		aiofile_file(const aiofile_file& x) : _handle(x._handle), _client_obj(x._client_obj), _callback_invoking(x._callback_invoking) {}
		//! \brief destructor
		~aiofile_file()
		{
			assert(_incoming_list.empty());
		}


		
		aio_file_handle				_handle;				//!< file handle
		terimber_aiofile_callback*	_client_obj;			//!< pointer to the object for callback notofication
		aiofile_pblock_alloc_list_t	_incoming_list;			//!< keeps incoming asynchronous requests
		size_t						_callback_invoking;		//!< counter of the callbacks invoking
	};

	//! \typedef aiofile_file_map_t
	//! \brief maps file ident yo file info
	typedef map< size_t, aiofile_file >									aiofile_file_map_t;
	//! \typedef aiofile_file_map_iterator_t
	//! \brief aiofile_socket_map iterator
	typedef map< size_t, aiofile_file >::iterator							aiofile_file_map_iterator_t;
	//! \typedef aiofile_reverse_map_t
	//! \brief reverses map file handle to file map interator
	typedef map< aio_file_handle, aiofile_file_map_iterator_t >			aiofile_reverse_map_t;
	//! \typedef aiofile_reverse_map_iterator_t
	//! \brief aiofile_reverse_map iterator
	typedef map< aio_file_handle, aiofile_file_map_iterator_t >::iterator	aiofile_reverse_map_iterator_t;
	//! \typedef aiofile_delay_key_t
	//! \brief maps file ident to delay counter
	typedef map< size_t, size_t >											aiofile_delay_key_t;

	//! \brief clears all block lists outgoing, abounded, initial 
	void 
	_clear_block_lists()
	{
		if (!_outgoing_list.empty())
		{
			for (aiofile_pblock_list_t::iterator out_iter = _outgoing_list.begin(); out_iter != _outgoing_list.end(); ++out_iter)
			{
				aiofile_block* block = *out_iter;
				_put_block(block);
			}

			_outgoing_list.clear();
		}

#if OS_TYPE == OS_WIN32
		if (!_abounded_list.empty())
		{
			for (aiofile_pblock_list_t::iterator tm_iter = _abounded_list.begin(); tm_iter != _abounded_list.end(); ++tm_iter)
			{
				aiofile_block* block = *tm_iter;
				_put_block(block);
			}

			_abounded_list.clear();
		}
#endif
		if (!_initial_list.empty())
		{
			for (aiofile_pblock_list_t::iterator in_iter = _initial_list.begin(); in_iter != _initial_list.end(); ++in_iter)
			{
				aiofile_block* block = *in_iter;
				_put_block(block);
			}

			_initial_list.clear();
		}
	}

	//! \brief gets new blocks
	inline 
	aiofile_block* 
	_get_block()
	{
		aiofile_block* ptr = _block_allocator.allocate();
		if (ptr)
		{
			new(ptr) aiofile_block();
		}

		return ptr;
	}

	//! \brief returns used block back
	inline 
	void 
	_put_block(aiofile_block* block)
	{
		block->~aiofile_block();
		_block_allocator.deallocate(block);
	}


public:
	//! \brief constructor
	aiofile(size_t capacity,								//!< additional threads for processing asynchronous completion callbacks
			size_t deactivate_time_msec						//!< timeout in milliseconds to despose unused threads
			);
	//! \brief destructor
	~aiofile();

	//! \brief activates
	bool 
	on();
	//! \brief deactivates
	void 
	off();

	//! \brief open file
	virtual 
	size_t 
	open(	const char* file_name,				//!< file name
			bool read_write,					//!< open for read or write only
			terimber_aiofile_callback* callback	//!< user callback class
			);
	//! \brief close file
	virtual 
	void 
	close(	size_t handle						//!< valid file ident
			);
	//! \brief sends buffer to specified file asynchronously
	virtual 
	int 
	write(	size_t handle,						//!< valid file ident
			size_t offset,						//!< file offset
			const void* buf,					//!< buffer 
			size_t len,							//!< length of buffer
			size_t timeout,						//!< timeout in milliseconds
			void* userdata						//!< user defined data
			);
	//! \brief receives buffer of bytes from specified file asynchronously
	virtual 
	int 
	read(	size_t handle,						//!< valid file ident
			size_t offset,						//!< file offset
			void* buf,							//!< buffer 
			size_t len,							//!< length of buffer
			size_t timeout,						//!< timeout in milliseconds
			void* userdata						//!< user defined data
			);	

	//! \brief makes the snapshot of internal state
	virtual 
	void 
	doxray();

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
	resolve_socket_address(const char* address,				//!< file address as string
							unsigned short port,			//!< file port
							sockaddr_in& addr				//!< file address as struct
							);

	//! \brief thread-safe function to find file handle by file ident
	aio_file_handle 
	find_socket_handle(	size_t ident						//!< file ident
						);

	//! \brief processes timeouted blocks
	void 
	process_timeouted_blocks();

	//! \brief completes asynchronous action 
	void 
	complete_block(		size_t sock_key,					//!< file ident
						aiofile_block* ov,					//!< asynchronous block pointer
						int err,							//!< error code, if any
						size_t processed					//!< bytes processed
						);

	//! \brief inserts block into incoming queue for processing in a separate thread
	int 
	_activate_block(	size_t ident,						//!< file ident
						aiofile_block* block				//!< asynchronous block pointer
						);

	//! \brief associates file handle with completion port
	size_t 
	_assign_file(	aio_file_handle handle,					//!< file handle
					terimber_aiofile_callback* callback		//!< callback
					);

	//! \brief cancels asynchronous operations
	void 
	_cancel_file(		aio_file_handle handle				//!< file handle
						);

#if OS_TYPE != OS_WIN32

	//! \brief cancels asynchronous operation
	void 
	_cancel_aio(		aio_file_handle handle,				//!< file handle
						OVERLAPPED* overlapped				//!< overlapped block
						);

#endif

	//! \brief closes file
	void 
	_close_file(		aio_file_handle handle				//!< file handle
						);


	//! \brief starts actual IO processing
	int 
	_process_block(		aiofile_block* block				//!< pointer to block
						);

	//! \brief writes the function for IO blocks processing
	int 
	_process_write(	aio_file_handle handle,					//!< file handle
					aiofile_block* block					//!< block pointer
					);
	//! \brief reads the function for IO blocks processing
	int 
	_process_read(	aio_file_handle handle,					//!< file handle
					aiofile_block* block					//!< block pointer
					);

	//! \brief waits for completion actions in separate thread Terimber Completion Port
	void 
	wait_for_io_completion();

public:
	 
	mutex							_mtx;					//!< multithreaded access to file map
	aiofile_file_map_t				_file_map;				//!< file map
	aiofile_reverse_map_t			_reverse_map;			//!< reverse file map
	aiofile_delay_key_t				_delay_key_map;			//!< map of the keys which can not be returned back to generator right now
	unique_key_generator			_file_generator;		//!< generates file idents
	aiofile_pblock_allocator_t		_incoming_list_allocator;//!< external allocator for list of pointers to blocks
	aiofile_block_allocator_t		_block_allocator;		//!< block allocator
	aiofile_pblock_list_t			_initial_list;			//!< keeps initial processing reuqests
	aiofile_pblock_list_t			_outgoing_list;			//!< keeps processed asynchronous requests

#if OS_TYPE == OS_WIN32
	aiofile_pblock_list_t			_abounded_list;			//!< keeps abounded asynchronous requests for Complition Port only
#endif

private:
	HANDLE							_aiofile_io_handle; 	//!< this Terimber port handle
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
