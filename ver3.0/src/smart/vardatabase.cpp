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
#include "smart/vardatabase.h"
#include "smart/varmap.hpp"
#include "smart/varvalue.hpp"
#include "base/map.hpp"
#include "base/vector.hpp"
#include "base/list.hpp"
#include "base/memory.hpp"
#include "base/string.hpp"
#include "base/common.hpp"
#include "base/keymaker.h"

terimber_vardatabase* 
terimber_vardatabase_factory::get_vardatabase()
{
	return new TERIMBER::vardatabase();
}


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

const char* request_dtd = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<!ENTITY % ddltype \"(CREATE | DROP)\"> \
<!ENTITY % dmltype \"(SELECT | INSERT | UPDATE | DELETE)\"> \
<!ENTITY % logic \"(AND | OR)\"> \
<!ENTITY % condition \"(GT | GE | LT | LE | EQ | NE | PM | FM)\"> \
<!ENTITY % quality \"(low | normal | high)\"> \
<!ENTITY % vtype \"(bool | sb1 | ub1 | sb2 | ub2 | sb4 | ub4 | sb8 | ub8 | float | double | numeric | guid | string | binary | mpart | mfuzzy)\"> \
<!ELEMENT request (table | query)>\
<!ELEMENT table (desc)*>\
<!ELEMENT desc EMPTY>\
<!ELEMENT query ((returns?, where) | (returns?, values) | (returns?, values, where) | (returns?, where))> \
<!ELEMENT returns (col)*> \
<!ELEMENT where (group | cond)> \
<!ELEMENT values (col)*> \
<!ELEMENT group (group | cond)+> \
<!ELEMENT cond EMPTY> \
<!ELEMENT col EMPTY> \
<!ATTLIST request \
timeout CTYPE vt_ub4 \"5000\"> \
<!ATTLIST table \
what %ddltype; #REQUIRED \
name CDATA #REQUIRED> \
<!ATTLIST desc \
name CDATA #REQUIRED \
type %vtype; #REQUIRED> \
<!ATTLIST query \
what %dmltype; #REQUIRED \
name CDATA #REQUIRED> \
<!ATTLIST group \
join %logic; #REQUIRED> \
<!ATTLIST cond \
how %condition; #REQUIRED \
pq %quality; #IMPLIED \
nq %quality; #IMPLIED \
deep CTYPE vt_bool #IMPLIED \
name CDATA #IMPLIED \
val CDATA #REQUIRED > \
<!ATTLIST col \
name CDATA #REQUIRED \
val CDATA #IMPLIED>";

const char* response_dtd = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<!ELEMENT response (row)*> \
<!ELEMENT row (col)*> \
<!ELEMENT col EMPTY> \
<!ATTLIST response \
errCode CTYPE vt_sb4 #REQUIRED \
errDesc CDATA #IMPLIED> \
<!ATTLIST row \
rowid CTYPE vt_ub4 #REQUIRED> \
<!ATTLIST col \
name CDATA #REQUIRED \
val CDATA #IMPLIED>";


vardatabase::vardatabase()
{
}

// virtual 
vardatabase::~vardatabase()
{
}

