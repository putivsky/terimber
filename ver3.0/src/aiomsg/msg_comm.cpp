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

#include "aiomsg/msg_comm.h"
#include "aiomsg/msg_lsnr.h"
#include "aiomsg/msg_conn.h"
#include "aiomsg/msg_sock.h"
#if OS_TYPE == OS_WIN32
#include "aiomsg/msg_rpc.h"
#endif

#include "base/list.hpp"
#include "base/map.hpp"
#include "base/template.hpp"
#include "base/memory.hpp"
#include "base/string.hpp"
#include "base/common.hpp"
#include "base/vector.hpp"
#include "base/number.hpp"
#include "base/keymaker.h"

#include "crypt/integer.hpp"
#include "crypt/arithmet.hpp"
#include "xml/xmlimpl.hpp"
#include "xml/sxs.hpp"

#include "ossock.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// debug message
#define MSG_DEBUG_ID			0x80000003


// messages tables
extern exception_table aiosockTable;
extern exception_table msgMsgTable;
// handshake max timeout

const ub8_t handshake_default_timeout = 3000;

const char* msg_xml_dtd = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<!ENTITY % kind \"(sock | rpc | p2p)\"> \
<!ELEMENT msgPort (listeners?, connections?)> \
<!ATTLIST msgPort \
address CTYPE vt_guid #REQUIRED \
debug CTYPE vt_guid #IMPLIED > \
<!ELEMENT listeners (listener)*> \
<!ELEMENT connections (connection)*> \
<!ELEMENT listener (accept | reject)?> \
<!ELEMENT reject (peer)+> \
<!ELEMENT accept (peer)+> \
<!ELEMENT peer EMPTY> \
<!ATTLIST peer \
address CTYPE vt_guid #REQUIRED \
security CTYPE vt_bool #IMPLIED \
password CDATA #IMPLIED > \
<!ELEMENT connection EMPTY> \
<!ATTLIST listener \
type %kind; #REQUIRED \
port CTYPE vt_ub2 #IMPLIED \
network CDATA #IMPLIED \
connections CTYPE vt_ub4 #IMPLIED \
ping CTYPE vt_ub4 #IMPLIED \
waste CTYPE vt_ub4 #IMPLIED \
info CDATA #IMPLIED \
security CTYPE vt_bool #IMPLIED \
password CDATA #IMPLIED > \
<!ATTLIST connection \
type %kind; #REQUIRED \
address CTYPE vt_guid #REQUIRED \
port CTYPE vt_ub2 #IMPLIED \
network CDATA #IMPLIED \
ping CTYPE vt_ub4 #IMPLIED \
waste CTYPE vt_ub4 #IMPLIED \
info CDATA #IMPLIED \
security CTYPE vt_bool #IMPLIED \
password CDATA #IMPLIED >";

static size_t meesages_in_work = 0;
////////////////////////////////////////////////////////////
// class msg_communicator
// static 
keylocker msg_communicator::_this_access(1024); // multithreaded locker for map
// static 
msg_communicator::this_map_t msg_communicator::_this_map;
// static 
msg_communicator* 
msg_communicator::loan_communicator(const guid_t& addr)
{
	// enter gate
	if (!_this_access.enter(1000))
		return false;

	this_map_t::iterator iter = _this_map.find(addr);
	if (iter == _this_map.end())
	{
		_this_access.leave();
		return 0;
	}
	else
		return *iter;
}
// static 
void 
msg_communicator::return_communicator(const guid_t& addr, msg_communicator* comm)
{
	// leave gate
	if (comm)
	{
		this_map_t::iterator iter = _this_map.find(addr);
		assert (iter != _this_map.end() && *iter == comm);
		_this_access.leave();
	}
}

////////////////////////////////////////////////////////////
msg_communicator::msg_communicator() :
	msg_queue_processor(this), 
	_thread_manager(max_thread_capacity, 60000),
	_aio_port(0, 60000)
{
}

msg_communicator::~msg_communicator()
{
	// clears resources
	try
	{
		// in case of partial init
		uninit();
	}
	catch (...)
	{
		assert(false);
	}
}

void 
msg_communicator::_register_this()
{
	// lock gate
	keylockerWriter gkeeper(_this_access, 1000);
	// try to find the dublicate
	this_map_t::iterator iter = _this_map.find(_address);
	if (iter != _this_map.end())
		// dublicate found
		exception::_throw("Dublicate communicator address");
	else
		// insert into static map
		_this_map.insert(_address, this);
}

void 
msg_communicator::_revoke_this()
{
	// lock gate
	keylockerWriter gkeeper(_this_access);
	// remove this from static map
	_this_map.erase(_address);
}


