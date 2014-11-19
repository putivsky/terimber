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

#ifndef _terimber_msg_c_h_
#define _terimber_msg_c_h_

#include "allinc.h"
#include "aiomsg/aiomsgfactory.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! returns codes for callback function
//! \brief success
#define MSG_RESULT_OK						0
//! \brief unknown error
#define MSG_RESULT_UNKNOWN_ERROR			0xffffffff
//! \brief not enough memory
#define MSG_RESULT_NOTMEMORY				0xfffffffe
//! \brief timeout
#define MSG_RESULT_TIMEOUT					0xfffffffd
//! \brief unknown destination address
#define MSG_RESULT_UNKNOWN_DESTINATION		0xfffffffc
//! \brief connection is broken
#define MSG_RESULT_CONNECTION_BROKEN		0xfffffffb
//! \brief access denied
#define MSG_RESULT_ACCESS_DENIED			0xfffffffa
//! \brief unknown ini file
#define MSG_RESULT_UNKNOWN_FILE				0xfffffff9
//! \brief unknown message ident
#define MSG_RESULT_UNKNOWN_MSGID			0xfffffff8
//! \brief invalid message format
#define MSG_RESULT_INVALID_MSGFORMAT		0xfffffff7
//! \brief invalid session
#define MSG_RESULT_INVALID_SESSION			0xfffffff5
//! priorities
//! \brief high priority
#define MSG_PRIORITY_SYSTEM					0				
//! \brief normal priority
#define MSG_PRIORITY_HIGH					1
//! \brief low priority
#define MSG_PRIORITY_NORMAL					2


#pragma pack()
END_TERIMBER_NAMESPACE

#endif 