// virtual 
bool 
vardatabase::process_xml_request(const char* request, size_t len, xml_designer* parser)
{
	// lets parser request first
	if (!parser->load(request, len, request_dtd, strlen(request_dtd))
		|| !parser->select_root())
	{
		string_t err = parser->error();
		parser->load(0, 0 , response_dtd, strlen(response_dtd));

		parser->add_child(ELEMENT_NODE, "response", 0, false);
		parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);
		parser->add_child(ATTRIBUTE_NODE, "errDesc", err, true);

		return true;
	}

	ub4_t timeout = 0;

	if (parser->select_attribute_by_name("timeout"))
	{
		const char* val = parser->get_value();

		if (val)
			str_template::strscan(val, 32, "%u", &timeout);

		parser->select_parent();
	}

	// can be table or query
	parser->select_first_child();

	const char* DDL_DML = parser->get_name();

	if (DDL_DML[0] == 't') // DDL - table
	{
		keylockerWriter writeguard(_masterkey, timeout);

		if (!writeguard)
		{
			parser->load(0, 0 , response_dtd, strlen(response_dtd));
			parser->add_child(ELEMENT_NODE, "response", 0, false);
			parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);
			parser->add_child(ATTRIBUTE_NODE, "errDesc", "Timeout occurred", true);
			return true;
		}

		parser->select_attribute_by_name("name");
		string_t name = parser->get_value();				
		parser->select_parent();

		parser->select_attribute_by_name("what");
		const char* what = parser->get_value();
		
		if (what[0] == 'D') // DROP
		{
			parser->select_parent();

			mutexKeeper keeper(_table_mtx);
			table_map_t::iterator it_table = _table_map.find(name);
			if (it_table != _table_map.end())
			{
				_table_map.erase(it_table);
			
				parser->load(0, 0 , response_dtd, strlen(response_dtd));
				parser->add_child(ELEMENT_NODE, "response", 0, false);
				parser->add_child(ATTRIBUTE_NODE, "errCode", "0", true);
			}
			else
			{
				// error, can not create the same table
				parser->load(0, 0 , response_dtd, strlen(response_dtd));
				parser->add_child(ELEMENT_NODE, "response", 0, false);
				parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);

				string_t err = "Table ";
				err += name;
				err += " does not exist in the database";

				parser->add_child(ATTRIBUTE_NODE, "errDesc", err, true);
			}
		}
		else // CREATE
		{
			parser->select_parent();

			mutexKeeper keeper(_table_mtx);
			vartable tbl;
			table_map_t::pairib_t it_table = _table_map.insert(name, tbl);
			if (it_table.first == _table_map.end())
			{
				// error, can not create the same table
				parser->load(0, 0 , response_dtd, strlen(response_dtd));
				parser->add_child(ELEMENT_NODE, "response", 0, false);
				parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);

				string_t err = "Not enough memory";
				parser->add_child(ATTRIBUTE_NODE, "errDesc", err, true);
			}
			else
			{
				if (it_table.second)
				{
					fill_schema(*it_table.first, parser);
					
					parser->load(0, 0 , response_dtd, strlen(response_dtd));
					parser->add_child(ELEMENT_NODE, "response", 0, false);
					parser->add_child(ATTRIBUTE_NODE, "errCode", "0", true);
				}
				else
				{
					// error, can not create the same table
					parser->load(0, 0 , response_dtd, strlen(response_dtd));
					parser->add_child(ELEMENT_NODE, "response", 0, false);
					parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);

					string_t err = "Table ";
					err += name;
					err += " already exists in the database";

					parser->add_child(ATTRIBUTE_NODE, "errDesc", err, true);
				}
			}
		}
	} // query
	else
	{
		keylockerReader readguard(_masterkey, timeout);

		if (!readguard)
		{
			parser->load(0, 0 , response_dtd, strlen(response_dtd));
			parser->add_child(ELEMENT_NODE, "response", 0, false);
			parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);
			parser->add_child(ATTRIBUTE_NODE, "errDesc", "Timeout occurred", true);
			return true;
		}

		parser->select_attribute_by_name("name");
		string_t name = parser->get_value();				
		parser->select_parent();

		mutexKeeper keeper(_table_mtx);
		table_map_t::iterator it_table = _table_map.find(name);
		if (it_table != _table_map.end())
		{
			it_table->_tmp_all.clear_extra();
			it_table->_con_all.clear_extra();

			parser->select_attribute_by_name("what");
			const char* what = parser->get_value();				
			

			if (what[0] == 'S') // SELECT - readonly		
			{
				parser->select_parent();

				keylockerReader qread(it_table->_key, timeout);
				if (!qread)
				{
					parser->load(0, 0 , response_dtd, strlen(response_dtd));
					parser->add_child(ELEMENT_NODE, "response", 0, false);
					parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);
					parser->add_child(ATTRIBUTE_NODE, "errDesc", "Timeout occurred", true);
					return true;
				}

				it_table->_tbl.process_query(parser, it_table->_con_all, it_table->_tmp_all, it_table->_inl_all);
			}
			else
			{
				parser->select_parent();

				keylockerWriter qwrite(it_table->_key, timeout);
				if (!qwrite)
				{
					parser->load(0, 0 , response_dtd, strlen(response_dtd));
					parser->add_child(ELEMENT_NODE, "response", 0, false);
					parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);
					parser->add_child(ATTRIBUTE_NODE, "errDesc", "Timeout occurred", true);
					return true;
				}

				it_table->_tbl.process_query(parser, it_table->_con_all, it_table->_tmp_all, it_table->_inl_all);
			}
		}
		else
		{
			// error, can not create the same table
			parser->load(0, 0 , response_dtd, strlen(response_dtd));
			parser->add_child(ELEMENT_NODE, "response", 0, false);
			parser->add_child(ATTRIBUTE_NODE, "errCode", "-1", true);

			string_t err = "Table ";
			err += name;
			err += " does not exist in the database";

			parser->add_child(ATTRIBUTE_NODE, "errDesc", err, true);
		}
	}

	return true;
}
 
