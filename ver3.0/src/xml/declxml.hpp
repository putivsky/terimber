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

#ifndef _terimber_declxml_hpp_
#define _terimber_declxml_hpp_

#include "xml/declxml.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

static const ub1_t s_white_space_mask		= 0x01;
static const ub1_t s_digit_mask				= 0x02;
static const ub1_t s_english_letter_mask	= 0x04;
static const ub1_t s_first_char_name_mask	= 0x08;
static const ub1_t s_char_name_mask			= 0x10;
static const ub1_t s_public_char_mask		= 0x20;
static const ub1_t s_versionnum_char_mask	= 0x40;
static const ub1_t s_encname_char_mask		= 0x80;

////////////////////////////////
// char detection related functions
////////////////////////////
// [3]    S    ::=    (#x20 | #x9 | #xD | #xA)+ 
xml_forceinline
bool 
is_white_space(ub1_t symbol)
{ 
	return (s_char_map[symbol] & s_white_space_mask) != 0; 
}

// [13]    PubidChar    ::=    #x20 | #xD | #xA | [a-zA-Z0-9] | [-'()+,./:=?;!*#@$_%] 
xml_forceinline 
bool 
is_public_char(ub1_t symbol)
{ 
	return (s_char_map[symbol] & s_public_char_mask) != 0; 
}

// [26]    VersionNum    ::=    ([a-zA-Z0-9_.:] | '-')+ 
xml_forceinline 
bool 
is_versionnum_char(ub1_t symbol)
{ 
	return (s_char_map[symbol] & s_versionnum_char_mask) != 0; 
}

// [81]    EncName    ::=  ,//  [A-Za-z] ([A-Za-z0-9._] | '-')* 
xml_forceinline 
bool 
is_encname_char(ub1_t symbol)
{ 
	return (s_char_map[symbol] & s_encname_char_mask) != 0; 
}

xml_forceinline 
bool 
is_digit(ub1_t symbol)
{ 
	return (s_char_map[symbol] & s_digit_mask) != 0; 
}

xml_forceinline 
bool 
is_letter(ub1_t symbol)
{ 
	return symbol > 0x7F || (s_char_map[symbol] & s_english_letter_mask) != 0; 
}

// [5]    Name    ::=    (Letter | '_' | ':') (NameChar)* 
xml_forceinline 
bool 
is_name_first_char(ub1_t symbol)
{ 
	return symbol > 0x7F || (s_char_map[symbol] & s_first_char_name_mask); 
}

// [4]    NameChar    ::=    Letter | Digit | '.' | '-' | '_' | ':' | CombiningChar | Extender 
xml_forceinline
bool
is_name_char(ub1_t symbol)
{ 
	return symbol > 0x7F || (s_char_map[symbol] & s_char_name_mask) != 0; 
}

xml_forceinline 
bool 
is_attribute_char(ub1_t symbol)
{ 
	return symbol != ch_ampersand && symbol != ch_open_angle; 
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_declxml_hpp_ 
