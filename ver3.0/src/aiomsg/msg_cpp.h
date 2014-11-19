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

#ifndef _terimber_msg_cpp_h_
#define _terimber_msg_cpp_h_

#include "aiomsg/msg_c.h"
#include "base/memory.h"
#include "base/string.h"
#include "base/list.h"
#include "base/number.h"


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \typedef room_byte_t
//! \brief array of bytes
typedef room_array< ub1_t > room_byte_t;

//! \enum transport_type
//! \brief transport type, expandable to rpc, p2p, etc 
enum transport_type
{
	unknown = 0,											//!< unknown transport
	sock,													//!< socket transport
	rpc														//!< rpc transport
};

//! message types
//! \brief user type send, reply is required
const size_t user_type_send				= 0x10000001;
//! \brief user type send asynchronously, reply is required
const size_t user_type_send_async		= 0x10000002;
//! \brief user type post, no reply expected
const size_t user_type_post				= 0x10000003;
//! \brief this is a reply on user send
const size_t user_type_reply			= 0x10000004;
//! \brief this is a reply on user send asynchronously
const size_t user_type_reply_async		= 0x10000005;
//! \brief mask for all user types
const size_t user_type_mask				= 0x10000007;
//! \brief handhake type
const size_t handshake_type				= 0x20000000;
//! \brief system type
const size_t system_type				= 0x40000000;

//! system internal messages
//! \brief hadnshake request
const size_t msg_id_handshake_request	= 0x10000001;
//! \brief handshake response
const size_t msg_id_handshake_reply		= 0x10000002;
//! \brief ping message
const size_t msg_id_ping				= 0x10000003;
//! \brief shutdown message
const size_t msg_id_shutdown			= 0x10000004;
//! \brief deafult timeout in millseconds
const size_t msg_default_timeout		= 10000;
//! \brief crypto block size
const size_t msg_crypt_block			= 16;

//! \brief checks if type is a user reply
inline 
bool 
is_request(size_t type) 
{ 
	return type != user_type_reply && type != user_type_reply_async; 
}

//! \class msg_cpp
//! \brief message class
//! internal message representation
class msg_cpp : public msg_t
{
	//! \brief hidden constructor
	msg_cpp(byte_allocator* allocator_, size_t size_);
	//! \brief hidden destructor
	virtual ~msg_cpp();
public:
	//! \brief "static" constructor
	static 
	inline 
	msg_cpp* 
	construct(		byte_allocator* allocator_,					//!< external allocator 
					size_t size_								//!< message size
				)
	{ 
		msg_cpp* msg = 0;
		void* msg_space = 0;
		// allocates memory for message
		// calls constructor using allocated memory
		// if size greater then zero then resize message body
		if ((msg_space = allocator_->allocate(sizeof(msg_cpp)))
			&& (msg = new(msg_space) msg_cpp(allocator_, size_))
		)
			return msg;
		else
			return 0;
	}
	//! \brief "static" destructor
	static 
	inline 
	byte_allocator* 
	destroy(		msg_cpp* msg							//!< message pointer
					)
	{
		// save internal allocator
		byte_allocator* all = msg->_allocator;
		// call destructor
		msg->~msg_cpp();
		// return allocator
		return all;
	}

	//! \brief packs message
	//! prepares block - continuant part of message for block sending
	void 
	pack_msg(		const room_byte_t* key					//!< private symmetric key, optional
					);
	//! \brief unpacks message
	//! fill msg_cpp fields taking information from block part
	void 
	unpack_msg(		const room_byte_t* key					//!< private symmetric key, optional
					);
	//! \brief resizes body size
	//! save previous part, if any
	bool 
	resize(			size_t size_							//!< message size
					);
	//! \brief returns the body size
	inline 
	size_t 
	get_size() const 
	{ 
		return _size; 
	}
	//! \brief returns pointer to body - read only
	inline 
	const ub1_t* 
	get_body() const 
	{ 
		return _body; 
	}

