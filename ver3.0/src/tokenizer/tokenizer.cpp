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

#include "tokenizer/tokenizer.h"
#include "base/list.hpp"
#include "base/map.hpp"
#include "base/string.hpp"
#include "base/common.hpp"
#include "base/memory.hpp"
#include "xml/defxml.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

static
inline 
size_t 
do_hash_lowercase(const char* x, size_t len)
{ 
	size_t res = 0;
	if (x) while (len-- && *x) res = (res << 1) ^ tolower(*x++);
	return res;
}

static 
inline 
token_type
detect_token_type(char symbol)
{
	switch (symbol)
	{
		case ch_space:
		case ch_hor_tab:
		case ch_lf:
		case ch_ver_tab:
		case ch_ff:
		case ch_caret:
			return TT_WHITESPACE;
		case ch_dash:
		case ch_ampersand:
		case ch_asterisk:
		case ch_at:
		case ch_back_slash:
		case ch_close_angle:
		case ch_close_curly:
		case ch_close_square:
		case ch_close_paren:
		case ch_dollar_sign:
		case ch_double_quote:
		case ch_equal:
		case ch_forward_slash:
		case ch_grave:
		case ch_open_angle:
		case ch_open_curly:
		case ch_open_square:
		case ch_open_paren:
		case ch_percent:
		case ch_pipe:
		case ch_plus:
		case ch_pound:
		case ch_tilde:
		case ch_underscore:
		case ch_single_quote:
			return TT_SYMBOL;
		case ch_colon:
		case ch_semicolon:
		case ch_bang:
		case ch_question:
		case ch_comma:
			return TT_PUNCTUATION;
		case ch_period:
			return TT_DOT;
		case ch_0:
		case ch_1:
		case ch_2:
		case ch_3:
		case ch_4:
		case ch_5:
		case ch_6:
		case ch_7:
		case ch_8:
		case ch_9:
			return TT_DIGIT;
		default:
			return TT_ALPHABETIC;
	}
}

tokenizer::tokenizer()
{
}

// destructor
tokenizer::~tokenizer()
{
	clear();
}

void 
tokenizer::clear()
{
	clear_abbr();
	clear_regex();
}


bool 
tokenizer::add_regex(const char* regex, size_t key, size_t min, size_t max)
{
	// creates pcre entry
	const char* error = 0;
	int erroffset;
	pcre* obj = pcre_compile(regex, PCRE_ANCHORED, &error, &erroffset, NULL);
	if (!obj)
	{
		_error = error;
		return false;
	}

	pcre_extra* extra = pcre_study(obj, 0, &error);

	pcre_key k(min, max);

	pcre_entry e(obj, extra, key);

	return _regex_map.end() != _regex_map.insert(k, e).first;
}
	
bool 
tokenizer::load_regex(const char* file_name)
{
	// opens file
	FILE* f = fopen(file_name, "r");

	if (!f)
	{
		_error = "Can't open file: ";
		_error += file_name;
	}


	char buf[4096];

	// gets line by line
	while (fgets(buf, 4095, f))
	{
		char* key_tab = strchr(buf, '\t');
		if (!key_tab)
		{
			_error = "Wrong file format, can't find key TAB after regular expression";
			fclose(f);
			return false;
		}

		char* min_tab = strchr(key_tab + 1, '\t');
		if (!min_tab)
		{
			_error = "Wrong file format, can't find min TAB after regular expression";
			fclose(f);
			return false;
		}
		
		char* max_tab = strchr(min_tab + 1, '\t');
		if (!max_tab)
		{
			_error = "Wrong file format, can't find max TAB after regular expression";
			fclose(f);
			return false;
		}

		// splits strings
		*key_tab = 0;
		*min_tab = 0;
		*max_tab = 0;


		ub4_t key = 0;
		ub4_t min = 0;
		ub4_t max = 0;

		if (1 != str_template::strscan(key_tab + 1, 32, "%u", &key))
		{
			_error = "Wrong file data, can't convert key to digit";
			fclose(f);
			return false;
		}

		if (1 != str_template::strscan(min_tab + 1, 32, "%u", &min))
		{
			_error = "Wrong file data, can't convert min to digit";
			fclose(f);
			return false;
		}

		if (1 != str_template::strscan(max_tab + 1, 32, "%u", &max))
		{
			_error = "Wrong file data, can't convert max to digit";
			fclose(f);
			return false;
		}

		if (!min || min > max)
		{
			_error = "Wrong file data, min must be > 0 and <= max chars to digit";
			fclose(f);
			return false;
		}

		// adds to map
		if (!add_regex(buf, key, min, max))
		{
			fclose(f);
			return false;
		}
	}
	
	fclose(f);
	return true;
}

