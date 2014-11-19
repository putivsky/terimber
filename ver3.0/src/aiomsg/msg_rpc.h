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

#ifndef _terimber_msg_rpc_h_
#define _terimber_msg_rpc_h_

#include "msg_conn.h"
#include "msg_lsnr.h"

#if OS_TYPE == OS_WIN32
#include <dce_port.h>
#else
#include <rpc/types.h>
#endif

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)


//! \class msg_rpc_connection
//! \brief classes implement the RPC protocol communication
class msg_rpc_connection : public msg_connection
{
public:
	//! \brief constructor for receiver
	msg_rpc_connection(msg_communicator* communicator_,		//!< communicator
					rpc_binding_handle_t handle_,			//!< rpc handle
					const conf_listener& linfo,				//!< listener info
					const conf_connection& info				//!< connection info
					);

	//! \brief constructor for initiator
	msg_rpc_connection(msg_communicator* communicator_,		//!< communicator
					rpc_binding_handle_t handle_,			//!< rpc handle
					const conf_connection& info				//!< connection info
					);

	//! \brief destructor
	virtual 
	~msg_rpc_connection();
	//! \brief establishes RPC connection due to info
	static 
	msg_connection* 
	connect(		msg_communicator* communicator_,		//!< communicator
					const conf_connection& info				//!< connection info
					);
	//! \brief send RPC message to the peer
	static 
	void 
	send_msg(		msg_communicator* communicator_,		//!< communicator
					msg_rpc_connection* _this,				//!< rpc connection
					rpc_binding_handle_t handle_,			//!< rpc handle
					msg_cpp* msg_							//!< message
					);
protected:
	//! \brief override the base functionality
	virtual 
	void 
	v_do_job(		size_t ident,							//!< thread ident
					void* user_data							//!< user data
					);
	
private:
	rpc_binding_handle_t _handle;							//!< keeps the RPC handle for connection
};

//! \class msg_rpc_listener
//! \brief RPC listener implementation
class msg_rpc_listener : public msg_listener
{
	//! \typedef rpc_listener_map_t
	//! \brief maps guid to rpc listener
	typedef map< guid_t, msg_rpc_listener* > rpc_listener_map_t;
public:
	//! \brief constructor
	msg_rpc_listener(msg_communicator* communicator_,		//!< communicator
					const conf_listener& info_				//!< listener info
					);
	//! \brief destructor
	virtual 
	~msg_rpc_listener();
	//! \brief returns the RPC listener type
	virtual 
	transport_type 
	get_type() const 
	{ return rpc; }
protected:
	//! \brief override activate/deactivate functions
	virtual void v_on();
	virtual void v_off();
};

//! \class msg_rpc_global
//! \brief initializes the glogal RPC runtime environmental
class msg_rpc_global
{
public:
	//! \brief constuctor
	msg_rpc_global();
	//! \brief destructor
	~msg_rpc_global();
	//! \brief common global initialization
	void 
	init(			size_t connections						//!< max incoming connections
					);
	
private:
	//! \brief clear global initialization
	void 
	_undo();
	
	bool _done;												//!< the current gloabal state
	bool _rpcStarter;										//< the current RPC starter
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_msg_rpc_h_

