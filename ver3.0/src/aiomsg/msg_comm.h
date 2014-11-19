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

#ifndef _terimber_msg_comm_h_
#define _terimber_msg_comm_h_

#include "base/except.h"
#include "base/map.h"
#include "base/keymaker.h"
#include "crypt/crypt.h"
#include "aiosock/aiosock.h"
#include "threadpool/timer.h"
#include "threadpool/threadpoolfactory.h"
#include "xml/sxml.h"
#include "aiomsg/msg_que.h"

// guid generator
extern guid_t uuid_gen();

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// forward declarations
class msg_connection;
class msg_communicator;
class msg_listener;

//! \brief the length of PKI
#ifdef _DEBUG
const size_t rsa_key_size = 256;
#else
const size_t rsa_key_size = 2048;
#endif

//! \brief default thread alert time
static const size_t stay_on_alert_time = 10000; //10 sec
//! \brief max thread capacity
static const size_t max_thread_capacity = 64; //64

//! \class msg_communicator
//! \brief kernel class in the messaging system communication
//! dispatches incoming messages, 
//! creates and destroys connection objects
//! makes a lot of dirty work:
//! dynamic casting, msg allocation/destruction
//! and more ...
class msg_communicator :	public msg_queue_processor, 
							public timer_callback,
							public terimber_log_helper
{
	//! \typedef listener_list_t
	//! \brief list of listener pointers
	typedef list< msg_listener* >									listener_list_t;
	//! \typedef connection_map_t
	//! \brief maps guid to message connection
	typedef map< guid_t, msg_connection* >							connection_map_t;
	//! \typedef this_map_t
	//! \brief maps guid to communicator connection
	typedef map< guid_t, msg_communicator* >						this_map_t;

public:
	//! \brief constructor
	msg_communicator();
	//! \brief destructor
	~msg_communicator();
	//! \brief initializes internal structure due to file xml content
	//! generates unique initializing key for owner
	//! the same key owner has to use for uinit function
	//! NB!!! throw exception class object
	void 
	init(		const char* info_,							//!< ini file 
				const char* ini_key_						//!< crypto key, optional
				);
	//! \brief initializes internal structure due to file xml content
	//! generates unique initializing key for owner
	//! the same key owner has to use for uinit function
	//! NB!!! throws exception class object
	void 
	init(		const void* info_,							//!< ini xml in memory
				size_t len									//!< size of buffer
				);

	//! \brief as previous but take already parsed document
	//! NB!!! throws exception class object
	void 
	init(		xml_designer* nav							//!< xml designer instance
				);
	//! \brief uinitializes message system
	//! closes all listeners and connection
	// NB!!! throws exception class object
	void 
	uninit();
	//! \brief adds connection to the connection map
	void 
	add_connection(msg_connection* connection				//!< connection pointer
				);
	// changes connection address
	void 
	change_connection_address(const guid_t& old_address,	//!< old connection address
			msg_connection* connection						//!< new connection pointer
				);
	

	//! \brief this function insert message into communicator queue
	//! gives the order to destroy connection
	//! so this function can be called the connection function itself
	void 
	shutdown_connection(msg_connection* connection			//!< connection pointer
				);
	//! \brief returns communicator address - this is module address
	inline 
	const guid_t& 
	get_address() const 
	{ 
		return _address; 
	}
	//! \brief constructs msg with body size
	//! NB!!! throws exception class object 
	msg_cpp* 
	construct_msg(size_t size								//!< message body size
				);
	//! \brief reallocates message body, saves the old part of body, if any
	bool 
	resize_msg(	msg_cpp* msg,								//!< message pointer		
				size_t size									//!< message size
				);
	//! \brief copies message content
	msg_cpp* 
	copy_msg(	msg_cpp* msg								//!< message pointer
				);
	//! \brief destroys msg_cpp object, free memory
	bool 
	destroy_msg(msg_cpp* msg								//!< message pointer
				);
	//! \brief returns transport key
	void 
	get_msg_key(room_byte_t& crypt_private,					//!< byte room for private key
				const room_byte_t& crypt_external,			//!< bytes for external crypt key
				const guid_t& session						//!< session ident
				) const;
	//! \brief casts C style message to CPP style message
	inline 
	static 
	msg_cpp* 
	cast(		msg_t* msg									//!< message pointer
				) 
	{ 
		try 
		{
			return dynamic_cast< msg_cpp * >(static_cast< msg_cpp * >(msg)); 
		} 
		catch (...) 
		{ 
			return 0; 
		} 
	}
	//! \brief casts C style const message to CPP style const message
	inline 
	static 
	const msg_cpp* 
	cast(		const msg_t* msg							//!< message pointer
				) 
	{ 
		try 
		{ 
			return dynamic_cast< const msg_cpp * >(static_cast< const msg_cpp * >(msg)); 
		} 
		catch (...) 
		{ 
			return 0; 
		} 
	}
	//! \brief sets receiver address for specified message
	inline 
	static 
	bool 
	set_receiver(	msg_cpp* msg,							//!< message pointer
					const guid_t& receiver					//!< receiver address
					)
	{ 
		if (!msg) 
			return false; 
		msg->_receiver = receiver; 
		return true; 
	}
	//! \brief returns the sender of specified message
	inline 
	static 
	bool 
	get_sender(		const msg_cpp* msg,						//!< message pointer
					guid_t& sender							//!< [out] sender address
					)
	{ 
		if (!msg) 
			return false; 
		sender = msg->_sender; 
		return true; 
	}
	//! \brief tries to find connection with specified address
	msg_connection* 
	find_connection(const guid_t& addr						//!< input address
					);
	//! \brief tries to find listener with specified address and type
	msg_listener* 
	find_listener(	transport_type type						//!< transport
					);
	//! \brief checks if specified connection is still alive
	//! and has not been removed from connection map
	bool 
	validate_connection(const msg_connection* connection	//!< pointer to connection
					) const;
	//! \brief put incoming message into the communicator
	//! communicator can redirect message or put message into queue
	//! NB!!! throws exception class object
	void 
	comm_msg(		msg_cpp* msg							//!< message pointer
					);
	//! \brief constructs handshake message as initiator
	//! NB!!! throws exception class object
	msg_cpp* 
	construct_handshake(const rsa* rsa						//!< RSA object
					);
	//! \brief analyzes incoming handshake message
	//! prepares outgoing message on incoming message
	//! NB!!! throws exception class object
	msg_cpp* 
	reply_handshake(const msg_cpp* msg,						//!< message pointer
					room_byte_t* symetric_private_key		//!< private symmetric key, optional
					);
	//! \brief checks the reply from connection peer
	//! NB!!! throws exception class object
	void 
	check_handshake(const guid_t& sessionid,				//!< session ident
					const msg_cpp* reply,					//!< message pointer
					const rsa* rsa,							//!< RSA object
					room_byte_t& symetric_private_key		//!< private symmetric key
					);		
	//! \brief constructs ping message as initiator
	//! NB !!! throws exception class object
	msg_cpp* 
	construct_ping();
	//! \brief gets socket manager
	aiosock& 
	get_aiosock();
	//! \brief gets thread manager
	threadpool& 
	get_thread_manager();
	//! \brief gets event pool
	event_pool_t&
	get_event_pool();

	//! dynamic configuration
	//! \brief adds connection
	//! trows exception functions
	void 
	add_connection_config(const conf_connection& atom		//!< connection info
					);
	//! \brief adds listener
	void 
	add_listener_config(const conf_listener& atom,			//!< listener info
					bool start								//!< start listener
					);

	//! \brief removes connection
	void 
	remove_connection_config(const guid_t& address			//!< connection address
					);
	//! \brief removes listener
	void 
	remove_listener_config(transport_type type				//!< listener type
					);
	//! \brief parses xml into connection info
	static 
	void 
	parse_connection(xml_designer* nav,						//!< xml designer instance
					conf_connection& atom					//!< [out] connection info
					);
	//! \brief parses xml into listener info
	static 
	void 
	parse_listener(	xml_designer* nav,						//!< xml designer instance
					conf_listener& atom						//!< [out] listener info
					);

	//! \brief loans the communicator
	static msg_communicator* loan_communicator(const guid_t& addr);
	//! \brief returns back the communicator pointer
	static void return_communicator(const guid_t& addr, msg_communicator* comm);

	//! \brief does xray
	void doxray();

	//! \brief outputs debug message
	void
	log_msg(		const msg_cpp* msg					//!< message pointer
					);

protected:
	//! \brief base functionality - override virtual function
	virtual 
	void 
	v_do_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user defined data
					);
	//! \brief turns on communicator to the working state
	//! turns on all listeners and established previous connections
	virtual 
	void 
	v_on();
	//! \brief turns off communicator
	//! turns off all listeners and established previuosly connections
	virtual 
	void 
	v_off();
	//! \brief wakeup wokring thread
	virtual 
	void 
	wakeup();
	//! \brief timer_callback
	virtual 
	void 
	notify(			size_t ident,							//!< timer ident
					size_t interval,						//!< fire interval
					size_t multiplier						//!< multiplier
					);

