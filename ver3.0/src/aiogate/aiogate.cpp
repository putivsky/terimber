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
#include "aiogate/aiogate.h"
//! include hpp inline code
#include "base/map.hpp"
#include "base/list.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"

// constructor
terimber_aiogate_factory::terimber_aiogate_factory()
{
}


// caller is responsible for deleting the aiogate object
// and until keep alive all terimber_aiogate_pin_factory objects
terimber_aiogate* 
terimber_aiogate_factory::get_terimber_aiogate(terimber_log* log, size_t addional_working_threads, size_t mem_usage_level)
{
	// creates new object
	TERIMBER::aiogate* obj = new TERIMBER::aiogate(addional_working_threads, mem_usage_level);
	if (obj)
	{
		// sets log pointer
		obj->log_on(log);
		// activates aiogate
		obj->on();
	}

	return obj;
}

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// consrtuctor
aiogate::aiogate(size_t addional_working_threads, size_t mem_usage) :
	_on(false),
	_pin_port(addional_working_threads, 60000), // 1 minutes for idle threads
	_pin_allocator(__max(__min(mem_usage / BUFFER_CHUNK, (size_t)64), (size_t)(1024*1024 / BUFFER_CHUNK)))
{
}

// destructor
aiogate::~aiogate()
{
	// off just in case
	off();
}

// port will call function after error occured
// virtual 
void 
aiogate::v_on_error(size_t handle, int err, aiosock_type mask, void* userdata)
{
	if (mask == AIOSOCK_ACCEPT) // ignores failed accepts
		return;

	// locks mutex
	mutexKeeper keeper(_pin_mtx);

	// try to find pin
	pin_map_t::iterator it_pin = _pin_map.find(handle);
	if (it_pin == _pin_map.end() // already removed from map
		|| !it_pin->_still_alive) // already dead
	{
		format_logging(0, __FILE__, __LINE__, en_log_warning, "pin %d not found", handle);
		return;
	}

	// removes the correspondent mask
	switch ((size_t)userdata)
	{
		case aiogate_accept_mask:
			assert(mask == AIOSOCK_ACCEPT);
			it_pin->_in_progress_mask &= ~aiogate_accept_mask;
			format_logging(0, __FILE__, __LINE__, en_log_info, "pin %d closed on accept error %d", handle, err);
			break;
		case aiogate_connect_mask:
			assert(mask == AIOSOCK_CONNECT);
			it_pin->_in_progress_mask &= ~aiogate_connect_mask;
			format_logging(0, __FILE__, __LINE__, en_log_info, "pin %d closed on connect error %d", handle, err);
			break;
		case aiogate_send_mask:
			assert(mask == AIOSOCK_SEND);
			it_pin->_in_progress_mask &= ~aiogate_send_mask;
			format_logging(0, __FILE__, __LINE__, en_log_info, "pin %d closed on send error %d", handle, err);
			break;
		case aiogate_recv_mask:
			assert(mask == AIOSOCK_RECV);
			it_pin->_in_progress_mask &= ~aiogate_recv_mask;
			format_logging(0, __FILE__, __LINE__, en_log_info, "pin %d closed on receive error %d", handle, err);
			break;
		default:
			break;
	}

	// unlocks mutex
	keeper.unlock();
	// initiates close (callback on_close will be fired)
	initiate_close(handle, (size_t)userdata, true);

}

bool 
aiogate::on()
{
	if (_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiogate is already on");
		return false;
	}

	// sets log pointer
	_pin_port.log_on(this);
	// turns aiosock port on
	_pin_port.on();

	job_task task(this, 0, INFINITE, 0);
	// starts thread
	_pin_thread.start();
	_pin_thread.assign_job(task);

	format_logging(0, __FILE__, __LINE__, en_log_info, "aiogate is initialized");

	// sets flag
	_on = true;
	return true;
}
	
void 
aiogate::off()
{
	if (!_on)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiogate is already off");
		return;
	}

	// stops thread
	_pin_thread.cancel_job();
	_pin_thread.stop();

	// turns aiosock port off
	_pin_port.off();
	// removes log pointer
	_pin_port.log_on(0);

	// locks mutex
	mutexKeeper keeper(_pin_mtx);

	// removes all clients from the pin list
	for (pin_map_t::iterator iter = _pin_map.begin(); iter != _pin_map.end();)
	{
		iter->_factory->destroy(iter->_pin);
		iter = _pin_map.erase(iter);
	}	

	// final close
	while (!_pin_list.empty())
	{
		pin_info_extra& info = _pin_list.front();
		info._factory->destroy(info._pin);
		_pin_list.pop_front();
	}

	// clears pin allocator
	_pin_allocator.clear_all();

	// resets flag
	_on = false;

	format_logging(0, __FILE__, __LINE__, en_log_info, "aiogate is uninitialized");
}