	//! \brief returns pointer to body - writable version
	inline 
	ub1_t* 
	get_body() 
	{ 
		return _body; 
	}
	//! \brief returns pointer to block - read only
	inline 
	const ub1_t* 
	get_block() const 
	{ 
		return _block; 
	}
	//! \brief returna pointer to block - writable version
	inline 
	ub1_t* 
	get_block() 
	{
		return _block; 
	}
	//! \brief estimates the size of whole message with body size
	static 
	inline 
	size_t 
	estimate_size(	size_t size_							//!< message size
					) 
	{ 
		return sizeof(msg_cpp) + block_size(size_); 
	}
	//! \brief culculate crypt size
	static 
	inline 
	size_t 
	crypt_size(		size_t size_							//!< input bytes to crypt
					) 
	{ 
		return size_ ? ALIGNED_MASK_SIZEOF(size_, msg_crypt_block) : 0; 
	}
	//! \brief returns the block size for body size
	static 
	inline 
	size_t 
	block_size(		size_t size_							//!< message size
					) 
	{ 
		return crypt_size(size_) + _block_body_offset(); 
	}
private:
	//! \brief returns the offset of body from the begining of the msg_cpp class instance
	static 
	inline 
	size_t 
	_block_body_offset()
	{ 
		return sizeof(ub4_t) // leading block size
			+ sizeof(msg_cpp) // whole class size
			- sizeof(void*) // minus vtable
			- sizeof(byte_allocator*) // minus allocator pointer
			- sizeof(void*) // minus _body pointer
			- sizeof(void*); // minus _block pointer	
	}

public:
	//! \brief network byte order 4 bytes
	static 
	inline 
	ub4_t 
	reverse32(		ub4_t value								//!< input value
					)
	{
		value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
		return ((value & 0xFFFF0000) >> 8) | ((value & 0x0000FFFF) << 8);
	}
	//! \brief network byte order 2 bytes
	static 
	inline 
	ub2_t 
	reverse16(		ub2_t value								//!< input value
					)
	{ 
		return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);	
	}

	//! \brief copies 4 bytes to the buffer in network format
	//! and move pointer to the buffer
	static 
	inline 
	size_t 
	pack32(			ub1_t*& dest,							//!< buffer pointer
					ub4_t x								//!< input value
					)
	{ 
		size_t size = sizeof(ub4_t);
		*(ub4_t*)dest = htonl(x);
		dest += size;
		return size;
	}
	//! \brief copies 4 bytes from the buffer in host format
	//! and move pointer to the buffer
	static 
	inline 
	size_t 
	unpack32(		const ub1_t*& x,						//!< input buffer pointer
					ub4_t& dest								//!< [out] value
					)
	{ 
		size_t size = sizeof(ub4_t);
		dest = ntohl(*(ub4_t*)x);
		x += size;
		return size;
	}

	//! \brief copies 2 bytes to the buffer in network format
	//! and move pointer to the buffer
	static 
	inline 
	size_t 
	pack16(			ub1_t*& dest,							//!< output buffer pointer
					ub2_t x								//!< input value
					)
	{ 
		size_t size = sizeof(ub2_t);
		*(ub2_t*)dest = htons(x);
		dest += size;
		return size;
	}

	//! \brief copies 2 bytes from the buffer in host format
	//! and move pointer to the buffer
	static 
	inline 
	size_t 
	unpack16(		const ub1_t*& x,						//!< input buffer pointer
					ub2_t& dest								//!< [out] value
					)
	{ 
		size_t size = sizeof(ub2_t);
		dest = ntohs(*(ub2_t*)x);
		x += size;
		return size;
	}

	//! \brief copies 32 bytes (GUID format) to the buffer in network format
	//! and move pointer to the buffer
	static 
	inline 
	size_t 
	packaddr(		ub1_t*& dest,							//!< output buffer pointer
					const guid_t& x						//!< input value
					)
	{ 
		size_t size = 0;
		size += pack32(dest, x.Data1);
		size += pack16(dest, x.Data2);
		size += pack16(dest, x.Data3);
		for (ub1_t index = 0; index < 8; ++index, size += sizeof(ub1_t))
			*dest++ = x.Data4[index];
		
		return size;
	}

	//! \brief copies 32 bytes (GUID format) from the buffer in host format
	//! and move pointer to the buffer
	static 
	inline 
	size_t 
	unpackaddr(		const ub1_t*& x,						//!< input buffer pointer
					guid_t& dest							//!< [out] value
					)
	{ 
		size_t size = 0;
		size += unpack32(x, (ub4_t&)dest.Data1);
		size += unpack16(x, (ub2_t&)dest.Data2);
		size += unpack16(x, (ub2_t&)dest.Data3);
		for (size_t index = 0; index < 8; ++index, size += sizeof(ub1_t))
			dest.Data4[index] = *x++;
		
		return size;
	}

	//! \brief copies 8 bytes (double format) to the buffer in network format
	//! and move pointer to the buffer
	static 
	inline 
	size_t 
	pack64(			ub1_t*& dest,							//!< output buffer pointer
					const ub8_t& x						//!< input value
					)
	{ 
		size_t size = sizeof(ub8_t);
		*(ub4_t*)dest = htonl((ub4_t)(x & 0x00000000ffffffff));
		*(ub4_t*)(dest + sizeof(ub4_t)) = htonl((ub4_t)((x >> 32) & 0x00000000ffffffff));
		dest += size;
		return size;
	}

	//! \brief copies 8 bytes (double format) from the buffer in host format
	//! and move pointer to the buffer
	static 
	inline 
	size_t 
	unpack64(		const ub1_t*& x,						//!< input buffer pointer
					ub8_t& dest								//!< [out] value
					)
	{ 
		size_t size = sizeof(ub8_t);
		dest = (ub8_t)ntohl(*(ub4_t*)x) + ((ub8_t)ntohl(*(ub4_t*)(x + sizeof(ub4_t))) << 32);
		x += size;
		return size;
	}

