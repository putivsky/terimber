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

#include "msg_rpc.h"
#include "imsg.h"

#include "base/string.hpp"
#include "base/list.hpp"
#include "base/memory.hpp"
#include "base/template.hpp"
#include "base/common.hpp"
#include "base/except.h"
#include "base/number.hpp"


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

extern exception_table msgMsgTable;

#define prc_s_duplicate_endpoint RPC_S_DUPLICATE_ENDPOINT
// description of the base message results
exception_item rpcMsgs[] =
{
	{ rpc_s_ok,							"Action succeeded" },
	{ ept_s_cant_perform_op,			"The server endpoint cannot perform the operation" },
	{ ept_s_invalid_entry,				"The entry is invalid" },
	{ ept_s_not_registered,				"There are no more endpoints available from the endpoint mapper" },
	{ rpc_s_already_listening,			"The RPC server is already listening" },
	{ rpc_s_already_registered,			"The object universal unique identifier (UUID) has already been registered" },
	{ rpc_s_binding_has_no_auth,		"The binding does not contain any authentication information" },
//	{ rpc_s_binding_imcomplete,			"RPC binding incomplete" },
	{ rpc_s_call_cancelled,				"The remote procedure call was cancelled" },
	{ rpc_s_call_failed,				"The remote procedure call failed" },
//	{ rpc_s_cant_bind_socket,			"RPC can not bind socket" },
	{ rpc_s_cant_create_socket,			"The endpoint cannot be created" },
	{ rpc_s_comm_failure,				"A communications failure occurred during a remote procedure call" },
	{ rpc_s_connect_no_resources,		"Not enough resources are available to complete this operation" },
	{ rpc_s_cthread_create_failed,		"RPC can not open more threads" },
	{ rpc_s_endpoint_not_found,			"No endpoint was found" },
	{ rpc_s_entry_already_exists,		"The entry already exists" },
	{ rpc_s_entry_not_found,			"The entry is not found" },
	{ rpc_s_fault_addr_error,			"An addressing error occurred in the RPC server" },
	{ rpc_s_fault_fp_div_by_zero,		"A floating-point operation at the RPC server caused a division by zero" },
	{ rpc_s_fault_fp_overflow,			"A floating-point overflow occurred at the RPC server" },
	{ rpc_s_fault_fp_underflow,			"A floating-point underflow occurred at the RPC server" },
	{ rpc_s_fault_int_div_by_zero,		"The RPC server attempted an integer division by zero" },
	{ rpc_s_fault_invalid_bound,		"The array bounds are invalid" },
	{ rpc_s_fault_invalid_tag,			"The tag is invalid" },
	{ rpc_s_fault_remote_no_memory,		"RPC server out of memory" },
	{ rpc_s_fault_unspec,				"The remote procedure call failed" },
	{ rpc_s_incomplete_name,			"The entry name is incomplete" },
	{ rpc_s_interface_not_found,		"The interface was not found" },
	{ rpc_s_internal_error,				"An internal error occurred in a remote procedure call (RPC)" },
	{ rpc_s_inval_net_addr,				"The network address is invalid" },
	{ rpc_s_invalid_arg,				"Invalid argument specified" },
	{ rpc_s_invalid_binding,			"The binding handle is invalid" },
	{ rpc_s_invalid_endpoint_format,	"The endpoint format is invalid" },
//	{ rpc_s_invalid_naf_id,				"Invalid NAF id" },
	{ rpc_s_invalid_name_syntax,		"The name syntax is invalid" },
	{ rpc_s_invalid_rpc_protseq,		"The RPC protocol sequence is invalid" },
	{ rpc_s_invalid_string_binding,		"The string binding is invalid" },
	{ rpc_s_invalid_vers_option,		"The version option is invalid" },
	{ rpc_s_max_calls_too_small,		"The maximum number of calls is too small" },
	{ rpc_s_mgmt_op_disallowed,			"Access denied" },
	{ rpc_s_name_service_unavailable,	"The name service is unavailable" },
	{ rpc_s_no_bindings,				"There are no bindings" },
	{ rpc_s_no_entry_name,				"The binding does not contain an entry name" },
	{ rpc_s_no_interfaces,				"No interfaces have been registered" },
//	{ rpc_s_no_interfaces_exported,		"No interfaces have been exported" },
	{ rpc_s_no_memory,					"Not enough memory" },
	{ rpc_s_no_more_elements,			"The list of RPC servers available for the binding of auto handles has been exhausted" },
	{ rpc_s_no_more_bindings,			"There are no more bindings" },
	{ rpc_s_no_more_members,			"There are no more members" },
	{ rpc_s_no_ns_permission,			"Access denied" },
	{ rpc_s_no_princ_name,				"No principal name registered" },
	{ rpc_s_no_protseqs,				"There are no protocol sequences" },
	{ rpc_s_no_protseqs_registered,		"No protocol sequences have been registered" },
	{ rpc_s_not_rpc_tower,				"The requested operation is not supported" },
	{ rpc_s_not_supported,				"No security context is available to allow impersonation" },
	{ rpc_s_not_authorized,				"Access denied" },
//	{ rpc_s_nothing_to_unexport,		"Nothing unexport" },
	{ rpc_s_object_not_found,			"The object universal unique identifier (UUID) was not found" },
	{ rpc_s_protocol_error,				"A remote procedure call (RPC) protocol error occurred" },
	{ rpc_s_protseq_not_supported,		"The RPC protocol sequence is not supported" },
	{ rpc_s_server_too_busy,			"The RPC server is too busy to complete this operation" },
	{ rpc_s_string_too_long,			"The string is too long" },
	{ rpc_s_type_already_registered,	"The type universal unique identifier (UUID) has already been registered" },
	{ rpc_s_unknown_authn_service,		"The authentication service is unknown" },
	{ rpc_s_unknown_authz_service,		"The authorization service is unknown" },
	{ rpc_s_unknown_if,					"The interface is unknown" },
	{ rpc_s_unknown_mgr_type,			"The manager type is unknown" },
	{ rpc_s_unknown_reject,				"The remote procedure call failed and did not execute" },
	{ rpc_s_unsupported_name_syntax,	"The name syntax is not supported" },
	{ rpc_s_unsupported_type,			"The universal unique identifier (UUID) type is not supported" },
	{ rpc_s_wrong_boot_time,			"The remote procedure call failed and did not execute" },
	{ rpc_s_wrong_kind_of_binding,		"The binding handle is not the correct type" },
	{ prc_s_duplicate_endpoint,			"The duplicate endpoint" },
	{ 0,								0 }
};

