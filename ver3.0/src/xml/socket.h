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

#ifndef _terimber_socket_h_
#define _terimber_socket_h_

#include "base/primitives.h"
#include "aiocomport/aiocomport.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class terimber_socket
//! \brief socket class for xml http streams
class terimber_socket
{
public:
	//! \brief constructor
	terimber_socket(int num = (int)INVALID_SOCKET				//!< socket handle
					);
	//! \brief destructor
	~terimber_socket();

	//! \brief closes and destroys terimber_socket
	bool 
	close();
	//! \brief opens and connects terimber_socket to the previously set address
	bool 
	connect();
	//! \brief sends known length data to destination while timeout
	int 
	send(			const char* data,						//!< buffer
					size_t len,								//!< buffer length
					size_t& timeout							//!< [in,out] timeout in milliseconds
					);
	//! \brief receives known length data from source while timeout
	int 
	receive(		char* data,								//!< pre-allocated buffer
					size_t len,								//!< buffer length
					size_t& timeout							//!< [in,out] timeout in milliseconds
					);
	//! \brief receives unknown length data from source while timeout
	int 
	receive_whatever(char* data,							//!< pre-allocated buffer
					size_t len,								//!< buffer length
					size_t& timeout,						//!< [in,out] timeout in milliseconds
					size_t& rlen							//!< [out] received bytes
					);
	
	//! \brief returns address
	inline
	const sockaddr_in& 
	get_address() const
	{
		return _addr; 
	}
	//! \brief sets address
	inline
	void 
	set_address(	const sockaddr_in& addr					//!< address to connect to
				)
	{ 
		_addr = addr; 
	}
	//! \brief sets options 
	static 
	void 
	set_options(	int number								//!< socket number
					);
	//! \brief resolves string address
	static 
	int 
	resolve_address(const char* str,						//!< string address
					ub2_t port,								//!< port
					sockaddr_in& addr						//!< [out] address
					);
	//! \brief sets send timeout
	static 
	void 
	set_send_timeout(int number,							//!< socket handle
					size_t timeout							//!< timeout in milliseconds
					);
	//! \brief sets receive timeout
	static 
	void 
	set_recv_timeout(int number,							//!< socket handle
					size_t timeout							//!< timeout in milliseconds
					);
	//! \brief invalidates socket
	int 
	set_invalid();
	//! \brief checks socket validity
	bool 
	is_invalid() const;
private:
	SOCKET				_number;							//!< socket handle
	sockaddr_in			_addr;								//!< socket address
};


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_socket_h_