msg_cpp* 
msg_communicator::construct_msg(size_t size)
{
	byte_allocator* all = 0;
	msg_cpp*		msg = 0;

	if ((all = _manager.loan_object((size_t)msg_cpp::estimate_size(size))) // loan allocator object
		&&	(msg = msg_cpp::construct(all, size))) // construct message object
		// success
	{
//		format_logging(0, __FILE__, __LINE__, en_log_info, "msgs: %d", ++meesages_in_work);
		return msg;
	}
	else
	{
		// failure
		if (msg) // destroys message
			destroy_msg(msg);
		else if (all) // return allocator if message construction failed
		{
			// for security reasons - clears all used memory
			_manager.return_object(all);
		}


		exception::_throw(MSG_RESULT_NOTMEMORY, &msgMsgTable);
		return 0;
	}
}

bool 
msg_communicator::destroy_msg(msg_cpp* msg_)
{
	if (!msg_)
		return false;

	//format_logging(0, __FILE__, __LINE__, en_log_info, "msgs: %d", --meesages_in_work);

	// saves pointer to allocator object
	byte_allocator* all = msg_cpp::destroy(msg_);
	// must be not null
	assert(all);
	// security
	all->reset(true);
	// returns allocator object
	_manager.return_object(all);
	// returns result
	return true;
}

msg_cpp* 
msg_communicator::copy_msg(msg_cpp* msg_)
{
	// gets message size
	size_t size = msg_->get_size();
	// constructs copy message
	msg_cpp* msg = construct_msg(size);

	// copies C part of message
	msg->msgid = msg_->msgid;
	msg->majver = msg_->majver;
	msg->minver = msg_->minver;
	msg->priority = msg_->priority;
	msg->timeout = msg_->timeout;

	// copies C++ part of message
	msg->_type = msg_->_type;
	msg->_timestamp = msg_->_timestamp;
	msg->_sender = msg_->_sender;
	msg->_receiver = msg_->_receiver;
	msg->_sessionid = msg_->_sessionid;

	// doesn't copy marker

	if (size) // copies body, if any
		memcpy(msg->get_body(), msg_->get_body(), size);

	// return copies message object
	return msg;
}

bool 
msg_communicator::resize_msg(msg_cpp* msg_, size_t size)
{
	// resizes message body
	return msg_->resize(size);
}


void 
msg_communicator::get_msg_key(room_byte_t& crypt_private, const room_byte_t& crypt_external, const guid_t& session) const
{ 
	// final key is a sum of communicator key & session crypt key
	integer first(crypt_private, (ub4_t)crypt_private.size());
	integer second(crypt_external, (ub4_t)crypt_external.size());
	integer third((const ub1_t*)&session, sizeof(guid_t));
	integer result;

	// it can be another algorithm
	result += first;
	result += second;
	result += third;

	crypt_private.reserve((ub4_t)result.min_encoded_size());
	result.encode(crypt_private, (ub4_t)crypt_private.size());
}

msg_connection* 
msg_communicator::_connect(const conf_connection& info_, ub8_t timeout)
{
	// checks type
	switch (info_._type)
	{
#if OS_TYPE == OS_WIN32
		case rpc:
			// establishes rpc connection
			return msg_rpc_connection::connect(_communicator, info_);
#endif
		case sock:
			// establishes socket connection
			return msg_sock_connection::connect(_communicator, info_);
		default: // unkown listener type
			assert(false);
	}

	return 0;
}

msg_connection* 
msg_communicator::find_connection(const guid_t& addr_)
{
	// locks mutex
	mutexKeeper keeper(_mtx_conn);
	// finds connection by the address
	connection_map_t::iterator iter = _connections.find(addr_);
	// returns result
	return iter != _connections.end() ? *iter : 0;
}

msg_listener* 
msg_communicator::find_listener(transport_type type_)
{
	// finds listener by address and type
	mutexKeeper keeper(_mtx_listeners);
	for (listener_list_t::iterator iter = _listeners.begin(); iter != _listeners.end(); ++iter)
		if ((*iter)->get_type() == type_)
			return *iter;

	return 0;
}

void 
msg_communicator::init(const char* info_, const char* ini_key_)
{
	xml_parser_creator nav_creator;
	xml_designer_keeper_t nav_keeper(nav_creator, 0);
	// parses xml
	// sets user credentials
	size_t ini_key_len = ini_key_ ? strlen(ini_key_) : 0;
#ifndef MSG_PRODUCTION
	ini_key_len = 0;
#endif

	if (ini_key_len)
	{
		room_byte_t ini_key(ini_key_len);
		ini_key.copy((const ub1_t*)ini_key_, ini_key_len);
		// loads to the memory and decrypt
		// ....
		size_t buf_len = 0;
		room_byte_t buf(buf_len);
		// then parses xml from memory
		// parses with dtd
		if (!nav_keeper->load(buf, buf.size(), (const void*)msg_xml_dtd, strlen(msg_xml_dtd)))
			exception::_throw(nav_keeper->error());
	}
	else
	{
		// parses with dtd
		if (!nav_keeper->load(info_, (const void*)msg_xml_dtd, strlen(msg_xml_dtd)))
			exception::_throw(nav_keeper->error());
	}
	
	nav_keeper->select_root();
	if (!nav_keeper->validate(true))
		exception::_throw(nav_keeper->error());

	init((xml_designer*)nav_keeper);
}

