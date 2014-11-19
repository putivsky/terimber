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

#include "base/memory.hpp"
#include "base/list.hpp"
#include "base/string.hpp"
#include "base/vector.hpp"
#include "base/common.hpp"

#include "xml/miscxml.hpp"
#include "xml/declxml.hpp"
#include "xml/defxml.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

static const char protocolHTTP[]		= { ch_h, ch_t, ch_t, ch_p, ch_null };
static const char protocolFTP[]			= { ch_f, ch_t, ch_p, ch_null };
static const char protocolFILE[]		= { ch_f, ch_i, ch_l, ch_e, ch_null };
static const char protocolTale[]		= { ch_colon, ch_forward_slash, ch_forward_slash, ch_null };
static const char str_Colon[]			= { ch_colon, ch_null };
static const char str_At[]				= { ch_at, ch_null };
static const char str_Question[]		= { ch_question, ch_null };
static const char str_Pound[]			= { ch_pound, ch_null };
static const char str_ForwardSlash[]	= { ch_forward_slash, ch_null };

// here implementation _fullpath for LINUX, and Mac OS X (Hun. 0905-08)
#if OS_TYPE == OS_LINUX || OS_TYPE == OS_UNIX || OS_TYPE == OS_MACOSX
char* _fullpath(char* buf, const char* relative, size_t len)
{
	if (!buf)
		buf = (char*)malloc(len);

	// get the current directory
	if (!getcwd(buf, len))
		return 0;

	if (!relative)
		return buf;
        else if (*relative == '/' || *relative == '\\') // absolute path
        {
		*buf = 0;
		strncat(buf, relative, len); 
		return buf;
	}
		

	const char* p = relative;

	if (!str_template::strcmp(p, "./", 3) 
		|| !str_template::strcmp(p, ".\\", 3))
		p += 2;

	// while relative path starts with ../ or ..\ skip it 
	size_t level = 0;
	while (!str_template::strcmp(p, "", 3) 
		|| !str_template::strcmp(p, "..\\", 3))
	{
		++level;
		p += 3;
	}

	char* pp = __min(strrchr(buf, '/'), strrchr(buf, '\\'));

	while (level)
	{
		if (!pp)
			return 0;

		*pp = 0;
		pp = __max(strrchr(buf, '/'), strrchr(buf, '\\'));
		--level;
	}

	// concat string
	size_t limit = strlen(buf);
	size_t add_len = strlen(p);

	if (limit + add_len + 1 >= len)
		return 0;

	strcat(buf, "/");
	strcat(buf, p);

	return buf;
}

#endif
//////////////////////////////////////////
xml_stream_attribute::xml_stream_attribute() : 
	_protocol(STREAM_UNKNOWN), _port(0), _query(false)
{
}

xml_stream_attribute::xml_stream_attribute(const char* url, bool full) : 
	_protocol(STREAM_UNKNOWN), _port(0), _query(false)
{
	if (!crack_xml_request(url, full))
		clear();
}

void 
xml_stream_attribute::clear()
{
	_protocol = STREAM_UNKNOWN;
	_host = 0;
	_port = 0;
	_user = 0;
	_password = 0;
	_path = 0;
	_file = 0;
	_extra = 0;
	_query = false;
}

bool 
xml_stream_attribute::complete_to_full(const xml_stream_attribute& full_url)
{
	if (_protocol != STREAM_RELATIVE)
		return false;

	_protocol = full_url._protocol;
	_host = full_url._host;
	_port = full_url._port;
	_user = full_url._user;
	_password = full_url._password;
	// concate pathes
	string_t old_path = _path;
	_path = full_url._path;
	_path += old_path;
	return true;
}

bool 
xml_stream_attribute::combine_url(string_t& url) const
{
	url = 0;

	switch (_protocol)
	{
		case STREAM_UNKNOWN:
			return false;
		case STREAM_LOCAL:
		case STREAM_RELATIVE:
			break;
		case STREAM_HTTP:
			url += protocolHTTP;
			url += protocolTale;
			break;
		case STREAM_FTP:
			url += protocolFTP;
			url += protocolTale;
			break;
		case STREAM_FILE:
			url += protocolFILE;
			url += protocolTale;
			break;
		default:
			assert(false);
	}

	if (_user.length())
	{
		url += _user;
		if (_password.length())
		{
			url += str_Colon;
			url += _password;
		}

		url += str_At;
	}

	if (_host.length())
		url += _host;

	if (_path.length())
		url += _path;

	if (_file.length())
		url += _file;

	if (_extra.length())
	{
		url += _query ? str_Question : str_Pound;
		url += _extra;
	}

	return true;
}

