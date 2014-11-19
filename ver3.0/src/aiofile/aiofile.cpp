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
#include "aiofile/aiofile.h"
#include "base/map.hpp"
#include "base/stack.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"


//! constructor
terimber_aiofile_factory::terimber_aiofile_factory()
{
}

//! destructor
terimber_aiofile_factory::~terimber_aiofile_factory()
{
}

//! creates a new aiofile instance
terimber_aiofile*
terimber_aiofile_factory::get_aiofile(terimber_log* log, size_t capacity, size_t deactivate_time_msec)
{
	// creates a new object
	terimber::aiofile* obj = new terimber::aiofile(capacity, deactivate_time_msec);
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


//! \brief initiation thread ident
const size_t aiofile_io_initiation_ident = 1;
//! \brief initiation thread stay on alert time
const size_t aiofile_io_initiation_thread_alert = 1000; // IO initiation and timeouts
//! \brief thread wait for completion Terimber Completion Port ident
const size_t aiofile_completion_io_port_ident = 2;
//! \brief waits until closure
const size_t aiofile_completion_io_port_thread_alert = INFINITE;
//! \brief working thread ident
const size_t aiofile_working_ident = 3;
//! \brief returns back to pool in 1 minute
const size_t aiofile_working_thread_alert = 60000; // working threads
 

////////////////////////////////////
aiofile_block::aiofile_block()
{ 
	clear();
}

aiofile_block::aiofile_block(const aiofile_block& x)
{
	*this = x;
}

aiofile_block& 
aiofile_block::operator=(const aiofile_block& x)
{
	if (this != &x)
	{
		memcpy(this, &x, sizeof(aiofile_block));
	}

	return *this;
}

void 
aiofile_block::clear()
{
	memset(this, 0, sizeof(aiofile_block));
}

void 
aiofile_block::set_timeout(size_t timeout)
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

aiofile::aiofile(size_t capacity, size_t deactivate_time_msec) : 
_file_map(less< size_t >(), 64)
,_reverse_map(less< aio_file_handle >(), 64)
,_file_generator(64)
,_outgoing_list(64)
,_aiofile_io_handle(0)
,_thread_pool(capacity + 3, deactivate_time_msec) // 3 + (xp, completion, working) + additional working threads
,_capacity(capacity)
,_on(false)
,_flag_io_port(false)
{
}

aiofile::~aiofile()
{
	// just in case
	off();
}

bool
aiofile::on()
{
	if (_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiofile already started");
		return false;
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "starting aiofile...");

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
		_aiofile_io_handle = ::CreateIoCompletionPort((HANDLE)INVALID_SOCKET, 0, 0, 0);
#else
		_aiofile_io_handle = TERIMBER::CreateIoCompletionPort((HANDLE)INVALID_SOCKET, 0, 0, TYPE_UNKNOWN);
#endif


	if (!_aiofile_io_handle)
	{
#if OS_TYPE == OS_WIN32
		::CloseHandle((HANDLE)_aiofile_io_handle);
#else
		TERIMBER::CloseHandle(_aiofile_io_handle);
#endif
		format_logging(0, __FILE__, __LINE__, en_log_info, "can not initiate completion port");
		return false;
	}

	_flag_io_port = true;
	_thread_pool.borrow_thread(aiofile_completion_io_port_ident, 0, this, aiofile_completion_io_port_thread_alert);
	_start_io_port.wait();

	// starts working thread
	_thread_pool.borrow_thread(aiofile_working_ident, 0, this, aiofile_working_thread_alert);

	// starts ininitial thread
	_in_thread.start();
	job_task task(this, aiofile_io_initiation_ident, aiofile_io_initiation_thread_alert, 0);
	_in_thread.assign_job(task);

	_on = true;

	format_logging(0, __FILE__, __LINE__, en_log_info, "aio file port is initialized");

	return _aiofile_io_handle != 0;
}


// uninit aio port
void
aiofile::off()
{
	if (!_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiofile already stopped");
		return;
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "stoping aiofile...");
	_in_thread.cancel_job();
	_in_thread.stop();


	format_logging(0, __FILE__, __LINE__, en_log_info, "send stop message to completion port");
#if OS_TYPE == OS_WIN32
	::PostQueuedCompletionStatus((HANDLE)_aiofile_io_handle, 0, 0, 0);
#else
	TERIMBER::PostQueuedCompletionStatus(_aiofile_io_handle, 0, 0, 0);
#endif

	_stop_io_port.wait();
	format_logging(0, __FILE__, __LINE__, en_log_info, "completion port stopped");


#if OS_TYPE == OS_WIN32
	// if I/O is running send quit message
	::CloseHandle((HANDLE)_aiofile_io_handle);
#else
	TERIMBER::CloseHandle(_aiofile_io_handle);
#endif


	_aiofile_io_handle = 0;

	format_logging(0, __FILE__, __LINE__, en_log_info, "Stoping thread pool");
	_thread_pool.revoke_client(this);
	_thread_pool.off();
	_thread_pool.log_on(0);

	format_logging(0, __FILE__, __LINE__, en_log_info, "Close all files");

	mutexKeeper guard(_mtx);
	for (aiofile_file_map_iterator_t iter = _file_map.begin(); iter != _file_map.end(); ++iter)
	{
		iter->_incoming_list.erase(_incoming_list_allocator, iter->_incoming_list.begin(), iter->_incoming_list.end());
		_cancel_file(iter->_handle);
		_close_file(iter->_handle);
	}


	format_logging(0, __FILE__, __LINE__, en_log_info, "cleans up resources");

	// clears
	_file_generator.clear();
	_file_map.clear();
	_reverse_map.clear();
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

	// resets flag
	_on = false;

	format_logging(0, __FILE__, __LINE__, en_log_info, "aio file port is uninitialized");
}

// virtual 
bool 
aiofile::v_has_job(size_t ident, void* data)
{
	// Completion port is closed leave return false
	if (_aiofile_io_handle == 0)
		return false;

	switch (ident)
	{
		case aiofile_completion_io_port_ident: // io port
			return _flag_io_port;
		case aiofile_io_initiation_ident:
			{
				// locks mutex
				mutexKeeper guard(_mtx);

				// first priority, inits asynchronous file actions
				if (!_initial_list.empty())
					return true;

				// finally checks timeouts
				date now;
				sb8_t unow = (sb8_t)now;
				// gets what user wants and does it
				for (aiofile_file_map_iterator_t iter_file = _file_map.begin(); iter_file != _file_map.end(); ++iter_file)
				{
					for (aiofile_pblock_alloc_list_t::iterator iter_block = iter_file->_incoming_list.begin(); 
																	iter_block != iter_file->_incoming_list.end(); ++iter_block)
					{
						// gets pointer to block
						aiofile_block* block = *iter_block;

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
		case aiofile_working_ident:
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
aiofile::wait_for_io_completion()
{
	// notifies main thread about starting this one
	_start_io_port.set();

	while (true) // infinite loop, until GetQueuedCompletionStatus returns zero key it's a signal to leave thread
	{
		aiofile_block* ov = 0;
		size_t file_key = os_minus_one;
		size_t num_bytes = 0;
#if OS_TYPE == OS_WIN32
		bool bRes = (TRUE == ::GetQueuedCompletionStatus((HANDLE)_aiofile_io_handle, 
														(DWORD*)&num_bytes, 
#if defined(_MSC_VER) && (_MSC_VER > 1200) 
														(ULONG_PTR*)
#else
														(DWORD*)
#endif
														&file_key, 
														(LPOVERLAPPED*)&ov, 
														INFINITE));

		int cRes = bRes ? 0 : ::GetLastError(); 
#else
		int cRes = TERIMBER::GetQueuedCompletionStatus(_aiofile_io_handle, 
														&num_bytes, 
														&file_key, 
														(LPOVERLAPPED*)&ov, 
														INFINITE);

#endif
		if (!file_key)
		{
			// resets flag
			_flag_io_port = false;
			// sends signal to main thread that this thread is about to close
			_stop_io_port.set();
			// breaks while loop
			break;
		}

		// processes completed block
		complete_block(file_key, ov, cRes, num_bytes);
	} // while
}

void 
aiofile::complete_block(size_t file_key, aiofile_block* ov, int err, size_t processed)
{
	// locks mutex
	mutexKeeper guard(_mtx);
	// finds the file info by file_key
	aiofile_file_map_iterator_t iter_file = _file_map.find(file_key);

	if (iter_file == _file_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "file key %d not found", file_key);
		return;
	}

	// loop for all incoming blocks for this particular file
	for (aiofile_pblock_alloc_list_t::iterator iter_block = iter_file->_incoming_list.begin(); iter_block != iter_file->_incoming_list.end(); ++iter_block)
	{
		// looking for specific overlapped pointer
		if (*iter_block != ov)
			continue;

		// gets block
		aiofile_block* block = *iter_block;
		// removes from incoming list 
		iter_file->_incoming_list.erase(_incoming_list_allocator, iter_block);
		// assigns error code if any
		block->_err = err;
		// assigns processed bytes
		block->_processed = processed;
		// put block to the output queue
		_outgoing_list.push_back(block);
		// unlocks mutex
		guard.unlock();
		// wakes up working thread any from range
		if (!_capacity || !_thread_pool.borrow_from_range(aiofile_working_ident, aiofile_working_ident + _capacity, 0, this, aiofile_working_thread_alert))
			_thread_pool.borrow_thread(aiofile_working_ident, 0, this, aiofile_working_thread_alert);

		return;
	} // for

	format_logging(0, __FILE__, __LINE__, en_log_info, "completed block not found for file %d, looking in abounded list", file_key); 

#if OS_TYPE == OS_WIN32

	// loop for all abounded blocks - block could be timeouted but still inside Completion Port, 
	// we can not destroy such a block, so we put it to the abounded blocks queue
	for (aiofile_pblock_alloc_list_t::iterator iter_abounded = _abounded_list.begin(); iter_abounded != _abounded_list.end(); ++iter_abounded)
	{
		if (*iter_abounded != ov)
			continue;

		// gets block
		aiofile_block* block = *iter_abounded;
		// removes block from abounded list
		_abounded_list.erase(iter_abounded);
		// destroys block
		_put_block(block);
		return;
	} // for

	format_logging(0, __FILE__, __LINE__, en_log_info, "completed block not found for file %d anywere", file_key); 
#endif

}

// virtual 
void 
aiofile::v_do_job(size_t ident, void* data)
{
	switch (ident)
	{
		case aiofile_completion_io_port_ident: // Terimber Completion Port
			wait_for_io_completion();
			break;
		case aiofile_io_initiation_ident:
			{
				//  locks mutex
				mutexKeeper guard(_mtx);

				if (_initial_list.empty())
					return;

				// gest first initial block
				aiofile_block* block = _initial_list.front();
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
					if (!_capacity || !_thread_pool.borrow_from_range(aiofile_working_ident, aiofile_working_ident + _capacity, 0, this, aiofile_working_thread_alert))
						_thread_pool.borrow_thread(aiofile_working_ident, 0, this, aiofile_working_thread_alert);
				}
			}
			break;
		case aiofile_working_ident:
		default:
			{
				aiofile_block* block = 0;
				aio_file_handle handle = 0;
				aio_file_handle accept_handle = 0;
				terimber_aiofile_callback* client_obj = 0;
	
				// locks mutex
				mutexKeeper guard(_mtx);

				if (_outgoing_list.empty())
					return; // output queue is empty - nothing to do.

				// gets first block in queue
				block = _outgoing_list.front();
				// removes it from queue
				_outgoing_list.pop_front();

				// finds correspondent file
				aiofile_file_map_t::iterator iter_file = _file_map.find(block->_file_ident);
				
				if (iter_file == _file_map.end())
				{
					format_logging(0, __FILE__, __LINE__, en_log_error, "file key %d not found", block->_file_ident);
					// block has been removed
					_put_block(block);
					return;
				}
				else
				{
					// gets client callback pointer
					client_obj = iter_file->_client_obj;
					// gets file handle
					handle = iter_file->_handle;
					// increment callback invoking counter
					++iter_file->_callback_invoking;
				}

				// processing
				if (block->_err)
				{
					// error occured
					// unlocks mutex
					guard.unlock();

					try
					{
						// invokes user callback for error processing
						client_obj->v_on_error(block->_file_ident, block->_err, block->_type, block->_userdata);
					}
					catch (...)
					{
						format_logging(0, __FILE__, __LINE__, en_log_error, "v_on_error exception for file %d", block->_file_ident);
						assert(false);
					}
				}
				else
				{
					// no errors
					// checks block type
					switch (block->_type)
					{
						case AIOFILE_WRITE:
							// unlocks mutex
							guard.unlock();

							try
							{
								// invokes user callback for write - user can change callback
								client_obj->v_on_write(block->_file_ident, (void*)block->_buf, block->_len, block->_processed, block->_userdata);
							}
							catch (...)
							{
								format_logging(0, __FILE__, __LINE__, en_log_error, "v_on_write exception for file %d", block->_file_ident);
								assert(false);
							}
							break;
						case AIOFILE_READ:
							// unlocks mutex
							guard.unlock();

							try
							{
								// invokes user callback for read - user can change callback
								client_obj->v_on_read(block->_file_ident, (void*)block->_buf, block->_len, block->_processed, block->_userdata);
							}
							catch (...)
							{
								format_logging(0, __FILE__, __LINE__, en_log_error, "v_on_read exception for file %d", block->_file_ident);
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
				// finds correspondent file
				iter_file = _file_map.find(block->_file_ident);
				
				// file not found
				if (iter_file == _file_map.end())
				{
					// checks delay map - missing file could be there
					aiofile_delay_key_t::iterator iter_delay = _delay_key_map.find(block->_file_ident);

					if (iter_delay != _delay_key_map.end())
					{
						if (--*iter_delay <= 0) // decrement dealy counter for file
						{
							// if counter is zero - erases it from delay map
							_delay_key_map.erase(iter_delay);
							// saves file ident back to generator
							_file_generator.save(block->_file_ident);
						}
					}
				}
				else
				{
					// decrement callback counter
					--iter_file->_callback_invoking;
				}

				// destroys block
				_put_block(block);
			} // block
			break;
	} // switch
}

// virtual 
size_t 
aiofile::open(const char* file_name, bool read_write, terimber_aiofile_callback* callback)
{
	if (!_aiofile_io_handle)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aio file port is not initialized");
		return 0;
	}


#if OS_TYPE == OS_WIN32
	DWORD desiredAccess = (read_write ? GENERIC_READ : GENERIC_WRITE);
	DWORD sharedMode = (read_write ? FILE_SHARE_READ : FILE_SHARE_WRITE);
	DWORD creationDisposition = (read_write ? OPEN_EXISTING : CREATE_ALWAYS);

	HANDLE handle = ::CreateFile(file_name, desiredAccess, sharedMode, 0, creationDisposition,  FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
	if (handle == INVALID_HANDLE_VALUE)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not open file %s, access mode %s", file_name, read_write ? "read" : "write");
		return 0;
	}
	::SetFilePointer(handle, 0, 0, FILE_BEGIN);
#else
	int oflag = (read_write ? (O_RDONLY|O_NONBLOCK) : (O_WRONLY|O_CREAT|O_TRUNC|O_NONBLOCK));
	int oshare = S_IRWXU | S_IRWXG | S_IRWXO;
	HANDLE handle = ::open(file_name, oflag, oshare);
	if (handle == -1)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not open file %s, access mode %s", file_name, read_write ? "read" : "write");
		return 0;
	}
#endif

	mutexKeeper guard(_mtx);
	return _assign_file(handle, callback);
}

// virtual 
void 
aiofile::close(size_t handle)
{
	if (!_aiofile_io_handle)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aio file port is not initialized");
		return;
	}

	mutexKeeper guard(_mtx);
	aiofile_file_map_iterator_t iter = _file_map.find(handle);

	if (iter == _file_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "file %d not found", handle);
		return;
	}

	// sets options here
	HANDLE hfile = iter->_handle;
	bool delay_key = false;

	// erases all incoming blocks
	iter->_incoming_list.erase(_incoming_list_allocator, iter->_incoming_list.begin(), iter->_incoming_list.end());
	// checks if callbacks are not invoking for this file
	if (iter->_callback_invoking)
	{
		delay_key = true;
		aiofile_delay_key_t::iterator iter_delay = _delay_key_map.find(handle);
		if (iter_delay != _delay_key_map.end())
			*iter_delay += (int)iter->_callback_invoking;
		else
			_delay_key_map.insert(handle, (int)iter->_callback_invoking);
	}

	_reverse_map.erase(iter->_handle);
	_file_map.erase(iter);

	for (aiofile_pblock_list_t::iterator in_iter = _initial_list.begin(); in_iter != _initial_list.end();)
	{
		aiofile_block* block = *in_iter;
		if (block->_file_ident == handle)
		{
			_put_block(block);
			in_iter = _initial_list.erase(in_iter);
		}
		else
			++in_iter;
	}

	for (aiofile_pblock_list_t::iterator out_iter = _outgoing_list.begin(); out_iter != _outgoing_list.end();)
	{
		aiofile_block* block = *out_iter;
		if (block->_file_ident == handle)
		{
			_put_block(block);
			out_iter = _outgoing_list.erase(out_iter);
		}
		else
			++out_iter;
	}

	if (hfile)
	{
		_cancel_file(hfile);
		_close_file(hfile);
	}


#if OS_TYPE == OS_WIN32
	// before erasing, moves all blocks to timeouted - Windows Completion Port will report it anyway
	for (aiofile_pblock_alloc_list_t::iterator iter_list = iter->_incoming_list.begin(); iter_list != iter->_incoming_list.end();)
	{			
		_abounded_list.push_back(*iter_list);
		iter_list = iter->_incoming_list.erase(_incoming_list_allocator, iter_list);
	}
#else
	// before erasing, moves all blocks to timeouted
	for (aiofile_pblock_alloc_list_t::iterator iter_list = iter->_incoming_list.begin(); iter_list != iter->_incoming_list.end();)
	{			
		aiofile_block* block = *iter_list;
		_put_block(block);
		iter_list = iter->_incoming_list.erase(_incoming_list_allocator, iter_list);
	}

#endif

	if (!delay_key)
		// returns handle back to generator
		_file_generator.save(handle);

	format_logging(0, __FILE__, __LINE__, en_log_info, "file handle %u is closed", handle);
}

// sends buffer to specified file asynchronously
// virtual 
int
aiofile::write(size_t handle, size_t offset, const void* buf, size_t len, size_t timeout, void* userdata)
{
	mutexKeeper guard(_mtx);
	// creates block
	aiofile_block* block = _get_block();
	block->set_timeout(timeout);
	
#if OS_TYPE == OS_WIN32
	block->Offset = (DWORD)offset;
	block->OffsetHigh = 0;
#else
	block->offset = offset;
#endif

	block->_type = AIOFILE_WRITE;
	block->_userdata = userdata;

	block->_buf = (char*)buf;
	block->_len = len;

	return _activate_block(handle, block);
}


// receive buffer of bytes from specified file asynchronously
// virtual 
int 
aiofile::read(size_t handle, size_t offset, void* buf, size_t len, size_t timeout, void* userdata)
{
	mutexKeeper guard(_mtx);
	// creates block
	aiofile_block* block = _get_block();
	block->set_timeout(timeout);

#if OS_TYPE == OS_WIN32
	block->Offset = (DWORD)offset;
	block->OffsetHigh = 0;
#else
	block->offset = offset;
#endif

	block->_type = AIOFILE_READ;
	block->_userdata = userdata;

	block->_buf = (char*)buf;
	block->_len = len;

	return _activate_block(handle, block);
}

// makes a snapshot of the internal state
// virtual 
void
aiofile::doxray()
{
	mutexKeeper guard(_mtx);

	size_t files = _file_map.size(), 
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

	format_logging(0, __FILE__, __LINE__, en_log_xray, "<aiofile files=\"%d\" delayed=\"%d\" initiated=\"%d\" completed=\"%d\" abounded=\"%d\" />",
		files, delay_actions, initiated_actions, completed_actions, abounded_actions);

#if OS_TYPE != OS_WIN32
	TERIMBER::DoXRay();
#endif
	_thread_pool.doxray();
}

size_t 
aiofile::_assign_file(aio_file_handle handle, terimber_aiofile_callback* callback)
{
	// creates a new map entry
	aiofile_file new_file(handle, callback);

	// generates a new ident
	size_t ident = _file_generator.generate();

	// inserts into map
	aiofile_file_map_iterator_t iter_file = _file_map.insert(ident, new_file).first;
	if (iter_file == _file_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		_close_file(new_file._handle);
		_file_generator.save(ident);
		return 0;
	}

	// inserts into reverse map
	aiofile_reverse_map_iterator_t iter_reverse = _reverse_map.insert(handle, iter_file).first;
	if (iter_reverse == _reverse_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		_close_file(new_file._handle);
		_file_map.erase(iter_file);
		_file_generator.save(ident);
		return 0;
	}

#if OS_TYPE == OS_WIN32
	// associates TCP file with Windows Completion Port
	if (!::CreateIoCompletionPort((HANDLE)new_file._handle, 
								(HANDLE)_aiofile_io_handle, 
								(DWORD)ident, 
								3))
#else
	// associates file with Terimber completion port, if it's not TCP and not Windows XP
	if (!TERIMBER::CreateIoCompletionPort(new_file._handle, 
								_aiofile_io_handle, 
								ident, 
								TYPE_FILE))

#endif
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not assign file handle to completion port");
		_close_file(new_file._handle);
		_reverse_map.erase(iter_reverse);
		_file_map.erase(iter_file);
		_file_generator.save(ident);
		return 0;
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "assign file handle %u, ident %u is open", new_file._handle, ident);

	return ident;
}

void 
aiofile::_close_file(aio_file_handle handle)
{
	format_logging(0, __FILE__, __LINE__, en_log_info, "file handle %d closed", handle);

#if OS_TYPE == OS_WIN32
	::CloseHandle(handle);
#else
	::close(handle);
#endif
}

void 
aiofile::_cancel_file(aio_file_handle handle)
{
	format_logging(0, __FILE__, __LINE__, en_log_info, "actions for file handle %d aborted", handle);

	// clears all tickets
#if OS_TYPE == OS_WIN32
	::CancelIo(handle);
#else
	TERIMBER::CancelIo(handle, 0);
#endif
}

#if OS_TYPE != OS_WIN32

//! \brief cancels asynchronous operation
void 
aiofile::_cancel_aio(		aio_file_handle handle,				//!< file handle
					OVERLAPPED* overlapped				//!< overlapped block
					)
{
	TERIMBER::CancelIo(handle, overlapped);
}

#endif

int 
aiofile::_process_block(aiofile_block* block)
{
	if (!_aiofile_io_handle)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiofile is not initialized");
		return -1;
	}

	aiofile_file_map_iterator_t iter_file = _file_map.find(block->_file_ident);
	if (iter_file == _file_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "file %d not found", block->_file_ident);
		return -1;
	}

	aio_file_handle handle = iter_file->_handle;
	terimber_aiofile_callback* client = iter_file->_client_obj;

	// puts block into the incoming list
	iter_file->_incoming_list.push_back(_incoming_list_allocator, block);

	//  assume that we took the top block for processing
	// but didn't remove it
	int res = 0;
	switch (block->_type)
	{
		case AIOFILE_WRITE:
			res = _process_write(handle, block);
			break;
		case AIOFILE_READ:
			res = _process_read(handle, block);
			break;
		default:
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
		)
#else	// for Linux
		&& res != EWOULDBLOCK
#endif
	)
	{
		// removes from incoming list
		iter_file->_incoming_list.pop_back(_incoming_list_allocator);

		// populates outgoing queue here
		block->_err = res;

		// inserts block into outgoing list 
		if (!res)
		{
			format_logging(0, __FILE__, __LINE__, en_log_info, "process block synchronously, handle %d", handle); 

			// inserts into output queue
			_outgoing_list.push_back(block);

			// wakes up any working thread from range
			if (!_capacity || !_thread_pool.borrow_from_range(aiofile_working_ident, aiofile_working_ident + _capacity, 0, this, aiofile_working_thread_alert))
				_thread_pool.borrow_thread(aiofile_working_ident, 0, this, aiofile_working_thread_alert);

			return 0;
		}
		else // deletes block
		{
			format_logging(0, __FILE__, __LINE__, en_log_info, "process block type %d failed, handle %d", block->_type, handle); 
			// returns error
			return res;
		}
	}
	else // this is a pending operation
	{
		format_logging(0, __FILE__, __LINE__, en_log_info, "process block type %d succeeded, handle %d", block->_type, handle); 
		block->_err = 0;
		block->_processed = 0;
		return 0;
	}
}