void 
msg_communicator::init(const void* info_, size_t len)
{
	xml_parser_creator nav_creator;
	xml_designer_keeper_t nav_keeper(nav_creator, 0);
	// parses xml
	// sets user credentials
	// parses with dtd
	if (!nav_keeper->load(info_, len, (const void*)msg_xml_dtd, strlen(msg_xml_dtd)))
		exception::_throw(nav_keeper->error());
	
	nav_keeper->select_root();
	if (!nav_keeper->validate(true))
		exception::_throw(nav_keeper->error());

	init((xml_designer*)nav_keeper);
}


void 
msg_communicator::init(xml_designer* nav)
{
	if (is_block()) // must be turn on
		exception::_throw("Communicator has been initialized");

	// sets nav to root
	if (!nav->select_root())
		exception::_throw("Invalid XML structure");
	// checks attributes
	if (!nav->has_attributes())
		exception::_throw("Invalid XML structure");

	// looks for address
	nav->select_attribute_by_name("address");
	if (!string_to_guid(_address, nav->get_value()))
		exception::_throw("Invalid address attribute format");

	// returns to parent
	nav->select_parent();


    // finds list of listeners and connections
	if (nav->has_children())
	{
		nav->select_first_child();
		// checks listeners/connections
		do
		{
			// checks element/ skip junk
			if (nav->get_type() != ELEMENT_NODE) continue;

			if (!strcmp(nav->get_name(), "listeners"))
			{
				// looks for children
				if (nav->has_children())
				{
					nav->select_first_child();

					do
					{
						if (nav->get_type() != ELEMENT_NODE) continue;

						conf_listener atom;
						atom._address = _address;
						parse_listener(nav, atom);
						add_listener_config(atom, false);
					}
					while (nav->select_next_sibling());
					// restores parent
					nav->select_parent();
				}
			}
			else // connections
			{
				// looks for children
				if (nav->has_children())
				{
					nav->select_first_child();
					do
					{
						if (nav->get_type() != ELEMENT_NODE) continue;

						// we are now at connection element
						conf_connection atom;
						parse_connection(nav, atom);
						add_connection_config(atom);
					}
					while (nav->select_next_sibling());
					// restore parent
					nav->select_parent();
				}
			}
		}
		while (nav->select_next_sibling());
	}

	// creates predefined listeners
	_create_listeners();

    // turns on communicator
	v_on();
}

void 
msg_communicator::add_connection_config(const conf_connection& atom)
{
	mutexKeeper keeper(_mtx_config);
	// checks dublicates
	for (msg_connection_list_t::const_iterator iter = _config._connections.begin(); iter != _config._connections.end(); ++iter)
		if (iter->_address == atom._address) // reject dublicate connections
			exception::_throw("Dublicate connection address");


	_config._connections.push_back(atom);
}

void
msg_communicator::add_listener_config(const conf_listener& atom, bool start)
{
	mutexKeeper keeper(_mtx_config);
	// checks dublicates
	for (msg_listener_list_t::const_iterator iter = _config._listeners.begin(); iter != _config._listeners.end(); ++iter)
		if (iter->_type == atom._type) // reject dublicate listeners
			exception::_throw("Dublicate listener type");

	// starts listener
	if (start)
		_create_listener(atom);

	_config._listeners.push_back(atom);
}

void
msg_communicator::remove_connection_config(const guid_t& address)
{
	mutexKeeper keeper(_mtx_config);
	// checks dublicates
	for (msg_connection_list_t::iterator iter = _config._connections.begin(); iter != _config._connections.end(); ++iter)
		if (iter->_address == address) // found connection
		{
			_config._connections.erase(iter);

			keeper.unlock();

			// shuts down connection
			msg_connection* conn = find_connection(address);
			if (conn)
				shutdown_connection(conn);

			break;
		}
}

void
msg_communicator::remove_listener_config(transport_type type)
{
	mutexKeeper keeper(_mtx_config);
	// checks dublicates
	for (msg_listener_list_t::iterator iter = _config._listeners.begin(); iter != _config._listeners.end(); ++iter)
		if (iter->_type == type) // found listener
		{
			_config._listeners.erase(iter);
			// stops and destroys listener
			mutexKeeper keeper(_mtx_listeners);
			for (listener_list_t::iterator iter_listener = _listeners.begin(); iter_listener != _listeners.end(); ++iter_listener)
			{
				if ((*iter_listener)->get_type() == type)
				{
					msg_listener* obj = *iter_listener;
					_listeners.erase(iter_listener);

					keeper.unlock();
					
					delete obj;
					break;
				} // if
			} // for
					
			break;
		} // if
}


void 
msg_communicator::uninit()
{
	// turns off communicator
	if (is_block() || !is_on())
		return;

	v_off();

	// clears configuration
	_config.clear();

	// destroys listeners
	_destroy_listeners();

	// closes all connections
	_destroy_connections();
}