// port will call function after successfully connecting to socket
// virtual 
void 
aiogate::v_on_connect(size_t handle, const sockaddr_in& peeraddr, void* userdata)
{
	// locks mutex
	mutexKeeper keeper(_pin_mtx);
	// tries to find pin
	pin_map_t::iterator it_pin = _pin_map.find(handle);
	if (it_pin == _pin_map.end() // already removed from map
		|| !it_pin->_still_alive) // already dead
	{
		format_logging(0, __FILE__, __LINE__, en_log_warning, "pin %d not found", handle);
		return;
	}

	// gets local & remote ip/port
	sockaddr_in local, remote;
	if (_pin_port.getsockaddr(handle, local)
		|| _pin_port.getpeeraddr(handle, remote)
		)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not get peer/sock address for pin %d", handle);
		// resets mask
		it_pin->_in_progress_mask &= ~aiogate_connect_mask;
		
		// unlocks mutex
		keeper.unlock();
		// initiates close procedure
		initiate_close(handle, aiogate_connect_mask, true);
		return;
	}

	// gets pin, iterator later could be invalid
	terimber_aiogate_pin* pin = it_pin->_pin;
	// invokes callback sandwich
	lock_pin(keeper, *it_pin, aiogate_connect_mask);
	try
	{
		// now we can use pin object safely
		pin->on_connect(local, remote, handle, this);
	}
	catch (...)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "on_connect callback exception, handle %d", handle);
		assert(false);
	}

	unlock_pin(keeper, handle, aiogate_connect_mask, true);
	format_logging(0, __FILE__, __LINE__, en_log_info, "connect initiated for pin %d", handle);
}

