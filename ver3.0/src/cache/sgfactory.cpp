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

#if OS_TYPE == OS_WIN32
#if defined(_MSC_VER) && (_MSC_VER > 1200) 
#include <winsock2.h>
#endif
#endif

#include "cache/sgfactory.h"
#include "base/memory.hpp"
#include "base/stack.hpp"
#include "base/list.hpp"
#include "smart/varmap.hpp"
#include "smart/varvalue.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

const size_t MAX_BUFFER_SIZE = 1024*1024;
//////////////////////////////////////////////////////////////////////////////
sgresources::sgresources(size_t capacity) : _capacity(capacity), _all_taken(0), _xml_taken(0) 
{
}

sgresources::~sgresources()
{
	mutexKeeper keeper(_mtx);

	assert(_all_taken == 0);
	assert(_xml_taken == 0);

	while (!_alls.empty())
	{
		delete _alls.top();
		_alls.pop();
	}

	while (!_xmls.empty())
	{
		delete _xmls.top();
		_xmls.pop();
	}
}

byte_allocator* 
sgresources::get_all()
{
	mutexKeeper keeper(_mtx);
	if (_all_taken == _capacity)
		return 0;

	if (_alls.empty())
	{
		byte_allocator* obj = new byte_allocator();

		if (obj)
			++_all_taken;

		return obj;
	}
	else
	{
		byte_allocator* obj = _alls.top();
		_alls.pop();
		++_all_taken;
		return obj;
	}
}

xml_designer* 
sgresources::get_xml()
{
	mutexKeeper keeper(_mtx);

	if (_xml_taken == _capacity)
		return 0;

	if (_alls.empty())
	{
		xml_factory acc;
		xml_designer* obj = acc.get_xml_designer();

		if (obj)
			++_xml_taken;

		return obj;
	}
	else
	{
		xml_designer* obj = _xmls.top();
		_xmls.pop();
		++_xml_taken;
		return obj;
	}
}

void 
sgresources::back_all(byte_allocator* obj)
{
	mutexKeeper keeper(_mtx);
	obj->clear_extra();
	_alls.push(obj);
	--_all_taken;
}

void 
sgresources::back_xml(xml_designer* obj)
{
	mutexKeeper keeper(_mtx);
	_xmls.push(obj);
	--_xml_taken;
}

//////////////////////////////////////////////////////////////////////////////

// class implements the terimber_aiogate_pin interface
// constructor
sgpin::sgpin(sgresources* resources, terimber_vardatabase* db) : _ident(0), _sgcallback(0), _resources(resources), _database(db), _all(0), _buf(0), _len(0), _offset(0)

{
}

// destructor
// virtual 
sgpin::~sgpin()
{
}

// accepts the new incoming pin
// virtual 
void
sgpin::on_accept(	const sockaddr_in& local, // ip/port of this pin
					const sockaddr_in& remote, // ip/port of remote peer
					size_t ident, // unique ident for this pin will be used with following callback
					terimber_aiogate* callback // callback for stargate
					)
{ 
	_ident = ident;
	_sgcallback = callback;
	// initiate recv
	_sgcallback->recv(_ident, false, 0);
}

// connects to the new pin
// virtual 
void 
sgpin::on_connect(const sockaddr_in& local,  // ip/port of this pin
					const sockaddr_in& remote, // ip/port of remote peer
					size_t ident, // unique ident for this pin will be used with following callback
					terimber_aiogate* callback // callback for stargate
					)
{
	// we are a server, so there is nothing to do here
	assert(false);
	// _ident = ident;
	// _sgcallback = callback;
}

