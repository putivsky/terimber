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
#include "smart/varobj.h"
#include "smart/varfactory.h"

#include "smart/varmap.hpp"
#include "smart/varvalue.hpp"
#include "smart/byterep.hpp"

#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/map.hpp"
#include "base/list.hpp"
#include "base/string.hpp"
#include "base/numeric.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

variant_factory::variant_factory() :
	_double_allocator(1024*128),
	_long_double_allocator(1024*128),
	_guid_allocator(1024*128)
{
}

variant_factory::~variant_factory()
{
}

bool 
variant_factory::create(vt_types type, var_value& val, size_t len)
{
	switch (type)
	{
		case vt_unknown:
		case vt_enum:
		case vt_empty:
			// not supported
			assert(false);
			return false;
			break;
		case vt_null:
			// nothing to do;
			memset(&val._value, 0, sizeof(terimber_xml_value));
			break;
		case vt_bool:
		case vt_sb1:
		case vt_ub1:
		case vt_sb2:
		case vt_ub2:
		case vt_sb4:
		case vt_ub4:
		case vt_float:
			// nothing to do;
			memset(&val._value, 0, sizeof(terimber_xml_value));
			break;
#ifdef OS_64BIT
		case vt_double:
		case vt_sb8:
		case vt_ub8:
		case vt_date:
			// nothing to do;
			memset(&val._value, 0, sizeof(terimber_xml_value));
			break;
#else
		case vt_double:
			val._value.dblVal = _double_allocator.allocate();
			break;
		case vt_date:
		case vt_sb8:
			val._value.intVal = (sb8_t*)_double_allocator.allocate();
			break;
		case vt_ub8:
			val._value.uintVal = (ub8_t*)_double_allocator.allocate();
			break;
#endif
		case vt_string:
			{
				sb1_t* p = (sb1_t*)_byte_allocator->allocate(len + 1);
				p[len] = 0;
				val._value.strVal = (const char*)p;
			}
			break;
		case vt_wstring:
			{
				wchar_t* p = (wchar_t*)_byte_allocator->allocate(2*(len + 1));
				p[len] = 0;
				val._value.wstrVal = p;
			}
			break;
		case vt_decimal:
		case vt_numeric:
			val._value.bufVal = (const ub1_t*)_byte_allocator->allocate(len);
			break;
		case vt_binary:
			{
				ub1_t* p = (ub1_t*)_byte_allocator->allocate(len + sizeof(size_t));
				*(size_t*)p = len;
				val._value.bufVal = p;
			}
			break;
		case vt_guid:
			val._value.guidVal = _guid_allocator.allocate();
			break;
	}

	return true;
}

