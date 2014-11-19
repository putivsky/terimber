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

#ifndef _terimber_defxml_hpp_
#define _terimber_defxml_hpp_

#include "xml/defxml.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

xml_forceinline
bool
usascii_to_utf8(ub4_t in, ub1_t* out, size_t& count)
{
	
	// before processing checks validity
	// adding
	switch (in)
	{
		case ch_hor_tab:
		case ch_lf:
		case ch_cr:
			break;
		default:
			if (in >= 0x20 && in <= 0xD7FF 
				|| in >= 0xE000 && in <= 0xFFFD
				|| in >= 0x10000 && in <= 0x10FFFF)
				break;
			else
				return false;
	} // switch
	// adding

	// 0000 0000-0000 007F   0xxxxxxx
	// 0000 0080-0000 07FF   110xxxxx 10xxxxxx
	// 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
	// 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	// 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	// 0400 0000-7FFF FFFF   1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	if (in < 0x80)
		count = 1;	
	else if (in < 0x800)
		count = 2;
	else if (in < 0x10000)
		count = 3;
	else if (in < 0x200000)
		count = 4;
	else if (in < 0x4000000)
		count = 5;
	else if (in <= 0x7FFFFFFF)
		count = 6;
	else
		return false;

	out[count] = 0;
	out += count;

	switch(count)
	{
		case 6 : *--out = (ub1_t)((in | 0x80UL) & 0xBFUL);	in >>= 6;
		case 5 : *--out = (ub1_t)((in | 0x80UL) & 0xBFUL);	in >>= 6;
		case 4 : *--out = (ub1_t)((in | 0x80UL) & 0xBFUL);	in >>= 6;
		case 3 : *--out = (ub1_t)((in | 0x80UL) & 0xBFUL);	in >>= 6;
		case 2 : *--out = (ub1_t)((in | 0x80UL) & 0xBFUL);	in >>= 6;
		case 1 : *--out = (ub1_t)(in | s_leadingByte[count - 1]);
	}
	
	return count != 1 || out[0] != 0;
}

xml_forceinline
bool
fixedN_to_utf8(encodingSchema schema, const ub1_t* in, size_t count, ub1_t* out, size_t& converted, size_t& processed, size_t& more)
{
	processed = 0;
	converted = 0;
	more = 0;
	size_t byte_count = 0;
	
	ub4_t value = 0, surrogate = 0;	

	while (count)
	{
		switch (schema)
		{
			case UTF_16B:
				// checks surrogate bytes
				value = (ub4_t(*in) << 8) | ub4_t(*(in + 1));
				surrogate = 0;
				if (value >= 0xD800 && value <= 0xDBFF)
				{
					if (count < 4)
					{
						more = 4 - count;
						return false;
					}

					count -= 2;
					in += 2;
					processed += 2;
					surrogate = value;
					value = (ub4_t(*in) << 8) | ub4_t(*(in + 1));
					value = ub4_t((surrogate - 0xD800) << 10) + ub4_t((value - 0xDC00) + 0x10000);
				}

				count -= 2;
				in += 2;
				processed += 2;
				break;
			case UTF_16L:
				// checks surrogate bytes
				value = (ub4_t(*(in + 1)) << 8) | ub4_t(*in);
				surrogate = 0;
				if (value >= 0xD800 && value <= 0xDBFF)
				{
					if (count < 4)
					{
						more = 4 - count;
						return false;
					}

					count -= 2;
					in += 2;
					processed += 2;
					surrogate = value;
					value = (ub4_t(*(in + 1)) << 8) | ub4_t(*in);
					value = ub4_t((surrogate - 0xD800) << 10) + ub4_t((value - 0xDC00) + 0x10000);
				}

				count -= 2;
				in += 2;
				processed += 2;
				break;
			case UCS_4B:
				value = (ub4_t(*in) << 24) | (ub4_t(*(in + 1)) << 16) | (ub4_t(*(in + 2)) << 8) | ub4_t(*(in + 3));
				count -= 4;
				in += 4;
				processed += 4;	
				break;
			case UCS_4BS:
				value = (ub4_t(*(in + 2)) << 24) | (ub4_t(*(in + 3)) << 16) | (ub4_t(*in) << 8) | ub4_t(*(in + 1));
				count -= 4;
				in += 4;
				processed += 4;	
				break;
			case UCS_4L:
				value = (ub4_t(*(in + 3)) << 24) | (ub4_t(*(in + 2)) << 16) | (ub4_t(*(in + 1)) << 8) | ub4_t(*in);
				count -= 4;
				in += 4;
				processed += 4;	
				break;
			case UCS_4LS:
				value = (ub4_t(*(in + 1)) << 24) | (ub4_t(*in) << 16) | (ub4_t(*(in + 3)) << 8) | ub4_t(*(in + 2));
				count -= 4;
				in += 4;
				processed += 4;	
				break;
			default:
				assert(false);
		}

		// before process check validity
		// adding
		switch (value)
		{
			case ch_hor_tab:
			case ch_lf:
			case ch_cr:
				break;
			default:
				if (value >= 0x20 && value <= 0xD7FF 
					|| value >= 0xE000 && value <= 0xFFFD
					|| value >= 0x10000 && value <= 0x10FFFF)
					break;
				else
					return false;
		} // switch
		// adding

		// detects value
		if (value < 0x80)
			byte_count = 1;	
		else if (value < 0x800)
			byte_count = 2;
		else if (value < 0x10000)
			byte_count = 3;
		else if (value < 0x200000)
			byte_count = 4;
		else if (value < 0x4000000)
			byte_count = 5;
		else if (value <= 0x7FFFFFFF)
			byte_count = 6;
		else
			return false;

		out += byte_count;
		converted += byte_count;

		switch(byte_count)
		{
			case 6 : *--out = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 5 : *--out = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 4 : *--out = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 3 : *--out = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 2 : *--out = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 1 : *--out = (ub1_t)(value | s_leadingByte[byte_count - 1]);
		} // switch

		out += byte_count;
	} // while

	return true;
}

