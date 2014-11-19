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

#ifndef _terimber_declxml_h_
#define _terimber_declxml_h_

#include "allinc.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

extern const ub1_t s_char_map[256];

#if OS_TYPE == OS_WIN32
//#if defined(_MSC_VER) && (_MSC_VER > 1200) 
#define xml_forceinline __forceinline
#else
#define xml_forceinline inline
#endif


// UTF-8 char representation
const char ch_null				= 0x00;
const char ch_hor_tab			= 0x09;
const char ch_lf				= 0x0A; // line feed
const char ch_ver_tab			= 0x0B;
const char ch_ff				= 0x0C;
const char ch_cr				= 0x0D; // caret return
const char ch_ampersand			= 0x26; // '&'
const char ch_asterisk			= 0x2A; // '*'
const char ch_at				= 0x40; // '@'
const char ch_back_slash		= 0x5C; // '\'
const char ch_bang				= 0x21; // '!'
const char ch_caret				= 0x5E; 
const char ch_close_angle		= 0x3E; // '>'
const char ch_close_curly		= 0x7D; // '}'
const char ch_close_paren		= 0x29; // ')'
const char ch_close_square		= 0x5D; // ']'
const char ch_colon				= 0x3A; // ':'
const char ch_comma				= 0x2C; // ','
const char ch_dash				= 0x2D; // '-'
const char ch_dollar_sign		= 0x24; // '$'
const char ch_double_quote		= 0x22; // '"'
const char ch_equal				= 0x3D; // '='
const char ch_forward_slash		= 0x2F; // '/'
const char ch_grave				= 0x60; // '`'
//const char ch_nel				= 0x85;
const char ch_open_angle		= 0x3C; // '<'
const char ch_open_curly		= 0x7B; // '{'
const char ch_open_paren		= 0x28; // '('
const char ch_open_square		= 0x5B; // '['
const char ch_percent			= 0x25; // '%'
const char ch_period			= 0x2E; // '.'
const char ch_pipe				= 0x7C; // '|'
const char ch_plus				= 0x2B; // '+'
const char ch_pound				= 0x23; // '#'
const char ch_question			= 0x3F; // '?'
const char ch_single_quote		= 0x27; // '''
const char ch_space				= 0x20; // ' '
const char ch_semicolon			= 0x3B; // ';'
const char ch_tilde				= 0x7E; // '~'
const char ch_underscore		= 0x5F; // '_'


const char ch_0					= 0x30;
const char ch_1					= 0x31;
const char ch_2					= 0x32;
const char ch_3					= 0x33;
const char ch_4					= 0x34;
const char ch_5					= 0x35;
const char ch_6					= 0x36;
const char ch_7					= 0x37;
const char ch_8					= 0x38;
const char ch_9					= 0x39;

const char ch_a					= 0x61;
const char ch_b					= 0x62;
const char ch_c					= 0x63;
const char ch_d					= 0x64;
const char ch_e					= 0x65;
const char ch_f					= 0x66;
const char ch_g					= 0x67;
const char ch_h					= 0x68;
const char ch_i					= 0x69;
const char ch_j					= 0x6A;
const char ch_k					= 0x6B;
const char ch_l					= 0x6C;
const char ch_m					= 0x6D;
const char ch_n					= 0x6E;
const char ch_o					= 0x6F;
const char ch_p					= 0x70;
const char ch_q					= 0x71;
const char ch_r					= 0x72;
const char ch_s					= 0x73;
const char ch_t					= 0x74;
const char ch_u					= 0x75;
const char ch_v					= 0x76;
const char ch_w					= 0x77;
const char ch_x					= 0x78;
const char ch_y					= 0x79;
const char ch_z					= 0x7A;
	
const char ch_A					= 0x41;
const char ch_B					= 0x42;
const char ch_C					= 0x43;
const char ch_D					= 0x44;
const char ch_E					= 0x45;
const char ch_F					= 0x46;
const char ch_G					= 0x47;
const char ch_H					= 0x48;
const char ch_I					= 0x49;
const char ch_J					= 0x4A;
const char ch_K					= 0x4B;
const char ch_L					= 0x4C;
const char ch_M					= 0x4D;
const char ch_N					= 0x4E;
const char ch_O					= 0x4F;
const char ch_P					= 0x50;
const char ch_Q					= 0x51;
const char ch_R					= 0x52;
const char ch_S					= 0x53;
const char ch_T					= 0x54;
const char ch_U					= 0x55;
const char ch_V					= 0x56;
const char ch_W					= 0x57;
const char ch_X					= 0x58;
const char ch_Y					= 0x59;
const char ch_Z					= 0x5A;

//! \enum encodingSchema
//! \brief supported encoding
enum encodingSchema
{
	AUTO,													//!< automatic detection
	UTF_8,													//!< utf-8
	US_ASCII,												//!< ascii
	//EBCDIC,
	UTF_16B,												//!< utf-16 big ending
	UTF_16L,												//!< utf-16 little ending
	UTF_16,													//!< utf-16 endings detected automatically
	UCS_4B,													//!< utf-32 (1234 byte order)		
	UCS_4BS,												//!< utf-32 (3412 byte order)
	UCS_4L,													//!< utf-32 (4321 byte order)
	UCS_4LS,												//!< utf-32 (2143 byte order)
	UCS_4,													//!< utf-32 endings detected automatically
	WINDOWS_1251,											//!< windows 1251
	UTF_ISO88591											//!< iso 88591
};

////////////////////////////////
// char detection related functions
////////////////////////////
// [3]    S    ::=    (#x20 | #x9 | #xD | #xA)+ 
//! \brief detect if symbol is whitespace
xml_forceinline
bool 
is_white_space(		ub1_t symbol							//!< input symbol
					);
// [13]    PubidChar    ::=    #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%] 
//! \brief detects if the symbol is public
xml_forceinline 
bool 
is_public_char(		ub1_t symbol							//!< input symbol
					);

// [26]    VersionNum    ::=    ([a-zA-Z0-9_.:] | '-')+ 
//! \brief check is symbol is valid vesrion number symbol
xml_forceinline 
bool 
is_versionnum_char(	ub1_t symbol							//!< input symbol
					);

// [81]    EncName    ::=  ,//  [A-Za-z] ([A-Za-z0-9._] | '-')* 
//! \brief checks if the symbol is EncName
xml_forceinline 
bool 
is_encname_char(	ub1_t symbol							//!< input symbol
					);
//! \brief checks if the symbol is digit
xml_forceinline 
bool 
is_digit(			ub1_t symbol							//!< input symbol
					);
//! \brief checks if the symbol is letter
xml_forceinline 
bool 
is_letter(			ub1_t symbol							//!< input symbol
					);
// [5]    Name    ::=    (Letter | '_' | ':') (NameChar)* 
//! \brief checks if the symbol is a legal name first char
xml_forceinline 
bool 
is_name_first_char(	ub1_t symbol							//!< input symbol
					);
// [4]    NameChar    ::=    Letter | Digit | '.' | '-' | '_' | ':' | CombiningChar | Extender 
//! \brief checks if the symbol is a legal name char
xml_forceinline 
bool 
is_name_char(		ub1_t symbol							//!< input symbol
					);

//! \brief checks if the symbol is a valid attribute char
xml_forceinline 
bool 
is_attribute_char(	ub1_t symbol							//!< input symbol
					);

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_declxml_h_ 
