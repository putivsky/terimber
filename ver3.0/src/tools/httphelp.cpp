#include "httphelp.h"
#include "base/list.hpp"
#include "base/string.hpp"
#include "base/memory.hpp"


namespace httphelp
{

void 
process_http_parameters(const char* url, 
								  TERIMBER::_list< http_pair >& params, 
								  TERIMBER::byte_allocator& all)
{
	if (!url)
		return;

	while (*url == '/' || *url =='\\')
		++url;
	// scan url 
	// 1. skip slashes and question mark
	const char* forward_slash = strrchr(url, '/');
	const char* back_slash = strrchr(url, '\\');
	const char* question_mark = strrchr(url, '?');

	const char* slash = __max(question_mark, __max(forward_slash, back_slash));
	const char* begin = __max(slash ? slash + 1 : 0,  url);

	// &param=value, value is optional
	while (begin)
	{
		const char* equal = strchr(begin, '=');
		const char* ampersand = strchr(begin, '&');

		const char* next = (equal ? equal : ampersand);

		int len = (next ? next - begin : strlen(begin));

		TERIMBER::string_t name(0, &all);
		decode_url_parameter(begin, len, name);

		// decode URL symbols

		TERIMBER::string_t value(0, &all);

		if (next)
			++next;

		// value
		if (equal)
		{
			ampersand = strchr(next, '&');
			// 
			len = (ampersand ? ampersand - next : strlen(next));

			decode_url_parameter(next, len, value);

			if (ampersand)
				++ampersand;
			next = ampersand;
		}

		// insert into list
		if (name.length())
		{
			http_pair p(name, value);
			params.push_back(all, p);
		}

		begin = next;
	}
}

void 
process_http_headers(const char* buffer, 
							TERIMBER::_list< http_pair >& headers, 
							TERIMBER::byte_allocator& all)
{
	if (!buffer)
		return;

	const char* newline = 0;
	const char* begin = buffer;

	while (newline = strstr(begin, "\r\n"))
	{
		if (newline == begin) // double new line
			break;

		const char* column = strchr(begin, ':');

		TERIMBER::string_t name(0, &all);
		TERIMBER::string_t value(0, &all);
		if (column < newline)
		{
			name.append(begin, column - begin);
			++column;
			while (*column == ' ') // skip leading spaces
				++column;
			value.append(column, newline - column);
		}
		else
			name.append(begin, newline - begin);


		http_pair p(name, value);
		headers.push_back(all, p);
		begin = newline + 2;
	}
}

void 
decode_url_parameter(const char* param, size_t len, TERIMBER::string_t& encoded)
{
	// <>+'\"#?\\&[]{}|
	const char* begin = 0;
	int ch = 0;
	encoded = 0;

	while (*param && len-- > 0)
	{
		switch (*param)
		{
			case '%':
				if (begin)
				{
					encoded.append(begin, param - begin);
					begin = 0;
				}

				if (TERIMBER::str_template::strscan(param, len, "%%%2X", &ch))
				{
					char arr[2] = {(char)ch, 0};
					encoded.append(arr, 1);
				}

				param += 2;
				len -= 2;
				break;
			case '+':
				if (begin)
				{
					encoded.append(begin, param - begin);
					begin = 0;
				}

				encoded.append(" ", 1);
				break;
			default:
				if (!begin)
					begin = param;
		} // switch

		++param;
	} // while

	if (begin)
		encoded.append(begin, param - begin);
}

bool 
find_parameter(const TERIMBER::_list< http_pair >& params, const char* name, const char*& ret, const char* def)
{
	for (TERIMBER::_list< http_pair >::const_iterator it = params.begin(); it != params.end(); ++it)
	{
		if (it->_name && !TERIMBER::str_template::strnocasecmp(it->_name, name, os_minus_one))
		{
			ret = it->_value;
			return true;
		}
	}

	ret = def;
	return false;
}

bool 
find_parameter(const TERIMBER::_list< http_pair >& params, const char* name, int& ret, int def)
{
	const char* strval = 0;
	const char* strdef = 0;

	if (find_parameter(params, name, strval, strdef)
		&& strval
		&& sscanf(strval, "%d", &ret) == 1
		)
	{
		return true;
	}

	ret = def;
	return false;
}

}