void
xml_stream_attribute::prepare_socket_request(string_t& url) const
{
	url = "GET ";

	if (_path.length() || _file.length())
	{
		url += _path;
		url += _file;
	}
	else
		url += "/";

	// correction for default page

	if (_extra.length())
	{
		url += _query ? str_Question : str_Pound;
		url += _extra;
	}

	url += " HTTP/1.0\r\n Host: ";
	url += _host;
    if (_port)
    {
		url += str_Colon;
		char buf[16];
		str_template::strprint(buf, 16, "%hu", _port);
		url += buf;
    }

    url += "\r\n\r\n";
}

void
xml_stream_attribute::prepare_http_request(const char* agent, string_t& url) const
{
	url = "GET ";

	if (_path.length() || _file.length())
	{
		url += _path;
		url += _file;
	}
	else
		url += "/";

	// correction for default page

	if (_extra.length())
	{
		url += _query ? str_Question : str_Pound;
		url += _extra;
	}

	url += " HTTP/1.0\r\nHost: ";
	url += _host;
    if (_port)
    {
		url += str_Colon;
		char buf[16];
		str_template::strprint(buf, 16, "%hu", _port);
		url += buf;
    }

	url += "\r\n";
	url += "User-Agent: ";
	url += agent;
	url += "\r\nCache-Control:no-cache\r\nAccept-Encoding:*\r\nConnection: Keep-Alive\r\nContent-Type: text/xml\r\nContent-Length: 0\r\n\r\n";
}

bool
xml_stream_attribute::crack_xml_request(const char* url, bool full)
{
	// internet format protocol://user:password@host:port/path?(#)extra
	// local c:\path
	// net \\server\path
	// relative path /path
	if (!url) return false;
	// skip white chars
	while (*url == ch_space) ++url;

	// make a pointer copy
	const char* url_ = url;
	static const char protocolDelimeter[]	= { ch_colon, ch_forward_slash, ch_back_slash, ch_null };
	static const char hostDelimeter[]		= { ch_forward_slash, ch_back_slash, ch_pound, ch_question, ch_null };
	static const char extraDelimeter[]		= { ch_pound, ch_question, ch_null };
	static const char fileDelimeter[]		= { ch_forward_slash, ch_back_slash, ch_null };

	// test local
	// local c:\path\file.ext
    if (is_letter(*url_) && *(url_ + 1) == ch_colon && (*(url_ + 2) == ch_forward_slash || *(url_ + 2) == ch_back_slash))
	{
		_protocol = STREAM_LOCAL;
		_host.assign(url_, 2);
		const char* file_begin_forward = strrchr(url_ + 2, ch_forward_slash);
		const char* file_begin_back = strrchr(url_ + 2, ch_back_slash);
		const char* file_begin = __max(file_begin_forward, file_begin_back);
		if (file_begin)
		{
			_file.assign(file_begin);
			_path.assign(url_ + 2, file_begin - url_ - 2);
		}
		else
			_file.assign(url_ + 2);

		return true;
	}

	// test net
	// net \\server\path\file.ext
	if ((*url_ == ch_forward_slash || *url_ == ch_back_slash)
		&& (*(url_ + 1) == ch_forward_slash || *(url_ + 1) == ch_back_slash))
	{
		_protocol = STREAM_LOCAL;
		const char* host_end = strpbrk(url_ + 2, fileDelimeter);
		_host.assign(url_, host_end ? host_end - url_ : os_minus_one);
		if (host_end)
		{
			const char* file_begin_forward = strrchr(host_end + 1, ch_forward_slash);
			const char* file_begin_back = strrchr(host_end + 1, ch_back_slash);
			const char* file_begin = __max(file_begin_forward, file_begin_back);
			if (file_begin)
			{
				_file.assign(file_begin);
				_path.assign(host_end, file_begin - host_end);
			}
			else
				_file.assign(host_end);
		}
		return true;
	}

	// internet format protocol://user:password@host:port/path?(#)extra
	// can be /path?(#)extra
	// or event simply path?(#)extra
	// looking for a protocol http, ftp, file
	if (!str_template::strnocasecmp(url_, protocolHTTP, strlen(protocolHTTP)))
	{
		_protocol = STREAM_HTTP;
		_port = 80;
		url_ += strlen(protocolHTTP);
	}
	else if (!str_template::strnocasecmp(url_, protocolFTP, strlen(protocolFTP)))
	{
		_protocol = STREAM_FTP;
		_port = 21;
		url_ += strlen(protocolFTP);
	}
	else if (!str_template::strnocasecmp(url_, protocolFILE, strlen(protocolFILE)))
	{
		_protocol = STREAM_FILE;
		_port = 0;
		url_ += strlen(protocolFILE);
	}
	else
	{
		_protocol = STREAM_RELATIVE;
#if OS_TYPE == OS_WIN32
		if (*url_ == ch_forward_slash || *url_ == ch_back_slash) // skip slash only for Windows
			++url_;
#endif

	}

	if (_protocol != STREAM_RELATIVE)
	{
		// http, ftp or file protocols must be accomplished by ://
		if (*url_ != ch_colon
			|| *(url_ + 1) != ch_forward_slash && *(url_ + 1) != ch_back_slash 
			|| *(url_ + 2) != ch_forward_slash && *(url_ + 2) != ch_back_slash)
			return false;

		// skip "://"
		url_ += 3;
		if (!*url_) // after protocol must be host at least
			return false;

		// analyze host
		const char* host_end = strpbrk(url_, hostDelimeter);
		if (host_end != url_) // we found host
		{
			// user:password@host:port
			const char* user_end = strchr(url_, ch_at);
			if (user_end)
			{
				const char* password_begin = strchr(url_, ch_colon);
				if (password_begin)
					// adjust password
					_password.assign(password_begin + 1, user_end - password_begin - 1); 

				// adjust user
				_user.assign(url_, password_begin ? password_begin - url_ : user_end - url_);
				// adjust host
				const char* port_begin = strchr(user_end + 1, ch_colon);
				if (port_begin)
				{
					if (!str_template::strscan(port_begin + 1, 32, "%hu", &_port))
						return false;

					_host.assign(user_end + 1, port_begin - user_end - 1);
				}
				else
					_host.assign(user_end + 1, host_end ? host_end - user_end - 1 : os_minus_one);
			} // if
			else // adjust host
			{
				const char* port_begin = strchr(url_, ch_colon);
				if (port_begin)
				{
					if (!str_template::strscan(port_begin + 1, 32, "%hu", &_port))
						return false;

					_host.assign(url_, port_begin - url_);
				}
				else
					_host.assign(url_, host_end ? host_end - url_ : os_minus_one);
			}
		} // if

		// skip url to end of host
		url_ = host_end;

		if (!url_)
			return true;
		else if (*url_ == ch_forward_slash || *url_ == ch_back_slash) // path
			++url_;
	} // if host
	else if (full) // relative must be complited
	{
		char buf[_MAX_PATH + 1];
		if (!_fullpath(buf, url_, _MAX_PATH))
			return false;

		_protocol = STREAM_LOCAL;
		_host.assign(buf, 2);
		const char* file_begin_forward = strrchr(buf + 3, ch_forward_slash);
		const char* file_begin_back = strrchr(buf + 3, ch_back_slash);
		const char* file_begin = __max(file_begin_forward, file_begin_back);
		if (file_begin)
		{
			_file.assign(file_begin);
			_path.assign(buf + 2, file_begin - buf - 2);
		}
		else
			_file.assign(buf + 2);
		return true;
	}

	// we are here to find relative path or not
	// starting with delimeter '?#' then no path
	if (*url_ != ch_pound && *url_ != ch_question)
	{
		const char* file_end = strpbrk(url_, extraDelimeter);
		const char* file_begin_forward = strrchr(url_, ch_forward_slash);
		const char* file_begin_back = strrchr(url_, ch_back_slash);
		const char* file_begin = __max(file_begin_forward, file_begin_back);
		if (file_begin)
		{
			_file.append(file_begin, file_end ? file_end - file_begin : os_minus_one);
			_path = str_ForwardSlash;
			_path.append(url_, file_begin - url_);
		}
		else
		{
			_file = str_ForwardSlash;
			_file.append(url_, file_end ? file_end - url_ : os_minus_one);
		}
		
		// skip url to end of path
		url_ = file_end;
		if (!url_)
			return true;
	}

	// skip also # or ?
	_extra.assign(url_ + 1);
	_query = *url_ == ch_question;

	return true;
}

bool
xml_stream_attribute::complete_to_full(const char* location, const char* full_path, string_t& res)
{
	xml_stream_attribute attr_new(location, false);
	*this = attr_new;
	if (_protocol == STREAM_UNKNOWN)
	{
		res = "Unknown url format: ";
		res += location;
		return false;
	}

	if (_protocol == STREAM_RELATIVE)
	{
		xml_stream_attribute attr_full(full_path, true);
		if (!complete_to_full(attr_full))
		{
			res = "Can't complete relative path: ";
			res += location;
			return false;
		}
	}

	return combine_url(res);
}

