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

#ifndef _terimber_defxml_h_
#define _terimber_defxml_h_

#include "xml/declxml.h"
#include "base/common.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// predifined string
const char str_xml[]		= { ch_x, ch_m, ch_l, ch_null };
// http://www.w3.org/XML/1998/namespace
const char str_w3c_xml[]	= { ch_h, ch_t, ch_t, ch_p, ch_colon, ch_forward_slash, ch_forward_slash, ch_w, ch_w, ch_w, ch_period, ch_w, ch_3, ch_period, ch_o, ch_r, ch_g, ch_forward_slash, ch_X, ch_M, ch_L, ch_forward_slash, ch_1, ch_9, ch_9, ch_8, ch_forward_slash, ch_n, ch_a, ch_m, ch_e, ch_s, ch_p, ch_a, ch_c, ch_e, ch_null };
const char str_xmlns[]		= { ch_x, ch_m, ch_l, ch_n, ch_s, ch_null };
const char str_version[]	= { ch_v, ch_e, ch_r, ch_s, ch_i, ch_o, ch_n, ch_null };
const char str_encoding[]	= { ch_e, ch_n, ch_c, ch_o, ch_d, ch_i, ch_n, ch_g, ch_null };
const char str_standalone[]	= { ch_s, ch_t, ch_a, ch_n, ch_d, ch_a, ch_l, ch_o, ch_n, ch_e, ch_null };
const char str_SYSTEM[]		= { ch_S, ch_Y, ch_S, ch_T, ch_E, ch_M, ch_null };
const char str_PUBLIC[]		= { ch_P, ch_U, ch_B, ch_L, ch_I, ch_C, ch_null };
const char str_DOCTYPE[]	= { ch_D, ch_O, ch_C, ch_T, ch_Y, ch_P, ch_E, ch_null };
const char str_ELEMENT[]	= { ch_E, ch_L, ch_E, ch_M, ch_E, ch_N, ch_T, ch_null };
const char str_ENTITY[]		= { ch_E, ch_N, ch_T, ch_I, ch_T, ch_Y, ch_null };
const char str_ENTITIES[]	= { ch_E, ch_N, ch_T, ch_I, ch_T, ch_I, ch_E, ch_S, ch_null };
const char str_ENTIT[]		= { ch_E, ch_N, ch_T, ch_I, ch_T, ch_null };
const char str_ATTRLIST[]	= { ch_A, ch_T, ch_T, ch_L, ch_I, ch_S, ch_T, ch_null };
const char str_NOTATION[]	= { ch_N, ch_O, ch_T, ch_A, ch_T, ch_I, ch_O, ch_N, ch_null };
const char str_EMPTY[]		= { ch_E, ch_M, ch_P, ch_T, ch_Y, ch_null };
const char str_ANY[]		= { ch_A, ch_N, ch_Y, ch_null };
const char str__PCDATA[]	= { ch_pound, ch_P, ch_C, ch_D, ch_A, ch_T, ch_A, ch_null };
const char str_yes[]		= { ch_y, ch_e, ch_s, ch_null };
const char str_no[]			= { ch_n, ch_o, ch_null };
const char str_CDATA[]		= { ch_C, ch_D, ch_A, ch_T, ch_A, ch_null };
const char str_CTYPE[]		= { ch_C, ch_T, ch_Y, ch_P, ch_E, ch_null };
const char str_PCDATA[]		= { ch_P, ch_C, ch_D, ch_A, ch_T, ch_A, ch_null };
const char str_IGNORE[]		= { ch_I, ch_G, ch_N, ch_O, ch_R, ch_E, ch_null };
const char str_INCLUDE[]	= { ch_I, ch_N, ch_C, ch_L, ch_U, ch_D, ch_E, ch_null };
const char str_ID[]			= { ch_I, ch_D, ch_null };
const char str_IDREF[]		= { ch_I, ch_D, ch_R, ch_E, ch_F, ch_null };
const char str_IDREFS[]		= { ch_I, ch_D, ch_R, ch_E, ch_F, ch_S, ch_null };
const char str_REF[]		= { ch_R, ch_E, ch_F, ch_null };
const char str_NMTOKEN[]	= { ch_N, ch_M, ch_T, ch_O, ch_K, ch_E, ch_N, ch_null };
const char str_NMTOKENS[]	= { ch_N, ch_M, ch_T, ch_O, ch_K, ch_E, ch_N, ch_S, ch_null };
const char str_REQUIRED[]	= { ch_R, ch_E, ch_Q, ch_U, ch_I, ch_R, ch_E, ch_D, ch_null };
const char str_IMPLIED[]	= { ch_I, ch_M, ch_P, ch_L, ch_I, ch_E, ch_D, ch_null };
const char str_FIXED[]		= { ch_F, ch_I, ch_X, ch_E, ch_D, ch_null };
const char str_NDATA[]		= { ch_N, ch_D, ch_A, ch_T, ch_A, ch_null };
const char str_xml_space[]	= { ch_x, ch_m, ch_l, ch_colon, ch_s, ch_p, ch_a, ch_c, ch_e, ch_null };
const char str_default[]	= { ch_d, ch_e, ch_f, ch_a, ch_u, ch_l, ch_t, ch_null };
const char str_preserve[]	= { ch_p, ch_r, ch_e, ch_s, ch_e, ch_r, ch_v, ch_e, ch_null };

