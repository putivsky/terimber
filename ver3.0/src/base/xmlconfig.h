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

#ifndef _terimber_xmlconfig_h_
#define _terimber_xmlconfig_h_

#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/map.hpp"
#include "xml/xmlimpl.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

typedef map< string_t, string_t > xmlconfiglist_t;
//! \class xmlconfig
//! \brief class encapsulates the xml config file as an INI or registry keys
class xmlconfig
{
public:
	//! \brief constructor
	xmlconfig(		const char* filename,					//!< file name
					const char* path,						//!< optional xml xpath from root element
					const char* key							//!< optional additional element under xpath
					) : 
		_parser(4096)
	{
		// assigns file name
		_filename = filename;
	
		string_t root;

		// tries to parse
		if (!_parser.load(filename, 0)
			|| !_parser.select_root())
		{
			// re-create root
			_parser.load(0, 0, 0, 0);
		}
		else
		{
			_root = _parser.get_name(); // assign root
			root = _root;
		}

		if (path && *path)
		{
			// if path specified 
			// tries to find path
			if (!_parser.select_xpath(path))
			{
				string_t next_key;
				const char* begin = path, *end = 0, *next = 0;

				while (0 != (next = strchr(begin, '/')))
				{
					end = next;

					next_key.assign(begin, end - begin);

					if (next_key != root)
					{
						string_t path_key(_root);
						if (_root.length())
							path_key += "/";
						path_key += next_key;

						_parser.select_root();
						if (!_parser.select_xpath(path_key))
						{
							_parser.select_root();
							_parser.select_xpath(_root);
							_parser.add_child(ELEMENT_NODE, next_key, 0, false);
						}

						if (_root.length())
							_root += "/";
						_root += next_key;
					}

					begin = next + 1;
				} // while

				// last one
				if (begin && *begin)
				{
					string_t path_key(_root);
					if (_root.length())
						path_key += "/";
					path_key += "/";
					path_key += begin;

					_parser.select_root();
					if (!_parser.select_xpath(path_key))
					{
						_parser.select_root();
						_parser.select_xpath(_root);
						_parser.add_child(ELEMENT_NODE, begin, 0, false);
					}

					if (_root.length())
						_root += "/";
					_root += begin;
				}
			} // if path not found
			else
			{
				_root = path;
			}
		} //if path

		if (key && *key)
		{
			string_t path_key(_root);
			if (_root.length())
				path_key += "/";
			path_key += key;

			_parser.select_root();
			if (!_parser.select_xpath(path_key))
			{
				_parser.select_root();
				_parser.select_xpath(_root);
				_parser.add_child(ELEMENT_NODE, key, 0, false);
			}

			if (_root.length())
				_root += "/";
			_root += key;
		}
	}

	//! \brief destructor
	~xmlconfig()
	{
		_parser.save((const char*)_filename, false);
	}

	//! \brief constructs the full path root+subkey
	void 
	make_path(		string_t& path,							//!< [out] full path
					const char* subkey						//!< subkey
					) const
	{
		path = _root;

		if (subkey)
		{
			path += "/";
			path += subkey;
		}
	}

	//! \brief sets value as string
	//! if it does not exist - creates a new one
	void 
	set(			const char* subkey,						//!< optional subkey
					const char* name,						//!< property name
					const char* val							//!< property value
					)
	{
		string_t path;
		make_path(path, subkey);

		if (_parser.select_root()
			&& _parser.select_xpath(path))
		{
			if (_parser.select_attribute_by_name(name))
				_parser.update_value(val);
			else
				_parser.add_child(ATTRIBUTE_NODE, name, val, false);
		}
		else if (_parser.select_root()
			&& _parser.select_xpath(_root))
		{
			if (subkey)
				_parser.add_child(ELEMENT_NODE, subkey, 0, false);

			_parser.add_child(ATTRIBUTE_NODE, name, val, false);
		}
	}

	//! \brief sets value as integer
	//! if it does not exist - creates a new one
	void 
	set(			const char* subkey,						//!< optinal subkey
					const char* name,						//!< property name
					int val									//!< property value
					)
	{
		char buf[32];
		str_template::strprint(buf, 32, "%d", val);
		set(subkey, name, buf);
	}

	//! \brief gets value as string
	//! returns false if property does not exist
	bool 
	get(			const char* subkey,						//!< optional subkey
					const char* name,						//!< property name
					string_t& val							//!< [out] property value
					) const
	{
		string_t path;
		make_path(path, subkey);

		if (!_parser.select_root()
			|| !_parser.select_xpath(path)
			|| !_parser.select_attribute_by_name(name)
			)
			return false;

		val = _parser.get_value();
		return true;
	}

