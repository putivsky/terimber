// Copyright (C) 2007 Ask.com Inc. (IAC)
// All rights reserved.
//
// Created by Yuri Putivsky
// 2007-03-08

#ifndef _keepers_hpp_
#define _keepers_hpp_


#include "base/template.hpp"
#include "odbcdb/terodbc.h"
#include "memdb/memdbacc.h"

// create template resource keepers

// dbserver keeper
class odbcserver_keeper_constructor
{
public:
	typedef dbserver TYPE;
	typedef size_t ARG;

	inline dbserver* create(size_t ident)
	{
		odbcaccess dbacc;
		return dbacc.get_dbserver(ident);
	}

	inline void destroy(dbserver* obj)
	{
		if (obj->is_in_transaction())
			obj->rollback();

		if (obj->is_open_sql())
			obj->close_sql();

		if (obj->is_connect())
			obj->disconnect();

		delete obj;
	}
};

// dbserver keeper
class file_keeper_constructor
{
public:
	class file_arg
	{
	public:
		file_arg(const char* name, const char* flags) : file_name(name), rwflags(flags) 
		{
		}
		const char* file_name;
		const char* rwflags;
	};

	typedef FILE TYPE;
	typedef file_arg ARG;

	inline FILE* create(const file_arg& arg)
	{
		if (!arg.file_name)
			return 0;

		return fopen(arg.file_name, arg.rwflags);
	}

	inline void destroy(FILE* obj)
	{
		if (obj)
			fclose(obj);
	}
};

// memtable keeper
class memtable_keeper_constructor
{
public:
	typedef terimber_memtable TYPE;
	typedef size_t ARG;

	inline terimber_memtable* create(size_t)
	{
		terimber_memtable_access memacc;
		return memacc.get_memtable();
	}

	inline void destroy(terimber_memtable* obj)
	{
		delete obj;
	}
};

// memindex keeper
class memindex_keeper_constructor
{
public:
	class memindex_arg
	{
	public:
		memindex_arg(size_t columns, const terimber_index_column_info* info) : _columns(columns), _info(info)
		{
		}

		size_t								_columns;	
		const terimber_index_column_info*	_info;
	};

	typedef terimber_memindex TYPE;
	typedef memindex_arg ARG;

	inline memindex_keeper_constructor(terimber_memtable* table) : _table(table)
	{
	}

	inline terimber_memindex* create(const memindex_arg& arg)
	{
		return _table->add_index(arg._columns, arg._info); 
	}

	inline void destroy(terimber_memindex* obj)
	{
		_table->remove_index(obj); 
	}
private:

	terimber_memtable*	_table;
};

// memlookup keeper
class memlookup_keeper_constructor
{
public:
	typedef terimber_memlookup TYPE;
	typedef size_t ARG;

	inline memlookup_keeper_constructor(terimber_memindex* index) : _index(index)
	{
	}

	inline terimber_memlookup* create(size_t)
	{
		return _index->add_lookup(0); 
	}

	inline void destroy(terimber_memlookup* obj)
	{
		_index->remove_lookup(obj); 
	}
private:

	terimber_memindex*	_index;
};

// memvector keeper
class memvector_keeper_constructor
{
public:
	typedef terimber_db_value_vector TYPE;
	typedef size_t ARG;

	inline memvector_keeper_constructor(terimber_memtable* table) : _table(table)
	{
	}

	inline terimber_db_value_vector* create(size_t len)
	{
		return _table->allocate_db_values(len); 
	}

	inline void destroy(terimber_db_value_vector* obj)
	{
		_table->destroy_db_values(obj); 
	}
private:

	terimber_memtable*	_table;
};

#endif