// virtual 
void 
msg_communicator::v_on()
{
	if (is_on())
		return;

	_thread_manager.log_on(this);

	_register_this();

	_thread_manager.on();
	
	_aio_port.log_on(this);
	_aio_port.on();

	// base functionality
	msg_queue_processor::v_on();

	// unblocks queue
	unblock();

	// listeners on
	_turn_on_listeners();

	// connections on
	_turn_on_connections();

	// ping timer on
	_ping_timer.activate(this, 0, 1000);

	format_logging(0, __FILE__, __LINE__, en_log_info, "msg communicator started");
}

// virtual 
void 
msg_communicator::v_off()
{
	if (!is_on())
		return;

	// ping timer off
	_ping_timer.deactivate();

	// turns off listeners
	_turn_off_listeners();

	// turns off connections
	_turn_off_connections();

	// blocks queue
	block();

	// clears messages
	msg_cpp* msg = 0;
	while (pop(msg))
		destroy_msg(msg);

	// base functionality
	msg_queue_processor::v_off();

	_aio_port.off();
	_aio_port.log_on(0);


	_thread_manager.off();
	_thread_manager.log_on(0);
	
	format_logging(0, __FILE__, __LINE__, en_log_info, "msg communicator stooped");
}

void 
msg_communicator::_turn_off_listeners()
{
	// loop for all created listeners
	mutexKeeper keeper(_mtx_listeners);
	for (listener_list_t::iterator iter = _listeners.begin(); iter != _listeners.end(); ++iter)
	{
		(*iter)->off();
		(*iter)->log_on(0);
	}
}

void 
msg_communicator::_turn_on_listeners()
{
	// loop for all created listeners
	mutexKeeper keeper(_mtx_listeners);
	for (listener_list_t::iterator iter = _listeners.begin(); iter != _listeners.end(); ++iter)
	{
		(*iter)->log_on(this);
		(*iter)->on();
	}
}

void 
msg_communicator::_destroy_listeners()
{
	// loop for all created listeners
	mutexKeeper keeper(_mtx_listeners);
	for (listener_list_t::iterator iter = _listeners.begin(); iter != _listeners.end(); ++iter)
		delete *iter;

	_listeners.clear();
}

void 
msg_communicator::_create_listeners()
{
	// loop for predifined listener info
	mutexKeeper keeper(_mtx_config);
	for (msg_listener_list_t::const_iterator iter = _config._listeners.begin(); iter != _config._listeners.end(); ++iter)
		_create_listener(*iter);
}

void 
msg_communicator::_create_listener(const conf_listener& atom)
{
	mutexKeeper keeper(_mtx_listeners);
	switch (atom._type) // checks listener type
	{
		case sock: // creates socket listener
			// adds to the listener list
			_listeners.push_back(new msg_sock_listener(_communicator, atom));
			break;
#if OS_TYPE == OS_WIN32
		case rpc:
			// adds to the listener list
			_listeners.push_back(new msg_rpc_listener(_communicator, atom));
			break;
#endif
		default: // unknown listener type
			assert(false);
	}
}

void 
msg_communicator::_turn_off_connections()
{
	// locks mutex
	mutexKeeper keeper(_mtx_conn);
	// loop for all established connections
	for (connection_map_t::iterator iter = _connections.begin(); iter != _connections.end(); ++iter)
	{
		(*iter)->off();
	}
}

void 
msg_communicator::_turn_on_connections()
{
	// locks mutex
	mutexKeeper keeper(_mtx_conn);
	// loop for all established connections
	for (connection_map_t::iterator iter = _connections.begin(); iter != _connections.end(); ++iter)
	{
		(*iter)->on();
	}
}

void 
msg_communicator::_destroy_connections()
{
	// loop for all created listeners
	mutexKeeper keeper(_mtx_conn);
	for (connection_map_t::iterator iter = _connections.begin(); iter != _connections.end(); ++iter)
		delete *iter;

	_connections.clear();
}

// virtual 
void 
msg_communicator::notify(size_t ident, size_t interval, size_t multiplier)
{
	// locks mutex
	mutexKeeper keeper(_mtx_conn);
	for (connection_map_t::iterator iter = _connections.begin(); iter != _connections.end(); ++iter)
    	if (!(*iter)->is_block() && (*iter)->is_last_activity_timeout()) // if we find nonblocked connection
			(*iter)->ping_notify();
}

// virtual 
void 
msg_communicator::wakeup()
{
	_thread_manager.borrow_thread(0, 0, this, stay_on_alert_time);
}

bool 
msg_communicator::validate_connection(const msg_connection* connection_) const
{ 
	// locks mutex
	mutexKeeper keeper(_mtx_conn);
	connection_map_t::const_iterator first = _connections.begin();
	connection_map_t::const_iterator last = _connections.end();

	for (; first != last; ++first)
		if (connection_ == *first)
			return true;

	return false; 
}