// stargate calls this callback when bytes came from peer
// returns true if immediately more bytes is expected - just for performance optimization
// virtual 
bool 
sgpin::on_recv(	const void* buf,  // array of bytes for sending
				size_t len, // the length of array of bytes
				const sockaddr_in& peeraddr,
				bool& expected_more
			)
{
	size_t offset = 0;
	expected_more = true;

	while (len && 
			(!_buf && _offset < sizeof(size_t)
			|| _buf && _offset < _len + sizeof(size_t))
			)
	{
		if (_offset < sizeof(size_t)) // we are still getting the first 4 bytes
		{
			assert(_all == 0);
			assert(_buf == 0);
			size_t clen = __min(len, sizeof(size_t) - _offset);
			memcpy(&_len + _offset, (const ub1_t*)buf + offset, clen);
			_offset += clen;
			offset += clen;
			len -= clen;
		}

		if (!_buf && _offset == sizeof(size_t)) // we have teh 4 leading bytes
		{
			assert(_all == 0);
			_all = _resources->get_all();
			if (!_all)
			{
				// not enough memory
				_sgcallback->close(_ident);
				return false;
			}

			if (_len > MAX_BUFFER_SIZE)
			{
				// not enough memory
				_sgcallback->close(_ident);
				return false;
			}

			_buf = _all->allocate(_len);
			if (!_buf)
			{
				// not enough memory
				_sgcallback->close(_ident);
				return false;
			}
		}

		if (len && _buf && _offset >= sizeof(size_t)) // gets the body of the request
		{
			assert(_all != 0);

			size_t clen = __min(len, _len + sizeof(size_t) - _offset);
			memcpy((ub1_t*)_buf + _offset - sizeof(size_t), (const ub1_t*)buf + offset, clen);
			
			// adjusts offset & len
			_offset += clen;
			offset += clen;
			len -= clen;


			if (_len + sizeof(size_t) == _offset)
			{
				xml_designer* parser = _resources->get_xml();
				if (!parser)
				{
					// not enough memory
					_sgcallback->close(_ident);
					return false;
				}

				parser->load(0, 0, 0, 0);

				// calls vardatabase
				_database->process_xml_request((const char*)_buf, _len, parser);
	
				// gets required length
				size_t slen = 0;
				parser->save(0, slen, false);

				// resets allocator
				_all->reset();

				void* sbuf = _all->allocate(slen + sizeof(size_t)); // allocate memory
				if (!sbuf)
				{
					// closes connection
					_sgcallback->close(_ident);
					return false;
				}

				memcpy(sbuf, &len, sizeof(size_t)); // set the first 4 bytes with the length of buffer
				parser->save((char*)sbuf + sizeof(size_t), slen, false); 

				//  sends results back
				if (!_sgcallback->send(_ident, sbuf, slen + sizeof(size_t), 0))
				{
					// closes connection
					_sgcallback->close(_ident);
					return false;
				}

				// resets offsets - prepares for the new message
				_offset = 0;
				_buf = 0;
				_resources->back_all(_all);
				_all = 0;
				expected_more = false;
			}
		}
	} 

	return true;
}

// stargate invokes this callback only if all bytes have been sent
// virtual 
void 
sgpin::on_send(const sockaddr_in& peeraddr)
{
	// nothing to do
}

//virtual 
void 
sgpin::on_bind(const sockaddr_in& local,  // ip/port of this pin
							size_t ident, // unique ident for this pin will be used with following callback
							terimber_aiogate* callback // aiogate itself
							)
{
	assert(false);
}

// stargate invokes this function when the pin connection is deactivated
// only internal action can be taken - no stargate calls anymore for this pin
// virtual 
void 
sgpin::on_close(ub4_t mask)
{
	// returns allocator back to resources, if any
	if (_all)
	{
		_resources->back_all(_all);
		_all = 0;
	}
}


//////////////////////////////////////////////////////////////
// class implements the terimber_stargate_pin_factory interface
// constructor
sgfactory::sgfactory() : _sgresources(64*1024)
{
}

// virtual 
sgfactory::~sgfactory()
{
}

// creates new pin
// virtual 
terimber_aiogate_pin* 
sgfactory::create(void* arg)
{
	// locks mutex
	mutexKeeper keeper(_mtx);
	// allocates memory for object
	sgpin* p = _sgpin_allocator.allocate();

	if (p)
	{
		// if memory is allocated, call constructor
		p = new(p) sgpin(&_sgresources, &_vardatabase);
	}

	// returns result
	return p;
}
	
// destroys pin
// virtual 
void 
sgfactory::destroy(terimber_aiogate_pin* pin)
{
	// must be not null
	assert(pin);

	// locks mutex
	mutexKeeper keeper(_mtx);
	// calls destructor directly
	pin->~terimber_aiogate_pin();
	// returns memory back to allocator
	_sgpin_allocator.deallocate(pin);
}



#pragma pack()
END_TERIMBER_NAMESPACE
