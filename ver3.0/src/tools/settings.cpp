#include "settings.h"
#include "base/string.hpp"
#include "base/memory.hpp"
#include "xml/xmlaccss.h"

settings::settings(const char* filename, const char* path, const char* key)
{
	xml_factory acc;
	_filename = filename;

	_parser = acc.get_xml_designer();
	if (!_parser->load((const char*)_filename, 0))
		return;

	if (path)
	{
		TERIMBER::string_t next_key_in_path;
		const char* begin = path, *end = 0, *next = 0;

		while (next = strchr(begin, '/'))
		{
			end = next;

			next_key_in_path.assign(begin, end - begin);
			
			_parser->add_child(ELEMENT_NODE, next_key_in_path, 0, false);
			
			if (_root.length()) _root += "/";
			_root += next_key_in_path;

			begin = next + 1;
		}

		if (begin && *begin)
		{
			if (_root.length()) _root += "/";
			_root += begin;
		}
	}

	if (_root.length()) _root += "/";
	_root += key;
}

void 
settings::make_xpath(TERIMBER::string_t& path, const char* subkey)
{
	path = _root;

	if (subkey)
	{
		path += "/";
		path += subkey;
	}
}

settings::~settings()
{
	if (_parser)
	{
		_parser->save((const char*)_filename, false);
		delete _parser;
	}
}

void 
settings::Set(const char* subkey, const char* name, const TERIMBER::string_t& value)
{
	TERIMBER::string_t path;
	make_xpath(path, subkey);

	if (_parser->select_root() && _parser->select_xpath(path))
	{
		if (_parser->select_attribute_by_name(name))
			_parser->update_value(value);
		else
			_parser->add_child(ATTRIBUTE_NODE, name, value, false);
	}
	else if (_parser->select_root() && _parser->select_xpath(_root))
	{
		if (subkey)
			_parser->add_child(ELEMENT_NODE, subkey, 0, false);
		_parser->add_child(ATTRIBUTE_NODE, name, value, false);
	}
}

bool 
settings::Get(const char* subkey, const char* name, TERIMBER::string_t& value)
{
	TERIMBER::string_t path;
	make_xpath(path, subkey);

	if (!_parser->select_root()
		|| !_parser->select_xpath(path)
		|| !_parser->select_attribute_by_name(name))
		return false;

	value = _parser->get_value();
	return true;
}

void 
settings::Set(const char* subkey, const char* name, int value)
{
	char buf[32];
	sprintf(buf, "%d", value);
	TERIMBER::string_t sval(buf);
	Set(subkey, name, sval);
}

bool 
settings::Get(const char* subkey, const char* name, int& value)
{
	TERIMBER::string_t sval;
	return Get(subkey, name, sval)
		&& sval.length() 
		&& sscanf(sval, "%d", &value) == 1;
}

void 
settings::GetSet(const char* subkey, const char* name, int& nValue)
{
	if (!Get(subkey, name, nValue))
		Set(subkey, name, nValue);
}

void 
settings::GetSet(const char* subkey, const char* name, TERIMBER::string_t& strValue)
{
	if (!Get(subkey, name, strValue))
		Set(subkey, name, strValue);
}
 