// global base messages table
exception_table rpcMsgTable(rpcMsgs);

static
inline
ub1_t* uustring(char* x, size_t len, const guid_t& y)
{
	str_template::strprint(x, len, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x", y.Data1, y.Data2, y.Data3,
		y.Data4[0], y.Data4[1], y.Data4[2], y.Data4[3], y.Data4[4], y.Data4[5], y.Data4[6], y.Data4[7]);
	return (ub1_t*)x;
}


// we support only local RPC protocol
static const char* g_protocol = "ncalrpc";
static const char* g_protocol_ = "ncalrpc:";
// analyze status
static void rpcStatus(int status_)
{
	if (status_ != rpc_s_ok) // if status is not RPC_S_OK throw exception as Windows error
		exception::_throw(status_, (exception_table*)&rpcMsgTable);
}

static const IMSG_SERVER_EPV g_epv = {ServerConnect, ServerPostBlock};
static char g_name[300];
//static const size_t max_call_requests = 1024*8;
// define global RPC object
msg_rpc_global g_rpc_global;
/////////////////////////////////////////////////////////////////
msg_rpc_global::msg_rpc_global() : _done(false), _rpcStarter(false)
{
}

void 
msg_rpc_global::init(size_t connections)
{
	if (_done) // do only once
		return;

	int status = 0;

	// get computer name here
	str_template::strcpy(g_name, g_protocol_, os_minus_one);
	if (gethostname(g_name + strlen(g_protocol_), 255))
		exception::_throw("Unknown computer name");

	// register protocol sequence
	rpc_server_use_protseq((ub1_t*)g_protocol, (int)connections, &status);
    rpcStatus(status);
	
	// turn RPC runtime library to the listen state
	rpc_server_listen((int)RPC_C_LISTEN_MAX_CALLS_DEFAULT, &status);
	if (status != rpc_s_already_listening) // can be a part of another program
	{
		rpcStatus(status);
		_rpcStarter = true;
	}
	else
		_rpcStarter = false;

	_done = true;
}