void 
msg_communicator::add_connection(msg_connection* connection_)
{
	// locks mutex
	mutexKeeper keeper(_mtx_conn);
	// checks
	if (_connections.end() != _connections.find(connection_->get_info()._address))
	{
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);
	}

//	assert(connection_->is_connected());
	// inserts new connectiion
	_connections.insert(connection_->get_info()._address, connection_);
	// turns on new connection
	connection_->on();
}

void 
msg_communicator::change_connection_address(const guid_t& old_address, msg_connection* connection_)
{
	// locks mutex
	mutexKeeper keeper(_mtx_conn);
	// checks
	connection_map_t::iterator iter = _connections.find(old_address);
	assert(iter != _connections.end());
	_connections.erase(iter);
	
	if (_connections.end() != _connections.find(connection_->get_info()._address))
	{
		delete connection_;
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);
	}

	// inserts new connectiion
	_connections.insert(connection_->get_info()._address, connection_);
}

void 
msg_communicator::shutdown_connection(msg_connection* connection_)
{
	if (connection_->block()) // makes sure that connection is blocked
	{
		format_logging(0, __FILE__, __LINE__, en_log_info, "shutdown conection: %x", connection_);

		// tries to turn off connection
		connection_->off();
		// revokes from thread pool
		_thread_manager.revoke_client(connection_);
	
		msg_creator creator(this);
		msg_pointer_t msg_(creator);

		try
		{
			// constructs close connection message
			msg_ = construct_msg(0);	
			format_logging(0, __FILE__, __LINE__, en_log_info, "contruct message");

			msg_->_type = system_type;
			msg_->msgid = msg_id_shutdown;
			msg_->_receiver = _address;
			msg_->_sender = connection_->get_info()._address;
			msg_->priority = MSG_PRIORITY_SYSTEM;
			push(msg_);
			msg_.detach();
		}
		catch (exception&)
		{
			// if communicator is already blocked
			// then connection will be closed by communicator
		}
	}
}

void 
msg_communicator::_close_connection(const guid_t& addr)
{
	// tries to find connection
	mutexKeeper keeper(_mtx_conn);
	connection_map_t::iterator iter = _connections.find(addr);
	if (iter != _connections.end())
	{
		// deletes connection object
		delete (*iter);
		// removes connection from connection list
		_connections.erase(iter);
	}
}

void 
msg_communicator::comm_msg(msg_cpp* msg_)
{
	if (is_block()) // communicator queue is blocked
		exception::_throw("Communicator is blocked");

	// checks ping
	if (msg_->_type == system_type && msg_->msgid == msg_id_ping)
	{
		// destroys ping message
		destroy_msg(msg_);
		return;
	}

	// tries to redirect message
	// erases session id for security purpose
	msg_->_sessionid = null_uuid;
	// pushes message to the communicator queue
	push(msg_);
}

// virtual 
void 
msg_communicator::v_do_job(size_t ident, void* user_data)
{
	msg_cpp* msg = 0;
	if (!pop(msg))
	{
		assert(false);
		return; // nothing to do
	}

	msg_creator creator(this);
	msg_pointer_t msg_(creator);

	msg_ = msg;

	try
	{
		// checks connection shutdown
		// a special case
		if (msg_->msgid == msg_id_shutdown && msg_->_type == system_type)
		{
			_close_connection(msg_->_sender);
			return;
		}

		// locks mutex
		mutexKeeper keeper(_mtx_conn);
		// finds connection
		msg_connection* connection = 0;
		connection_map_t::iterator iter = _connections.find(msg_->_receiver);
		if (iter == _connections.end()) // not found
		{
			keeper.unlock();

			// tries to establish new connection
			mutexKeeper guard(_mtx_config);
			msg_connection_list_t::const_iterator conf_iter = _config._connections.begin();
			
			for (; conf_iter != _config._connections.end(); ++conf_iter)
			{
				if (conf_iter->_address == msg_->_receiver)
					break;
			}

			if (conf_iter != _config._connections.end())
			{
				conf_connection cinfo(*conf_iter);

				guard.unlock();
				// tries to establish connection through factory
				// adds connection to map
				connection = _connect(cinfo, __max(handshake_default_timeout, msg_->timeout));
			} //for

			keeper.lock();
		}
		else
			connection = *iter;
		
		if (!connection)
			exception::_throw(MSG_RESULT_UNKNOWN_DESTINATION, &msgMsgTable);

		// pushes message to the found connection queue
		connection->push_msg(msg_);
		msg_.detach();
	} // try
	catch (exception& x)
	{
		if (msg_->_type == user_type_send || msg_->_type == user_type_send_async)
		{
			// it can be, if config file is not correct
			try
			{
				msg_pointer_t err_(creator, 0);
				msg_pack::make_reply_msg(msg_, err_);
				msg_pack::make_error_msg(err_, x.what());

				push(err_);
				err_.detach();
			}
			catch (exception&)
			{
			}
		} //if
	} // catches
	catch (...)
	{
		assert(false);
	}
}

