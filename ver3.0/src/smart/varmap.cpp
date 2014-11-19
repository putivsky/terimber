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

#include "smart/varmap.hpp"
#include "smart/varvalue.hpp"
#include "base/string.hpp"
#include "base/memory.hpp"


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// constuctor with memory reallocation
// ordinary value constractor
// @factory can be null for temporary objects
main_map_key::main_map_key(vt_types type, const var_value& val, variant_factory* factory)
{
	if (factory)
	{
		factory->clone(type, _var_res._val, val);
	}
	else
	{
		_var_res._val = val;
	}
}

// constuctor with memory reallocation
// for partial match only
// @factory can be null for temporary objects
main_map_key::main_map_key(const char* val, variant_factory* factory)
{
	_var_res._key._offsets = 0;

	if (factory)
	{
		_var_res._key._res = 0;

		var_value val_in, val_out;
		val_in._not_null = val != 0;
		val_in._value.strVal = val;

		factory->clone(vt_string, val_out, val_in);

		_var_res._key._res = val_out._value.strVal;
			
		ub1_t offlen = 0;
		ub4_t offsets[0xff];

		// tokenizes string
		const char* begin = val;
		bool space_found = false;
		
		while (val && *val == ' ')
			++val;

		while (val && *val && offlen < 0xff)
		{
			// spaces allowed whitespace, tab, new line vertical tab
			// #x20 | #x9 | #xD | #xA
			switch (*val)
			{
				case 0x20:
				case 0x09:
				case 0x0D:
				case 0x0A:
					if (!space_found)
						space_found = true;
					break;
				default:
					if (space_found)
					{
						offsets[offlen] = (ub4_t)(val - begin);
						space_found = false;
						++offlen;
					}
			} // switch

			++val;
		}

		if (offlen)
		{
			// allocates offsets
			factory->create(vt_numeric, val_out, sizeof(ub4_t) * (offlen + 1));
			ub4_t* p = (ub4_t*)val_out._value.bufVal;
			
			memcpy(p + 1, offsets, sizeof(ub4_t) * offlen);
			*p = offlen;
			_var_res._key._offsets = p;
		}
	}
	else
	{
		_var_res._key._res = val;
	}
}

main_map_key::main_map_key(size_t fuzzyident, const char* val, variant_factory* factory)
{
	_var_res._ngram._fuzzy_key = fuzzyident;

	if (factory)
	{
		_var_res._ngram._res = 0;

		var_value val_in, val_out;
		val_in._not_null = val != 0;
		val_in._value.strVal = val;

		factory->clone(vt_string, val_out, val_in);

		_var_res._ngram._res = val_out._value.strVal;
	}
	else
	{
		_var_res._key._res = val;
	}
}

main_map_key::main_map_key(const main_map_key& x)
{
	memcpy(&_var_res, &x._var_res, sizeof(var_key));
}

#pragma pack()
END_TERIMBER_NAMESPACE