// cleans up regular expressions
void 
tokenizer::clear_regex()
{
	for (regex_map_t::iterator i = _regex_map.begin(); i != _regex_map.end(); ++i)
	{
		if (i->_this) free (i->_this);
		if (i->_extra) free (i->_extra);
	}

	_regex_map.clear();
}

// adds abbreviation to the internal map
bool 
tokenizer::add_abbreviation(const char* abbr)
{
	// makes a hash
	size_t key = do_hash_lowercase(abbr, os_minus_one);
	string_t val(abbr);

	return _abbr_map.end() != _abbr_map.insert(key, val).first;
}

// loads abbreviations from file
// file has to have the following format
// abbr\n
bool 
tokenizer::load_abbr(const char* file_name)
{
	// opens file
	FILE* f = fopen(file_name, "r");

	if (!f)
	{
		_error = "Can't open file: ";
		_error += file_name;
	}


	char buf[512];

	// gets line by line
	while (fgets(buf, 511, f))
	{
		char* dot = strchr(buf, '.');
		if (!dot)
		{
			_error = "Wrong file data, can't find final dot";
			fclose(f);
			return false;
		}

		*dot = 0;

		// adds to map
		if (!add_abbreviation(buf))
		{
			fclose(f);
			return false;
		}
	}
	
	fclose(f);
	return true;
}

// cleans up abbreviations
void 
tokenizer::clear_abbr()
{
	_abbr_map.clear();
}

// tokenizes string into array of tokens,
bool 
tokenizer::tokenize(const char* str, tokenizer_output_sequence_t& out, byte_allocator& all, size_t flags) const
{
	// checks null
	if (!str)
	{
		return false;
	}

	// remembers the begining
	const char* x = str;
	const char* begin = 0;

	tokenizer_output_sequence_t tmp_list;

	token_type prev_tt = TT_UNKNOWN, curr_tt = TT_UNKNOWN;

	// scans source chars
	while (*x)
	{
		switch (curr_tt = detect_token_type(*x))
		{
			case TT_WHITESPACE:
			case TT_ALPHABETIC:
			case TT_DIGIT: // this can be repeated many times
				if (prev_tt == curr_tt)
					break; // ok this is the same next char - continue to aggregate them
			default: // this the end of aggregated sequence or non-aggregated char
				if (begin)
				{
					// inserts into list
					token_info item(prev_tt, x - begin, 0);
					tmp_list.push_back(all, item);
				}

				begin = x;
		}

		prev_tt = curr_tt;
		++x;
	} // while

	// last samurai
	if (begin)
	{
		// inserts into list
		token_info item(prev_tt, x - begin, 0);
		tmp_list.push_back(all, item);
	}

	// tries to find regular expressions
	if (flags & T_REGEX)
		do_regex(str, tmp_list);
	// tries to resolve abbreviations
	if (flags & T_ABBR)
		do_abbr(str, tmp_list);
	// tries to resolve hyphens
	if (flags & T_HYPHEN)
		do_hyphen(str, tmp_list);

	// allocates the result list on the external allocator
	out.clear();

	for (tokenizer_output_sequence_t::const_iterator i = tmp_list.begin(); i != tmp_list.end(); ++i)
		out.push_back(all, *i);

	return true;
}

size_t 
tokenizer::match(const char* x, size_t len, size_t tokens, size_t& key) const
{
	size_t ret = 0;
	pcre_key k(1, tokens);
	regex_map_t::const_iterator i = _regex_map.lower_bound(k);

	while (i != _regex_map.end()
		&& i.key()._min <= tokens)
	{
		size_t length = 0;
		int ovector[30];
		int rc = pcre_exec(i->_this, i->_extra, x, (int)len, 0, PCRE_ANCHORED, ovector, 30);

		if (rc >= 0) // found
		{
			length = ovector[1] - ovector[0];
			if (length > ret)
			{
				ret = length;
				key = i->_key;
			}
		}

		++i;
	} // while

	return ret;
}