void 
msg_rpc_global::_undo()
{
	size_t status = 0;

	// stop global RPC listening
	if (_rpcStarter)
	{
		// stop listening
		rpc_mgmt_stop_server_listening(0, &status);
	}

	_done = false;
}

msg_rpc_global::~msg_rpc_global()
{
	if (_done)
		_undo();
}

///////////////////////////////////
// static communicator keeper
class rpc2_communicator_keeper
{
public:
	rpc2_communicator_keeper(const guid_t& addr) : _address(addr) { _communicator = msg_communicator::loan_communicator(_address); }
	~rpc2_communicator_keeper() { if (_communicator) msg_communicator::return_communicator(_address, _communicator); }

	msg_communicator* operator->() { return _communicator; }
	msg_communicator* operator()() { return _communicator; }
	bool operator!() { return _communicator == 0; }
	operator bool() { return _communicator != 0; }

private:
	msg_communicator*	_communicator;
	const guid_t&	_address;
};

////////////////////////////////////////////////////////
msg_rpc_listener::msg_rpc_listener(msg_communicator* communicator_, const conf_listener& info_) :
	msg_listener(communicator_, info_)
{
}

// virtual 
msg_rpc_listener::~msg_rpc_listener()
{
}


// virtual 
void 
msg_rpc_listener::v_on()
{
	if (is_on())
		return;

	int status = 0;
	// do global initialization
	g_rpc_global.init(_info._connections);

	// register protocol sequence
	char buf[64];
	rpc_server_use_protseq_ep((ub1_t*)g_protocol, (int)_info._connections, uustring(buf, sizeof(buf), _info._address), &status);
    rpcStatus(status);

	// register server handle
	rpc_server_register_if(ServerIMSG_ServerIfHandle, &_info._address, &g_epv, &status);	

    rpcStatus(status);

	// get server binding handles
	rpc_binding_vector_t* vec = 0;
	rpc_server_inq_bindings(&vec, &status);
	rpcStatus(status);

	// register interface
	// prepare uuid
	//uuid_vector_t uvec;
	UUID_VECTOR uvec;
	uvec.Count = 1;
	uvec.Uuid[0] = (UUID*)&_info._address;

	rpc_ep_register(ServerIMSG_ServerIfHandle, vec, &uvec, 0, &status);

	if (vec)
	{
		size_t status_ = 0;
		rpc_binding_vector_free(&vec, &status_);
	}

	rpcStatus(status);

	// set type for object
	rpc_object_set_type(&_info._address, &_info._address, &status);
	rpcStatus(status);

	msg_base::v_on();
}

// virtual 
void 
msg_rpc_listener::v_off()
{
	if (!is_on())
		return;

	msg_base::v_off();

	int status = 0;

	// get server binding handles
	rpc_binding_vector_t* vec = 0;
	rpc_server_inq_bindings(&vec, &status);
	rpcStatus(status);

	// register interface
//	uuid_vector_t uvec;
//	uvec.count = 1;
//	uvec.uuid[0] = (uuid_t*)&_info._address;

	UUID_VECTOR uvec;
	uvec.Count = 1;
	uvec.Uuid[0] = (UUID*)&_info._address;

	rpc_ep_unregister(ServerIMSG_ServerIfHandle, vec, &uvec, &status);

	if (vec)
	{
		size_t status_ = 0;
		rpc_binding_vector_free(&vec, &status_);
	}

	rpcStatus(status);

	// reset type for object
	rpc_object_set_type(&_info._address, 0, &status);

	// unregister RPC server
	rpc_server_unregister_if(ServerIMSG_ServerIfHandle, &_info._address, &status);

	rpcStatus(status);

}
 
