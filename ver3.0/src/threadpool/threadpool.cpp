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

#include "threadpool/threadpool.h"
#include "base/map.hpp"
#include "base/list.hpp"
#include "base/template.hpp"
#include "base/common.hpp"
#include "base/memory.hpp"

static const size_t housekeeper_timeout = 10000; // 10 seconds

terimber_threadpool_factory::terimber_threadpool_factory()
{
}

terimber_threadpool_factory::~terimber_threadpool_factory()
{
}


terimber_threadpool* 
terimber_threadpool_factory::get_thread_pool(terimber_log* log, size_t capacity, size_t deactivate_time_msec)
{
	TERIMBER::threadpool* obj = new TERIMBER::threadpool(capacity, deactivate_time_msec);
	if (obj)
	{
		obj->log_on(log);
		obj->on();
	}

	return obj;
}


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

/////////////////////////////////////////////////
threadpool::threadpool(size_t capacity, size_t deactivate_time_msec) :
	_on(false),
	_capacity(capacity),
	_thread_in_use(0),
	_thread_pool(thread_creator::static_constructor(), capacity), 
	_deactivate_time_msec(deactivate_time_msec)
{
}

threadpool::~threadpool()
{
	off();
}

bool 
threadpool::on()
{
	if (_on)
		return false;

	format_logging(0, __FILE__, __LINE__, en_log_info, "starting thread pool...");

	// starts housekeeping thread
	job_task job(this, 0, housekeeper_timeout, 0);
	_housekeeper.start();
	_on = true;
	_housekeeper.assign_job(job);
	format_logging(0, __FILE__, __LINE__, en_log_info, "thread pool started");
	return true;
}

void 
threadpool::off()
{
	if (!_on)
		return;

	format_logging(0, __FILE__, __LINE__, en_log_info, "stopping thread pool...");

	// locks mutex
	mutexKeeper guard(_clients_mtx);

	_on = false;

	// let housekeeping thread close
	guard.unlock();

	// stops housekeeping thread
	_housekeeper.cancel_job();
	_housekeeper.stop();

	guard.lock();

	// cleans up clients
	_clean_up_clients(0);

	// retuns threads back to pool
	while (!_dispose_queue.empty())
	{
		thread* obj = _dispose_queue.front()._obj;
		_dispose_queue.pop_front();
		guard.unlock();
		_thread_pool.return_object(obj);
		guard.lock();
	}

	guard.unlock();

	// clears pool
	_thread_pool.clear();

	format_logging(0, __FILE__, __LINE__, en_log_info, "thread pool stopped");
}


// virtual 
bool 
threadpool::v_has_job(size_t ident, void* user_data)
{
	// locks mutex
	mutexKeeper guard(_clients_mtx);

	if (!_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "thread pool is not activated");
		return false;
	}

	// is this houskeeping thread?
	if (user_data == 0)
	{
		// checks if we need to do anything
		if (!_dispose_queue.empty())
			return true;

		guard.unlock();
		// ok, we have a second, lets deactivate
		_thread_pool.deactivate(_deactivate_time_msec); 
		return false;
	}

	// delegates the call
	// gets client pointer
	terimber_thread_employer* client = (terimber_thread_employer*)user_data;

	client_info_map_t::iterator it_client;
	ident_info_map_t::iterator it_info;

	// tries to find client + thread ident in the map
	if (!_validate_client(ident, client, it_client, it_info)) // we got invalid pointer, or thread is gone
		return false;

	// sanity check
	assert(it_info->_client == client);
	assert(it_info->_ident == ident);

	// assumes the best scenario
	it_info->_wasted_calls = 0;

	// makes a copy
	client_thread_info rinfo(*it_info);

	// unlocks mutex
	guard.unlock();

	// requests jobs
	if (rinfo._client->v_has_job(rinfo._ident, rinfo._data))
		return true; // gone for work


	// no jobs to do, we are unlucky
	// locks again
	guard.lock();

	if (!_validate_client(ident, client, it_client, it_info)) // we got invalid pointer, or thread is gone
		return false;

	// sanity check
	assert(it_info->_client == client);
	assert(it_info->_ident == ident);
	
	// checks wasted times
	if (it_info->_wasted_calls == 1) // not first time, we already got wrong answer - move to pool
	{
		// desposes thread
		_dispose_queue.push_back(*it_info);
		// removes from client map
		it_client->erase(_map_allocator, it_info);
		// checks if this client does not have more threads
		if (it_client->empty())
			_clients_map.erase(it_client); // erases client
	}
	else
	{
		++it_info->_wasted_calls;
	}

	return false;
}

