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

#include "base/list.hpp"
#include "base/vector.hpp"
#include "base/common.hpp"
#include "base/date.h"
#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/numeric.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

////////////////////////////////////////////////////////////
terimber_xml_value 
parse_value(vt_types type, const char* x, size_t length, byte_allocator* _allocator)
{
	sb2_t ss;
	ub2_t us;
	terimber_xml_value dest;
	memset(&dest, 0, sizeof(terimber_xml_value));
	if (!x) return dest;

	size_t len = (length == os_minus_one) ? str_template::strlen(x) : length;
	switch(type)
	{
		case vt_empty:
		case vt_null:
			break;
		case vt_bool:
			dest.boolVal = (!str_template::strcmp(x, "true", len) || !str_template::strcmp(x, "1", len));
			break;
		case vt_sb1:
			str_template::strscan(x, len, "%hd", &ss);
			dest.cVal = (sb1_t)ss;
			break;
		case vt_ub1:
			str_template::strscan(x, len, "%hu", &us);
			dest.bVal = (ub1_t)us;
			break;
		case vt_sb2:
			str_template::strscan(x, len, "%hd", &dest.iVal);
			break;
		case vt_ub2:
			str_template::strscan(x, len, "%hu", &dest.uiVal);
			break;
		case vt_sb4:
			str_template::strscan(x, len, "%ld", &dest.lVal);
			break;
		case vt_ub4:
			str_template::strscan(x, len, "%lu", &dest.ulVal);
			break;
		case vt_float:
			str_template::strscan(x, len, "%f", &dest.fltVal);
			break;
		case vt_double:
			{
#ifdef OS_64BIT
				str_template::strscan(x, len, "%lf", &dest.dblVal);
#else
				check_pointer(_allocator);
				double* dummy = (double*)check_pointer(_allocator->allocate(sizeof(double)));
				str_template::strscan(x, len, "%lf", dummy);
				dest.dblVal = dummy;
#endif
			}
			break;
		case vt_date:
			{
				date date_dummy(x, len);
#ifdef OS_64BIT
				dest.intVal = date_dummy;
#else
				check_pointer(_allocator);
				sb8_t* dummy = (sb8_t*)check_pointer(_allocator->allocate(sizeof(sb8_t)));
				*dummy = date_dummy;
				dest.intVal = dummy;
#endif
			}
			break;
		case vt_guid:
			{
				check_pointer(_allocator);
				guid_t* dummy = (guid_t*)check_pointer(_allocator->allocate(sizeof(guid_t)));
				string_to_guid(*dummy, x);
				dest.guidVal = dummy;
			}
			break;
		case vt_sb8:
			{
#ifdef OS_64BIT
				str_template::strscan(x, len, I64d, &dest.intVal);
#else
				check_pointer(_allocator);
				sb8_t* dummy = (sb8_t*)check_pointer(_allocator->allocate(sizeof(sb8_t)));
				str_template::strscan(x, len, I64d, dummy);	
				dest.intVal = dummy;
#endif
			}
			break;
		case vt_ub8:
			{
#ifdef OS_64BIT
				str_template::strscan(x, len, I64u, &dest.uintVal);
#else
				check_pointer(_allocator);
				ub8_t* dummy = (ub8_t*)check_pointer(_allocator->allocate(sizeof(ub8_t)));
				str_template::strscan(x, len, I64u, dummy);	
				dest.uintVal = dummy;
#endif
			}
			break;
		case vt_binary:
			{
				check_pointer(_allocator);
				ub1_t* dummy = (ub1_t*)check_pointer(_allocator->allocate(len / 2 + sizeof(ub4_t)));
				hex_to_binary(dummy, x, len);
				dest.bufVal = dummy;
			}
			break;
		case vt_decimal:
		case vt_numeric:
			{
				check_pointer(_allocator);
				numeric num(x, len, _ch_period);
				ub1_t* dummy = (ub1_t*)check_pointer(_allocator->allocate(num.orcl_len()));
				num.persist_orcl(dummy);
				dest.bufVal = dummy;
			}
			break;
		case vt_string:
			{
				check_pointer(_allocator);
				dest.strVal = (const char*)str_template::strcpy((char*)check_pointer(_allocator->allocate(len + 1)), x, len);
			}
			break;
		case vt_wstring:
			{
				check_pointer(_allocator);
				dest.wstrVal = str_template::multibyte_to_unicode(/*CP_UTF8,*/ *_allocator, (const char*)x, len);
			}
			break;
		default:
			assert(false);
			break;
	}

	return dest;
}