msg_cpp* 
msg_communicator::_pack_keys(const integer& n, const integer& e)
{
	ub4_t l_n = n.min_encoded_size();
	ub4_t l_e = e.min_encoded_size();

	msg_cpp* msg = construct_msg(l_n + l_e + 2 * sizeof(ub4_t));

	ub1_t* buf = msg->get_body();

	// puts the size in bytes of n key
	*(ub4_t*)buf = htonl(l_n);
	// shifts buffer pointer
	buf += sizeof(ub4_t);
	// puts the n key bytes
	n.encode(buf, l_n);
	// shifts buffer pointer
	buf += l_n;
	// puts the size in bytes of e key
	*(ub4_t*)buf = htonl(l_e);
	// shifts buffer pointer
	buf += sizeof(ub4_t);
	// puts the e key bytes
	e.encode(buf, l_e);

	return msg;
}

void 
msg_communicator::_extract_keys(const msg_cpp* msg, integer& n, integer& e)
{
	// extracts n, e keys
	const ub1_t* buf = msg->get_body();
	size_t size = msg->get_size();
	
	// checks minimum size
	if (size < 2 * (sizeof(ub4_t) + 1))
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);


	// gets the n key size
	ub4_t l_n = ntohl(*(ub4_t*)buf);
	
	if (size < l_n + 2 * sizeof(ub4_t) + 1)
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);

	// shifts buffer
	buf += sizeof(ub4_t);
	// parses n key
	n.decode(buf, l_n);
	// shifts buffer
	buf += l_n;

	// gets the e key size
	ub4_t l_e = ntohl(*(ub4_t*)buf);

	if (size != l_e + l_n + 2 * sizeof(ub4_t))
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);

	// shifts buffer
	buf += sizeof(ub4_t);
	// parses e key
	e.decode(buf, l_e);
}

msg_cpp* 
msg_communicator::construct_handshake(const rsa* rsa)
{
	msg_cpp* msg = rsa ? _pack_keys(rsa->get_n(), rsa->get_e()) : construct_msg(0);

	msg->priority = MSG_PRIORITY_SYSTEM;
	msg->_type = handshake_type; // handshake initiator
	msg->msgid = msg_id_handshake_request;
	msg->timeout = handshake_default_timeout; 
	msg->_sessionid = uuid_gen();
	msg->_sender = _address;
	return msg;
}

msg_cpp* 
msg_communicator::reply_handshake(const msg_cpp* msg_, room_byte_t* symetric_private_key_)
{
	if (msg_->_type != handshake_type // handshake reply
		|| msg_->msgid != msg_id_handshake_request // handshake reply
		|| msg_->_receiver != _address)
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);

	msg_cpp* reply = 0;
	
	if (symetric_private_key_)
	{
		integer n, e;
		_extract_keys(msg_, n, e);
		rsa rsa(e, n);
		reply = _generate_crypt_private_key(rsa, *symetric_private_key_);
	}
	else
	{
		reply = construct_msg(0);
	}

	// sets reply
	reply->_sender = _address;

	reply->_receiver = msg_->_sender;
	reply->priority = MSG_PRIORITY_SYSTEM;
	reply->_type = handshake_type;
	reply->msgid = msg_id_handshake_reply;
	reply->timeout = handshake_default_timeout;
	reply->_sessionid = msg_->_sessionid;

	return reply;
}

void 
msg_communicator::check_handshake(const guid_t& sessionid, const msg_cpp* reply_, const rsa* rsa, room_byte_t& symetric_private_key_)
{
	if (reply_->_type != handshake_type // handshake reply
		|| reply_->msgid != msg_id_handshake_reply // handshake reply
		//|| reply_->_destination != _address
		|| reply_->_receiver != _address
		|| reply_->_sessionid != sessionid)
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);

	if (rsa) 
		_decrypt_private_key(reply_, *rsa, symetric_private_key_); 
}

msg_cpp* 
msg_communicator::_generate_crypt_private_key(const rsa& rsa, room_byte_t& symetric_private_key_)
{
	// generates private symmetric key
	random_generator rng;
	integer in_key(rng, rsa_key_size / 16);
	symetric_private_key_.reserve(in_key.min_encoded_size());
	in_key.encode(symetric_private_key_, in_key.min_encoded_size());

	integer out_key;
	rsa.encode(in_key, out_key);
	ub4_t l_key = out_key.min_encoded_size();

	msg_cpp* msg = construct_msg(l_key + sizeof(ub4_t));

	ub1_t* buf = msg->get_body();
	*(ub4_t*)buf = htonl(l_key);
	out_key.encode(buf + sizeof(ub4_t), l_key);
	
	return msg;
}

