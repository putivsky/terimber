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

#include "aiomsg/msgimpl.h"
#include "base/except.h"
#include "base/string.hpp"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/memory.hpp"
#include "base/template.hpp"
#include "base/number.hpp"
#include "base/common.hpp"

#include "xml/xmlimpl.hpp"
#include "xml/sxs.hpp"

#include "aiomsg/msg_user.h"

const char* msg_connection_dtd = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<!ENTITY % kind \"(rpc | sock | http | p2p)\"> \
<!ELEMENT connection EMPTY> \
<!ATTLIST connection \
type %kind; #REQUIRED \
address CTYPE vt_guid #REQUIRED \
port CTYPE vt_ub2 #IMPLIED \
network CDATA #IMPLIED \
ping CTYPE vt_ub4 #IMPLIED \
info CDATA #IMPLIED \
security CTYPE vt_bool #IMPLIED \
password CDATA #IMPLIED >";

const char* msg_listener_dtd = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<!ELEMENT listener (accept | reject)?> \
<!ELEMENT reject (peer)+> \
<!ELEMENT accept (peer)+> \
<!ELEMENT peer EMPTY> \
<!ATTLIST peer \
address CTYPE vt_guid #REQUIRED \
security CTYPE vt_bool #IMPLIED \
password CDATA #IMPLIED > \
<!ATTLIST listener \
port CTYPE vt_ub2 #IMPLIED \
network CDATA #IMPLIED \
connections CTYPE vt_ub4 #IMPLIED \
ping CTYPE vt_ub4 #IMPLIED \
info CDATA #IMPLIED \
security CTYPE vt_bool #IMPLIED \
password CDATA #IMPLIED >";

aiomsgfactory::aiomsgfactory()
{
}

// virtual
aiomsgfactory::~aiomsgfactory()
{
}

terimber_aiomsg*
aiomsgfactory::get_aiomsg(terimber_log* log)
{
	terimber::aiomsg* obj = new terimber::aiomsg();
	if (obj)
		obj->log_on(log);

	return obj;
}



BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

///////////////////////////////////////////////////
aiomsg::aiomsg() : _user_conn(0)
{
}

// 
aiomsg::~aiomsg() 
{
	stop();
}

bool 
aiomsg::init(const char* info, const char* ini_key)
{
	try
	{
        _communicator.init(info, ini_key);
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error = x.what();
		return false;
	}

	return true;
}

bool 
aiomsg::init(const void* buffer, size_t len)
{
	// parses

	try
	{
		// parses
		_communicator.log_on(this);
        _communicator.init(buffer, len);
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error = x.what();
		return false;
	}

	return true;
}

bool
aiomsg::uninit() 
{
	try
	{
		stop();
		_communicator.uninit();
		_communicator.log_on(0);
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error = x.what();
		return false;
	}

	return true;
}

bool 
aiomsg::start(msg_callback_notify* callback, size_t additional_thread_count) 
{
	// gets address info
	conf_connection atom;
	atom._ping = 1000; // checks every second
	atom._address = _communicator.get_address();
	try
	{
		check_on();
		stop();
		_user_conn = msg_user_connection::connect(&_communicator, callback, atom, additional_thread_count);
		_user_conn->on();
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error = x.what();
		return false;
	}

	return true;
}

bool
aiomsg::stop() 
{

	try
	{
		//check_on();
		if (_user_conn)
		{
			_user_conn->off();
			_communicator.shutdown_connection(_user_conn);
			_user_conn = 0;
		}
		return true;
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error = x.what();
		return false;
	}

	return true;
}

//! port error function
const char*
aiomsg::get_port_error() const 
{
	return _error; 
}

const guid_t&
aiomsg::get_port_address() const 
{
	return _communicator.get_address();
}

msg_t*
aiomsg::construct(size_t size) 
{
	try
	{
		check_on();
		return _communicator.construct_msg(size);
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return 0;
	}
}

bool	
aiomsg::resize(msg_t* msg, size_t size) 
{
	try
	{
		check_on();
		return _communicator.resize_msg(_communicator.cast(msg), size); 
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}
}

bool	
aiomsg::destroy(msg_t* msg) 
{
	try
	{
		check_on();
		return _communicator.destroy_msg(_communicator.cast(msg)); 
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}
}

size_t
aiomsg::get_size(const msg_t* msg) const 
{
	try
	{
		check_on();
		const msg_cpp* msg_ = msg_communicator::cast(msg);
		return msg_ ? msg_->get_size() : 0;
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return 0;
	}
}

bool 
aiomsg::write_buffer(msg_t* msg, size_t offset, const void* buf, size_t len)
{
	try
	{
		check_on();
		msg_cpp* msg_ = msg_communicator::cast(msg);
		if (!msg_ || msg_->get_size() < offset + len)
			return false;
		
		memcpy(msg_->get_body() + offset, buf, len);
		return true;
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return 0;
	}
}

const void*
aiomsg::get_buffer(const msg_t* msg) const 
{
	try
	{
		check_on();
		const msg_cpp* msg_ = msg_communicator::cast(msg);
		return msg_ ? msg_->get_body() : 0;
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return 0;
	}
}

bool	
aiomsg::set_receiver(msg_t* msg, const guid_t& receiver) 
{
	try
	{
		check_on();
		return msg_communicator::set_receiver(msg_communicator::cast(msg), receiver); 
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}
}

bool 
aiomsg::get_sender(const msg_t* msg, guid_t& sender) const 
{
	try
	{
		check_on();
		return msg_communicator::get_sender(msg_communicator::cast(msg), sender); 
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}
}

bool	
aiomsg::send(bool copy, msg_t* msg, msg_t** reply) 
{
	try
	{
		check_on();
		check_ident();
 		msg_cpp* reply_ = 0;

		if (!_user_conn->send(copy, msg_communicator::cast(msg), reply_))
			terimber::exception::_throw(_user_conn->get_last_error());

		// set result
		if (reply) 
			*reply = static_cast< msg_t* >(reply_);
		else if (reply_)
			_communicator.destroy_msg(reply_);

	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}

	return true;
}

guid_t
aiomsg::send_async(bool copy, msg_t* msg) 
{
	guid_t retVal;
	try
	{
		check_on();
		check_ident();

		if (null_uuid == (retVal = _user_conn->send_async(copy, _communicator.cast(msg))))
			terimber::exception::_throw(_user_conn->get_last_error());
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
	}

	return retVal;
}

bool	
aiomsg::post(bool copy, msg_t* msg) 
{
	try
	{
		check_on();
		check_ident();

		if (!_user_conn->post(copy, msg_communicator::cast(msg)))
			terimber::exception::_throw(_user_conn->get_last_error());
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}

	return true;
}

bool 
aiomsg::add_connection(const char* xml_description)
{
	if (!xml_description)
	{
		_error = "Null pointer provided";
		return false;
	}

	// parse xml_descrition
	xml_parser_creator nav_creator;
	xml_designer_keeper_t nav_keeper(nav_creator, 0);
	if (!nav_keeper->load(xml_description, strlen(xml_description), (const void*)msg_connection_dtd, strlen(msg_connection_dtd)))
	{
		_error = nav_keeper->error();
		return false;
	}


	try
	{
		check_on();

		// skips junk
		nav_keeper->select_root();
		conf_connection atom;
		// parses connection
		msg_communicator::parse_connection(nav_keeper, atom);
		_communicator.add_connection_config(atom);
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}

	return true;
}

bool 
aiomsg::add_listener(const char* xml_description)
{
	if (!xml_description)
	{
		_error = "Null pointer provided";
		return false;
	}

	// parses xml_descrition
	xml_parser_creator nav_creator;
	xml_designer_keeper_t nav_keeper(nav_creator, 0);
	if (!nav_keeper->load(xml_description, strlen(xml_description), (const void*)msg_listener_dtd, strlen(msg_listener_dtd)))
	{
		_error = nav_keeper->error();
		return false;
	}


	try
	{
		check_on();

		// skips junk
		nav_keeper->select_root();
		conf_listener atom;
		// parses connection
		msg_communicator::parse_listener(nav_keeper, atom);
		// adds and starts
		_communicator.add_listener_config(atom, true);
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}

	return true; 
}


// to remove connection dynamically
// @address - address of connection
bool 
aiomsg::remove_connection(const guid_t& address)
{
	try
	{
		check_on();
		_communicator.remove_connection_config(address);
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}
	return true;
}

// to remove listener dynamically
// @address - address of connection
bool 
aiomsg::remove_listener(const char* type)
{
	if (!type)
	{
		_error = "Null pointer provided";
		return false;
	}

	try
	{
		check_on();
		transport_type type_ = sock;
		_communicator.remove_listener_config(type_);
	}
	catch (terimber::exception& x)
	{
		mutexKeeper keeper(_mtx);
		_error= x.what();
		return false;
	}
	return true;
}

////////////////////////////////////////////
void 
aiomsg::check_on() const
{
	if (!_communicator.is_on())
		terimber::exception::_throw("Communicator is not initialized");
}

void 
aiomsg::check_ident() const
{
	if (!_communicator.validate_connection(_user_conn))
		terimber::exception::_throw("Connection not found");
}

//! \brief does xray
//virtual
void
aiomsg::doxray()
{
	_communicator.doxray();
}


#pragma pack()
END_TERIMBER_NAMESPACE
