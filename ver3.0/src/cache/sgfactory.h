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
#ifndef _terimber_sgfactory_h_
#define _terimber_sgfactory_h_


#include "base/stack.h"
#include "base/primitives.h"
#include "smart/vardatabase.h"
#include "xml/xmlaccss.h"
#include "aiogate/aiogate.h"


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

class sgresources
{
	typedef stack< byte_allocator* >	allocator_container_t;
	typedef stack< xml_designer* >		xml_container_t;
public:

	sgresources(size_t capacity);
	~sgresources();

	byte_allocator* get_all();
	xml_designer* get_xml();

	void back_all(byte_allocator* obj);
	void back_xml(xml_designer* obj);

private:
	size_t								_capacity;
	mutex								_mtx;

	allocator_container_t				_alls;
	size_t								_all_taken;

	xml_container_t						_xmls;
	size_t								_xml_taken;
};

// class implements the terimber_aiogate_pin interface
class sgpin : public terimber_aiogate_pin
{
public:
	// constructor
	sgpin(sgresources* resources, terimber_vardatabase* db);
	// destructor
	virtual ~sgpin();

	// accepts new incoming pin
	virtual void on_accept(	const sockaddr_in& local, // ip/port of this pin
							const sockaddr_in& remote, // ip/port of remote peer
							size_t ident, // unique ident for this pin will be used with following callback
							terimber_aiogate* callback // callback for stargate
							);

	// connect to the new pin
	virtual void on_connect(const sockaddr_in& local,  // ip/port of this pin
							const sockaddr_in& remote, // ip/port of remote peer
							size_t ident, // unique ident for this pin will be used with following callback
							terimber_aiogate* callback // callback for stargate
							);
	
	// stargate calls this callback when bytes came from peer
	// returns true if immediately more bytes is expected - just for performance optimization
	virtual bool on_recv(	const void* buf,  // array of bytes for sending
							size_t len, // the length of array of bytes
							const sockaddr_in& peeraddr,
							bool& expected_more
							);
	
	virtual void on_bind(const sockaddr_in& local,  // ip/port of this pin
							size_t ident, // unique ident for this pin will be used with following callback
							terimber_aiogate* callback // aiogate itself
							);

	// stargate invokes this callback only if all bytes have been sent
	virtual void on_send(const sockaddr_in& peeraddr);
	
	// stargate invokes this function when pin connection is deactivate
	// only internal action can be taken - no stargate calls anymore for this pin
	virtual void on_close(ub4_t mask);

private:
	size_t							_ident;
	terimber_aiogate*				_sgcallback;
	sgresources*					_resources;
	terimber_vardatabase*			_database;
	byte_allocator*					_all;
	void*							_buf;
	size_t							_len;
	size_t							_offset;
};

// class implements the terimber_stargate_pin_factory interface
class sgfactory : public terimber_aiogate_pin_factory
{
	typedef node_allocator< sgpin >	sgpin_allocator_t;
	
public:

	// constructor
	sgfactory();
	// destructor
	virtual ~sgfactory();

	// creates new pin
	virtual terimber_aiogate_pin* create(void* arg);
	
	// destroys pin
	virtual void destroy(terimber_aiogate_pin* pin);

private:
	mutex					_mtx;
	sgpin_allocator_t		_sgpin_allocator;
	sgresources				_sgresources;
	vardatabase				_vardatabase;
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_sgfactory_h_
