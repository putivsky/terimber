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

#ifndef _terimber_varvalue_hpp_
#define _terimber_varvalue_hpp_

#include "base/numeric.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

inline 
bool 
operator_less(vt_types type, const var_value& x, const var_value& y)
{
	if (!x._not_null)
		return y._not_null;
	else if (!y._not_null)
		return false;

	switch (type)
	{
		case vt_unknown:
		case vt_enum:
			assert(false);
		case vt_empty:
		case vt_null:
			return false;
		case vt_bool:
			return x._value.boolVal < y._value.boolVal;
		case vt_sb1:
			return x._value.cVal < y._value.cVal;
		case vt_ub1:
			return x._value.bVal < y._value.bVal;
		case vt_sb2:
			return x._value.iVal < y._value.iVal;
		case vt_ub2:
			return x._value.uiVal < y._value.uiVal;
		case vt_sb4:
			return x._value.lVal < y._value.lVal;
		case vt_ub4:
			return x._value.ulVal < y._value.ulVal;
		case vt_float:
			return x._value.fltVal < y._value.fltVal;
#ifdef OS_64BIT
		case vt_double:
			return x._value.dblVal < y._value.dblVal;
		case vt_sb8:
			return x._value.intVal < y._value.intVal;
		case vt_ub8:
			return x._value.uintVal < y._value.uintVal;
		case vt_date:
			return x._value.intVal < y._value.intVal;
#else
		case vt_double:
			return *x._value.dblVal < *y._value.dblVal;
		case vt_sb8:
			return *x._value.intVal < *y._value.intVal;
		case vt_ub8:
			return *x._value.uintVal < *y._value.uintVal;
		case vt_date:
			return *x._value.intVal < *y._value.intVal;
#endif
		case vt_string:
			return str_template::strcmp(x._value.strVal, y._value.strVal, os_minus_one) < 0;
		case vt_wstring:
			return str_template::strcmp(x._value.wstrVal, y._value.wstrVal, os_minus_one) < 0;
		case vt_decimal:
		case vt_numeric:
			// we need for numeric binaries static inline? function for compare
			// oracle format for numeric
			// convert numeric
			return numeric::compare_orcl(x._value.bufVal, y._value.bufVal) < 0;
		case vt_binary:
			if (*(size_t*)x._value.bufVal < *(size_t*)y._value.bufVal)
				return true;
			else if (*(size_t*)y._value.bufVal < *(size_t*)x._value.bufVal)
				return false;
			else
				return memcmp(x._value.bufVal, y._value.bufVal, *(size_t*)x._value.bufVal) < 0;
		case vt_guid:
			return memcmp(x._value.bufVal, y._value.guidVal, sizeof(guid_t)) < 0;
	}

	return true;
}


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_varvalue_hpp_