//////////////////////////////////////////////////////////////
// static
void
buffer_loader::load(const char* url, const char* location, mem_pool_t& small_pool, mem_pool_t& big_pool, paged_buffer& buffer, bool subset)
{
	string_t full_path;
	xml_stream_attribute attr_new;
	if (!attr_new.complete_to_full(location, url, full_path))
	{
		string_t ex("Can't create full path from location: ");
		ex += location;
		ex += " and url: ";
		ex += url;
		exception::_throw(ex);
	}

	stream_input_common stream(small_pool, big_pool, 0, subset);
	if (!stream.open(attr_new))
	{
		string_t ex("Can't open external file: ");
		ex += location;
		exception::_throw(ex);
	}

	ub1_t* lbuf = (ub1_t*)buffer.get_tmp_allocator().allocate(os_def_size);
	size_t len;
	while (0 != (len = stream.pull(lbuf, os_def_size)))
		buffer.append(lbuf, len);

	stream.close();
}


////////////////////////////////
stream_input_memory::stream_input_memory(const ub1_t* buffer, size_t size, mem_pool_t& small_pool, mem_pool_t& big_pool, size_t xml_size, bool subset) :
	byte_source(small_pool, big_pool, xml_size, 0, subset), 
	_external_buffer(buffer), 
	_external_size(size), 
	_external_pos(0) 
{}

// virtual 
bool 
stream_input_memory::data_request(ub1_t* buf, size_t& len)
{
	if (_external_pos == _external_size)
		return false;

	len = __min(len, _external_size - _external_pos);
	memcpy(buf, _external_buffer + _external_pos, len);
	_external_pos += len;
	return true;
}


//////////////////////////////////////////
///////////////////////////////////////////
stream_input_common::stream_input_common(mem_pool_t& small_pool, mem_pool_t& big_pool, size_t xml_size, bool subset, size_t timeout) :
	byte_source(small_pool, big_pool, xml_size, 0, subset),
	terimber_socket(),
	_protocol(STREAM_UNKNOWN),
	_desc(0),
	_timeout(timeout),
	_file_length(0)
{
}
	
stream_input_common::~stream_input_common()
{
	close();
}

// virtual 
bool 
stream_input_common::data_request(ub1_t* buf, size_t& len)
{ 
	switch (_protocol)
	{
		case STREAM_LOCAL:
			{
				size_t shift = 0;
				size_t requested_len = len;
				len = 0;
				while (requested_len)
				{
					int gotLen = (int)::fread(buf + shift, 1, requested_len, _desc);
					if (gotLen < 0)
					{
						len = 0;
						close();
						return false;
					}
					
					if (gotLen == 0)
						break;

					requested_len -= gotLen;
					len += gotLen;
					shift += gotLen;
				}

				return len != 0;
			}
		case STREAM_HTTP:
			{
				if (!_file_length || !len) return false;
				len = __min(_file_length, (ub4_t)len);
				if (terimber_socket::receive((char*)buf, len, _timeout))
				{
					close();
					len = 0;
					return false;
				}
			
				_file_length -= (ub4_t)len;
				return true;
			}
		case STREAM_FTP: // don't support yet
		case STREAM_FILE: // don't support yet
		case STREAM_UNKNOWN: // don't know what to do
		case STREAM_RELATIVE: // don't support releative path
		default:
			len = 0;
			return false;
	} // switch
}