////////////////////////////////////////////////////////////////
msg_rpc_connection::msg_rpc_connection(msg_communicator* communicator_, rpc_binding_handle_t handle_, const conf_listener& linfo, const conf_connection& info) :
	msg_connection(communicator_, linfo, info), _handle(handle_)
{
}

msg_rpc_connection::msg_rpc_connection(msg_communicator* communicator_, rpc_binding_handle_t handle_, const conf_connection& info) :
	msg_connection(communicator_, info), _handle(handle_)
{
}

msg_rpc_connection::~msg_rpc_connection()
{
	size_t status = 0;
	// free RPC handle
	rpc_binding_free(&_handle, &status);
}


// virtual 
void 
msg_rpc_connection::v_do_job(size_t ident, void* user_data)
{
	assert(ident == queue_thread_ident);

	msg_creator creator(_communicator);
 
	msg_cpp* msg_ = 0;
	if (!pop(msg_))
	{
		assert(false);
		return;
	}	

	msg_pointer_t msg(creator);
	msg = msg_;
 
	try
	{
		assert(_handle != 0);
		// send message
		send_msg(_communicator, this, _handle, msg);

		_communicator->log_msg(msg);

		// set last time activity
		set_last_activity();
	}
	catch (exception&)
	{
		// shutdown connection
		_communicator->shutdown_connection(this);
	}
}

// static 
msg_connection*
msg_rpc_connection::connect(msg_communicator* communicator_, const conf_connection& info_)
{
	int status = 0;
	ub1_t* block_reply = 0;
	rpc_binding_handle_t handle = 0;
	msg_rpc_connection* connection = 0;

	msg_creator creator(communicator_);
 
	try
	{
		// create handle from string binding
		char buf[64]; 
		ub1_t* full_bind_string = 0;
		uustring(buf, sizeof(buf), info_._address);
		// create handle from string binding
		rpc_string_binding_compose((ub1_t*)buf, (ub1_t*)g_protocol, (ub1_t*)(g_name + strlen(g_protocol_)), (ub1_t*)buf, 0, &full_bind_string, &status);
		rpcStatus(status);

		rpc_binding_from_string_binding(full_bind_string, &handle, &status);
		if (status != rpc_s_ok)
		{
			size_t s;
			rpc_string_free(&full_bind_string, &s);
			rpcStatus(status);
		}

		rpc_string_free(&full_bind_string, &status);

		// set RPC timeout
		rpc_mgmt_set_com_timeout(handle, rpc_c_binding_min_timeout, &status);
		rpcStatus(status);

		// create new rpc connection object as a initiator
		connection = new msg_rpc_connection(communicator_, handle, info_);
		if (!connection)
			exception::_throw(MSG_RESULT_NOTMEMORY, &msgMsgTable);

		msg_pointer_t msg(creator);
		msg = connection->prepare_handshake_msg();
		// pack message
		msg->pack_msg(0);
		unsigned long size_reply = 0;

		try
		{
			// try to establish RPC connection to client
			ClientConnect(handle, ntohl(*(ub4_t*)msg->get_block()), msg->get_block(), &size_reply, &block_reply);
		}
		catch (...) // RPC exception
		{
			// something wrong or RPC server is not available
			exception::_throw(MSG_RESULT_CONNECTION_BROKEN, &msgMsgTable);
		}

		// get body size
		int body_size = (int)size_reply - (int)msg_cpp::block_size(0);
		if (!size_reply)
			exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);
		if (body_size < 0 || !block_reply)
			exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);

		// construct reply
		msg_pointer_t reply(creator, body_size);
		// copy reply body to the block
		memcpy(reply->get_block(), block_reply, size_reply);
		// unpack reply message
		reply->unpack_msg(0);

		// validate handshake reply
		connection->validate_handshake_reply(reply);

		// add connection
		communicator_->add_connection(connection);

		// free reply block 
		if (block_reply)
			midl_user_free(block_reply);

		return connection;
	}
	catch (exception& x)
	{
		// free reply block
		if (block_reply)
			midl_user_free(block_reply);

		if (handle)
			rpc_binding_free(&handle, &status);

		if (!connection)
			delete connection;

		// rethrow exception
		throw x;
	}
}