// port will call function after successfully sending buffer to socket
// virtual 
void 
aiogate::v_on_send(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& peeraddr, void* userdata)
{
	// locks mutex
	mutexKeeper keeper(_pin_mtx);
	// tries to find pin
	pin_map_t::iterator it_pin = _pin_map.find(handle);
	if (it_pin == _pin_map.end() // already removed from map
		|| !it_pin->_still_alive) // already dead
	{
		format_logging(0, __FILE__, __LINE__, en_log_warning, "pin %d not found", handle);
		return;
	}

	pin_info& r_info = *it_pin;

	// we always send the top chunk
	assert(r_info._shead->_ptr + r_info._shead->_begin == buf);

	// adjusts fixed buffer offset
	// for UDP - just ignore the chunked bytes
	r_info._shead->_begin += (r_info._tcp_udp ? processed : requested);

	// checks if we are done with current chunk
	if (r_info._shead->_begin == r_info._shead->_end)
	{
		// checks the next chunk in a linked list
		fixed_size_buffer* next = r_info._shead->_next;
		// return current chunk back to allocator
		_pin_allocator.deallocate(r_info._shead);

		// resets the top chunk
		if (next)
			r_info._shead = next;
		else
			r_info._shead = r_info._stail = 0;
	}

	//  checks if we need to send more bytes
	if (r_info._shead)
	{
		sockaddr_in toaddr;
		size_t requested = 0;
		// keeps mutex locked and mask is set
		if (!r_info._tcp_udp) // UDP case - get correct address
		{
			udp_header* uheader= (udp_header*)(r_info._shead->_ptr + r_info._shead->_begin); // get UDP header
			requested = uheader->_payload;
			toaddr = uheader->_addr;
			// skip UDP header in the memory page
			r_info._shead->_begin += sizeof(udp_header);
		}
		else
		{
			requested = r_info._shead->_end - r_info._shead->_begin;
		}

		if (_pin_port.send(handle, r_info._shead->_ptr + r_info._shead->_begin, requested, r_info._send_timeout, r_info._tcp_udp ? 0 : &toaddr, (void*)(size_t)aiogate_send_mask))
		{
			format_logging(0, __FILE__, __LINE__, en_log_error, "can not initiate send for pin %d", handle);
			
			r_info._in_progress_mask &= ~aiogate_send_mask;

			keeper.unlock();
			// initiates close
			initiate_close(handle, aiogate_send_mask, true);
			return;
		}

		format_logging(0, __FILE__, __LINE__, en_log_info, "send initiated for pin %d", handle);
	}
	else
	{
		// while callback we want user to allow adding new blocks
		// gets pin
		terimber_aiogate_pin* pin = r_info._pin;

		// invokes callback sandwich
		lock_pin(keeper, r_info, aiogate_send_mask);
		try
		{
			pin->on_send(peeraddr);
		}
		catch (...)
		{
			format_logging(0, __FILE__, __LINE__, en_log_error, "on_send callback exception, handle %d", handle);
			assert(false);
		}

		unlock_pin(keeper, handle, aiogate_send_mask, true);

		// checks if we need to initiate send

		// finds pin
		it_pin = _pin_map.find(handle);
		
		if (it_pin == _pin_map.end() // already removed from map
		|| !it_pin->_still_alive) // already dead
		{
			// send can close socket
			return;
		}

		// if we need to send and we are not in progress
		if (it_pin->_shead && !(it_pin->_in_progress_mask & aiogate_send_mask))
		{
			it_pin->_in_progress_mask |= aiogate_send_mask;

			sockaddr_in toaddr = peeraddr;
			size_t requested = 0;
			// keeps mutex locked and mask is set
			if (!r_info._tcp_udp) // UDP case - get correct address
			{
				udp_header* uheader= (udp_header*)(r_info._shead->_ptr + r_info._shead->_begin); // get UDP header
				requested = uheader->_payload;
				toaddr = uheader->_addr;
				// skip UDP header in the memory page
				r_info._shead->_begin += sizeof(udp_header);
			}
			else
			{
				requested = r_info._shead->_end - r_info._shead->_begin;
			}

			if (_pin_port.send(handle, it_pin->_shead->_ptr + it_pin->_shead->_begin, requested, it_pin->_send_timeout, &toaddr, (void*)(size_t)aiogate_send_mask))
			{
				format_logging(0, __FILE__, __LINE__, en_log_error, "can not initiate send for pin %d", handle);
	
				it_pin->_in_progress_mask &= ~aiogate_send_mask;
				
				keeper.unlock();
				initiate_close(handle, aiogate_send_mask, true);
				return;
			}

			format_logging(0, __FILE__, __LINE__, en_log_info, "send initiated for pin %d", handle);
		}
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "send processed pin %d", handle);
} 

// port will call function after successfully receiving buffer from socket
// virtual 
void 
aiogate::v_on_receive(size_t handle, void* buf, size_t requested, size_t processed, const sockaddr_in& peeraddr, void* userdata)
{
	// locks mutex
	mutexKeeper keeper(_pin_mtx);
	// tries to find pin
	pin_map_t::iterator it_pin = _pin_map.find(handle);
	if (it_pin == _pin_map.end() // already removed from map
		|| !it_pin->_still_alive) // already dead
	{
		format_logging(0, __FILE__, __LINE__, en_log_warning, "pin %d not found", handle);
		return;
	}

	if (!processed		// no bytes arrived
		&& it_pin->_tcp_udp // TCP - connection is broken
		)
	{
		format_logging(0, __FILE__, __LINE__, en_log_info, "receive zero byte - initiate pin closure %d", handle);
		//  initiates close - peer closed socket
		keeper.unlock();
		initiate_close(handle, aiogate_recv_mask, true);
		return;
	}

	pin_info& r_info = *it_pin;

	// we receive always the big recv buffer or small leader buffer
	assert(r_info._rbuf && r_info._rbuf->_ptr == buf || !r_info._rbuf && &r_info._leader == buf);

	// gets pin
	terimber_aiogate_pin* pin = r_info._pin;

	// invokes callback sandwich
	lock_pin(keeper, r_info, aiogate_recv_mask);
	bool expected_more = false;

	bool recv_continue = false;
	
	try
	{
		recv_continue = pin->on_recv((const ub1_t*)buf, processed, peeraddr, expected_more);
	}
	catch (...)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "on_recv callback exception, handle %d", handle);
		assert(false);
	}
	
	bool alive = unlock_pin(keeper, handle, aiogate_recv_mask, true);

	keeper.unlock();

	if (recv_continue && alive)
		recv(handle, expected_more, &peeraddr);

	format_logging(0, __FILE__, __LINE__, en_log_info, "receive processed pin %d", handle);
}

