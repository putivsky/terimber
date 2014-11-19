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

#ifndef _terimber_varvalue_h_
#define _terimber_varvalue_h_

#include "allinc.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class var_value 
//! \brief class decribe the extension to the 
//! terimber_db_value is a variant substitute
class var_value
{
public:
	terimber_xml_value		_value;							//!< value itself
	bool					_not_null;						//!< indicator of NULL value - variant specific
	bool					_specified;						//!< flag specified
};

//! \class var_string_searchable_key
//! \brief searchable string
class var_string_searchable_key
{
public:
	
	const char*					_res;						//!< string resource
	const ub4_t*				_offsets;					//!< array of offsets to the tokens, empty if resource is one word
};

//! \class var_string_fuzzy_key
//! \brief fuzzy match string key
class var_string_fuzzy_key
{
public:
	
	const char*					_res;						//!< string resource
	size_t						_fuzzy_key;					//!< array of offsets to the tokens, first 4 bytes is array size, = 0 if it's one word
};

//! \union var_key
//! \brief combination of different key types
union var_key
{
	class var_value					_val;
	class var_string_searchable_key	_key;
	class var_string_fuzzy_key		_ngram;
};

//! \brief operator less 
inline 
bool 
operator_less(		vt_types type,							//!< variant type
					const var_value& x,						//!< first value
					const var_value& y						//!< second value
					);

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_varvalue_h_