// static 
void
msg_rpc_connection::send_msg(msg_communicator* communicator_, msg_rpc_connection* _this, rpc_binding_handle_t handle_, msg_cpp* msg_)
{
	// pack message
	msg_->pack_msg(_this->get_crypt_key());
	ub1_t* remoteError = 0;

	try
	{
		communicator_->format_logging(0, __FILE__, __LINE__, en_log_info, "before ClientPostBlock, comm: %d, handle: %d", communicator_, handle_); 
		// try to send message to client
		ClientPostBlock(handle_, ntohl(*(ub4_t*)msg_->get_block()), msg_->get_block(), &remoteError);
	}
	catch (...) // rpc exception
	{
		communicator_->format_logging(0, __FILE__, __LINE__, en_log_info, "failed ClientPostBlock, comm: %d, handle: %d", communicator_, handle_); 
		exception::_throw(MSG_RESULT_CONNECTION_BROKEN, &msgMsgTable);
	}

	communicator_->format_logging(0, __FILE__, __LINE__, en_log_info, "succeeded ClientPostBlock, comm: %d, handle: %d", communicator_, handle_); 

	// check error
	if (remoteError)
	{
		// create error reply only for outgoing messages
		if (msg_->_type == user_type_send || msg_->_type == user_type_send_async)
		{
			try
			{
				// construct reply
				msg_creator creator(communicator_);
				msg_pointer_t reply(creator, 0);
				// set error body text
				msg_pack::make_reply_msg(msg_, reply);
				msg_pack::make_error_msg(reply, (const char*)remoteError);
				// push message to communicator queue
				communicator_->comm_msg(reply);
				reply.detach();
			}
			catch (exception&)
			{
			}
		}

		// free RPC resources
		midl_user_free(remoteError);
	}
}