// inserts block into incoming queue for processing in a separate thread
int 
aiofile::_activate_block(size_t ident, aiofile_block* block)
{
	// finds file
	aiofile_file_map_iterator_t iter_file = _file_map.find(ident);
	if (iter_file == _file_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "file %d not found", ident);
		_put_block(block);
		return -1;
	}

	// assigns file handle
	aio_file_handle handle = iter_file->_handle;
	// assigns user callback
	terimber_aiofile_callback* client = iter_file->_client_obj;

	// assigns file ident
	block->_file_ident = ident;

	// puts block into the initial list
	_initial_list.push_back(block);

	// wakes up initial thread
	_in_thread.wakeup();

	return 0;
}


int
aiofile::_process_write(aio_file_handle handle, aiofile_block* block)
{
	// even operation will complete without pending
	// WriteFile will notify IO Complition Port
#if OS_TYPE == OS_WIN32
	return ::WriteFile(handle, (LPVOID)block->_buf, (DWORD)block->_len, (LPDWORD)&block->_processed, block) ?
		ERROR_IO_PENDING : 
		(block->_err = ::GetLastError());
#else
	return TERIMBER::WriteFile(handle, block->_buf, block->_len, block) ?
		EWOULDBLOCK : 
		(block->_err = errno);
#endif
}

