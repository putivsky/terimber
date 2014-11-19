#ifndef _httphelp_h_
#define _httphelp_h_

#include "base/list.h"
#include "base/string.h"
#include "base/memory.h"

// set of http pairs (name:value) for instance parameters from URL, or HTTP headers
// value can be NULL
class http_pair
{
public:
	http_pair(const char* name, const char* value) : _name(name), _value(value)
	{
	}

	const char*	_name;
	const char*	_value;
};


namespace httphelp
{
	void process_http_parameters(const char* url, TERIMBER::_list< http_pair >& params, TERIMBER::byte_allocator& all);
	void process_http_headers(const char* buffer, TERIMBER::_list< http_pair >& headers, TERIMBER::byte_allocator& all);
	void decode_url_parameter(const char* param, size_t len, TERIMBER::string_t& encoded);
	bool find_parameter(const TERIMBER::_list< http_pair >& params, const char* name, const char*& ret, const char* def);
	bool find_parameter(const TERIMBER::_list< http_pair >& params, const char* name, int& ret, int def);
}

#endif
