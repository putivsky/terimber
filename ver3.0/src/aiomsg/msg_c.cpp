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

#include "allinc.h"
#include "aiomsg/msg_c.h"
#include "base/except.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// description of the base message results
exception_item msgMsgs[] =
{
	{ MSG_RESULT_OK,					"Action succeeded" },
	{ MSG_RESULT_UNKNOWN_ERROR,			"Unspecified error occured" },
	{ MSG_RESULT_NOTMEMORY,				"Not enough memory" },
	{ MSG_RESULT_TIMEOUT,				"Timeout occured" },
	{ MSG_RESULT_UNKNOWN_DESTINATION,	"Unknown destination" },
	{ MSG_RESULT_CONNECTION_BROKEN,		"Connection broken" },
	{ MSG_RESULT_ACCESS_DENIED,			"Access denied" },
	{ MSG_RESULT_UNKNOWN_MSGID,			"Unknown message ident %d" },
	{ MSG_RESULT_UNKNOWN_FILE,			"Unknown file %s" },
	{ MSG_RESULT_INVALID_MSGFORMAT,		"Invalid message format" },
	{ MSG_RESULT_INVALID_SESSION,		"Invalid session ident" },
	{ 0,								0 }
};

// global base messages table
exception_table msgMsgTable(msgMsgs);

#pragma pack()
END_TERIMBER_NAMESPACE