// virtual 
void 
threadpool::v_do_job(size_t ident, void* user_data)
{
	mutexKeeper guard(_clients_mtx);

	if (!_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "thread pool is not activated");
		return;
	}

	// is this the houskeeping thread?
	if (user_data == 0)
	{
		if (_dispose_queue.empty())
			return;

		// gets from front - the fresh one
		client_thread_info info(_dispose_queue.front());
		// removes it
		_dispose_queue.pop_front();

		// sanity check
		assert(_thread_in_use  > 0);

		// decrements thread counter
		--_thread_in_use;

		// unlocks mutex
		guard.unlock();

		// return thread back to pool
		_thread_pool.return_object(info._obj);
		format_logging(0, __FILE__, __LINE__, en_log_info, "return thread %d for client %d back to pool", ident, info._client);
	}
	else // delegates the call
	{
		// gets client pointer
		terimber_thread_employer* client = (terimber_thread_employer*)user_data;

		client_info_map_t::iterator it_client;
		ident_info_map_t::iterator it_info;

		if (!_validate_client(ident, client, it_client, it_info)) // we got invalid pointer, or thread is gone
			return;

		// sanity check
		assert(it_info->_client == client);
		assert(it_info->_ident == ident);

		// makes a copy
		client_thread_info rinfo(*it_info);

		// unlocks mutex
		guard.unlock();

		// executes the client job in this thread
		rinfo._client->v_do_job(rinfo._ident, rinfo._data);

		format_logging(0, __FILE__, __LINE__, en_log_paranoid, "execute v_do_job thread %d for client %d", ident, rinfo._client);
	}
}

bool
threadpool::_validate_client(size_t ident, terimber_thread_employer* obj, client_info_map_t::iterator& it_client,  ident_info_map_t::iterator& it_info)
{
	// looking for client
	return ((it_client = _clients_map.find(obj)) == _clients_map.end()) ? false : ((it_info = it_client->find(ident)) != it_client->end());
}

// virtual 
bool 
threadpool::borrow_thread(size_t ident, void* data, terimber_thread_employer* client, size_t stay_on_alert_time_msec)
{
	// this is one stop shop for thread request
	if (!client) // checks pointer
	{
		assert(false);
		format_logging(0, __FILE__, __LINE__, en_log_error, "null pointer for user callback");
		return false;
	}

	// locks mutex
	mutexKeeper guard(_clients_mtx);

	if (!_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "thread pool is not activated");
		return false;
	}

	client_info_map_t::iterator it_client;
	ident_info_map_t::iterator it_info;

	// try to find client in a map
	if (_validate_client(ident, client, it_client, it_info))
	{
		// we were lucky - the client is still there
		assert(it_info->_obj);

		// resets wasted
		it_info->_wasted_calls = 0;

		// makes a copy
		client_thread_info rinfo(*it_info);

		// unlocks
		guard.unlock();

		// wakes up thread
		rinfo._obj->wakeup();

		format_logging(0, __FILE__, __LINE__, en_log_paranoid, "wakeup thread %d for client %d", ident, client);
	}
	else // not found- initiate new thread
	{
		// we have reached the max capacity
		if (_thread_in_use == _capacity)
		{
			format_logging(0, __FILE__, __LINE__, en_log_error, "no threads are available, max capacity is reached");
			return false;
		}

		// inserts new client; there wasn't not yet
		ident_info_map_t dummy;
		client_info_map_t::pairib_t it_client = _clients_map.insert(client, dummy);

    if (it_client.first == _clients_map.end())
    {
			format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
			return false;
    }

		// inits client info except thread
		client_thread_info info(0, ident, data, client); 

		// prepares task empoyer = this, ident = ident, timeout = stay_on_alert_time_msec, data = client
		job_task task(this, ident, stay_on_alert_time_msec, client);

		info._obj = _thread_pool.loan_object(task);

		assert(info._obj);

		// inserts info into the map
    if (it_client.first->insert(_map_allocator, ident, info).first == it_client.first->end())
    {
			_clients_map.erase(it_client.first);
			format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
			return false;
		}

		// increments thread counter
		++_thread_in_use;

		// unlocks
		guard.unlock();

		format_logging(0, __FILE__, __LINE__, en_log_paranoid, "open new thread %d for client %d", ident, client);
	}

	return true;
}