void 
tokenizer::do_regex(const char* phrase, tokenizer_output_sequence_t& tokens) const
{
	// checks if the regular expressions exist
	if (_regex_map.empty())
		return;

	size_t offset = 0;
	tokenizer_output_sequence_t::iterator i_first = tokens.begin(), i_second = tokens.begin();
	
	pcre_key k(1, 0xffffffff);
	regex_map_t::const_iterator u = _regex_map.upper_bound(k);
	assert(u != _regex_map.end());

	size_t max_count = u.key()._max;

	while (i_first != tokens.end())
	{
		tokenizer_output_sequence_t::iterator i_matched = tokens.end();
		size_t count = 1, len = 0, maxlen = 0;
		size_t maxmatch = 0;
		size_t maxkey = 0;

		// looks through all tokens
		for (i_second = i_first; i_second != tokens.end() && count <= max_count; ++i_second, ++count)
		{
			// adjusts len
			len += i_second->_len;
			size_t key = 0;
			size_t matched = match(phrase + offset, len, count, key);

			if (matched > maxmatch)
			{
				i_matched = i_second;
				maxlen = len;
				maxkey = key;
				maxmatch = matched;
			}
		} // for

		if (i_matched != tokens.end())
		{
			// glues tokens into regular expression
			i_first->_len = maxlen;
			i_first->_type = TT_REGEX;
			i_first->_key = maxkey;

			if (i_first != i_matched)
			{
				i_second = i_first;
				tokens.erase(++i_second, ++i_matched);
			}

			offset += maxlen;
			++i_first;
		}
		else
		{
			offset += i_first->_len;
			++i_first;
		}
	} // while
}

void 
tokenizer::do_abbr(const char* phrase, tokenizer_output_sequence_t& tokens) const
{
	size_t offset = 0;
	for (tokenizer_output_sequence_t::iterator i_first = tokens.begin(); i_first != tokens.end(); ++i_first)
	{
		switch (i_first->_type)
		{
			case TT_ALPHABETIC:
				{
					tokenizer_output_sequence_t::iterator i_second = i_first;
					++i_second;

					if (i_first != tokens.end()
						&& i_second->_type == TT_DOT)
					{
						// makes hash
						size_t hash_value = do_hash_lowercase(phrase + offset, i_first->_len);
						abbreviation_map_t::const_iterator i_find = _abbr_map.lower_bound(hash_value);

						while (i_find != _abbr_map.end()
							&& i_find.key() == hash_value)
						{
							if (i_first->_len == i_find->length()
								&& !str_template::strnocasecmp((const char*)*i_find, phrase + offset, i_first->_len)
								)
							{
								i_first->_len += 1;
								i_first->_type = TT_ABBR;
								tokens.erase(i_second);
								break;
							}

							++i_find;
						}

					} // if
				}
				break;
		} // switch

		offset += i_first->_len;
	} // for
}

void 
tokenizer::do_hyphen(const char* phrase, tokenizer_output_sequence_t& tokens) const
{
	size_t offset = 0;
	bool find_dash = false;
	bool find_token = false;

	for (tokenizer_output_sequence_t::iterator i_first = tokens.begin(); i_first != tokens.end(); ++i_first)
	{
		switch (i_first->_type)
		{
			case TT_ALPHABETIC:
			case TT_DIGIT:
				{
					tokenizer_output_sequence_t::iterator i_second = i_first;
					bool dash_flip_flop = false;
					size_t count = 0;
					size_t len = 0;

					while (i_second != tokens.end()
						&& (!dash_flip_flop && (i_second->_type == TT_ALPHABETIC || i_second->_type == TT_DIGIT)
							|| dash_flip_flop && i_second->_type == TT_SYMBOL && *(phrase + offset + len) == ch_dash)
						)
					{
						if (!dash_flip_flop)
						{
							len += i_second->_len + (count ? 1 : 0);
							count += (count ? 2 : 1);
						}

						dash_flip_flop = !dash_flip_flop;
						++i_second;
					} // while

					if (count > 3)
					{
						i_first->_len = len;
						i_first->_type = TT_COMPOSE;
						i_second = i_first;
						++i_second;
						while (--count && i_second != tokens.end())
							i_second = tokens.erase(i_second);
					}
				}
				break;
		} // switch

		offset += i_first->_len;
	} // for
}

#pragma pack()
END_TERIMBER_NAMESPACE
