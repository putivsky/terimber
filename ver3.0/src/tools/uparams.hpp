// Copyright (C) 2007 Ask.com Inc. (IAC)
// All rights reserved.
//
// Created by Yuri Putivsky
// 2007-05-30

#ifndef _uparams_hpp_
#define _uparams_hpp_

#pragma pack(4)

struct uparam 
{
	const char*		_name;
	int				_type;// 0 - int, 1 - double
	double			_dval;
	size_t			_ival;
};


static 
inline 
int
find_uparam_index(const char* name, const uparam arr[], size_t len)
{
	for (size_t i = 0; i < len; ++i)
	{
		if (!TERIMBER::str_template::strnocasecmp(arr[i]._name, name, os_minus_one))
			return i;
	}

	return -1;
}

static 
inline 
const char*
get_uparam_name(size_t index, const uparam arr[], size_t len)
{
	if (index < len)
		return arr[index]._name;

	return 0;
}

static 
inline
bool 
scan_uparam_func(const char* name, const char* value, 
				 uparam& param, 
				 const uparam ethalons[],
				 size_t len)
{
	if (!name || !value)
	{
		assert(false);
		return false;
	}

	int index = find_uparam_index(name, ethalons, len);
	if (index == -1)
		return false;

	param = ethalons[index];
	switch (param._type)
	{
		case 0:
			return 1 == TERIMBER::str_template::strscan(value, "%d", &param._ival);
		case 1:
			return 1 == TERIMBER::str_template::strscan(value, "%lf", &param._dval);
		default:
			return false;
	}


	return false;
}

static 
inline
const char* 
persist_uparam_func(TERIMBER::byte_allocator& all, const uparam& param)
{
	char* buf = (char*)all.allocate(64);
	if (!buf)
		return 0;

	switch (param._type)
	{
		case 0:
			TERIMBER::str_template::strprint(buf, 64, "%d", param._ival);
			break;
		case 1:
			TERIMBER::str_template::strprint(buf, 64, "%f", param._dval);
			break;
		default:
			break;
	}

	return buf;
}

#pragma pack()

#endif