const char str_apos[]		= { ch_a, ch_p, ch_o, ch_s, ch_null };
const char str_quote[]		= { ch_q, ch_u, ch_o, ch_t, ch_null };
const char str_amp[]		= { ch_a, ch_m, ch_p, ch_null };
const char str_lt[]			= { ch_l, ch_t, ch_null };
const char str_gt[]			= { ch_g, ch_t, ch_null };

const char str_ch_apos[]	= { ch_single_quote, ch_null };
const char str_ch_quote[]	= { ch_double_quote, ch_null };
const char str_ch_amp[]		= { ch_ampersand, ch_null };
const char str_ch_lt[]		= { ch_open_angle, ch_null };
const char str_ch_gt[]		= { ch_close_angle, ch_null };
const char str_ch_colon[]	= { ch_colon, ch_null };

// '<?xml '
const ub1_t UTF8Pre[]		= { 0x3C, 0x3F, 0x78, 0x6D, 0x6C };
const ub1_t EBCDICPre[]		= { 0x4C, 0x6F, 0xA7, 0x94, 0x93 };
const ub1_t UTF16BPre[]		= { 0x00, 0x3C, 0x00, 0x3F, 0x00, 0x78, 0x00, 0x6D, 0x00, 0x6C };
const ub1_t UTF16LPre[]		= { 0x3C, 0x00, 0x3F, 0x00, 0x78, 0x00, 0x6D, 0x00, 0x6C, 0x00 };
const ub1_t UCS4BPre[]		= { 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x6D, 0x00, 0x00, 0x00, 0x6C };
const ub1_t UCS4LPre[]		= { 0x3C, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x6D, 0x00, 0x00, 0x00, 0x6C, 0x00, 0x00, 0x00 };

const ub1_t UTF8BOM[]		= { 0xEF, 0xBB, 0xBF };
const ub1_t UTF16BBOM[]		= { 0xFE, 0xFF };
const ub1_t UTF16LBOM[]		= { 0xFF, 0xFE };
const ub1_t UCS4BBOM[]		= { 0x00, 0x00, 0xFE, 0xFF };
const ub1_t UCS4LBOM[]		= { 0xFF, 0xFE, 0x00, 0x00 };

const ub1_t s_leadingByte[6] = {0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

//! \brief converts ascii char to utf-8 char sequence
xml_forceinline
bool 
usascii_to_utf8(	ub4_t in,								//!< input ascii char
					ub1_t* out,								//!< output utf-8 buffer
					size_t& count							//!< count of processed utf-8 chars
					);
//! \brief converts fixed width char buffer into utf-8 biffer
xml_forceinline
bool 
fixedN_to_utf8(		encodingSchema schema,					//!< encoding schema
					const ub1_t* in,						//!< input buffer
					size_t count,							//!< input buffer length
					ub1_t* out,								//!< output buffer
					size_t& converted,						//!< input chars converted
					size_t& processed,						//!< output chars processed
					size_t& more							//!< more input chars required to process output utf-8 char, crop input chars
					);
//! \brief checks if the input buffer is valid utf-8 string
xml_forceinline
bool 
utf8_to_utf8(		const ub1_t* in,						//!< input utf-8 buffer
					size_t count,							//!< input buffer length
					size_t& processed,						//!< input char processed
					size_t& more							//!< more input chars required to process output utf-8 char, crop input chars
					);
//! \brief converts the Windows encoding to utf-8
xml_forceinline
void 
windowsN_to_utf8(	encodingSchema schema,					//!< encoding schema
					const ub1_t* in,						//!< input Windows encoding buffer
					size_t count,							//!< input buffer length
					ub1_t* out,								//!< output buffer
					size_t& converted,						//!< input chars converted
					size_t& processed						//!< output chars processed
					);
//! \brief converts the ISO encoding to utf-8
xml_forceinline
void 
isoN_to_utf8(		encodingSchema schema,					//!< encoding schema
					const ub1_t* in,						//!< input ISO encoding buffer
					size_t count,							//!< input buffer length
					ub1_t* out,								//!< output buffer
					size_t& converted,						//!< input chars converted
					size_t& processed						//!< output chars processed
					);
//! \brief tokenizes the string into list of tokens
void 
tokenValues(		const char* x,							//!< input string
					_list< const char* >& values,			//!< [out] list of tokens
					byte_allocator& allocator_				//!< external allocator
					);

//! \brief windows 1251 encoding table
extern 
const ub2_t 
encoding_table_winodws_1251[256];

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_defxml_h_ 