bool 
stream_input_common::open(const xml_stream_attribute& location)
{
	if (_protocol != STREAM_UNKNOWN) return false;

	string_t url;

	if (!location.combine_url(url)) return false;

	switch (location._protocol)
	{
		case STREAM_UNKNOWN: // don't know what to do
		case STREAM_RELATIVE: // don't support releative path
			return false;
		case STREAM_LOCAL:
			if (!(_desc = ::fopen(url, "rb"))) 
				return false;
			break;
		case STREAM_HTTP:
			{
				// parse url and prepare HTTP GET request
				_file_length = 0;
				string_t http_request;
				// prepare HTTP text
				location.prepare_socket_request(http_request);
				// try to resolve server name to the IP address
				sockaddr_in addr;
				if (resolve_address(location._host, location._port, addr)) return false;
				// set socket address
				set_address(addr);
				// try to establish socket connection
				if (!connect()) return false;
				// send http get request
				if (terimber_socket::send(http_request, http_request.length(), _timeout))	{ terimber_socket::close(); return false; }
				// extract response size
				// read header
				// HTTP/1.0 200 OK
				const size_t buf_size = 256;
				char header[buf_size];
				http_request = 0;
				size_t shift = 0;
				size_t counter = 0;
				for (counter = 0; counter < 4;)
				{
					if (terimber_socket::receive(header + shift, 1, _timeout)) { terimber_socket::close(); return false; }

					if (header[shift] == ch_cr && (counter == 0 || counter == 2)
						|| header[shift] == ch_lf && (counter == 1 || counter == 3))
						++counter;
					else
						counter = 0;

					// in case of a large headers
					if (++shift == buf_size - 1)
					{
						// set final byte to zero
						header[shift] = 0;
						// append text to string
						http_request += header;
						// move memory
						shift = 0;
					} // if
				} // for

				// set text
				header[shift] = 0;
				// append text to string
				http_request += header;

				// find HTTP 200 OK
				// HTTP
				const char* str = strstr(http_request, "HTTP");
				if (!str) { close(); return false; }
				// find space after HTTP
				if (!(str = strchr(str, ' '))) { close(); return false; }
				// skip
				while (*str == ' ') ++str;
				if (strncmp(str, "200", 3)) { close(); return false; }
				// read file length
				// Content-Length:
				if (!(str = strstr(str, "Content-Length:"))) { close(); return false; }
				// skip Content-Length:
				str += strlen("Content-Length:");
				while (*str == ' ') ++str;
				if (!str_template::strscan(str, 32, "%d", &_file_length)) { close(); return false; }
			}
			break;
		case STREAM_FTP: // don't support yet
		case STREAM_FILE: // don't support yet
			return false;
	}

	_protocol = location._protocol;
	_url = url;
	return true;
}

void 
stream_input_common::close()
{
	switch (_protocol)
	{
		case STREAM_UNKNOWN: // don't know what to do
			return;
		case STREAM_RELATIVE: // don't support releative path
			break;
		case STREAM_LOCAL:
			fclose(_desc);
			_desc = 0;
			break;
		case STREAM_HTTP:
			terimber_socket::close();
			_file_length = 0;
			break;
		case STREAM_FTP: // don't support yet
		case STREAM_FILE: // don't support yet
			break;
	}

	_protocol = STREAM_UNKNOWN;
}

////////////////////////////////////////////////////////////////
stream_output_file::stream_output_file(mem_pool_t& small_pool, mem_pool_t& big_pool, size_t xml_size) : 
	byte_consumer(small_pool, big_pool, xml_size), 
	_desc(0)
{
}

bool 
stream_output_file::open(const char* location)
{
	if (_desc)
		return false;

	if (!(_desc = ::fopen(location, "wb")))
		return false;

	return true;
}

void 
stream_output_file::close()
{
	if (_desc)
	{
		// 
		::fclose(_desc);
		_desc = 0;
	}
}

// virtual
bool 
stream_output_file::data_persist(const ub1_t* buf, size_t len)
{
	if (!_desc)
		return false;
	
	size_t shift = 0;
	while (len)
	{
		int putLen = (int)::fwrite(buf + shift, 1, len, _desc);
		if (putLen < 0)
			return false;
		
		if (putLen == 0)
			break;

		len -= putLen;
		shift += putLen;
	}

	return !len;
}

// virtual
stream_output_file::~stream_output_file()
{
	//flush();
	close();
}

///////////////////////////////////////////////
memory_output_stream::memory_output_stream(mem_pool_t& small_pool, mem_pool_t& big_pool, size_t xml_size, ub1_t* buf, size_t buf_size) :
	byte_consumer(small_pool, big_pool, xml_size), _buf(buf), _buf_size(buf_size), _filled_size(0), _required_size(0)
{
}

size_t 
memory_output_stream::get_required_size() const
{
	return _required_size;
}

bool 
memory_output_stream::is_overflow() const
{
	return _required_size - _filled_size > 0;
}

// virtual 
bool 
memory_output_stream::data_persist(const ub1_t* buf, size_t len)
{
	// check availability
	size_t copy_len = __min(len, _buf_size - _filled_size);
	if (_buf && copy_len)
	{
		memcpy(_buf + _filled_size, buf, copy_len);
		_filled_size += copy_len;
	}

	_required_size += len;
	return true; // 
	//len -= copy_len;
	//return !len;
}

#pragma pack()
END_TERIMBER_NAMESPACE

