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

#ifndef _terimber_storexml_h_
#define _terimber_storexml_h_

#include "xml/declxml.h"
#include "base/vector.h"
#include "base/common.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \typedef store_list_t
//! \brief list of byte buffers pointers
typedef _list< ub1_t* >		store_list_t;

////////////////////////////////
//! \class byte_source
//! \brief base that does all the work except getting bytes from stream
class byte_source
{
public:
	//! \brief constructor
	byte_source(	mem_pool_t& small_pool,					//!< small memory pool
					mem_pool_t& big_pool,					//!< big memory pool
					size_t xml_size,						//!< xml size - just a tip
					const char* url,						//!< optional location
					bool subset								//!< subset flag
					);

	//! \brief destructor
	virtual 
	~byte_source();
	//! \brief picks byte
	xml_forceinline 
	ub1_t 
	pick();
	//! \brief pops byte operator++(int)
	xml_forceinline 
	ub1_t 
	pop();
	//! \brief pips byte operator++()
	xml_forceinline 
	ub1_t 
	pip();
	//! \brief skips junk chars defined by is_white_space() function
	//! if not then throw exception with specified message
	xml_forceinline 
	void 
	skip_white_space(bool mustPresent = false,				//!< flag of white space presence
					const char* message = 0					//!< error message
					);
	//! \brief throws exception adding line and char postion information
	void 
	throw_exception(const char* msg_text					//!< message text
					);
	//! \brief skips string
	//! if not, then throws exception with specified message
	void 
	skip_string(	const char* x,							//!< string to skip
					const char* message						//!< error message
					);

	//! \brief skips one byte
	//! if not, then throws exception with specified message
	void 
	skip_sign(		ub1_t symbol,							//!< byte to skip
					bool skip_before,						//!< check white spaces before
					bool skip_after,						//!< check white spaces after
					const char* message						//!< error message
					);
	//! \brief skips quote symbol
	//! it can be single quote or double quote
	//! if input parameter = 0 then function detects the type of quote
	//! and returns the detected quote symbol
	//! if input parameter = quote symbol function skip the specified quote symbol
	//! in case of problems function throws exception
	ub1_t 
	skip_quote(		ub1_t symbol = 0						//!< optional quote symbol
					);
	//! \brief returns current char position
	inline 
	size_t 
	current_pos() const;
	//! \brief inserts into head of sequence
	void 
	push(			const ub1_t* x,							//!< buffer
					size_t len								//!< buffer length
					);
	//! \brief pushes byte back to buffer head
	inline 
	void 
	push(			ub1_t x									//!< byte
					);

	//! \brief pulls block of bytes from stream
	size_t 
	pull(			ub1_t* x,								//!< pre-allocated buffer
					size_t len								//!< buffer length
					);
	//! \brief returns detected encoding schema
	inline 
	encodingSchema 
	get_encoding() const;
	//! \brief sets encoding
	inline 
	void 
	set_encoding(encodingSchema schema);
	//! \brief gets stream location
	inline 
	const char* 
	get_location() const;

protected:
	//! \brief pure virtual function
	//! getting bytes from stream
	virtual 
	bool 
	data_request(	ub1_t* buf,								//!< pre-allocated buffer
					size_t& len								//!< [in,out] [in] buffer length, [out] retrieved bytes
					) = 0;

