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

#include "osdetect.h"

//! For VC7, and VC8
#if OS_TYPE == OS_WIN32
#if defined(_MSC_VER) && (_MSC_VER >= 1200) 
#include <winsock2.h>
#endif
#endif

#include "xml/socket.h"
#include "base/date.h"
#include "base/list.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

terimber_socket::terimber_socket(int num) :
	_number(num)
{
	memset(&_addr, 0, sizeof(sockaddr_in));
}

terimber_socket::~terimber_socket()
{
	close();
}

// static 
void 
terimber_socket::set_options(int number)
{
	linger lingerStruct;
	lingerStruct.l_onoff = 1;         
	lingerStruct.l_linger = 0;
	::setsockopt(number, SOL_SOCKET, SO_LINGER, (char *)&lingerStruct, sizeof(lingerStruct));


	int noDelay = 1;
	::setsockopt(number, IPPROTO_TCP, TCP_NODELAY, (const char*)&noDelay, sizeof(noDelay));

	int keepAlive = 1;
	::setsockopt(number, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepAlive, sizeof(keepAlive));
	
	// sets option for all types
	int reUse = 1;
	::setsockopt(number, SOL_SOCKET, SO_REUSEADDR, (const char*)&reUse, sizeof(reUse));

}

// static 
int 
terimber_socket::resolve_address(const char* str, ub2_t port, sockaddr_in& addr)
{
	in_addr ipAddr;
	if (str) // remote socket
	{
		bool askDNS = false;
		for (const char* s = str; *s ; ++s)
			if (!isdigit( *s ) && *s != '.') // if a domain name was specified
			{
				askDNS = true;
				break;
			}

		if (askDNS)
		{
			struct hostent* host = gethostbyname(str);
			if( host == 0 )
				return WSAEFAULT;
			memcpy(&ipAddr, host->h_addr, host->h_length);
		}
		else
		{
			ipAddr.s_addr = inet_addr(str);
			if (ipAddr.s_addr == INADDR_NONE)
				return WSAEFAULT;
		}
	}
	else
		ipAddr.s_addr = htonl(INADDR_LOOPBACK);

	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = ipAddr;
	return 0;
}

// static
void 
terimber_socket::set_send_timeout(int number, size_t timeout)
{
#if OS_TYPE == OS_WIN32
	size_t						
#else
	timeval
#endif
	timeout_;
	if (timeout == INFINITE)
	{
#if OS_TYPE == OS_WIN32 
		timeout_ = 0;
#else
		timeout_.tv_sec = timeout_.tv_usec = 0;
#endif
	}
	else
	{
#if OS_TYPE == OS_WIN32 
		timeout_ = timeout;
#else
		timeout_.tv_sec = timeout / 1000;
		timeout_.tv_usec = (timeout % 1000) * 1000 + 1;
#endif
	}

	::setsockopt(number, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout_, sizeof(timeout_));
}

// static
void 
terimber_socket::set_recv_timeout(int number, size_t timeout)
{
#if OS_TYPE == OS_WIN32
	size_t						
#else
	timeval
#endif
	timeout_;
	if (timeout == INFINITE)
	{
#if OS_TYPE == OS_WIN32 
		timeout_ = 0;
#else
		timeout_.tv_sec = timeout_.tv_usec = 0;
#endif
	}
	else
	{
#if OS_TYPE == OS_WIN32 
		timeout_ = timeout;
#else
		timeout_.tv_sec = timeout / 1000;
		timeout_.tv_usec = (timeout % 1000) * 1000 + 1;
#endif
	}

	::setsockopt(number, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout_, sizeof(timeout_));
}

bool 
terimber_socket::close()
{
	if (INVALID_SOCKET == _number)
		return false;

	static char rbuf[512];

	// shuts down send first
#if OS_TYPE == OS_WIN32
	::shutdown(_number, SD_SEND);
#else
	::shutdown(_number, SHUT_WR);
#endif

	// if TCP tries to receive the rest of bytes
	fd_set recv_set;
	FD_ZERO(&recv_set); 
	FD_SET(_number, &recv_set);

	struct timeval timeout_val = {0, 1000};
	int res = 0;

	do
	{
		res = ::select((int)_number + 1, &recv_set, 0, 0, &timeout_val);
	
		if (res)
		{
			res = ::recv(_number, rbuf,  512, 0);
			if (res  > 0 && res < 512)
				break;
		}
	}
	while (res > 0);

	// shuts down receive
#if OS_TYPE == OS_WIN32
	::shutdown(_number, SD_RECEIVE);
	// closes socket
	::closesocket(_number);
#else
	::shutdown(_number, SHUT_RD);
	// closes socket
	::close(_number);
#endif	
	_number = INVALID_SOCKET;
	memset(&_addr, 0, sizeof(sockaddr_in));
	return true;
}