private:
	//! \brief tries to establish the new connection according to info_
	//! NB!!! throws exception class object
	msg_connection* 
	_connect(		const conf_connection& info_,			//!< connection info
					ub8_t timeout							//!< timeout in milliseconds
					);

	//! \brief removes connection from the connection map
	void 
	_close_connection(const guid_t& addr					//!< connection address
					);
	//! \brief packs rsa 
	msg_cpp* 
	_generate_crypt_private_key(const rsa& rsa,				//!< RSA object
					room_byte_t& symetric_private_key		//!< private symmetric key
					);
	//! \brief adds encryption keys to message
	msg_cpp* 
	_pack_keys(		const integer& n,						//!< neutral PKI key
					const integer& e						//!< encryption PKI key
					);
	//! \brief extracts encryption keys from message
	void 
	_extract_keys(	const msg_cpp* msg,						//!< message pointer
					integer& n,								//!< neutral PKI key
					integer& e								//!< encryption PKI key
					);
	//! \brief extracts private symmetric key
	void 
	_decrypt_private_key(const msg_cpp* reply,				//!< message pointer
					const rsa& rsa,							//!< RSA object
					room_byte_t& symetric_private_key		//!< private symmetric key
					);
	//! \brief turns off all listeners
	void 
	_turn_off_listeners();
	//! \brief turns on all listeners
	void 
	_turn_on_listeners();
	//! \brief destroys all listeners
	void 
	_destroy_listeners();
	//! \brief turns off all connections
	void 
	_turn_off_connections();
	//! \brief turns on all connections
	void 
	_turn_on_connections();
	//! \brief destroys all connections
	void 
	_destroy_connections();
	//! \brief creates all listenrs
	void 
	_create_listeners();
	//! \brief creates a listener from info
	void 
	_create_listener(const conf_listener& atom				//!< listener info
					);

	//! \brief registers this pointer at the static map
	void _register_this();
	//! \brief revokes this pointer from the static map
	void _revoke_this();