bool 
vardatabase::fill_schema(vartable& tbl, xml_designer* parser)
{
	if (!parser->select_first_child())
		return true;

	_list< var_property_schema > l_tmp;

	do
	{
		// 
		if (ELEMENT_NODE != parser->get_type())
			continue;

		var_property_schema dummy;

		_list< var_property_schema >::iterator schema = l_tmp.push_back(tbl._tmp_all, dummy);
		if (schema == l_tmp.end())
			return false;


		schema->_is_searchable = false;
		schema->_is_fuzzy_match = false;

		parser->select_attribute_by_name("name");
		TERIMBER::string_t name(parser->get_value(), &tbl._tmp_all);
		schema->_name = name;
		parser->select_parent();

		parser->select_attribute_by_name("type");
		const char* val = parser->get_value();

		switch (val[0])
		{
			case 'b': // bool, binary
				{
					if (val[1] == 'o')
						schema->_type = vt_bool;
					else
						schema->_type = vt_binary;
				}
				break;
			case 's': // sb1, sb2, sb4, sb8, string
				{
					if (val[1] == 't') // string
						schema->_type = vt_string;
					else
					{
						switch (val[2])
						{
							case '1':
								schema->_type = vt_sb1;
								break;
							case '2':
								schema->_type = vt_sb2;
								break;
							case '4':
								schema->_type = vt_sb4;
								break;
							case '8':
								schema->_type = vt_sb8;
								break;
						}
					}
				}
				break;
			case 'u': // ub1, ub2, ub4 , ub8
				{
					switch (val[2])
					{
						case '1':
							schema->_type = vt_ub1;
							break;
						case '2':
							schema->_type = vt_ub2;
							break;
						case '4':
							schema->_type = vt_ub4;
							break;
						case '8':
							schema->_type = vt_ub8;
							break;
					}
				}
				break;
			case 'f': // float
				schema->_type = vt_float;
				break;
			case 'd': // double
				schema->_type = vt_double;
				break;
			case 'n': // numeric
				schema->_type = vt_numeric;
				break;
			case 'g': // guid
				schema->_type = vt_guid;
				break;
			case 'm': // mpart or mfuzzy
				schema->_type = vt_string;
				if (val[1] == 'p')
					schema->_is_searchable = true;
				else // fuzzy
					schema->_is_fuzzy_match = true;
				break;
			default:
				assert(false);
		}

		parser->select_parent();
	}
	while (parser->select_next_sibling());

	if (l_tmp.empty())
		return true;

	size_t cols = l_tmp.size();

    if (!tbl._schema.resize(tbl._all, cols))
		return false;

	size_t index = 0;
	for (_list< var_property_schema >::const_iterator it = l_tmp.begin(); it != l_tmp.end(); ++it, ++index)
	{
		tbl._schema[index] = *it;
		// make a clean copy of name
		tbl._schema[index]._name = copy_string(it->_name, tbl._all, os_minus_one);
	}

	return true;
}

#pragma pack()
END_TERIMBER_NAMESPACE
