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

#ifndef _terimber_miscxml_h_
#define _terimber_miscxml_h_

#include "xml/storexml.h"
#include "xml/socket.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \enum xml_stream_protocol
//! \brief byte source protocol
enum xml_stream_protocol
{
	STREAM_UNKNOWN,											//!< unknown protocol
	STREAM_LOCAL,											//!< local file
	STREAM_RELATIVE,										//!< relative path
	STREAM_HTTP,											//!< http protocol
	STREAM_FTP,												//!< ftp protocol
	STREAM_FILE												//!< file protocol
};

//! \class xml_stream_attribute
//! \brief xml stream attribute processing
class xml_stream_attribute
{
public:
	//! \brief default constructor
	xml_stream_attribute();
	//! \brief constructor
	xml_stream_attribute(const char* url,					//!< url
					bool full								//!< full url flag
					);
	//! \brief breaks url into parts
	bool 
	crack_xml_request(const char* url,						//!< url
					bool full								//!< full url flag
					);
	//! \brief construct url 
	bool 
	complete_to_full(const xml_stream_attribute& full_url	//!< input full url
					);
	//! \brief combines the full url from internal parts
	bool 
	combine_url(	string_t& url							//!< [out] url
					) const;
	//! \brief prepares socket request
	void 
	prepare_socket_request(string_t& url					//!< [out] socket request
					) const;
	//! \brief prepares http request
	void 
	prepare_http_request(const char* agent,					//!< http agent name
					string_t& url							//!< [out] http request
					) const;
	//! \brief completes url
	bool 
	complete_to_full(const char* location,					//!< location
					const char* full_path,					//!< full path
					string_t& res							//!< [out] full url
					);
private:
	//! \brief clears internal parts
	void 
	clear();
public:
	xml_stream_protocol		_protocol;						//!< protocol
	string_t				_host;							//!< host
	ub2_t					_port;							//!< port
	string_t				_user;							//!< user name
	string_t				_password;						//!< password
	string_t				_path;							//!< path
	string_t				_file;							//!< file name
	string_t				_extra;							//!< http parameters
	bool					_query;							//!< query flag
};

//! \class stream_input_memory
//! \brief buffer in memory interface of stream
class stream_input_memory : public byte_source
{
public:
	//! \brief constructor
	stream_input_memory(const ub1_t* buffer,				//!< input pointer to memory buffer
					size_t size,							//!< buffer size
					mem_pool_t& small_pool,					//!< small memory pool
					mem_pool_t& big_pool,					//!< big memory pool
					size_t xml_size,						//!< xml size - just a tip
					bool subset								//!< subset flag
					);

protected:
	//! \brief gets actual bytes from buffer
	virtual 
	bool 
	data_request(	ub1_t* buf,								//!< pre-allocated buffer
					size_t& len								//!< [in,out] [in] buffer length, [out] obtained bytes
					);

private:
	const ub1_t*	_external_buffer;						//!< pointer to the external memory buffer
	size_t			_external_size;							//!< buffer size
	size_t			_external_pos;							//!< current position
};

//////////////////////////////////////////////////////////////////
//! \class stream_input_common
//! \brief interface of stream implementation for reading from file, socket
class stream_input_common : public byte_source, 
							public terimber_socket
{
public:
	//! \brief constructor
	stream_input_common(mem_pool_t& small_pool,				//!< small memory pool
					mem_pool_t& big_pool,					//!< big memory pool
					size_t xml_size,						//!< xml size - just a tip
					bool subset,							//!< subset flag
					size_t timeout = 10000					//!< timeout for sockets
					);
	//! \brief opens stream source
	bool 
	open(			const xml_stream_attribute& location	//!< location attribute
					);
	//! \brief closes stream
	void 
	close();
	//! \brief destructor
	virtual 
	~stream_input_common();

protected:
	//! \brief gets actual data from stream
	virtual 
	bool 
	data_request(	ub1_t* buf,								//!< pre-allocated buffer
					size_t& len								//!< [in,out] [in] buffer size, [out] obtained bytes
					);
private:
	xml_stream_protocol		_protocol;						//!< stream protocol
	FILE*					_desc;							//!< file decriptor
	size_t					_timeout;						//!< timeout
	ub4_t					_file_length;					//!< file length
};

///////////////////////////////////////////
//! \class memory_output_stream
//! \brief interface of stream for xml in memory
class memory_output_stream : public byte_consumer
{
public:
	//! \brief constructor
	memory_output_stream(mem_pool_t& small_pool,			//!< small memory pool
					mem_pool_t& big_pool,					//!< big memory pool
					size_t xml_size,						//!< xml size - just a tip
					ub1_t* buf,								//!< output buffer
					size_t buf_size							//!< output buffer size
					);
	//! \brief returns required size
	size_t 
	get_required_size() const;
	//! \brief checks if the buffer would overflow
	bool 
	is_overflow() const;
protected:
	//! \brief saves the actual bytes
	virtual 
	bool 
	data_persist(	const ub1_t* buf,						//!< buffer to save
					size_t len								//!< buffer length
					);
private:
	ub1_t*		_buf;										//!< internal buffer
	size_t		_buf_size;									//!< internal buffer size
	size_t		_filled_size;								//!< bytes processed
	size_t		_required_size;								//!< required size
};

///////////////////////////////////////////
//! \class stream_output_file
//! \brief file interface of stream
class stream_output_file : public byte_consumer
{
public:
	//! \brief constructor
	stream_output_file(mem_pool_t& small_pool,				//!< small memory pool
					mem_pool_t& big_pool,					//!< big memory pool
					size_t xml_size							//!< xml size - just a tip
					);
	//! \brief opens file
	bool 
	open(			const char* location					//!< file name
					);
	//! \brief closes file
	void 
	close();
	//! \brief destructor
	virtual 
	~stream_output_file();

protected:
	//! \brief saves bytes to file
	virtual 
	bool 
	data_persist(	const ub1_t* buf,						//!< buffer to save
					size_t len								//!< buffer length
					);
private:
	FILE*	_desc;											//!< file descriptor
};

////////////////////////////////////////////////////////
//! \class buffer_loader
//! \brief helper class to load bytes from stream
class buffer_loader
{
public:
	//! \brief loads ub1_t stream from location
	//! it can be file, http, ftp locations
	//! and put chars into paged buffer
	static 
	void 
	load(			const char* url,						//!< url
					const char* location,					//!< location
					mem_pool_t& small_pool,					//!< small memory pool
					mem_pool_t& big_pool,					//!< bug memory pool
					paged_buffer& buffer,					//!< paged buffer
					bool subset								//!< subset flag
					);
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_miscxml_h_ 