// port will call function after successfully accepting the new incoming connection
// user can change the callback, by default it's an object which created a listener
// virtual 
void 
aiogate::v_on_accept(size_t handle, size_t handle_accepted, terimber_aiosock_callback*& callback, const sockaddr_in& peeraddr, void* userdata)
{
	// locks mutex
	mutexKeeper keeper(_pin_mtx);
	// tries to find listener
	listener_map_t::iterator it_listener = _listener_map.find(handle);
	if (it_listener == _listener_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "listener %d not found", handle);
		// closes handle
		_pin_port.close(handle_accepted);
		return; // no more such listener
	}

	// tries to create a pin
	terimber_aiogate_pin* pin = it_listener->_factory->create(it_listener->_arg);
	if (!pin)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not create pin on accept, listener %d", handle);
		//  can not create a pin
		_pin_port.close(handle_accepted);
		return;
	}


	// inserts into pin map
	pin_info info;
	info._factory = it_listener->_factory;
	info._pin = pin;
	info._tcp_udp = true; // TCP oriented connection

	pin_map_t::pairib_t it_pin;
	try
	{
		it_pin = _pin_map.insert(handle_accepted, info);
	}
	catch (exception&)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		// not enough memory
		// closes socket
		_pin_port.close(handle_accepted);
		// destroys pin
		it_listener->_factory->destroy(pin);
		return;
	}

	// gets local & remote ip/port
	sockaddr_in local, remote;
	if (_pin_port.getsockaddr(handle_accepted, local)
		|| _pin_port.getpeeraddr(handle_accepted, remote)
		)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not get peer/sock address for pin %d", handle);
		// cleanup
		_pin_map.erase(it_pin.first);
		_pin_port.close(handle_accepted);
		it_listener->_factory->destroy(pin);
		return;
	}

	// invokes callback sandwich
	lock_pin(keeper, *it_pin.first, aiogate_accept_mask);
	try
	{
		pin->on_accept(local, remote, handle_accepted, this);
	}
	catch (...)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "on_accept callback exception, handle %d", handle_accepted);
		assert(false);
	}
	unlock_pin(keeper, handle_accepted, aiogate_accept_mask, false);

	format_logging(0, __FILE__, __LINE__, en_log_info, "accept processed pin %d", handle);
}

//////////////////////////////////////////////////////////////////////
void 
aiogate::lock_pin(mutexKeeper& mtx, pin_info& info, ub4_t mask)
{
	assert(mtx); // must be locked
	// it's possible that two threads will try to invoke callbacks, for instance on_send and on_recv
	assert(!(info._callback_invoking_mask & mask));

	info._callback_invoking_mask |= mask;
	mtx.unlock();
}

bool 
aiogate::unlock_pin(mutexKeeper& mtx, size_t handle, ub4_t mask, bool unmask_in_progress)
{
	assert(!mtx);

	mtx.lock();

	// tries to find the same pin
	pin_map_t::iterator it_pin = _pin_map.find(handle);

	if (it_pin == _pin_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "pin %d not found", handle);
		return false;
	}

	assert(it_pin->_callback_invoking_mask & mask);

	// clears mask
	it_pin->_callback_invoking_mask &= ~mask;

	// clears progress if any
	if (unmask_in_progress)
		it_pin->_in_progress_mask &= ~mask;

	if (it_pin->_callback_invoking_mask)
		return true; // there are more callbacks in a process

	// checks if pin is still alive
	if (!it_pin->_still_alive)
	{
		// closes socket
		mtx.unlock();
		initiate_close(handle, mask, true);
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// terimber_aiogate methods

// virtual 
size_t 
aiogate::listen(	const char* address, // server ip or name, can be null - localhost will be used
					unsigned short port, // listening port
					size_t max_connection, // max waited connections
					unsigned short buffered_acceptors, // how many acceptor will be waiting for incoming pins
					terimber_aiogate_pin_factory* factory, // factory, which knows how to create pin
					void* arg)
{
	if (!factory) // factory is null
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "factory pointer is null");
		return 0;
	}

	size_t ident = _pin_port.create(this, true);
	if (!ident)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "aiosock can not create socket");
		return 0;
	}

	// socket created succesfully
	// creates pin info entry
	mutexKeeper keeper(_pin_mtx);

	listener_info info(factory, arg);
	listener_map_t::iterator it_listener = _listener_map.insert(ident, info).first;

	if (it_listener == _listener_map.end())
	{
		//  not enough memory
		// error - closes socket
		_pin_port.close(ident);
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		return 0;
	}

	// initiate listening
	if (_pin_port.listen(ident, port, max_connection, address, buffered_acceptors, (void*)(size_t)aiogate_accept_mask))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not start listener on address %d and port %hu", address, port);
		// well, maybe next time...
		_listener_map.erase(it_listener);
		// error - close socket
		_pin_port.close(ident);
		return 0;
	}

	// ok we started the listening process
	format_logging(0, __FILE__, __LINE__, en_log_info, "listener started on address %d and port %hu", address, port);
	return ident;
}

