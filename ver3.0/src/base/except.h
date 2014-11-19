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

#ifndef _terimber_except_h_
#define _terimber_except_h_

#include "base/string.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class exception_item
//! \brief exception item as an exception table row
class exception_item
{
public:
	//! \brief less operator for sort algorithm
	bool operator <(const exception_item& x) const;
	//! \brief equal operator for find algorithm
    bool operator ==(const exception_item& x ) const;
	
	size_t			_code;									//!< exception code
	const char*		_desc;									//!< exception description
};

//! \class exception_table
//! \brief base class for exception tables
struct exception_table
{
	//! \brief constructor
	exception_table(exception_item* items					//!< array of items	
					);
	//! \brief returns exception description according to code
	const char* 
	get_error(		size_t code								//!< exception code
	);

private:
	exception_item*		_items;								//!< pointer to the first item
	size_t				_len;								//!< count of items
};

//! \class exception
//! \brief supports exception
class exception
{
public:
	//! \brief constructor
	exception(		size_t id,								//!< exception ident
					const wchar_t* desc						//!< exception description (wide string)
					);
	//! \brief constructor
	exception(		size_t id,								//!< exception ident
					const char* desc						//!< exception description (narrow string)
					);
	//! \brief destructor
	virtual ~exception();
	//! \brief returns exception code
	size_t 
	get_code() const;
	//! \brief the exception description is searched in specified table
	//! or operation system error is used
	static 
	void 
	extract_os_error(size_t code,							//!< exception code
					string_t& err							//!< [out] error description
					);
	//! \brief creates exception object
	//! and throws exception
	//! the exception description is searched in specified table
	static 
	void 
	_throw(			size_t code,							//!< exception code
					exception_table* table					//!< exception table
					);
	//! \brief creates exception object
	//! and throws exception
	static 
	void 
	_throw(			const wchar_t* desc						//!< exception description (wide string)
					);
	//! \brief creates exception object
	//! and throws exception
	static 
	void 
	_throw(			const char* desc						//!< exception description (narrow string)
					);
	//! \brief returns exception description
	//! and throws exception
	const char* 
	what() const;
private: 
	size_t			_id;									//!< exception ident
	string_t		_reason;								//!< exception description in utf-8
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_except_h_