	//! \brief parses xml declaration
	//! [23]    XMLDecl    ::=    '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
	encodingSchema 
	parseXMLDecl();
	//! \brief parses text declaration
	//! [77]    TextDecl    ::=    '<?xml' VersionInfo? EncodingDecl S? '?>' 
	encodingSchema 
	parseTextDecl();

private:
	//! \brief converts count bytes from stream to internal buffer
	bool 
	convert_chars(size_t count);
	//! \brief checks the encoding
	bool 
	taste_buffer();
	//! \brief does auto conversion
	bool 
	auto_convert();
	//! \brief gets bytes from stream
	ub1_t 
	go_shopping();
	//! \brief parses xml declaration info
	bool 
	parseXMLDeclInfo();
	//! \brief parses version
	void 
	parseVersion();
	//! \brief parses encoding
	encodingSchema 
	parseEncoding();
	//! \brief parses standalone info
	size_t 
	parseStandalone();
	//! \brief validates the specified schema
	encodingSchema 
	checkEncodingSchema(const char* schema					//!< external encoding
					);
	//! \brief scans the quoted value to make sure it is valid
	void 
	scanQuotedValue(ub1_t* value,							//!< pre-allocated buffer						
					size_t len,								//!< buffer length
					bool (*fn)(ub1_t),						//!< validity function
					const char* message						//!< error message
					);

public:
	//! \brief gets standalone flag
	size_t 
	get_standalone() const; // -1 - null, 1 - yes, 0 -no
	//! \brief gets subset flag
	bool get_subset() const;
	//! \brief gets version
	size_t 
	get_version() const;
	//! \brief xml size
	inline 
	size_t 
	get_xml_size() const;

protected:
	//! \brief gets buffer position
	inline 
	size_t 
	get_buffer_pos() const; 
	//! \brief resets internal buffer
	void 
	reset_buffer();
protected:
	byte_allocator*		_depot_allocator;					//!< depot allocator
	byte_allocator*		_convert_allocator;					//!< conversion allocator
	byte_allocator*		_store_allocator;					//!< temporary store allocator
	byte_allocator*		_list_allocator;					//!< store buffers allocator
	mem_pool_t&			_small_pool;						//!< small memory pool
	mem_pool_t&			_big_pool;							//!< big memory pool
	const size_t		_xml_size;							//!< xml size
	string_t			_url;								//!< the location of xml stream
private:
	bool				_subset;							//!< subset flag
	ub1_t*				_buffer;							//!< current buffer
	ub1_t*				_convert_buffer;					//!< convert buffer
	size_t				_buffer_pos;						//!< current buffer position
	ub1_t				_symbol;							//!< current symbol
	store_list_t		_active_store;						//!< store buffers in use
	store_list_t		_used_store;						//!< store buffers for reusing
	size_t				_line_counter;						//!< line number
	size_t				_char_counter;						//!< char position (for multibyte chars)
	size_t				_pos_counter;						//!< byte position
	encodingSchema		_encodingSchema;					//!< encoding schema
	size_t				_version;							//!< version
	size_t				_standalone;						//!< standalone flag
	bool				_end;								//!< end of stream flag
};

//////////////////////////////////////////////////////////////////
//! \class byte_consumer
//! \brief base class for output streams 
//! provides all the functionality except writing bytes into output stream
class byte_consumer
{
public:
	//! \brief constructor
	byte_consumer(	mem_pool_t& small_pool,					//!< small memory pool
					mem_pool_t& big_pool,					//!< big memory pool
					size_t xml_size							//!< xml size - just a tip
					);
	//! \brief destructor
	virtual
	~byte_consumer();
	//! \brief inserts into tail of sequence
	void 
	push(			const ub1_t* x,							//!< buffer
					size_t len								//!< buffer length
					);
	//! \brief inserts one symbol
	inline 
	void 
	push(			ub1_t x									//!< symbol
					);
	//! \brief inserts string
	inline 
	void 
	push(			const char* x							//!< string
					);
	//! \brief returns xml size
	inline 
	size_t 
	get_xml_size() const;
	//! \brief returns buffer position
	inline 
	size_t 
	get_buffer_pos() const;
	//! \brief flushes buffer to output stream
	void 
	flush();

protected:
	//! \brief pure virtual function
	//! writes buffer to the output stream
	virtual 
	bool 
	data_persist(	const ub1_t* buf,						//!< buffer
					size_t len								//!< buffer length
					) = 0;

private:
	byte_allocator*		_depot_allocator;					//!< depot allocator
	mem_pool_t&			_small_pool;						//!< small memory pool
	mem_pool_t&			_big_pool;							//!< big memory pool
	const size_t		_xml_size;							//!< xml size
	ub1_t*				_buffer;							//!< internal buffer
	size_t				_buffer_pos;						//!< internal buffer position
};


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_storexml_h_ 
