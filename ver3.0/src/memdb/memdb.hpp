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

#ifndef _terimber_memdb_hpp_
#define _terimber_memdb_hpp_

#include "memdb/memdb.h"
#include "base/common.hpp"
#include "db/db.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

static 
inline 
int 
compare_db_value(dbtypes type, const terimber_db_value& first, const terimber_db_value second, bool case_insensitive)
{
	if (first.nullVal)
		return second.nullVal ? 0 : -1;
	else if (second.nullVal)
		return 1;

	return compare_value(dbserver_impl::convert_types(type), first.val, second.val, false, case_insensitive);
}

inline 
bool 
memdb_rowset_less::operator()(const memdb_rowset_citerator_t& first, const memdb_rowset_citerator_t& second) const
{
	// check the min of columns info & both iterators
	size_t length = _info.size();
	bool first_lookup = false;
	bool second_lookup = false;

	if (first->_status == status_lookup)
	{
		first_lookup = true;
		length = first->_row.size();
	}
	else if (second->_status == status_lookup) // we just inside lookup, adjust length
	{
		second_lookup = true;
		length = second->_row.size();
	}

	if (first->_status == status_deleted
		|| second->_status == status_deleted)
		return first->_status < status_deleted;

	// look through all index columns
	for (size_t index = 0; index < length; ++index)
	{
		int res = compare_db_value(_info[index]._type, first->_row[first_lookup ? index : _info[index]._index], second->_row[second_lookup ? index : _info[index]._index], _info[index]._case_insensitive);
		if (res == 0)
			continue;
		else
			return _info[index]._asc_sort ? res < 0 : res > 0;
	}
	
	return false;
}

template < class S, class C >
bool 
copy_db_row(const S* source, memdb_row& row, C& cols, size_t col_count, byte_allocator& all, string_t& err)
{
	// copy values
	for (size_t icol = 0; icol < col_count; ++icol)
	{
		row._row[icol].nullVal = source->get_value_is_null(icol);
		if (!row._row[icol].nullVal)
		{
			switch (cols[icol]._type)
			{
				case db_bool:
					row._row[icol].val.boolVal = source->get_value_as_bool(icol);
					break;
				case db_sb1:
					row._row[icol].val.cVal = source->get_value_as_char(icol);
					break;
				case db_ub1:
					row._row[icol].val.bVal = source->get_value_as_byte(icol);
					break;
				case db_sb2:
					row._row[icol].val.iVal = source->get_value_as_short(icol);
					break;
				case db_ub2:
					row._row[icol].val.uiVal = source->get_value_as_word(icol);
					break;
				case db_sb4:
					row._row[icol].val.lVal = source->get_value_as_long(icol);
					break;
				case db_ub4:
					row._row[icol].val.ulVal = source->get_value_as_dword(icol);
					break;
				case db_float:
					row._row[icol].val.fltVal = source->get_value_as_float(icol);
					break;
				case db_double:
	#ifdef OS_64BIT
					row._row[icol].val.dblVal = source->get_value_as_double(icol);
	#else
					{
						double* dummy = 0;
						dummy = (double*)all.allocate(sizeof(double));
						if (!dummy)
						{
							err = "no enough memory";
							return false;
						}

						*dummy = source->get_value_as_double(icol);
						row._row[icol].val.dblVal = dummy;
					}
	#endif
					break;
				case db_sb8:
				case db_date:
	#ifdef OS_64BIT
					row._row[icol].val.intVal = source->get_value_as_long64(icol);
	#else
					{
						sb8_t* dummy = 0;
						dummy = (sb8_t*)all.allocate(sizeof(sb8_t));
						if (!dummy)
						{
							err = "no enough memory";
							return false;
						}
						*dummy = source->get_value_as_long64(icol);
						row._row[icol].val.intVal = dummy;
					}
	#endif
					break;
				case db_ub8:
	#ifdef OS_64BIT
					row._row[icol].val.uintVal = source->get_value_as_dword64(icol);
	#else
					{
						ub8_t* dummy = 0;
						dummy = (ub8_t*)all.allocate(sizeof(ub8_t));
						if (!dummy)
						{
							err = "no enough memory";
							return false;
						}
						*dummy = source->get_value_as_dword64(icol);
						row._row[icol].val.uintVal = dummy;
					}
	#endif
					break;
				case db_decimal:
				case db_numeric:
					{
						const char* buf = source->get_value_as_numeric(icol, '.');
						size_t len = (buf) ? strlen(buf) : 0;
						numeric conv(buf, len, '.', &all);
						ub1_t* buff = (ub1_t*)all.allocate(conv.orcl_len());
						if (!buff)
						{
							err = "no enough memory";
							return false;
						}

						if (!conv.persist_orcl(buff))
						{
							err = "invalid numeric format";
							return false;
						}

						row._row[icol].val.bufVal = buff;
					}
					break;
				case db_string:
					row._row[icol].val.strVal = copy_string(source->get_value_as_string(icol), all, os_minus_one);
					break;
				case db_wstring:
					row._row[icol].val.wstrVal = copy_string(source->get_value_as_wstring(icol), all, os_minus_one);
					break;
				case db_binary:
					{
						const ub1_t* buf = source->get_value_as_binary_ptr(icol);
						size_t len = *(size_t*)buf;
						ub1_t* dummy = (ub1_t*)all.allocate(len + sizeof(size_t));
						if (!dummy)
						{
							err = "no enough memory";
							return false;
						}
						// set length
						*(size_t*)dummy = len;
						if (len) // copy data
							memcpy(dummy + sizeof(size_t), buf + sizeof(size_t), len);

						row._row[icol].val.bufVal = dummy;
					}
					break;
				case db_guid:
					{
						guid_t* dummy = (guid_t*)all.allocate(sizeof(guid_t));
						if (!dummy)
						{
							err = "no enough memory";
							return false;
						}
						source->get_value_as_guid(icol, *dummy);
						row._row[icol].val.guidVal = dummy;
					}
					break;
			} // switch
		} // if
	} // for

	return true;
}



#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_memdb_h_