const char*
persist_value(vt_types type, const terimber_xml_value& x, byte_allocator* allocator_)
{
	char* dest = 0;
	assert(allocator_);
	switch(type)
	{
		case vt_empty:
		case vt_null:
			break;
		case vt_bool:
			dest = (char*)check_pointer(allocator_->allocate(2));
			dest[0] = x.boolVal ? _ch1 : _ch0;
			dest[1] = 0;
			break;
		case vt_sb1:
			dest = (char*)check_pointer(allocator_->allocate(8));
			str_template::strprint(dest, 64, "%hd", (sb2_t)x.cVal);
			break;
		case vt_ub1:
			dest = (char*)check_pointer(allocator_->allocate(8));
			str_template::strprint(dest, 64, "%hu", (ub2_t)x.bVal);
			break;
		case vt_sb2:
			dest = (char*)check_pointer(allocator_->allocate(64));
			str_template::strprint(dest, 64, "%hd", x.iVal);
			break;
		case vt_ub2:
			dest = (char*)check_pointer(allocator_->allocate(64));
			str_template::strprint(dest, 64, "%hu", x.uiVal);
			break;
		case vt_sb4:
			dest = (char*)check_pointer(allocator_->allocate(64));
			str_template::strprint(dest, 64, "%ld", x.lVal);
			break;
		case vt_ub4:
			dest = (char*)check_pointer(allocator_->allocate(64));
			str_template::strprint(dest, 64, "%lu", x.ulVal);
			break;
		case vt_float:
			dest = (char*)check_pointer(allocator_->allocate(64));
			str_template::strprint(dest, 64, "%f", (double)x.fltVal);
			break;
		case vt_double:
			if (x.dblVal)
			{
				dest = (char*)check_pointer(allocator_->allocate(64));
				str_template::strprint(dest, 64, "%f", 
#ifdef OS_64BIT
					x.dblVal
#else
					*x.dblVal
#endif
					);
			}
			break;
		case vt_date:
			if (x.dblVal)
			{
				dest = (char*)check_pointer(allocator_->allocate(64));
				date date_dummy(
#ifdef OS_64BIT
					x.intVal
#else
					*x.intVal
#endif
	
				);
				date_dummy.get_date(dest);
			}
			break;
		case vt_guid:
			if (x.guidVal)
			{
				dest = (char*)check_pointer(allocator_->allocate(sizeof(guid_t) * 2 + 1));
				guid_to_string(dest, *x.guidVal);
			}
			break;
		case vt_sb8:
			if (x.intVal)
			{
				dest = (char*)check_pointer(allocator_->allocate(64));
				str_template::strprint(dest, 64, I64d,
#ifdef OS_64BIT
					x.intVal
#else
					*x.intVal
#endif
					);
			}
			break;
		case vt_ub8:
			if (x.uintVal)
			{
				dest = (char*)check_pointer(allocator_->allocate(64));
				str_template::strprint(dest, 64, I64u, 
#ifdef OS_64BIT
					x.uintVal
#else
					*x.uintVal
#endif
					);
			}
			break;
		case vt_binary:
			if (x.bufVal)
			{
				dest = (char*)check_pointer(allocator_->allocate(*(ub4_t*)x.bufVal * 2));
				binary_to_hex(dest, x.bufVal);
			}
			break;
		case vt_decimal:
		case vt_numeric:
			if (x.bufVal)
			{
				numeric num;
				num.parse_orcl(x.bufVal);
				dest = (char*)check_pointer(allocator_->allocate(num.precision() + 4)); // rooms for sign, leading zero, decimal point and last zero byte
				num.format(dest, _ch_period);
			}
			break;
		case vt_string:
			dest = (char*)x.strVal;
			break;
		case vt_wstring:
			if (x.wstrVal)
			{
				dest = (char*)str_template::unicode_to_multibyte(/*CP_UTF8,*/ *allocator_, x.wstrVal); 
			}
			break;
		default:
			assert(false);
			break;
	}

	return dest;
}

////////////////////////////////////////////////////
//////////////////////////////////////////
paged_buffer::paged_buffer(byte_allocator& data_allocator, byte_allocator& tmp_allocator, size_t xml_size) :
	_data_allocator(data_allocator), _tmp_allocator(tmp_allocator), _size(xml_size), _pos(0)
{
	_ptr = _primary = (ub1_t*)_data_allocator.allocate(_size + 1);
}

paged_buffer::~paged_buffer()
{
}

const char* 
paged_buffer::_persist()
{
	size_t total_len = size();
	char* retVal = 0;
	if (total_len && (retVal = (char*)_tmp_allocator.allocate(total_len + 1)))
	{
		retVal[total_len] = 0;
		// copies primary first
		memcpy(retVal, _primary, _size);
		size_t shift = _size;

		for (paged_store_t::iterator iter = _buffer.begin(); iter != _buffer.end(); ++iter)
		{
			size_t len = (&*iter != &_buffer.back()) ? _size : _pos;
			memcpy(retVal + shift, iter->begin(), len);
			shift += len;
		}
	}

	return retVal;
}

const ub1_t* 
paged_buffer::_persist(size_t& size_)
{
	size_t total_len = size();
	size_ = total_len;
	ub1_t* retVal = 0;
	if (total_len && (retVal = (ub1_t*)_tmp_allocator.allocate(total_len)))
	{
		retVal[total_len] = 0;
		// copies primary first
		memcpy(retVal, _primary, _size);
		size_t shift = _size;
		for (paged_store_t::iterator iter = _buffer.begin(); iter != _buffer.end(); ++iter)
		{
			size_t len = (&*iter != &_buffer.back()) ? _size : _pos;
			memcpy(retVal + shift, iter->begin(), len);
			shift += len;
		}
	}

	return retVal;
}


void
paged_buffer::add_page()
{
	// allocates new vector
	paged_vector_t vec;
	_buffer.push_back(_tmp_allocator, vec);
	paged_vector_t& entry = _buffer.back();
	entry.resize(_tmp_allocator, _size + 1);
	_ptr = entry.begin();
	_pos = 0;
}

//////////////////////////////////////////////////////////////
#pragma pack()
END_TERIMBER_NAMESPACE