#ifdef __cplusplus
extern "C"{
#endif 

void ServerConnect(rpc_binding_handle_t handle_, unsigned long szsend, unsigned char* sender, unsigned long* szrecv, unsigned char** receiver)
{
	int status = 0;
	*szrecv = 0;
	*receiver = 0;
	rpc_binding_handle_t handle = 0;
	msg_rpc_connection* connection = 0;

	try
	{
		// get body size
		int body_size = (int)szsend - (int)msg_cpp::block_size(0);
		if (body_size < 0)
			exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);


		// unbind handle
		guid_t address;
		// get binding string
		rpc_binding_inq_object(handle_, &address, &status);
		rpcStatus(status);

		// get communicator
		rpc2_communicator_keeper keeper(address);
		msg_creator creator(keeper());	
		msg_pointer_t msg(creator, body_size);

		// fill message body
		memcpy(msg->get_block(), sender, szsend);
		// unpack message
		msg->unpack_msg(0);

		// try to lock communicator and find listener
		msg_listener* listener_ = 0;
		if (!keeper || !(listener_ = keeper->find_listener(rpc)) || !listener_->is_on())
			exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);

		/*
		// create handle from string binding
		rpc_binding_from_string_binding((ub1_t*)g_name, &handle, &status);
		rpcStatus(status);

		// set object uuid
		rpc_binding_set_object(handle, &msg->_sender, &status);
		rpcStatus(status);
		/*/

		///*
		// create handle from string binding
		char buf[64]; 
		ub1_t* full_bind_string = 0;
		uustring(buf, sizeof(buf), msg->_sender);
		// create handle from string binding
		rpc_string_binding_compose((ub1_t*)buf, (ub1_t*)g_protocol, (ub1_t*)(g_name + strlen(g_protocol_)), (ub1_t*)buf, 0, &full_bind_string, &status);
		rpcStatus(status);

		rpc_binding_from_string_binding(full_bind_string, &handle, &status);
		if (status != rpc_s_ok)
		{
			size_t s;
			rpc_string_free(&full_bind_string, &s);
			rpcStatus(status);
		}

		rpc_string_free(&full_bind_string, &status);
		//*/

		// set RPC timeout
		rpc_mgmt_set_com_timeout(handle, rpc_c_binding_min_timeout, &status);
		rpcStatus(status);

		// create new rpc connection with dummy settings
		conf_connection info_;
		info_ = listener_->get_info();
		connection = new msg_rpc_connection(keeper(), handle, listener_->get_info(), info_);
		if (!connection)
			exception::_throw(MSG_RESULT_NOTMEMORY, &msgMsgTable);

		// prepare reply
		msg_pointer_t reply(creator);
		// prepare handshake reply
		reply = connection->prepare_handshake_reply(msg);

		// pack response
		reply->pack_msg(0);
		// set size
		*szrecv = ntohl(*(ub4_t*)reply->get_block());
		// allocate rpc memory
		*receiver = (ub1_t*)midl_user_allocate(*szrecv);
		if (*receiver) // copy message block
			memcpy(*receiver, reply->get_block(), *szrecv);
		else
			*szrecv = 0;			

		// add connection to the list
		keeper->add_connection(connection);
	}
	catch (exception&)
	{
		// delete connection
		if (connection)
			delete connection;

		// free RPC resources if any
		if (handle)
			rpc_binding_free(&handle, &status);
	}
}

void ServerPostBlock(rpc_binding_handle_t handle_, unsigned long sz, ub1_t* buf, ub1_t** reply)
{
	int status = 0;
	
	try
	{
		// unbind handle
		guid_t address;
		// get binding string
		rpc_binding_inq_object(handle_, &address, &status);
		rpcStatus(status);
		
		// try to lock communicator and find listener
		rpc2_communicator_keeper keeper(address);
		msg_listener* listener_ = 0;
		if (!keeper || !(listener_ = keeper->find_listener(rpc)) || !listener_->is_on())
			exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);

		msg_communicator* communicator_ = keeper();

		communicator_->format_logging(0, __FILE__, __LINE__, en_log_info, "inside ServerPostBlock, comm: %d, handle: %d", communicator_, handle_); 

		// get body size
		int body_size = (int)sz - (int)msg_cpp::block_size(0);
		if (body_size < 0 || !buf)
			exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);

		// construct message
		msg_creator creator(keeper());
		
		msg_pointer_t msg(creator, body_size);
		// copy block bytes
		memcpy(msg->get_block(), buf, sz);
		// unpack message
		msg->unpack_msg(0);

		// check present
		msg_connection* con = 0;
		if (!(con = keeper->find_connection(msg->_sender)) ||
			!con->is_on() || con->is_block() || con->get_info()._session != msg->_sessionid)
			exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);

		// ignore any system message
		if (msg->_type & user_type_mask)
		{
			// unpack message with encryption
			if (con->get_rsa()) // need to unpack again
				msg->unpack_msg(con->get_crypt_key());
		}
		else if (msg->_type != system_type || msg->msgid != msg_id_ping)
			// ignore ping
			// something unusual
			exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);

		// process incoming message
		con->process_incoming_message(msg);

		// detach message
		msg.detach();
	}
	catch (exception& x)
	{
		// form error
		const char* reason = x.what();
		size_t len = strlen(reason) + 1;
		// allocate RPC memory
		*reply = (ub1_t*)midl_user_allocate(len);
		if (*reply) // copy error text
			memcpy(*reply, reason, len);
	}
}

#ifdef __cplusplus
}
#endif

#pragma pack()
END_TERIMBER_NAMESPACE