public:
	ub4_t			_type;									//!< type of message (system, handshake, user ...)
	guid_t			_marker;								//!< marker for identification of the reply message
	sb8_t			_timestamp;								//!< date&time of message
	guid_t			_sender;								//!< message sender
	guid_t			_receiver;								//!< message receiver
	guid_t			_sessionid;								//!< connection session identificator
private:
	ub4_t			_size;									//!< size of message body
	byte_allocator*	_allocator;								//!< pointer to allocator
	ub1_t*			_body;									//!< poiter to message body
	ub1_t*			_block;									//!< pointer to the block
};

//! \class msg_pack
//! \brief pack error and reply messages
class msg_pack
{
public:
	//! \brief prepares error message on incoming message with specified text
	static 
	void 
	make_error_msg(	msg_cpp* msg,							//!< message pointer
					const char* text						//!< error text
					);
	//! \brief prepares the template reply for incoming message
	//! fill header information
	static 
	void 
	make_reply_msg(	const msg_cpp* msg,						//!< input message pointer
					msg_cpp* reply							//!< output message pointer
					);
};

//! \class conf_peer
//! \brief configuration for listener peers
class conf_peer
{
public:
	//! \brief default constructor
	conf_peer() : 
		_support_crypt(false), 
		_crypt(2)
	{ 
		_address = null_uuid; 
	}
	//! \brief copies constructor
	conf_peer(const conf_peer& x) : 
		_crypt(x._crypt) 
	{ 
		*this = x; 
	}
	//! \brief assigns operator
	conf_peer& 
	operator=(const conf_peer& x)
	{
		if (this != &x)
		{
			_address = x._address;
			_support_crypt = x._support_crypt;
			_crypt = x._crypt;
		}

		return *this;
	}


	guid_t			_address;								//!< address
	bool			_support_crypt;							//!< flag if crypt is required
	room_byte_t		_crypt;									// private symmetric crypto key
};

//! \typedef msg_peer_list_t
//! \brief list of listner peer configurations
typedef list< conf_peer > msg_peer_list_t;