bool 
terimber_socket::connect()
{
	if (INVALID_SOCKET == _number) // needs to create terimber_socket
	{
		// creates terimber_socket
		if (INVALID_SOCKET == (_number = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
			return false;

		set_options((int)_number);

		// connects
		if (::connect(_number, (struct sockaddr*)&_addr, sizeof(struct sockaddr_in))) // tries to connect
		{
			close();
 			return false;
		}
		else
			return true;
	}
	else
		return false;
}

int 
terimber_socket::send(const char* data, size_t len, size_t& timeout)
{
	int ret = 0;

	if (INVALID_SOCKET == _number) 
		return WSAENOTSOCK;

	fd_set send_set;
	date local_time;
	size_t shift = 0;

	for (;;)
	{
		FD_ZERO(&send_set); 
		FD_SET(_number, &send_set);

		struct timeval wait = {(long)timeout / 1000, (long)timeout % 1000 * 1000};
		ret = ::select((int)_number + 1, 0, &send_set, 0, &wait);		
		
		if (!ret)
			return WSAETIMEDOUT;

		if (SOCKET_ERROR == ret)
			return ret;

		ret = ::send((int)_number, data + shift, (int)len, 0 ); // send data

		if (SOCKET_ERROR == ret)
			return ret;

		if (!ret)
			return WSAECONNRESET;
		else if (ret == len)
			break;
		else
			len -= ret, shift += ret;

		if ((int)len < 0)
			return WSAEMSGSIZE;

		timeout -= (size_t)date().get_difference(local_time);
		if ((int)timeout < 0)
			return WSAETIMEDOUT;
	}

	return 0;
}

// receives known length of data from source while timeout
int 
terimber_socket::receive(char* data, size_t len, size_t& timeout)
{
	int ret = 0;

	if (INVALID_SOCKET == _number) 
		return WSAENOTSOCK;

	fd_set rec_set;
	date local_time;
	size_t shift = 0;

	for (;;) 
	{
		FD_ZERO(&rec_set); 
		FD_SET(_number, &rec_set);

		if (timeout != INFINITE)
		{
			struct timeval timeout_val = {(long)timeout / 1000, (long)timeout % 1000 * 1000 };
			ret = ::select((int)_number + 1, &rec_set, 0, 0, &timeout_val);		
		}
		else
		{
			ret = ::select((int)_number + 1, &rec_set, 0, 0, 0);	
		}
		
		if (!ret)
			return WSAETIMEDOUT;

		if (SOCKET_ERROR == ret)
			return ret;

		// checks the availability of data
		// argp parameter points to an unsigned long value 
		unsigned long arg = 0;

#if OS_TYPE == OS_WIN32
		ret = ::ioctlsocket(_number, FIONREAD, &arg);
#else
		ret = ::ioctl(_number, FIONREAD, &arg);
#endif

		if (SOCKET_ERROR == ret)
			return ret;

		ret = ::recv((int)_number, data + shift, (int)__min(len, (size_t)arg), 0);

		if (SOCKET_ERROR == ret)
			return ret;

		if (!ret)
			return WSAECONNRESET;
		else if (ret == len)
			break;
		else
			len -= ret, shift += ret;

		if (timeout != INFINITE)
		{
			timeout -= (size_t)date().get_difference(local_time);
			if ((int)timeout < 0)
				return WSAETIMEDOUT;
		}
	}

	return 0;
}

// receives known length data from source while timeout
int 
terimber_socket::receive_whatever(char* data, size_t len, size_t& timeout, size_t& rlen)
{
	int ret = 0;

	if (INVALID_SOCKET == _number) 
		return WSAENOTSOCK;

	fd_set rec_set;
	date local_time;
	rlen = 0;

	FD_ZERO(&rec_set); 
	FD_SET(_number, &rec_set);

	if (timeout != INFINITE)
	{
		struct timeval timeout_val = {(long)timeout / 1000, (long)timeout % 1000 * 1000 };
		ret = ::select((int)_number + 1, &rec_set, 0, 0, &timeout_val);		
	}
	else
	{
		ret = ::select((int)_number + 1, &rec_set, 0, 0, 0);	
	}
	
	if (!ret)
		return WSAETIMEDOUT;

	if (SOCKET_ERROR == ret)
		return ret;

	// checks the availability of data
	// argp parameter points to an unsigned long value 
	unsigned long arg = 0;

#if OS_TYPE == OS_WIN32
	ret = ::ioctlsocket(_number, FIONREAD, &arg);
#else
	ret = ::ioctl(_number, FIONREAD, &arg);
#endif

	if (SOCKET_ERROR == ret)
		return ret;

	// check availability only
	if (!len)
		return arg != 0 ? 0 : WSAEFAULT;

	ret = ::recv((int)_number, data, (int)__min(len, (size_t)arg), 0);

	if (SOCKET_ERROR == ret)
		return ret;

	if (!ret)
		return WSAECONNRESET;
	else
		rlen = ret;

	return 0;
}

int 
terimber_socket::set_invalid()
{
	int number = (int)_number;
	_number = INVALID_SOCKET;
	return number;
}

bool 
terimber_socket::is_invalid() const
{
	return _number == INVALID_SOCKET;
}

#pragma pack()
END_TERIMBER_NAMESPACE