xml_forceinline
bool
utf8_to_utf8(const ub1_t* in, size_t count, size_t& processed, size_t& more)
{
	processed = 0;
	more = 0;
	size_t byte_count = 0;
	size_t max_byte = 0;
	ub4_t value = 0;

	while (count)
	{
		switch (byte_count)
		{
			case 0:
				// checks
				max_byte = 0;
				if (*in <= 0x7F)
				{
					// adding
					value = *in;
					// adding
					break;
				}

				if (*in < 0xC0 || *in > 0xFD)
					return false;

				if (*in < 0xE0)
					max_byte = byte_count = 1;
				else if (*in < 0xF0)
					max_byte = byte_count = 2;
				else if (*in < 0xF8)
					max_byte = byte_count = 3;
				else if (*in < 0xFC)
					max_byte = byte_count = 4;
				else
					max_byte = byte_count = 5;

				// 
				value = *in & ((2 << (5 - byte_count)) - 1);
				break;
			default:
				if (*in > 0xBF || *in < 0x80)
					return false;

				// adding
				value <<= 6;
				value |= *in & 0x3F;
				// adding

				--byte_count;
		} // switch

		// adding
		if (!byte_count)
		{
			switch (value)
			{
				case ch_hor_tab:
				case ch_lf:
				case ch_cr:
					break;
				default:
					if (value >= 0x20 && value <= 0xD7FF 
						|| value >= 0xE000 && value <= 0xFFFD
						|| value >= 0x10000 && value <= 0x10FFFF)
						break;
					else
						return false;
			} // switch
		}
		// adding

		++in;
		--count;
		++processed;
	} // while

	if (byte_count)
	{
		processed += byte_count;
		processed -= max_byte + 1;
		more = byte_count;
	}

	return true;
}

xml_forceinline
void windowsN_to_utf8(encodingSchema schema, const ub1_t* in, size_t count, ub1_t* out, size_t& converted, size_t& processed)
{
	processed = 0;
	converted = 0;
	size_t byte_count = 0;
	
	ub2_t value = 0;	

	while (count)
	{
		switch (schema)
		{
			case WINDOWS_1251:
				value = encoding_table_winodws_1251[*in];
				count -= 1;
				in += 1;
				processed += 1;
				break;
			default:
				assert(false);
		}

		// detects value
		if (value < 0x80)
			byte_count = 1;	
		else if (value < 0x800)
			byte_count = 2;
		else
			byte_count = 3;

		out += byte_count;
		converted += byte_count;

		switch(byte_count)
		{
			case 3 : *--out = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 2 : *--out = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 1 : *--out = (ub1_t)(value | s_leadingByte[byte_count - 1]);
		} // switch

		out += byte_count;
	} // while
}

xml_forceinline
void isoN_to_utf8(encodingSchema schema, const ub1_t* in, size_t count, ub1_t* out, size_t& converted, size_t& processed)
{
	processed = 0;
	converted = 0;
	size_t byte_count = 0;
	
	ub1_t value = 0;	

	while (count)
	{
		switch (schema)
		{
			case UTF_ISO88591:
				value = *in;
				count -= 1;
				in += 1;
				processed += 1;
				break;
			default:
				assert(false);
		}

		// detects value
		if (value < 0x80)
			byte_count = 1;	
		else
			byte_count = 2;

		out += byte_count;
		converted += byte_count;

		switch(byte_count)
		{
			case 2 : *--out = (ub1_t)((value | 0x80UL) & 0xBFUL);	value >>= 6;
			case 1 : *--out = (ub1_t)(value | s_leadingByte[byte_count - 1]);
		} // switch

		out += byte_count;
	} // while
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_defxml_hpp_ 