// virtual 
bool 
threadpool::borrow_from_range(size_t from, size_t to, void* data, terimber_thread_employer* client, size_t stay_on_alert_time_msec)
{
	// this is a one stop shop for the thread request
	if (!client || from > to) // checks pointer
	{
		assert(false);
		format_logging(0, __FILE__, __LINE__, en_log_error, "null pointer for user callback or invalid range");
		return false;
	}

	// save for logging
	size_t from_ = from, to_ = to;
	// locks mutex
	mutexKeeper guard(_clients_mtx);

	if (!_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "thread pool is not activated");
		return false;
	}

	// tries to find sleeping threads first
	ident_info_map_t dummy;
	client_info_map_t::pairib_t it_client = _clients_map.insert(client, dummy);

  if (it_client.first == _clients_map.end())
  {
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		return false;
  }

	if (!it_client.second) // client is there already
	{
		bool hole_flag = false;
	
		ident_info_map_t::iterator it_info = it_client.first->lower_bound(from);

		// until we have client threads and the thread ident does not exceed upper boundary
		while (it_info != it_client.first->end()
				&& it_info.key() <= to)
		{
			if (from != it_info.key()) // found a hole
				break;

			if (it_info->_obj->get_state() == THREAD_SLEEPING)
			{
				// resets wasted
				it_info->_wasted_calls = 0;

				// makes a copy
				client_thread_info rinfo(*it_info);

				// unlocks
				guard.unlock();

				// wakes up thread
				rinfo._obj->wakeup();

				format_logging(0, __FILE__, __LINE__, en_log_paranoid, "wakeup thread %d for client %d", from, rinfo._client);
			
				return true;
			}
			else
			{
				// checks the next ident
				++from;
				++it_info;
			}
		}

		// we are here because we did not find the available thread, checks if we are still in the range
		if (from > to || _thread_in_use == _capacity) // nothing is available
		{
			format_logging(0, __FILE__, __LINE__, en_log_info, "no threads are available within the range [%d, %d] for client %d", from_, to_, client);
			// wake up the first thread in range
			return false;
		}
	}
	else // new client is required
	{
		// we have reach the max capacity
		if (_thread_in_use == _capacity)
		{
			_clients_map.erase(it_client.first);
			format_logging(0, __FILE__, __LINE__, en_log_error, "no threads are available, max capacity is reached");
			return false;
		}
	}

	// inits client info, except thread
	client_thread_info info(0, from, data, client); 

	// prepares task empoyer = this, ident = ident, timeout = stay_on_alert_time_msec, data = client
	job_task task(this, from, stay_on_alert_time_msec, client);

	info._obj = _thread_pool.loan_object(task);

	assert(info._obj);

	// inserts info into the map
  if (it_client.first->insert(_map_allocator, from, info).first == it_client.first->end())
  {
		_clients_map.erase(it_client.first);
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		return false;
	}

	// increments thread counter
	++_thread_in_use;

	// unlocks
	guard.unlock();

	format_logging(0, __FILE__, __LINE__, en_log_paranoid, "open new thread %d for client %d", from, client);

	return true;
}

// virtual 
void 
threadpool::revoke_client(terimber_thread_employer* client)
{
	if (client)
	{
		// locks mutex 
		mutexKeeper guard(_clients_mtx);
		_clean_up_clients(client);
		format_logging(0, __FILE__, __LINE__, en_log_info, "revoke client %d", client);
	}
}

void 
threadpool::_clean_up_clients(terimber_thread_employer* client)
{
// removes from map
	if (client)
	{
		client_info_map_t::iterator it_client = _clients_map.find(client);
		
		if (it_client != _clients_map.end())
		{
			while (!it_client->empty())
			{
				ident_info_map_t::iterator it_info = it_client->begin();
				_dispose_queue.push_back(*it_info);
				it_client->erase(_map_allocator, it_info);
			}

			_clients_map.erase(it_client);
		}

		_housekeeper.wakeup();
	}
	else
	{
		while (!_clients_map.empty())
		{
			client_info_map_t::iterator it_client = _clients_map.begin();

			while (!it_client->empty())
			{
				ident_info_map_t::iterator it_info = it_client->begin();
				_dispose_queue.push_back(*it_info);
				it_client->erase(_map_allocator, it_info);
			}

			_clients_map.erase(it_client);
		}
	}
}

// virtual 
void 
threadpool::doxray()
{
	// locks mutex 
	mutexKeeper guard(_clients_mtx);

	size_t threads = _thread_in_use,
		capacity = _capacity,
		clients = _clients_map.size(),   
		disposal = _dispose_queue.size();

	guard.unlock();

	format_logging(0, __FILE__, __LINE__, en_log_xray, "<threadpool threads=\"%d\" capacity=\"%d\" clients=\"%d\" disposal=\"%d\" />",
		threads, capacity, clients, disposal);
}

#pragma pack()
END_TERIMBER_NAMESPACE