// stops listener with specified ident 
// virtual 
void 
aiogate::deaf(size_t ident)
{
	// locks mutex
	mutexKeeper keeper(_pin_mtx);

	// tries to find
	listener_map_t::iterator it_listener = _listener_map.find(ident);
	if (it_listener != _listener_map.end())
	{
		// removes from map
		_listener_map.erase(it_listener);
		// closes socket
		_pin_port.close(ident);
	
		format_logging(0, __FILE__, __LINE__, en_log_info, "listener %d stopped", ident);
	}
}

// initiates connection
// returns ident if asynchronous connection process has been initiated
// when asynchronous connection completed (failed or succeeded)
// connect --> factory->create() --> (pin->on_connect() OR pin->on_close())
// virtual 
size_t 
aiogate::connect(	const char* remote, // remote host ip or name
					unsigned short rport, // remote port
					const char* local, // local host ip or name - optional, can be null
					unsigned short lport, // local port - optional can be zero
					size_t timeout, // timeout in milliseconds 
					terimber_aiogate_pin_factory* factory, // factory, which knows how to create pin
					void* arg)
{
	if (!factory) // factory is null
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "factory pointer is null");
		return 0;
	}

	size_t ident = _pin_port.create(this, true);
	if (!ident)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not create socket");
		return 0;
	}


	// tries to create a pin
	terimber_aiogate_pin* pin = factory->create(arg);
	if (!pin)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not create pin for argument %d", (size_t)arg);
		//  can not create a pin
		_pin_port.close(ident);
		return 0;
	}


	// socket created succesfully
	// creates pin info entry
	mutexKeeper keeper(_pin_mtx);

	// inserts into pin map
	pin_info info;
	info._factory = factory;
	info._pin = pin;
	info._tcp_udp = true; // TCP oriented connection

	pin_map_t::iterator it_pin = _pin_map.insert(ident, info).first;
	if (it_pin == _pin_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		// not enough memory
		_pin_port.close(ident);
		factory->destroy(pin);
		return 0;
	}


	// checks if local host is specified
	if (local && lport && _pin_port.bind(ident, local, lport))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not bind specified local host %s and port %hu", local, lport);
		_pin_map.erase(it_pin);
		// error - close socket
		_pin_port.close(ident);
		factory->destroy(pin);
		return 0;
	}
	
	it_pin->_in_progress_mask |= aiogate_connect_mask;

	if (_pin_port.connect(ident, remote, rport, timeout, (void*)(size_t)aiogate_connect_mask))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not initiate connect to host %s and port %hu", remote, rport);

		it_pin->_in_progress_mask &= ~aiogate_connect_mask;

		_pin_map.erase(it_pin);
		// error - closes socket
		_pin_port.close(ident);
		factory->destroy(pin);
		return 0;
	}

	format_logging(0, __FILE__, __LINE__, en_log_info, "connect initiated to host %s and port %hu", remote, rport);
	return ident;
}