	//! \brief gets value as integer
	//! returns false if property does not exist
	bool 
	get(			const char* subkey,						//!< optional subkey
					const char* name,						//!< property name
					int& val								//!< [out] property value
					) const
	{
		string_t s;
		return get(subkey, name, s)
				&& s.length()
				&& sscanf(s, "%d", &val) == 1;
	}

	//! \brief gets value as string
	//! if property does not exist it will be created
	void 
	getset(			const char* subkey,						//!< optional subkey 
					const char* name,						//!< property name
					string_t& val							//!< [in, out] property value
					)
	{
		if (!get(subkey, name, val))
			set(subkey, name, val);
	}

	//! \brief gets value as integer
	//! if property does not exist it will be created
	void 
	getset(			const char* subkey,						//!< optional subkey 
					const char* name,						//!< property name
					int& val								//!< [in, out] property value
					)
	{
		if (!get(subkey, name, val))
			set(subkey, name, val);
	}

	bool
	getlist(		const char* subkey,						//!< optional subkey 
					const char* name,						//!< list name
					xmlconfiglist_t& val					//!< [out] property list
					)
	{
		string_t path;
		make_path(path, subkey);
		const char* n, *v;

		if (!_parser.select_root()
			|| !_parser.select_xpath(path)
			|| !_parser.select_first_child()
			)
			return false;

		do
		{
			if (_parser.get_type() != ELEMENT_NODE
				|| !(n = _parser.get_name())
				|| TERIMBER::str_template::strnocasecmp(n, name, os_minus_one)
				)
				continue;

			if (_parser.select_attribute_by_name("name"))
			{
				n = _parser.get_value();
				_parser.select_parent();
			}

			if (_parser.select_attribute_by_name("value"))
			{
				v = _parser.get_value();
				_parser.select_parent();
			}

			if (n && v)
			{
				xmlconfiglist_t::iterator it = val.find(n);
				if (it != val.end())
					*it = v;
				else
					val.insert(n, v);
			}
		}
		while (_parser.select_next_sibling());

		return true;
	}

	void
	setlist(		const char* subkey,						//!< optional subkey 
					const char* name,						//!< list name
					const xmlconfiglist_t& val				//!< [out] property list
					)
	{
		string_t path;
		make_path(path, subkey);

		if (_parser.select_root()
			&& _parser.select_xpath(path))
		{
			; // path exists
		}
		else if (_parser.select_root()
			&& _parser.select_xpath(_root))
		{
			if (subkey)
				_parser.add_child(ELEMENT_NODE, subkey, 0, false);
		}
		else
			return;

		const char* n;
		for (xmlconfiglist_t::const_iterator it = val.begin(); it != val.end(); ++it)
		{
			bool found = false;

			// navigate for all childrens 
			if (_parser.select_first_child())
			{
				do
				{
					if (_parser.get_type() != ELEMENT_NODE
						|| !(n = _parser.get_name())
						|| TERIMBER::str_template::strnocasecmp(n, name, os_minus_one)
						)
						continue;

					if (_parser.select_attribute_by_name("name"))
					{
						n = _parser.get_value();
						_parser.select_parent();
					}

					if (n && !TERIMBER::str_template::strnocasecmp(n, it.key(), os_minus_one))
					{
						if (_parser.select_attribute_by_name("value"))
						{
							_parser.update_value(*it);
							_parser.select_parent();
						}
						else
							_parser.add_child(ATTRIBUTE_NODE, "value", *it, true);

						found = true;
					}
				}
				while (!found && _parser.select_next_sibling());

				_parser.select_parent();
			}

			if (!found)
			{
				_parser.add_child(ELEMENT_NODE, name, 0, false);
				_parser.add_child(ATTRIBUTE_NODE, "name", it.key(), true);
				_parser.add_child(ATTRIBUTE_NODE, "value", *it, true);
				_parser.select_parent();
			}
		}
	}

	void
	getsetlist(		const char* subkey,						//!< optional subkey 
					const char* name,						//!< list name
					xmlconfiglist_t& val					//!< [out] property list
					)
	{
		if (!getlist(subkey, name, val))
			setlist(subkey, name, val);
	}

private:
	xml_designer_impl	_parser;							//!< xml designer
	string_t			_filename;							//!< file name
	string_t			_root;								//!< xml root path
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif



