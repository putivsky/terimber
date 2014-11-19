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


#ifndef _terimber_msg_conn_h_
#define _terimber_msg_conn_h_

#include "aiomsg/msg_cpp.h"
#include "aiomsg/msg_comm.h"
#include "base/primitives.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \brief messages queue thread min ident
static const size_t queue_thread_ident = 255;

//! \class msg_connection
//! \brief base class for all types of connections
class msg_connection : public msg_queue_processor
{
protected:
	//! \enum msg_conn_state
	enum msg_conn_state
	{
		CONN_STATE_CLOSE,									//!< close state
		CONN_STATE_HANDSHAKE_INITIATOR,						//!< connection established, handshake in progress
		CONN_STATE_HANDSHAKE_RECEIVER,						//!< connection accepted, handshake in progress
		CONN_STATE_CONNECTED								//!< connection is ready for sending/receiving messages
	};
public:
	//! \brief constructor for receiver
	msg_connection(	msg_communicator* communicator,			//!< pointer to communicator 
					const conf_listener& linfo,				//!< listener acceptence settings
					const conf_connection& info				//!< connection info
					);
	//! \brief constructor for initiator
	msg_connection(	msg_communicator* communicator,			//!< pointer to communicator 
					const conf_connection& info				//!< connection info
					);
	//! \brief destructor
	virtual ~msg_connection();
	//! \brief before pushing message into connection queue
	//! assigns the session id of this connection
	virtual 
	void 
	push_msg(		msg_cpp* msg							//!< message pointer
					);
	//! \brief returns private symmetric crypt key
	inline 
	const room_byte_t* 
	get_crypt_key() 
	{ 
		return _info._support_crypt && _info._crypt_private.size() > 2 ? &_info._crypt_private : 0; 
	}
	//! \brief refreshes the last activity date
	inline 
	void 
	set_last_activity() 
	{ 
		mutexKeeper keeper(_mtx_res); 
		_last_activity = date(); 
	}
	//! \brief checks if it's time for ping
	inline 
	bool 
	is_last_activity_timeout() 
	{ 
		mutexKeeper keeper(_mtx_res); 
		return _last_activity.is_time_over(_info._ping); 
	}
	//! \brief inserts ping message into output queue
	virtual 
	void 
	ping_notify();
	//! \brief returns the connection info
	inline 
	const conf_connection& 
	get_info() 
	{ 
		return _info; 
	}

	//! \brief returns RSA object pointer (optional)
	const rsa* get_rsa() const
	{
		return _info._support_crypt ? &_rsa : 0;
	}

	bool
	is_connected() const
	{
		return CONN_STATE_CONNECTED == _state;
	}

	//! \brief prepares handshake message as a initiator
	msg_cpp* 
	prepare_handshake_msg();

	//! \brief prepares handshake reply as a receiver
	msg_cpp* 
	prepare_handshake_reply(msg_cpp* msg);

	//! \brief validates handshake reply as a initiator
	void 
	validate_handshake_reply(msg_cpp* msg);

	void 
	process_incoming_message(msg_cpp* msg					//!< pointer to message
						);

protected:
	//! \brief overrides the base functionality
	virtual 
	void 
	wakeup();

protected:
	conf_connection		_info;								//!< connection info
	conf_listener		_linfo;								//!< listener acceptence settings
	msg_conn_state		_state;								//!< connection state
	rsa					_rsa;								//!< crypto object
private:
	date				_last_activity;						//!< keep last activity date
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_msg_comm_h_

