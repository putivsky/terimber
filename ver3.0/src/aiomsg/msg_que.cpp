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
#include "base/list.hpp"
#include "base/memory.hpp"
#include "base/common.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

msg_queue_processor::msg_queue_processor(msg_communicator* communicator) : msg_base(communicator)
{
}

msg_queue_processor::~msg_queue_processor()
{
}

//! \brief action on turning on
//virtual 
void 
msg_queue_processor::v_on() 
{
	msg_base::v_on();
	wakeup();
}
//! \brief action on turning off
//virtual 
void 
msg_queue_processor::v_off() 
{
	_communicator->get_thread_manager().revoke_client(this);
	msg_base::v_off();
}


bool 
msg_queue_processor::v_has_job(size_t ident, void* user_data)
{
	return !is_block() // queue is not blocked
			&& peek(); // there are messages in the queue
}

#pragma pack()
END_TERIMBER_NAMESPACE