bool 
variant_factory::clone(vt_types type, var_value& out, const var_value& in)
{
	out._not_null = in._not_null;
	memset(&out._value, 0, sizeof(terimber_xml_value));
	
	switch (type)
	{
		case vt_unknown:
		case vt_enum:
		case vt_empty:
			// not supported
			assert(false);
			return false;
			break;
		case vt_null:
			// nothing to do;
			break;
		case vt_bool:
		case vt_sb1:
		case vt_ub1:
		case vt_sb2:
		case vt_ub2:
		case vt_sb4:
		case vt_ub4:
		case vt_float:
			// nothing to do;
			memcpy(&out._value, &in._value, sizeof(terimber_xml_value));
			break;
#ifdef OS_64BIT
		case vt_double:
		case vt_sb8:
		case vt_ub8:
		case vt_date:
			// nothing to do;
			memcpy(&out._value, &in._value, sizeof(terimber_xml_value));
			break;
#else
		case vt_double:
			if (in._not_null)
			{
				double* p = _double_allocator.allocate();
				if (!p)
					return false;

				*p = *in._value.dblVal;
				out._value.dblVal = p;
			}
			break;
		case vt_date:
		case vt_sb8:
			if (in._not_null)
			{
				sb8_t* p = (sb8_t*)_double_allocator.allocate();
				if (!p)
					return false;

				*p = *in._value.intVal;
				out._value.intVal = p;
			}
			break;
		case vt_ub8:
			if (in._not_null)
			{
				ub8_t* p = (ub8_t*)_double_allocator.allocate();
				if (!p)
					return false;

				*p = *in._value.uintVal;
				out._value.uintVal = p;
			}
			break;
#endif
		case vt_string:
			if (in._not_null)
			{
				size_t len = in._value.strVal ? str_template::strlen(in._value.strVal) : 0;
				if (len)
				{
					char* p = (char*)_byte_allocator->allocate(len + 1);
					if (!p)
						return false;

					p[len] = 0;
					memcpy(p, in._value.strVal, len);
					out._value.strVal = p;
				}
			}
			break;
		case vt_wstring:
			if (in._not_null)
			{
				size_t len = in._value.wstrVal ? str_template::strlen(in._value.wstrVal) : 0;
				if (len)
				{
					wchar_t* p = (wchar_t*)_byte_allocator->allocate(2*(len + 1));
					if (!p)
						return false;

					p[len] = 0;
					memcpy(p, in._value.wstrVal, 2*len);
					out._value.wstrVal = p;
				}
			}
			break;
		case vt_decimal:
		case vt_numeric:
			if (in._not_null && in._value.bufVal)
			{
				_tmp_allocator.reset();
				numeric num(&_tmp_allocator);
				if (!num.parse_orcl(in._value.bufVal))
					return false;

				size_t len = num.orcl_len();

				if (len)
				{
					ub1_t* p = (ub1_t*)_byte_allocator->allocate(len);
					if (!p)
						return false;

					if (!num.persist_orcl(p))
						return false;

					out._value.bufVal = p;
				}
			}
			break;
		case vt_binary:
			if (in._not_null)
			{
				size_t len = in._value.bufVal ? *(size_t*)in._value.bufVal : 0;
				if (len)
				{
					ub1_t* p = (ub1_t*)_byte_allocator->allocate(len + sizeof(size_t));
					if (!p)
						return false;

					*(size_t*)p = len;
					memcpy(p + sizeof(size_t), in._value.bufVal + sizeof(size_t), len);
					out._value.bufVal = p;
				}
			}
			break;
		case vt_guid:
			if (in._not_null && in._value.guidVal)
			{
				guid_t* p = _guid_allocator.allocate();
				if (!p)
					return false;

				memcpy(p, in._value.guidVal, sizeof(guid_t));
				out._value.guidVal = p;
			}
			break;
	}

	return true;
}

void 
variant_factory::destroy(vt_types type, var_value& val)
{
	switch (type)
	{
		case vt_unknown:
		case vt_enum:
		case vt_empty:
			// not supported
			assert(false);
			break;
		case vt_null:
			break;
		case vt_bool:
		case vt_sb1:
		case vt_ub1:
		case vt_sb2:
		case vt_ub2:
		case vt_sb4:
		case vt_ub4:
		case vt_float:
			break;
#ifdef OS_64BIT
		case vt_double:
		case vt_sb8:
		case vt_ub8:
		case vt_date:
			break;
#else
		case vt_double:
			if (val._value.dblVal)
				_double_allocator.deallocate((void*)val._value.dblVal);
			break;
		case vt_date:
		case vt_sb8:
			if (val._value.intVal)
				_double_allocator.deallocate((void*)val._value.intVal);
			break;
		case vt_ub8:
			if (val._value.uintVal)
				_double_allocator.deallocate((void*)val._value.uintVal);
			break;
#endif
		case vt_string:
			if (val._value.strVal)
				_byte_allocator->deallocate((char*)val._value.strVal);
			break;
		case vt_wstring:
			if (val._value.wstrVal)
				_byte_allocator->deallocate((char*)val._value.wstrVal);
			break;
		case vt_decimal:
		case vt_numeric:
		case vt_binary:
			if (val._value.bufVal)
				_byte_allocator->deallocate((char*)val._value.bufVal);
			break;
		case vt_guid:
			if (val._value.guidVal)
				_guid_allocator.deallocate((void*)val._value.guidVal);
			break;
	}
}

//! \brief reuse memory
void
variant_factory::reset()
{
	_byte_allocator->reset();
	_double_allocator.reset();
	_long_double_allocator.reset();
	_guid_allocator.reset();
	_tmp_allocator.reset();
}

#pragma pack()
END_TERIMBER_NAMESPACE
