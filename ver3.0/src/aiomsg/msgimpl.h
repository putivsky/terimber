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

#ifndef _terimber_aiomsg_h_
#define _terimber_aiomsg_h_

#include "aiomsg/msg_c.h"
#include "aiomsg/msg_comm.h"
#include "aiomsg/msg_user.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class aiomsg
//! \brief implements terimber_aiomsg interface
class aiomsg : public terimber_aiomsg
{
public:
	//! \brief constructor
	aiomsg();
	//! \brief destructor
	virtual ~aiomsg();
	//! \brief initial functions to activate and deactivate message transport
	virtual 
	bool 
	init(		const char* info, 							//!< a full path to the ini xml file with port initialization information
				const char* ini_key							//!< crypto key for decription ini xml file - reserved for production version
				);

	//! \brief initial functions to activate and deactivate message transport
	virtual 
	bool 
	init(		const void* buffer, 						//!< a pointer to xml memory
				size_t len									//!< parameter is a length of xml in memory
				);

	//! \brief function uninitializes the message port
	virtual 
	bool 
	uninit();
	
	//! \brief function for activate and deactivate callback port
	//! it make sense to specify additional_thread_count > 0 if the order incoming messages doesn't matter
	//! otherwise specify additional_thread_count = 0 and process incoming messages one by one in order as they arrived 
	virtual 
	bool 
	start(	msg_callback_notify* callback,					//!< pointer to the callback class instance
			size_t additional_thread_count					//!< user defines how many threads can call incoming callback function at the same time
			);
	//! \brief function stops message processing
	virtual 
	bool 
	stop();

	//! \brief messages port error function
	virtual 
	const char* 
	get_port_error() const;

	//! \brief returns the port address - comes from init file
	virtual 
	const guid_t& 
	get_port_address() const;

	//! \brief internal allocate, reallocate, and free memory
	//! caller has to create ALL messages through construct function
	//! and call destroy function to free memory
	//! returns pointer to the message object or null
	virtual 
	msg_t* 
	construct(	size_t size									//!< size of message in bytes
				);
	//! \brief resizes the message
	//! the old memory will be preserved 
	virtual 
	bool	
	resize(		msg_t* msg,									//!< pointer to the message to be resized
				size_t size									//!< new size of message in bytes
				);
	//! \brief destroys message
	virtual 
	bool	
	destroy(	msg_t* msg									//!< pointer to the message to be destroyed
				);

	//! \brief messages content management functions
	//! function returns the message size
	virtual 
	size_t 
	get_size(	const msg_t* msg							//!< pointer to the message
				) const;
	
	//! \brief function returns true if offset + len <= message size (see resize function)
	virtual 
	bool 
	write_buffer(msg_t* msg,								//!< pointer to the message
				size_t offset,								//!< offset from the beginning of message body
				const void* buf,							//!< pointer to the buffer
				size_t len									//!< length of writing buffer
				);
	//! \brief constant variant of previous function
	virtual 
	const void* 
	get_buffer(	const msg_t* msg							//!< pointer to the constant message
				) const;

	//! \brief messages from/to settings
	virtual 
	bool	
	set_receiver(msg_t* msg,								//!< pointer to the message
				const guid_t& receiver						//!< reference to receiver address
				);
	//! \brief gets sender address
	virtual 
	bool 
	get_sender(	const msg_t* msg,							//!< pointer to the message
				guid_t& sender								//!< reference where function put sender address to
				) const;

	//! Transport functions
	//! \brief function sends message and waits for the reply for timeout milliseconds (garantee delivery)
	//! if function returns false the caller is still responsible for message destruction 
	//! regardless the copy parameter value
	virtual
	bool	
	send(	bool copy,										//!< if set to true, the function will make a copy of the message and caller can reuse message again
															//! otherwise message port will take care about it
			msg_t* msg,										//!< pointer to message to be sent, if function returns true and copy == false then message port is responsible for message destruction
			msg_t** reply									//!< returns the reply from receiver, caller is responsible to destroy the reply message
			);

	//! \brief function sends message asynchronically (garantee delivery) and returns unique identificator (zero means error)
	//! which will be used as input parameter for asynchronous callback function
	//! if function returns zero the caller is still responsible for message destruction 
	//! regardless the copy parameter value
	//! retun value, if non zero is unique identificator, which will be used for firing asynchronous reply, see above
	virtual 
	guid_t 
	send_async(bool copy,									//!< if set to true, the function will make a copy of the message and caller can reuse message again
															//! otherwise message port will take care about it
			msg_t* msg										//!< pointer to message to be sent, if function returns true and copy == false then message port is responsible for message destruction
			);

	//! \brief function post message and don't wait for a reply - this is a non-garantee delivery
	//! if function returns false the caller is still responsible for message destruction 
	//! regardless the copy parameter value
	virtual 
	bool	
	post(	bool copy,										//!< if set to true, the function will make a copy of the message and caller can reuse message again
			msg_t* msg										//!< pointer to message to be sent, if function returns true and copy == false then message port is responsible for message destruction
															//! otherwise message port will take care about it
			);

	//! \brief additional functionality to manage connections dynamically
	//! @xml_description - xml string describes connection as xml node of follow XDTD
	//! extended DTD
	//! <!ENTITY % kind "(rpc | sock | p2p)">
	//! <!ELEMENT connection EMPTY>
	//! <!ATTLIST connection
	//! address CTYPE vt_guid #REQUIRED
	//! port CTYPE vt_ub2 #IMPLIED
	//! network CDATA #IMPLIED
	//! ping CTYPE vt_ub4 #IMPLIED
	//! info CDATA #IMPLIED
	//! security CTYPE vt_bool #IMPLIED
	//! password CDATA #IMPLIED >
	//! encpytion is reserved for production version
	virtual 
	bool 
	add_connection(const char* xml_description
				);

	//! \brief additional functionality to manage listeners dynamically
	//! extended DTD
	//! <!ELEMENT listener (accept | reject)?>
	//! <!ELEMENT reject (peer)+>
	//! <!ELEMENT accept (peer)+>
	//! <!ELEMENT peer EMPTY>
	//! <!ATTLIST peer
	//! address CTYPE vt_guid #REQUIRED
	//! security CTYPE vt_bool #IMPLIED
	//! password CDATA #IMPLIED >
	//! <!ATTLIST listener
	//! port CTYPE vt_ub2 #IMPLIED
	//! network CDATA #IMPLIED
	//! connections CTYPE vt_ub4 #IMPLIED
	//! ping CTYPE vt_ub4 #IMPLIED
	//! waste CTYPE vt_ub4 #IMPLIED
	//! info CDATA #IMPLIED
	//! security CTYPE vt_bool #IMPLIED
	//! password CDATA #IMPLIED >
	virtual 
	bool 
	add_listener(const char* xml_description				//<! xml string describes connection as xml node of follow XDTD
				);

	//! \brief removes connection dynamically
	virtual 
	bool 
	remove_connection(const guid_t& address					//!< address of connection
				);

	//! \brief removes listener dynamically
	virtual 
	bool 
	remove_listener(const char* type						//!< type of listener (sock)
				);

	//! \brief do xray
	virtual
	void
	doxray();

private:
	//! \brief checks if communicator is on
	//! NB!!! throws exception
	void 
	check_on() const;
	//! \brief checks validity of user connection
	//! NB!!! throws exception
	void 
	check_ident() const;
private:
	msg_communicator		_communicator;					//!< messages communicator
	mutable string_t		_error;							//!< last error
	mutex					_mtx;							//!< mutex
	msg_user_connection*	_user_conn;						//!< pointer to user connection
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif 