// UDP only methods
// binds to the specified socket address
// virtual 
size_t 
aiogate::bind(const char* address, 
			  unsigned short port,
			  terimber_aiogate_pin_factory* factory, // factory, which knows how to create pin
			  void* arg // user defined argument - will be used for terimber_aiogate_pin_factory::create method as an input argument
			  )
{
	if (!factory) // factory is null
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "factory pointer is null");
		return 0;
	}

	size_t ident = _pin_port.create(this, false);
	if (!ident)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not create socket");
		return 0;
	}


	// tries to create a pin
	terimber_aiogate_pin* pin = factory->create(arg);
	if (!pin)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not create pin for argument %d", (size_t)arg);
		//  can not create a pin
		_pin_port.close(ident);
		return 0;
	}


	// socket created succesfully
	// creates pin info entry
	mutexKeeper keeper(_pin_mtx);

	// inserts into pin map
	pin_info info;
	info._factory = factory;
	info._pin = pin;
	info._tcp_udp = false; // UDP oriented connection

	pin_map_t::iterator it_pin = _pin_map.insert(ident, info).first;
	if (it_pin == _pin_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "not enough memory");
		// not enough memory
		_pin_port.close(ident);
		factory->destroy(pin);
		return 0;
	}

	if (_pin_port.bind(ident, address, port))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not bind specified host %s and port %hu", address, port);
		_pin_map.erase(it_pin);
		// error - closes socket
		_pin_port.close(ident);
		factory->destroy(pin);
		return 0;
	}

	// gets local & remote ip/port
	sockaddr_in local;
	if (_pin_port.getsockaddr(ident, local))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not get peer/sock address for pin %d", ident);
		_pin_map.erase(it_pin);
		// error - closes socket
		_pin_port.close(ident);
		factory->destroy(pin);
		return 0;
	}

	// gets pin, iterator later could be invalid
	// invokes callback sandwich
	lock_pin(keeper, *it_pin, aiogate_bind_mask);
	try
	{
		// now we can use the pin object safely
		pin->on_bind(local, ident, this);
	}
	catch (...)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "on_bind callback exception, handle %d", ident);
		assert(false);
	}
	unlock_pin(keeper, ident, aiogate_bind_mask, true);

	format_logging(0, __FILE__, __LINE__, en_log_info, "bind processed pin %d", ident);

	return ident;
}
// UDP
// terimber_aiogate_callback methods

// sends buf bytes asynchronously
// virtual 
bool 
aiogate::send(	size_t ident, // unique identificator
				const void* buf, // buffer to send
				size_t len, // length of buffer
				const sockaddr_in* toaddr
				)
{
	if (!buf || !len)
		return false;

	terimber_aiogate_buffer bulk;
	bulk.buf = buf;
	bulk.len = len;
	return send_bulk(ident, &bulk, 1, toaddr);
}

// virtual 
bool 
aiogate::send_bulk(	size_t ident, // unique identificator
						const terimber_aiogate_buffer* bulk, // buffers to send
						size_t count, // length of bulk
						const sockaddr_in* toaddr
						)
{
	if (!count || !bulk)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "buffer pointer is null or count is zero, ident %d", ident);
		return false;
	}

	mutexKeeper keeper(_pin_mtx);
	pin_map_t::iterator it_pin = _pin_map.find(ident);
	if (it_pin == _pin_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "pin %d not found", ident);
		return false;
	}

	if (!it_pin->_tcp_udp // UDP - toaddr is required
		&& !toaddr)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "pin %d is UDP connection - address is required", ident);
		return false;
	}

	// - storing in paged chunks and sending 
	for (size_t index = 0; index < count; ++index)
	{
		const void* buf = bulk[index].buf;
		size_t len = bulk[index].len;

		if (!buf || !len)
			continue;

		// UDP protection
		if (!it_pin->_tcp_udp && len > BUFFER_CHUNK + sizeof(udp_header))
		{
			format_logging(0, __FILE__, __LINE__, en_log_error, "pin %d is UDP connection - chunk size exceeeds the allowed maximum", ident);
			return false;
		}

		size_t offset = 0;
		// adds to the buffer
		while (len)
		{
			if (!it_pin->_stail)
			{
				fixed_size_buffer* p = _pin_allocator.allocate();

				if (!p)
				{
					format_logging(0, __FILE__, __LINE__, en_log_error, "no enough memory");
					// not enough memory
					return false;
				}
				else
					p = new (p) fixed_size_buffer();
				

				it_pin->_shead = it_pin->_stail = p;
			}

			if (it_pin->_tcp_udp
				&& it_pin->_stail->_end < BUFFER_CHUNK // for TCP even one byte of space is good
				|| !it_pin->_tcp_udp
				&& (BUFFER_CHUNK - it_pin->_stail->_end) >= (len + sizeof(udp_header)) // for UDP - the whole chunk must fit to the page
				)
			{
				// rooms are available
				size_t clen = __min(BUFFER_CHUNK - it_pin->_stail->_end, len);

				// TCP - copy bytes as it is
				if (it_pin->_tcp_udp)
				{
					memcpy(it_pin->_stail->_ptr + it_pin->_stail->_end, (const ub1_t*)buf + offset, clen);
					// adjust buffers
					it_pin->_stail->_end += clen;
				}
				else // UDP
				{
					// add header
					udp_header h;
					h._addr = *toaddr;
					h._payload = (ub4_t)len;
					memcpy(it_pin->_stail->_ptr + it_pin->_stail->_end, &h, sizeof(h));
					it_pin->_stail->_end += sizeof(h);
					memcpy(it_pin->_stail->_ptr + it_pin->_stail->_end, (const ub1_t*)buf + offset, clen);
					it_pin->_stail->_end += clen;
				}

				len -= clen;
				offset += clen;
			}
			else
			{
				// adds new chunk
				fixed_size_buffer* p = _pin_allocator.allocate();

				if (!p)
				{
					format_logging(0, __FILE__, __LINE__, en_log_error, "no enough memory");
					// not enough memory
					return false;
				}
				else
					p = new(p) fixed_size_buffer();

				it_pin->_stail->_next = p;
				it_pin->_stail = p;
			}
		}
	} // for counts

	if (!(it_pin->_in_progress_mask & aiogate_send_mask))
	{
		it_pin->_in_progress_mask |= aiogate_send_mask;

		sockaddr_in peeraddr;
		size_t requested = 0;
		// keeps mutex locked and mask is set
		if (!it_pin->_tcp_udp) // UDP case - get correct address
		{
			udp_header* uheader= (udp_header*)(it_pin->_shead->_ptr + it_pin->_shead->_begin); // get UDP header
			requested = uheader->_payload;
			peeraddr = uheader->_addr;
			// skip UDP header in the memory page
			it_pin->_shead->_begin += sizeof(udp_header);
		}
		else
		{
			requested = it_pin->_shead->_end - it_pin->_shead->_begin;
		}

		if (_pin_port.send(ident, it_pin->_shead->_ptr + it_pin->_shead->_begin, requested, INFINITE, it_pin->_tcp_udp ? 0 : &peeraddr, (void*)(size_t)aiogate_send_mask))
		{
			format_logging(0, __FILE__, __LINE__, en_log_error, "can not initiate send for pin %d", ident);
			it_pin->_in_progress_mask &= ~aiogate_send_mask;
			return false;
		}
	}

	format_logging(0, __FILE__, __LINE__, en_log_paranoid, "send bulk initiated for pin %d", ident);
	return true;
}