//! \class conf_listener
//! \brief common class for config unit information
class conf_listener
{
public:
	//! \brief default constructor
	conf_listener() : 
		_type(unknown), 
		_connections(0), 
		_ping((ub4_t)INFINITE), 
		_port(0), 
		_support_crypt(false), 
		_crypt_accept(2)
	{
	}
	//! \brief copies constructor
	conf_listener(const conf_listener& x) : 
		_crypt_accept(x._crypt_accept) 
	{ 
		*this = x; 
	}
	//! \brief assigns operator
	conf_listener& 
	operator=(const conf_listener& x)
	{
		if (this != &x)
		{
			_type = x._type;
			_address = x._address;
			_port = x._port;
			_network = x._network;
			_info = x._info;
			_connections = x._connections;
			_ping = x._ping;
			_support_crypt = x._support_crypt;
			_crypt_accept = x._crypt_accept;
			_accept = x._accept;
			_reject = x._reject;
		}

		return *this;
	}

	transport_type	_type;									//!< type of atom
	guid_t			_address;								//!< address
	ub2_t			_port;									//!< port for socket & http connections
	string_t		_network;								//!< network for socket & http connections
	string_t		_info;									//!< additional information
	ub4_t			_ping;									//!< ping interval time in msec
	bool			_support_crypt;							//!< flag supporting crypt
	ub4_t			_connections;							//!< max connections
	room_byte_t		_crypt_accept;							//!< private default crypto key
	msg_peer_list_t _accept;								//!< list allowed acceptors
	msg_peer_list_t _reject;								//!< list disallowed acceptors
};

//! \class conf_connection
//! common class for connection config information
class conf_connection
{
public:
	//! \brief default constructor
	conf_connection() : 
		_type(unknown), 
		_ping((ub4_t)INFINITE), 
		_port(0), 
		_support_crypt(false), 
		_crypt_private(2), 
		_crypt_external(2)
	{ 
		_address = null_uuid; _session = null_uuid; 
	}
	//! \brief copy constructor
	conf_connection(const conf_connection& x) : 
		_crypt_private(x._crypt_private), 
		_crypt_external(x._crypt_external)
	{ 
		*this = x; 
	}
	//! \brief assign operator
	conf_connection& operator=(const conf_connection& x)
	{
		if (this != &x)
		{
			_type = x._type;
			_address = x._address;
			_port = x._port;
			_session = x._session;
			_info = x._info;
			_network = x._network;
			_ping = x._ping;
			_support_crypt = x._support_crypt;
			_crypt_private = x._crypt_private;
			_crypt_external = x._crypt_external;
		}

		return *this;
	}

	//! \brief copy operator from listener info
	conf_connection& operator=(const conf_listener& x)
	{
		_type = x._type;
		_address = x._address;
		_port = x._port;
		_info = x._info;
		_network = x._network;
		_ping = x._ping;

		return *this;
	}

	transport_type	_type;									//!< type of atom
	guid_t			_address;								//!< address
	ub2_t			_port;									//!< port for socket & http connections
	string_t		_network;								//!< network for socket & http connections
	string_t		_info;									//!< additional information
	ub4_t			_ping;									//!< ping interval time in msec
	bool			_support_crypt;							//!< flag supporting crypt
	room_byte_t		_crypt_private;							//!< private crypto key
	room_byte_t		_crypt_external;						//!< external crypto key
	guid_t			_session;								//!< session ident
};


//! \typedef msg_listener_list_t
//! \brief list of listener configurations
typedef list< conf_listener > msg_listener_list_t;
//! \typedef msg_connection_list_t
//! \brief list of connection configurations
typedef list< conf_connection > msg_connection_list_t;

//! \class conf_unit
//! \brief class describes unit of a configuration
class conf_unit
{
public:
	//! \brief constructor
	conf_unit() 
	{ 
		clear(); 
	}
	//! \brief copies constructor
	conf_unit(const conf_unit& x) 
	{ 
		*this = x; 
	}
	//! \brief assigns operator
	conf_unit& operator=(const conf_unit& x)
	{	
		if (this != &x)
		{
			clear();
			_listeners = x._listeners;
			_connections = x._connections;
		}

		return *this;
	}

	//! \brief clears information
	void clear()
	{ 
		_listeners.clear();
		_connections.clear();
	}

	msg_listener_list_t		_listeners;						//!< list of listeners' information
	msg_connection_list_t	_connections;					//!< list of connections' information
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_msg_cpp_h_