private:
	guid_t						_address;					//!< internal communicator address
	guid_t						_debug_address;				//!< debug information address
	mutex						_mtx_listeners;				//!< multithreaded locker for listeners map
	listener_list_t				_listeners;					//!< list of listeners
	mutex						_mtx_conn;					//!< multithreaded locker for connection map
	connection_map_t			_connections;				//!< connection map
	mem_pool_t					_manager;					//!< memory allocation manager
	threadpool					_thread_manager;			//!< thread pool
	aiosock						_aio_port;					//!< socket port
	timer						_ping_timer;				//!< ping timer
	mutex						_mtx_config;				//!< multithreaded locker for config unit
	conf_unit					_config;					//!< configuration information
	event_pool_t				_event_pool;				//!< pool of events
	static keylocker			_this_access;				//!< locker for "this" communicator instance 
															//!< to prevent static function calls without valid communicator instance
	static this_map_t			_this_map;					//!< keeps the communicators map
};

//! \class msg_creator
//! \brief msg resource keeper
class msg_creator : public proto_creator< msg_creator, msg_cpp, size_t >
{
public:
	//! \brief constructor 
	msg_creator(	msg_communicator* communicator			//!< pointer to communicator
					) : 
		_communicator(communicator) 
	{
	}
	//! \brief creator function
	msg_cpp* 
	create(			size_t n								//!< message size
					) 
	{ 
		return _communicator->construct_msg(n); 
	}
	//! \brief destroy function
	void 
	destroy(		msg_cpp* obj
					) 
	{ 
		if (obj) 
			_communicator->destroy_msg(obj); 
	}
private:
	msg_communicator*	_communicator;						//!< pointer to communicator
};

//! \typedef msg_pointer_t
//! \brief smart pointer to message creator
typedef smart_pointer< msg_creator > msg_pointer_t;

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_msg_comm_h_