// initiates receive process, 
// @expect_delivery - gives a tip to aiogate 
// either use the big buffer or just a small one 
// in order to save the memory usage for a unknown waiting time 
// virtual 
bool 
aiogate::recv(	size_t ident,  // unique identificator
				bool expect_delivery, // memory usage advisor 
				const sockaddr_in* fromaddr
				)
{
	// initiate recv only if it's not in progress
	mutexKeeper keeper(_pin_mtx);
	pin_map_t::iterator it_pin = _pin_map.find(ident);
	if (it_pin == _pin_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "pin %d not found", ident);
		return false;
	}

	if (it_pin->_in_progress_mask & aiogate_recv_mask)
	{
		format_logging(0, __FILE__, __LINE__, en_log_paranoid, "pin %d is already in receive mode", ident);
		return true;
	}

	if (expect_delivery)
	{
		if (!it_pin->_rbuf)
		{
			it_pin->_rbuf = _pin_allocator.allocate();
			if (!it_pin->_rbuf)
			{
				format_logging(0, __FILE__, __LINE__, en_log_error, "no enough memory");
				return false;
			}
			else
				it_pin->_rbuf = new (it_pin->_rbuf) fixed_size_buffer();
		}
	}
	else
	{
		if (it_pin->_rbuf)
		{
			_pin_allocator.deallocate(it_pin->_rbuf);
			it_pin->_rbuf = 0;
		}
	}

	// sets receive flag
	it_pin->_in_progress_mask |= aiogate_recv_mask;

	// invokes socket port function
	if (_pin_port.receive(ident, expect_delivery ? (void*)it_pin->_rbuf->_ptr : &it_pin->_leader, expect_delivery ? BUFFER_CHUNK : sizeof(it_pin->_leader), it_pin->_recv_timeout, fromaddr, (void*)(size_t)aiogate_recv_mask))
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "can not initiate receive for pin %d", ident);
		it_pin->_in_progress_mask &= ~aiogate_recv_mask;
		return false;
	}

	format_logging(0, __FILE__, __LINE__, en_log_paranoid, "receive initiated for pin %d", ident);
	return true;
}