void
msg_communicator::_decrypt_private_key(const msg_cpp* reply_, const rsa& rsa, room_byte_t& symetric_private_key_)
{
	const ub1_t* buf_reply = reply_->get_body();
	size_t size = reply_->get_size();

	if (size < sizeof(ub4_t) + 1)
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);

	ub4_t l_key = ntohl(*(ub4_t*)buf_reply);

	if (size != l_key + sizeof(ub4_t))
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);

	buf_reply += sizeof(ub4_t);

	integer in_key, out_key;
	in_key.decode(buf_reply, l_key);
	rsa.decode(in_key, out_key);
	symetric_private_key_.reserve(out_key.min_encoded_size());
	out_key.encode(symetric_private_key_, out_key.min_encoded_size());
}

msg_cpp* 
msg_communicator::construct_ping()
{
	msg_cpp* msg = construct_msg(0);

	msg->_type = system_type; // ping initiator
	msg->msgid = msg_id_ping;
	msg->_sender = _address;
	return msg;
}

// gets aio manager
aiosock& 
msg_communicator::get_aiosock()
{
	return _aio_port;
}

// gets thread manager
threadpool& 
msg_communicator::get_thread_manager()
{
	return _thread_manager;
}

// gets event pool
event_pool_t&
msg_communicator::get_event_pool()
{
	return _event_pool;
}

void 
msg_communicator::doxray()
{
	mutexKeeper lguard(_mtx_listeners);
	size_t listeners = _listeners.size();
	lguard.unlock();

	mutexKeeper cguard(_mtx_conn);
	size_t connections = _connections.size();
	cguard.unlock();

	size_t free_all = 0, busy_all = 0; 
	_manager.get_stats(free_all, busy_all);
	size_t free_ev = 0, busy_ev = 0; 
	_event_pool.get_stats(free_ev, busy_ev);

	format_logging(0, __FILE__, __LINE__, en_log_xray, "<aiomsg listeners=\"%d\" connections = \"%d\" free_all = \"%d\" busy_all = \"%d\" free_ev = \"%d\" busy_ev = \"%d\" />",
		listeners, connections, free_all, busy_all, free_ev, busy_ev);

	_thread_manager.doxray();
	_aio_port.doxray();
}

