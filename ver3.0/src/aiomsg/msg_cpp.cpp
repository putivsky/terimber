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

#include "aiomsg/msg_cpp.h"
#include "base/date.h"

#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/except.h"
#include "base/list.hpp"
#include "base/common.hpp"
#include "base/number.hpp"

#include "xml/xmlimpl.h"
#include "crypt/crypt.hpp"
#include "crypt/base64.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

extern exception_table msgMsgTable;

msg_cpp::msg_cpp(byte_allocator* allocator_, size_t size_) :
	_type(user_type_send),
	_marker(null_uuid),
	_timestamp(date()),
	_allocator(allocator_),
	_sender(null_uuid),
	_receiver(null_uuid),
	_sessionid(null_uuid),
	_size(0),
	_body(0),
	_block(0)
{
	assert(_allocator);
	msgid = MSG_EMPTY_ID;
	majver = 0;
	minver = 0;
	timeout = msg_default_timeout;
	priority = MSG_PRIORITY_NORMAL;
	resize(size_);
}

// virtual 
msg_cpp::~msg_cpp()
{
}

void 
msg_cpp::pack_msg(const room_byte_t* key)
{
	ub1_t* pt = (ub1_t*)_block + sizeof(ub4_t);
	// Big-Endian
	// first 4 bytes contain the size of the rest of the buffer
	// header size (include size of body and body row bytes)

	// c part
	pack32(pt, msgid);
	pack32(pt, majver);
	pack32(pt, minver);
	pack32(pt, priority);
	pack64(pt, timeout);
	// c part
	
	// cpp part
	pack32(pt, _type);
	//pack32(pt, _marker);
	packaddr(pt, _marker);
	packaddr(pt, _sender);
	packaddr(pt, _receiver);
	packaddr(pt, _sessionid);
	pack64(pt, _timestamp);
	pack32(pt, _size);
	// cpp part

	if (_size && key)
	{
		crypt ch(*key, key->size(), MD5);
		size_t ch_size = (size_t)_size;
		ch.encode((ub1_t*)_body, ch_size, RIJNDAEL); 
	}
}

void 
msg_cpp::unpack_msg(const room_byte_t* key)
{
	const ub1_t* pt = (const ub1_t*)_block;
	// checks size
	ub4_t size = 0;
	unpack32(pt, size);

	if (size < block_size(0))
		exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);

	try
	{
		// c part
		unpack32(pt, msgid);
		unpack32(pt, majver);
		unpack32(pt, minver);
		unpack32(pt, priority);
		unpack64(pt, timeout);
		// c part

		// cpp part
		unpack32(pt, (ub4_t&)_type);
		//unpack32(pt, _marker);
		unpackaddr(pt, _marker);
		unpackaddr(pt, _sender);
		unpackaddr(pt, _receiver);
		unpackaddr(pt, _sessionid);
		unpack64(pt, (ub8_t&)_timestamp);
		unpack32(pt, _size);

		if (size != block_size(_size))
			exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);
		
		if (_size)
		{
			_body = (ub1_t*)_block + _block_body_offset();
			if (key)
			{
				crypt ch(*key, key->size(), MD5);
				size_t ch_size = (size_t)crypt_size(_size);
				ch.decode((ub1_t*)_body, ch_size, RIJNDAEL); 
			}
		}
		else
			_body = 0;
		// cpp part
	}
	catch (exception& x)
	{
		throw x;
	}
	catch (...)
	{
		exception::_throw(MSG_RESULT_INVALID_MSGFORMAT, &msgMsgTable);
	}
}


bool 
msg_cpp::resize(size_t size_)
{
	ub1_t* buf = 0;
	// checks first allocation inside constructor
	if (!_block)
	{
		buf = (ub1_t*)_allocator->allocate((size_t)block_size(size_));
		if (!buf)
			return false;
		//else memset(buf + _block_body_offset(), 0, size_);

		_block = buf;
	}
	else if (_size < size_) // need reallocate
	{
		// allocates new buffer
		buf = (ub1_t*)_allocator->allocate((size_t)block_size(size_));
		if (!buf)
			return false;
		//else memset(buf + _block_body_offset() + _size, 0, size_ - _size);
	

		// copies previous body
		if (_size)
		{
			assert(_body && _block);
			memcpy(buf + _block_body_offset(), _body, (size_t)_size);
		}

		_block = buf;
	}
	else if (_size > size_) // corrects the size only
	{
		buf = (ub1_t*)_block;
		memset(buf + _block_body_offset() + size_, 0, _size - size_);
		_size = (ub4_t)size_;
	}
	else // equal size
		return true;

	// sets new size
	_size = (ub4_t)size_;

	// sets body pointer
	if (_size)
		_body = buf + _block_body_offset();
	else
		_body = 0;

	// sets block size
	pack32(buf, (ub4_t)block_size(_size));

	return true;
}

//////////////////////////////////////////////////////////////////////////
// static 
void 
msg_pack::make_reply_msg(const msg_cpp* msg_, msg_cpp* reply_)
{
	reply_->_receiver = msg_->_sender;
	reply_->_sender = msg_->_receiver;

	// assigns marker 
	reply_->_marker = msg_->_marker;
	switch (msg_->_type)
	{
		case user_type_send:
			reply_->_type = user_type_reply;
			break;
		case user_type_send_async:
			reply_->_type = user_type_reply_async;
			break;
		case handshake_type:
			reply_->_type = handshake_type;
			break;
		case system_type:
			reply_->_type = system_type;
			break;
		default:
			assert(false);
	}
}

// static 
void 
msg_pack::make_error_msg(msg_cpp* msg_, const char* text_)
{
	msg_->msgid = MSG_ERROR_ID; // error
	const char* err_text = text_ ? text_ : "unknown error";
	size_t len = strlen(err_text);
	msg_->resize(len + 1);
	memcpy(msg_->get_body(), err_text, len + 1);
}

#pragma pack()
END_TERIMBER_NAMESPACE