// sets send timeout
// virtual 
bool 
aiogate::set_send_timeout(size_t ident,
					size_t timeout
					) // timeout in milliseconds 
{
	mutexKeeper keeper(_pin_mtx);
	pin_map_t::iterator it = _pin_map.find(ident);
	if (it == _pin_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_info, "pin %d not found", ident);
		return false;
	}

	it->_send_timeout = timeout;
	return true;
}

// sets send timeout
// virtual 
bool 
aiogate::set_recv_timeout(size_t ident,
					size_t timeout
					) // timeout in milliseconds 
{
	mutexKeeper keeper(_pin_mtx);
	pin_map_t::iterator it = _pin_map.find(ident);
	if (it == _pin_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "pin %d not found", ident);
		return false;
	}

	it->_recv_timeout = timeout;
	return true;
}

// closes connection pin
// virtual 
bool 
aiogate::close(size_t ident)
{
	// locks mutex
	mutexKeeper keeper(_pin_mtx);
	pin_map_t::iterator it = _pin_map.find(ident);
	if (it == _pin_map.end())
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "pin %d not found", ident);
		return false;
	}

	keeper.unlock();

	initiate_close(ident, 0, true);
	// checks if callbck is invoke - delay close
	// if not, move pin info to the list
	format_logging(0, __FILE__, __LINE__, en_log_info, "closure procedure for pin %d initiated", ident);
	return true;
}

// virtual 
bool 
aiogate::v_has_job(size_t ident, void* user_data)
{
	// locks mutex
	mutexKeeper keeper(_pin_mtx);
	return !_pin_list.empty();
}

// virtual 
void 
aiogate::v_do_job(size_t ident, void* user_data)
{
	// locks mutex
	mutexKeeper keeper(_pin_mtx);
	if (_pin_list.empty())
		return;

	pin_info_extra info = _pin_list.front();
	_pin_list.pop_front();

	keeper.unlock();

	final_close(info);
}

//////////////////////////////////////////////////////////////////
void 
aiogate::initiate_close(size_t ident, size_t mask, bool invoke_callback)
{
	// locks mutex
	mutexKeeper keeper(_pin_mtx);
	pin_map_t::iterator it = _pin_map.find(ident);
	if (it == _pin_map.end()
		|| it->_callback_invoking_mask)
		// we can not remove
	{
		// but we can mark the pin as dead
		it->_still_alive = false;
		return;
	}

	// puts to the separate thread, which will handle final close
	pin_info_extra info(*it, ident, mask, invoke_callback);
	_pin_list.push_back(info);
	_pin_map.erase(it);
	keeper.unlock();
	_pin_thread.wakeup();
}

void 
aiogate::final_close(pin_info_extra& info)
{
	// invokes callback if required
	try
	{
		if (info._invoke_callback)
			info._pin->on_close((ub4_t)info._mask);
	}
	catch (...)
	{
		format_logging(0, __FILE__, __LINE__, en_log_error, "on_clode callback exception, handle %d", info._ident);
		assert(false);
	}

	// terimber socket port will close a socket, and cancel all asynchronous send/recv
	_pin_port.close(info._ident);

	// this is unsafe place again
	// locks mutex
	mutexKeeper keeper(_pin_mtx);

	// if the recv buffer has been allocated - release it
	if (info._rbuf)
		_pin_allocator.deallocate(info._rbuf);

	// releases all send buffers if any
	while (info._shead)
	{
		fixed_size_buffer* buf = info._shead;
		info._shead = info._shead->_next;
		_pin_allocator.deallocate(buf);
	}

	keeper.unlock();

	// destroys pin
	info._factory->destroy(info._pin);
}

//! \brief makes the snapshot of internal state
//virtual 
void 
aiogate::doxray()
{
	mutexKeeper guard(_pin_mtx);

	size_t pins = _pin_map.size(), 
		listeners = _listener_map.size(),   
		closed = _pin_list.size(),
		memory = _pin_allocator.capacity() * _pin_allocator.count();

	guard.unlock();

	format_logging(0, __FILE__, __LINE__, en_log_xray, "<aiogate pins=\"%d\" listeners=\"%d\" closed=\"%d\" memory=\"%d\" />",
		pins, listeners, closed, memory);
	
	_pin_port.doxray();
}

#pragma pack()
END_TERIMBER_NAMESPACE
