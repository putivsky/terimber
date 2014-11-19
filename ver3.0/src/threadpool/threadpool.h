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

#ifndef _terimber_threadpool_h_
#define _terimber_threadpool_h_

#include "base/map.h"
#include "threadpool/thread.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class client_thread_info
//! \brief client ticket info
class client_thread_info
{
public:
	//! \brief default constructor
	client_thread_info() :
		_ident(0), _data(0), _client(0), _wasted_calls(0), _obj(0)
	{
	}

	//! \brief constructor
	client_thread_info(thread* obj,							//!< thread pointer
					size_t ident,							//!< thread ident
					void* data,								//!< user defined data
					terimber_thread_employer* client		//!< user callback
					) :
		_ident(ident), _data(data), _client(client), _wasted_calls(0), _obj(obj)
	{
	}

	size_t						_ident;						//!< thread ident
	void*						_data;						//!< user defined data
	terimber_thread_employer*	_client;					//!< user callback
	size_t						_wasted_calls;				//!< wasted calls, when v_has_job returned false
	thread*						_obj;						//!< thread object
};

//! \class threadpool
//! \brief defines implementation of thread pool
//! multiple clients can borrow the threads with the same id
class threadpool : public terimber_threadpool, 
					public terimber_thread_employer
{
	//! \typedef ident_info_node_t
	//! \brief maps thread ident to client info
	typedef base_map< size_t, client_thread_info >						ident_info_node_t;
	//! \typedef ident_info_map_all_t
	//! \brief node allocator for ident_info_node_t map
	typedef node_allocator< ident_info_node_t::_node >					ident_info_map_all_t;
	//! \typedef ident_info_map_t
	//! \brief maps thread ident to client info
	typedef _map< size_t, client_thread_info, ident_info_map_all_t >	ident_info_map_t;
	//! \typedef client_info_map_t
	//! \brief map user callback pointer to ident_info_map_t
	typedef map< terimber_thread_employer*, ident_info_map_t >			client_info_map_t;
	//! \typedef queue_info_t
	//! \brief list pf client thread info prepared for disposal
	typedef list< client_thread_info >									queue_info_t;


public:
	//! \brief constructor
	threadpool(		size_t capacity,						//!< max capacity
					size_t deactivate_time_msec				//!< deactivation interval in milliseconds
					);
	//! \brief destructor
	~threadpool();
	//! \brief user can call function even inside v_do_real_job function - see above
	virtual 
	bool 
	borrow_thread(	size_t ident,							//!< thread ident will be used as input parameter for client thread functions
					void* data,								//!< user defined data will be used as input parameter for client thread functions
					terimber_thread_employer* client,		//!< user callback
					size_t stay_on_alert_time_msec			//!< time in milliseconds until unused thread will go back to the pool
					);
	//! \brief user can ask for a little help and wake up any available thread in the provided range of idents
	virtual 
	bool 
	borrow_from_range(size_t from,							//!< from ident
					size_t to,								//!< to ident
					void* data,								//!< user defined data will be used as input parameter for client thread functions
					terimber_thread_employer* client,		//!< user callback
					size_t stay_on_alert_time_msec			//!< time in milliseconds until unused thread will go back to the pool
					);
	//! \brief sometimes we need to stop calling client functions immideately
	//! however the client function calles already in progress should be completed
	virtual 
	void 
	revoke_client(	terimber_thread_employer* client		//!< user callback
					);
	//! \brief does xray
	virtual 
	void 
	doxray();
	//! \brief turns on
	bool on();
	//! \brief turns off
	void off();

protected:
	//! \brief checks job
	virtual 
	bool 
	v_has_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);
	//! \brief does job
	virtual 
	void 
	v_do_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);
private:
	//! \brief validates the client callback pointer
	bool 
	_validate_client(size_t ident,							//!< thread ident
					terimber_thread_employer* client,		//!< user callback pointer
					client_info_map_t::iterator& it_client,	//!< correspondent iterator
					ident_info_map_t::iterator& it_info		//!< found iterator
					);
	//! \brief disposes of all threads for the current user callback
	void
	_clean_up_clients(terimber_thread_employer* client		//!< user callback
					);
private:
	bool					_on;							//!< flag on/off
	size_t					_capacity;						//!< max thread pool capacity
	size_t					_thread_in_use;					//!< thread counter
	thread					_housekeeper;					//!< house keeping thread for disposal 
	thread_pool_t			_thread_pool;					//!< thread pool
	queue_info_t			_dispose_queue;					//!< disposal queue
	client_info_map_t		_clients_map;					//!< client map
	ident_info_map_all_t	_map_allocator;					//!< map allocator
	mutex					_clients_mtx;					//!< mutex
	size_t					_deactivate_time_msec;			//!< deactivation time in milliseconds
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif
