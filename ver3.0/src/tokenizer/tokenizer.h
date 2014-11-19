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

#ifndef _terimber_tokenizer_h_
#define _terimber_tokenizer_h_

#include "allinc.h"
#include "base/list.h"
#include "base/map.h"
#include "base/string.h"
#include "pcre/pcre.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

const size_t T_REGEX = 0x00000001;
const size_t T_ABBR = 0x00000002;
const size_t T_HYPHEN = 0x00000004;
const size_t T_ALL = T_REGEX | T_ABBR | T_HYPHEN;

//! \enum token_type 
//! \brief atomic token type
enum token_type
{
	TT_UNKNOWN = 0,											//!< unknown type
	TT_REGEX,												//!< regular expressionm 2005-11-11 12:00:00.333
	TT_ABBR,												//!< abbreviation Dr., Mr., Ms., Gen.
	TT_COMPOSE,												//!< composite token semi-final, Cup-2005, F-117
	TT_ALPHABETIC,											//!< alpha-betic Terimber					
	TT_DIGIT,												//!< digit 1, 345
	TT_WHITESPACE,											//!< white space blank, tabs, new lines, caret, ...
	TT_SYMBOL,												//!< symbol #$%^...
	TT_PUNCTUATION,											//!< punctuation ,:;!?-
	TT_DOT													//!< dot
};

//! \class token_info
//! \brief token information
class token_info
{
public:
	//! \brief constructor
	token_info(		token_type type,						//!< token type
					size_t len,								//!< length in bytes
					size_t key								//!< key for regular expression
					) :					
		_type(type), 
		_len(len), 
		_key(key) 
	{
	}

	token_type	_type;										//!< token type
	size_t		_len;										//!< length in bytes
	size_t		_key;										//!< key for regular expression
};

//! \class pcre_key
//! \brief pcre key
class pcre_key
{
public:
	//! \brief constructor
	pcre_key(		size_t min,								//!< min tokens 
					size_t max								//!< max tokens
					) : 
		_min(min), 
		_max(max) 
	{
	}

	size_t			_min;									//!< min tokens in regular expression
	size_t			_max;									//!< max tokens in regular expression

	//! \brief operator<
	inline 
	bool 
	operator<(const pcre_key& x) const
	{ 
		return _max < x._max; 
	}
};

//! \class pcre_entry
//! \brief pcre entry
class pcre_entry
{
public:
	//! \brief constructor
	pcre_entry(		pcre* th,								//!< pcre engine instance
					pcre_extra* ex,							//!< pcre extra pointer
					size_t key								//!< pcre key
					) : _this(th), _extra(ex), _key(key) {}

	pcre*			_this;									//!< pcre engine instance
	pcre_extra*		_extra;									//!< pcre extra pointer
	size_t			_key;									//!< pcre key
};

//! \typedef tokenizer_output_sequence_t
//! \brief list of detected tokens
typedef _list< token_info >									tokenizer_output_sequence_t;
//! \typedef abbreviation_map_t
//! \brief abbreviation map
typedef map< size_t, string_t, less< size_t >, true >		abbreviation_map_t;
//! \typedef regex_map_t
//! \brief pcre expression multimap
typedef map< pcre_key, pcre_entry, less< pcre_key >, true >	regex_map_t;

//! \class tokenizer
//! \brief class tokenize input string finding atomic tokens
class tokenizer
{
public:
	//! \brief constructor
	tokenizer();
	//! \brief destructor
	~tokenizer();

	//! \brief adds regular expressions to the internal map
	//! tokenizer will recognize the longest possible regular expression
	bool 
	add_regex(		const char* regex,						//!< regular expression
					size_t key,								//!< a parameter, which will be assigned to the detected regular expression
					size_t min,								//!< minimum atomic tokens in a regular expression
					size_t max								//!< maximum atomic tokens in a regular expression
					);
	
	//! \brief loads regular expressions from file
	//! file has to have the following format
	//! regex tab key tab min tab max endline
	bool 
	load_regex(		const char* file_name					//!< file name
					);

	//! \brief cleans up regular expressions
	void 
	clear_regex();

	//! \brief adds abbreviation to the internal map
	bool 
	add_abbreviation(const char* abbr						//!< abbreviation
					);

	//! \brief loads abbreviations from file
	//! file has to have the following format
	//! abbr\n
	bool 
	load_abbr(		const char* file_name					//!< file name
					);

	//! \brief cleans up abbreviations
	void 
	clear_abbr();

	//! \brief tokenizes string into array of tokens,
	bool 
	tokenize(		const char* str,						//!< input string
					tokenizer_output_sequence_t& out,		//!< [out] tokenized atomic tokens
					byte_allocator& all,					//!< allocator
					size_t flags = T_ALL					//!< tokenization flags 
					) const;

	//! \brief gets the last error
	const string_t& 
	get_last_error() const 
	{ 
		return _error; 
	}
private:
	//! \brief clears all internal resouces
	void 
	clear();
	//! \brief processes regular expression, if any
	void 
	do_regex(		const char* phrase,						//!< input phrase
					tokenizer_output_sequence_t& tokens		//!< [in,out] atomic tokens
					) const;
	//! \brief processes abbreviations
	void 
	do_abbr(		const char* phrase,						//!< input phrase
					tokenizer_output_sequence_t& tokens		//!< [in,out] atomic tokens
					) const;
	//! \brief does hyphens
	void 
	do_hyphen(		const char* phrase,						//!< input phrase
					tokenizer_output_sequence_t& tokens		//!< [in,out] atomic tokens
					) const;

	//! \brief matches the input string and the regular expression
	size_t 
	match(			const char* x,							//!< input string
					size_t len,								//!< length
					size_t tokens,							//!< tokens to match
					size_t& key								//!< [out] regex key
					) const;
private:
	string_t					_error;						//!< error
	abbreviation_map_t			_abbr_map;					//!< map of hashed abbreviations
	regex_map_t					_regex_map;					//!< map of regular expression

};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif
