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

#include "aiomsg/msg_lsnr.h"
#include "base/list.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/number.hpp"
#include "base/except.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

extern exception_table msgMsgTable;

msg_listener::msg_listener(msg_communicator* communicator_, const conf_listener& info_) :
	msg_base(communicator_), _info(info_)
{
}

// virtual 
msg_listener::~msg_listener()
{
}

// checks if the listener accepted the connection
// static
void
msg_listener::accept_address(const guid_t& addr, const conf_listener& linfo, conf_connection& cinfo)
{
	if (linfo._accept.empty())
	{
		for (msg_peer_list_t::const_iterator iter = linfo._reject.begin(); iter != linfo._reject.end(); ++iter)
			if (iter->_address == addr)
				exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable); 
	}
	else
	{
		for (msg_peer_list_t::const_iterator iter = linfo._accept.begin(); iter != linfo._accept.end(); ++iter)
			if (iter->_address == addr)
			{
				cinfo._address = addr;
				cinfo._support_crypt = iter->_support_crypt;
				cinfo._crypt_external = iter->_crypt;
				return;
			}
					
		exception::_throw(MSG_RESULT_ACCESS_DENIED, &msgMsgTable);
	}

	cinfo._address = addr;
	cinfo._support_crypt = linfo._support_crypt;
	cinfo._crypt_external = linfo._crypt_accept;
}

#pragma pack()
END_TERIMBER_NAMESPACE