void
msg_communicator::log_msg(const msg_cpp* msg)
{
	date timestamp(msg->_timestamp);
	char sbuf[sizeof(guid_t)*2 + 1];
	char rbuf[sizeof(guid_t)*2 + 1];
	char tbuf[25]; // YYYY-MM-DD HH:MM:SS.MMMM - 24

#if OS_TYPE == OS_WIN32
	format_logging(0, __FILE__, __LINE__, en_log_info, "msgid=%d majver=%d minver=%d sender=%s receiver=%s timeout=%I64u, size=%u timestamp=%s", 
#else
	format_logging(0, __FILE__, __LINE__, en_log_info, "msgid=%d majver=%d minver=%d sender=%s receiver=%s timeout=%llu, size=%u timestamp=%s", 
#endif

	msg->msgid, msg->majver, msg->minver,
	guid_to_string(sbuf, msg->_sender),
	guid_to_string(rbuf, msg->_receiver),
	msg->timeout,
	msg->get_size(),
	timestamp.get_date(tbuf)
	);
}

/////////////////////////////////////////////////
// static 
void 
msg_communicator::parse_connection(xml_designer* nav, conf_connection& atom)
{
	
	// type CTYPE %kind; CDATA #REQUIRED
	// address CTYPE vt_guid CDATA #REQUIRED
	// port CTYPE vt_uint16 CDATA #IMPLIED
	// network CDATA #IMPLIED
	// ping CTYPE vt_uint32 CDATA #IMPLIED
	// waste CTYPE vt_ub4 #IMPLIED
	// info CDATA #IMPLIED
	// security CTYPE vt_bool %bool; #IMPLIED
	// password CDATA #IMPLIED


	// looks for attributes				
	nav->select_attribute_by_name("type");
	if (!str_template::strcmp(nav->get_value(), "rpc", os_minus_one))
		atom._type = rpc;
	else if (!str_template::strcmp(nav->get_value(), "sock", os_minus_one))
		atom._type = sock;
//	else if (!str_template::strcmp(nav->get_value(), "p2p"))
//		atom._type = p2p;

	nav->select_parent();

	nav->select_attribute_by_name("address");
	if (!string_to_guid(atom._address, nav->get_value()))
		exception::_throw("Invalid parent attribute format");
	nav->select_parent();

	if (nav->select_attribute_by_name("port"))
	{
		str_template::strscan(nav->get_value(), 32, "%hu", &atom._port);
			nav->select_parent();
	}
	else
		atom._port = 3337;

	if (nav->select_attribute_by_name("network"))
	{
		atom._network = nav->get_value();
		nav->select_parent();
	}
	else
		atom._network = "localhost";

	if (nav->select_attribute_by_name("ping"))
	{
		str_template::strscan(nav->get_value(), 32, "%u", &atom._ping);
		nav->select_parent();
	}
	else
		atom._ping = 1000;


	if (nav->select_attribute_by_name("info"))
	{
		atom._info = nav->get_value();
		nav->select_parent();
	}

	if (nav->select_attribute_by_name("security"))
	{
		atom._support_crypt = strcmp(nav->get_value(), "1") == 0;
		nav->select_parent();
		// gets password because security is specified
		if (atom._support_crypt && nav->select_attribute_by_name("password"))
		{
			const char* password = nav->get_value();
			size_t len = password ? strlen(password) : 0;
			if (len)
				atom._crypt_external.reserve(len).copy((const ub1_t*)password, len);
			
			nav->select_parent();
		}
	}
	else
		atom._support_crypt = false;

#ifndef MSG_PRODUCTION
	atom._support_crypt = false;
#endif
}


// static 
void 
msg_communicator::parse_listener(xml_designer* nav, conf_listener& atom)
{
	// type CTYPE %kind; #REQUIRED
	// port CTYPE vt_uint16 #IMPLIED
	// network CDATA #IMPLIED
	// connections CTYPE vt_uint32 #IMPLIED
	// ping CTYPE vt_uint32 #IMPLIED
	// waste CTYPE vt_ub4 #IMPLIED
	// info CDATA #IMPLIED
	// security CTYPE vt_bool %bool; #IMPLIED
	// password CDATA #IMPLIED

	// looks for attributes				
	nav->select_attribute_by_name("type");
	if (!str_template::strcmp(nav->get_value(), "rpc", os_minus_one))
		atom._type = rpc;
	else if (!str_template::strcmp(nav->get_value(), "sock", os_minus_one))
		atom._type = sock;
//	else if (!str_template::strcmp(nav->get_value(), "p2p"))
//		atom._type = p2p;

	nav->select_parent();

	if (nav->select_attribute_by_name("port"))
	{
		str_template::strscan(nav->get_value(), 32, "%hu", &atom._port);
		nav->select_parent();
	}
	else
		atom._port = 0;

	if (nav->select_attribute_by_name("network"))
	{
		atom._network = nav->get_value();
		nav->select_parent();
	}
	else
		atom._network = "localhost";

	if (nav->select_attribute_by_name("connections"))
	{
		str_template::strscan(nav->get_value(), 32, "%u", &atom._connections);
		if (!atom._connections)
			atom._connections = 1;
		nav->select_parent();
	}

	if (nav->select_attribute_by_name("ping"))
	{
		str_template::strscan(nav->get_value(), 32, "%u", &atom._ping);
		nav->select_parent();
	}
	else
		atom._ping = 1000;

	if (nav->select_attribute_by_name("info"))
	{
		atom._info = nav->get_value();
		nav->select_parent();
	}

	if (nav->select_attribute_by_name("security"))
	{
		atom._support_crypt = strcmp(nav->get_value(), "1") == 0;
		nav->select_parent();
		// gets password because security is specified
		if (atom._support_crypt && nav->select_attribute_by_name("password"))
		{
			const char* password = nav->get_value();
			size_t len = password ? strlen(password) : 0;
			if (len)
				atom._crypt_accept.reserve(len).copy((const ub1_t*)password, len);
			
			nav->select_parent();
		}
	}
	else
		atom._support_crypt = false;


	// looks for accept/reject peers
	// looks for children
	if (nav->has_children())
	{
		nav->select_first_child();
		do
		{
			// all accept/reject elements
			if (nav->get_type() != ELEMENT_NODE) continue;
			bool accept = strcmp(nav->get_name(), "accept") == 0;

			// looks for all peer
			if (nav->select_first_child())
			{
				do
				{
					if (nav->get_type() != ELEMENT_NODE) continue;
					//address CTYPE vt_guid #REQUIRED
					//security CTYPE vt_bool #IMPLIED
					//password CDATA #IMPLIED >
					conf_peer peer;

					nav->select_attribute_by_name("address");
					if (!string_to_guid(peer._address, nav->get_value()))
						exception::_throw("Invalid peer address format");
					nav->select_parent();

					if (nav->select_attribute_by_name("security"))
					{
						peer._support_crypt = strcmp(nav->get_value(), "1") == 0;
						nav->select_parent();
						// get password because security is specified
						if (peer._support_crypt && nav->select_attribute_by_name("password"))
						{
							const char* password = nav->get_value();
							size_t len = password ? strlen(password) : 0;
							if (len)
								peer._crypt.reserve(len).copy((const ub1_t*)password, len);
							
							nav->select_parent();
						}
					}
					else if (accept)// gets settings from listener
					{
						peer._support_crypt = atom._support_crypt;
						peer._crypt = atom._crypt_accept;
					}

#ifndef MSG_PRODUCTION
					peer._support_crypt = false;
#endif
					accept ? atom._accept.push_back(peer) : atom._reject.push_back(peer);

				} while (nav->select_next_sibling());

				// restores parent
				nav->select_parent();
			} // if all peers
		} while (nav->select_next_sibling());
		
		nav->select_parent();
	} // if accept/reject


#ifndef MSG_PRODUCTION
	atom._support_crypt = false;
#endif
}

#pragma pack()
END_TERIMBER_NAMESPACE