int 
aiofile::_process_read(aio_file_handle handle, aiofile_block* block)
{
	// even operation will complete without pending
	// ReadFile will notify IO Completion Port
#if OS_TYPE == OS_WIN32
	return ::ReadFile(handle, (LPVOID)block->_buf, (DWORD)block->_len, (LPDWORD)&block->_processed, block) ?
		ERROR_IO_PENDING : 
		(block->_err = ::GetLastError());
#else
	return TERIMBER::ReadFile(handle, block->_buf, block->_len, block) ?
		EWOULDBLOCK : 
		(block->_err = errno);
#endif
}

void 
aiofile::process_timeouted_blocks()
{
	// gets current date
	date now;
	sb8_t unow = (sb8_t)now;

	// locks mutex
	mutexKeeper guard(_mtx);
	// gets what user wants and does it
	for (aiofile_file_map_iterator_t iter_file = _file_map.begin(); iter_file != _file_map.end(); ++iter_file)
	{
		size_t socket_key = iter_file.key();
		for (aiofile_pblock_alloc_list_t::iterator iter_block = iter_file->_incoming_list.begin(); 
														iter_block != iter_file->_incoming_list.end();)
		{
			// gest pointer to block
			aiofile_block* block = *iter_block;

			if (block->_expired == 0	// infinite timeout
				|| unow > block->_expired) // not expired yet
			{
				++iter_block;
				continue;
			}

			// gets file information
			aio_file_handle handle = iter_file->_handle;
			terimber_aiofile_callback* client_obj = iter_file->_client_obj;

			// erases block from list
			iter_block = iter_file->_incoming_list.erase(_incoming_list_allocator, iter_block);

			// sets callback counter
			++iter_file->_callback_invoking;
			// unlocks mutex
			guard.unlock();

			format_logging(0, __FILE__, __LINE__, en_log_error, "timeouted %s action for file %d", 
						(block->_type == AIOFILE_READ ? "read" : "write"),
				socket_key);

			// calls error callback
			try
			{
				client_obj->v_on_error(block->_file_ident, 
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
			
			// finds correspondent file
			aiofile_file_map_t::iterator iter_find = _file_map.find(block->_file_ident);
			
			if (iter_find == _file_map.end())
			{
				// checks delay map
				aiofile_delay_key_t::iterator iter_delay = _delay_key_map.find(block->_file_ident);

				if (iter_delay != _delay_key_map.end())
				{
					assert(*iter_delay > 0);

					if (--*iter_delay <= 0)
					{
						_delay_key_map.erase(iter_delay);
						_file_generator.save(block->_file_ident);
					}
				}
				// else - file port is about to close - nothing to do
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
	} // for file
}

#pragma pack()
END_TERIMBER_NAMESPACE
