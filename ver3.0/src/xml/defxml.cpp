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

#include "xml/defxml.hpp"
#include "xml/declxml.hpp"
#include "xml/miscxml.hpp"
#include "xml/mngxml.hpp"
#include "xml/storexml.hpp"
 
#include "base/common.hpp"
#include "base/list.hpp"
#include "base/string.hpp"
#include "base/memory.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

const ub1_t s_char_map[256] = 
{
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x01,0x21,0x00, 0x00,0x21,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x21,0x20,0x00,0x20, 0x20,0x20,0x00,0x20, 0x20,0x20,0x20,0x20, 0x20,0xF0,0xF0,0x20,
	0xF2,0xF2,0xF2,0xF2, 0xF2,0xF2,0xF2,0xF2, 0xF2,0xF2,0x78,0x20, 0x00,0x20,0x00,0x20,

	0x20,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0xFC,
	0xFC,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0x00, 0x00,0x00,0x00,0xF8,
	0x00,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0xFC,
	0xFC,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0xFC, 0xFC,0xFC,0xFC,0x00, 0x00,0x00,0x00,0x00,
	
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00
};

// parses xml valid values
//static
void 
tokenValues(const char* x, _list< const char* >& values, byte_allocator& tmp_allocator)
{
	paged_buffer keeper(tmp_allocator, tmp_allocator, os_def_size);

	if (!x)
		xml_exception_throw("Empty names syntax", 0);

	while (is_white_space(*x))
		++x;

	while (*x)
	{	
		if (is_white_space(*x))
		{
			// makes a copy of values
			values.push_back(tmp_allocator, copy_string(keeper.persist(), tmp_allocator, keeper.size()));
			keeper.reset();
			while (*x && is_white_space(*x))
				++x;
		}
		else
		{
			keeper << *x;
			++x;
		}
	}

	if (keeper.size())
		values.push_back(tmp_allocator, keeper.persist());
}

const ub2_t encoding_table_winodws_1251[256] =
{
0x0000,//  [ ] [ ]  NULL
0x0001,//  [ ] [ ]  START OF HEADING
0x0002,//  [ ] [ ]  START OF TEXT
0x0003,//  [ ] [ ]  END OF TEXT
0x0004,//  [ ] [ ]  END OF TRANSMISSION
0x0005,//  [ ] [ ]  ENQUIRY
0x0006,//  [ ] [ ]  ACKNOWLEDGE
0x0007,//  [ ] [ ]  BELL
0x0008,//  [ ] [ ]  BACKSPACE
0x0009,//  [ ] [ ]  HORIZONTAL TABULATION
0x000A,//  [ ] [ ]  LINE FEED
0x000B,//  [ ] [ ]  VERTICAL TABULATION
0x000C,//  [ ] [ ]  FORM FEED
0x000D,//  [ ] [ ]  CARRIAGE RETURN
0x000E,//  [ ] [ ]  SHIFT OUT
0x000F,//  [ ] [ ]  SHIFT IN
0x0010,//  [ ] [ ]  DATA LINK ESCAPE
0x0011,//  [ ] [ ]  DEVICE CONTROL ONE
0x0012,//  [ ] [ ]  DEVICE CONTROL TWO
0x0013,//  [ ] [ ]  DEVICE CONTROL THREE
0x0014,//  [ ] [ ]  DEVICE CONTROL FOUR
0x0015,//  [ ] [ ]  NEGATIVE ACKNOWLEDGE
0x0016,//  [ ] [ ]  SYNCHRONOUS IDLE
0x0017,//  [ ] [ ]  END OF TRANSMISSION BLOCK
0x0018,//  [ ] [ ]  CANCEL
0x0019,//  [ ] [ ]  END OF MEDIUM
0x001A,//  [ ] [ ]  SUBSTITUTE
0x001B,//  [ ] [ ]  ESCAPE
0x001C,//  [ ] [ ]  FILE SEPARATOR
0x001D,//  [ ] [ ]  GROUP SEPARATOR
0x001E,//  [ ] [ ]  RECORD SEPARATOR
0x001F,//  [ ] [ ]  UNIT SEPARATOR
0x0020,//  [ ] [ ]  SPACE
0x0021,//  [!] [!]  EXCLAMATION MARK
0x0022,//  ["] ["]  QUOTATION MARK
0x0023,//  [#] [#]  NUMBER SIGN
0x0024,//  [$] [$]  DOLLAR SIGN
0x0025,//  [%] [%]  PERCENT SIGN
0x0026,//  [&] [&]  AMPERSAND
0x0027,//  ['] [']  APOSTROPHE
0x0028,//  [(] [(]  LEFT PARENTHESIS
0x0029,//  [)] [)]  RIGHT PARENTHESIS
0x002A,//  [*] [*]  ASTERISK
0x002B,//  [+] [+]  PLUS SIGN
0x002C,//  [,] [,]  COMMA
0x002D,//  [-] [-]  HYPHEN-MINUS
0x002E,//  [.] [.]  FULL STOP
0x002F,//  [/] [/]  SOLIDUS
0x0030,//  [0] [0]  DIGIT ZERO
0x0031,//  [1] [1]  DIGIT ONE
0x0032,//  [2] [2]  DIGIT TWO
0x0033,//  [3] [3]  DIGIT THREE
0x0034,//  [4] [4]  DIGIT FOUR
0x0035,//  [5] [5]  DIGIT FIVE
0x0036,//  [6] [6]  DIGIT SIX
0x0037,//  [7] [7]  DIGIT SEVEN
0x0038,//  [8] [8]  DIGIT EIGHT
0x0039,//  [9] [9]  DIGIT NINE
0x003A,//  [:] [:]  COLON
0x003B,//  [;] [;]  SEMICOLON
0x003C,//  [<] [<]  LESS-THAN SIGN
0x003D,//  [=] [=]  EQUALS SIGN
0x003E,//  [>] [>]  GREATER-THAN SIGN
0x003F,//  [?] [?]  QUESTION MARK
0x0040,//  [@] [@]  COMMERCIAL AT
0x0041,//  [A] [A]  LATIN CAPITAL LETTER A
0x0042,//  [B] [B]  LATIN CAPITAL LETTER B
0x0043,//  [C] [C]  LATIN CAPITAL LETTER C
0x0044,//  [D] [D]  LATIN CAPITAL LETTER D
0x0045,//  [E] [E]  LATIN CAPITAL LETTER E
0x0046,//  [F] [F]  LATIN CAPITAL LETTER F
0x0047,//  [G] [G]  LATIN CAPITAL LETTER G
0x0048,//  [H] [H]  LATIN CAPITAL LETTER H
0x0049,//  [I] [I]  LATIN CAPITAL LETTER I
0x004A,//  [J] [J]  LATIN CAPITAL LETTER J
0x004B,//  [K] [K]  LATIN CAPITAL LETTER K
0x004C,//  [L] [L]  LATIN CAPITAL LETTER L
0x004D,//  [M] [M]  LATIN CAPITAL LETTER M
0x004E,//  [N] [N]  LATIN CAPITAL LETTER N
0x004F,//  [O] [O]  LATIN CAPITAL LETTER O
0x0050,//  [P] [P]  LATIN CAPITAL LETTER P
0x0051,//  [Q] [Q]  LATIN CAPITAL LETTER Q
0x0052,//  [R] [R]  LATIN CAPITAL LETTER R
0x0053,//  [S] [S]  LATIN CAPITAL LETTER S
0x0054,//  [T] [T]  LATIN CAPITAL LETTER T
0x0055,//  [U] [U]  LATIN CAPITAL LETTER U
0x0056,//  [V] [V]  LATIN CAPITAL LETTER V
0x0057,//  [W] [W]  LATIN CAPITAL LETTER W
0x0058,//  [X] [X]  LATIN CAPITAL LETTER X
0x0059,//  [Y] [Y]  LATIN CAPITAL LETTER Y
0x005A,//  [Z] [Z]  LATIN CAPITAL LETTER Z
0x005B,//  [[] [[]  LEFT SQUARE BRACKET
0x005C,//  [\] [\]  REVERSE SOLIDUS
0x005D,//  []] []]  RIGHT SQUARE BRACKET
0x005E,//  [^] [^]  CIRCUMFLEX ACCENT
0x005F,//  [_] [_]  LOW LINE
0x0060,//  [`] [`]  GRAVE ACCENT
0x0061,//  [a] [a]  LATIN SMALL LETTER A
0x0062,//  [b] [b]  LATIN SMALL LETTER B
0x0063,//  [c] [c]  LATIN SMALL LETTER C
0x0064,//  [d] [d]  LATIN SMALL LETTER D
0x0065,//  [e] [e]  LATIN SMALL LETTER E
0x0066,//  [f] [f]  LATIN SMALL LETTER F
0x0067,//  [g] [g]  LATIN SMALL LETTER G
0x0068,//  [h] [h]  LATIN SMALL LETTER H
0x0069,//  [i] [i]  LATIN SMALL LETTER I
0x006A,//  [j] [j]  LATIN SMALL LETTER J
0x006B,//  [k] [k]  LATIN SMALL LETTER K
0x006C,//  [l] [l]  LATIN SMALL LETTER L
0x006D,//  [m] [m]  LATIN SMALL LETTER M
0x006E,//  [n] [n]  LATIN SMALL LETTER N
0x006F,//  [o] [o]  LATIN SMALL LETTER O
0x0070,//  [p] [p]  LATIN SMALL LETTER P
0x0071,//  [q] [q]  LATIN SMALL LETTER Q
0x0072,//  [r] [r]  LATIN SMALL LETTER R
0x0073,//  [s] [s]  LATIN SMALL LETTER S
0x0074,//  [t] [t]  LATIN SMALL LETTER T
0x0075,//  [u] [u]  LATIN SMALL LETTER U
0x0076,//  [v] [v]  LATIN SMALL LETTER V
0x0077,//  [w] [w]  LATIN SMALL LETTER W
0x0078,//  [x] [x]  LATIN SMALL LETTER X
0x0079,//  [y] [y]  LATIN SMALL LETTER Y
0x007A,//  [z] [z]  LATIN SMALL LETTER Z
0x007B,//  [{] [{]  LEFT CURLY BRACKET
0x007C,//  [|] [|]  VERTICAL LINE
0x007D,//  [}] [}]  RIGHT CURLY BRACKET
0x007E,//  [~] [~]  TILDE
0x007F,//  [ ] [ ]  DELETE
0x0402,//  [?] [?]  CYRILLIC CAPITAL LETTER DJE
0x0403,//  [?] [?]  CYRILLIC CAPITAL LETTER GJE
0x201A,//  [‚] [‚]  SINGLE LOW-9 QUOTATION MARK
0x0453,//  [?] [?]  CYRILLIC SMALL LETTER GJE
0x201E,//  [„] [„]  DOUBLE LOW-9 QUOTATION MARK
0x2026,//  […] […]  HORIZONTAL ELLIPSIS
0x2020,//  [†] [†]  DAGGER
0x2021,//  [‡] [‡]  DOUBLE DAGGER
0x20AC,//  [€] [€]  EURO SIGN
0x2030,//  [‰] [‰]  PER MILLE SIGN
0x0409,//  [?] [?]  CYRILLIC CAPITAL LETTER LJE
0x2039,//  [‹] [‹]  SINGLE LEFT-POINTING ANGLE QUOTATION MARK
0x040A,//  [?] [?]  CYRILLIC CAPITAL LETTER NJE
0x040C,//  [?] [?]  CYRILLIC CAPITAL LETTER KJE
0x040B,//  [?] [?]  CYRILLIC CAPITAL LETTER TSHE
0x040F,//  [?] [?]  CYRILLIC CAPITAL LETTER DZHE
0x0452,//  [?] [?]  CYRILLIC SMALL LETTER DJE
0x2018,//  [‘] [‘]  LEFT SINGLE QUOTATION MARK
0x2019,//  [’] [’]  RIGHT SINGLE QUOTATION MARK
0x201C,//  [“] [“]  LEFT DOUBLE QUOTATION MARK
0x201D,//  [”] [”]  RIGHT DOUBLE QUOTATION MARK
0x2022,//  [•] [•]  BULLET
0x2013,//  [–] [–]  EN DASH
0x2014,//  [—] [—]  EM DASH
0x0020 ,//  [ ] [ ]  UNDEFINED
0x2122,//  [™] [™]  TRADE MARK SIGN
0x0459,//  [?] [?]  CYRILLIC SMALL LETTER LJE
0x203A,//  [›] [›]  SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
0x045A,//  [?] [?]  CYRILLIC SMALL LETTER NJE
0x045C,//  [?] [?]  CYRILLIC SMALL LETTER KJE
0x045B,//  [?] [?]  CYRILLIC SMALL LETTER TSHE
0x045F,//  [?] [?]  CYRILLIC SMALL LETTER DZHE
0x00A0,//  [ ] [ ]  NO-BREAK SPACE
0x040E,//  [?] [?]  CYRILLIC CAPITAL LETTER SHORT U
0x045E,//  [?] [?]  CYRILLIC SMALL LETTER SHORT U
0x0408,//  [?] [?]  CYRILLIC CAPITAL LETTER JE
0x00A4,//  [¤] [¤]  CURRENCY SIGN
0x0490,//  [?] [?]  CYRILLIC CAPITAL LETTER GHE WITH UPTURN
0x00A6,//  [¦] [¦]  BROKEN BAR
0x00A7,//  [§] [§]  SECTION SIGN
0x0401,//  [?] [?]  CYRILLIC CAPITAL LETTER IO
0x00A9,//  [©] [©]  COPYRIGHT SIGN
0x0404,//  [?] [?]  CYRILLIC CAPITAL LETTER UKRAINIAN IE
0x00AB,//  [«] [«]  LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
0x00AC,//  [¬] [¬]  NOT SIGN
0x00AD,//  [ ] [ ]  SOFT HYPHEN
0x00AE,//  [®] [®]  REGISTERED SIGN
0x0407,//  [?] [?]  CYRILLIC CAPITAL LETTER YI
0x00B0,//  [°] [°]  DEGREE SIGN
0x00B1,//  [±] [±]  PLUS-MINUS SIGN
0x0406,//  [?] [?]  CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
0x0456,//  [?] [?]  CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
0x0491,//  [?] [?]  CYRILLIC SMALL LETTER GHE WITH UPTURN
0x00B5,//  [µ] [µ]  MICRO SIGN
0x00B6,//  [¶] [¶]  PILCROW SIGN
0x00B7,//  [·] [·]  MIDDLE DOT
0x0451,//  [?] [?]  CYRILLIC SMALL LETTER IO
0x2116,//  [?] [?]  NUMERO SIGN
0x0454,//  [?] [?]  CYRILLIC SMALL LETTER UKRAINIAN IE
0x00BB,//  [»] [»]  RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
0x0458,//  [?] [?]  CYRILLIC SMALL LETTER JE
0x0405,//  [?] [?]  CYRILLIC CAPITAL LETTER DZE
0x0455,//  [?] [?]  CYRILLIC SMALL LETTER DZE
0x0457,//  [?] [?]  CYRILLIC SMALL LETTER YI
0x0410,//  [?] [?]  CYRILLIC CAPITAL LETTER A
0x0411,//  [?] [?]  CYRILLIC CAPITAL LETTER BE
0x0412,//  [?] [?]  CYRILLIC CAPITAL LETTER VE
0x0413,//  [?] [?]  CYRILLIC CAPITAL LETTER GHE
0x0414,//  [?] [?]  CYRILLIC CAPITAL LETTER DE
0x0415,//  [?] [?]  CYRILLIC CAPITAL LETTER IE
0x0416,//  [?] [?]  CYRILLIC CAPITAL LETTER ZHE
0x0417,//  [?] [?]  CYRILLIC CAPITAL LETTER ZE
0x0418,//  [?] [?]  CYRILLIC CAPITAL LETTER I
0x0419,//  [?] [?]  CYRILLIC CAPITAL LETTER SHORT I
0x041A,//  [?] [?]  CYRILLIC CAPITAL LETTER KA
0x041B,//  [?] [?]  CYRILLIC CAPITAL LETTER EL
0x041C,//  [?] [?]  CYRILLIC CAPITAL LETTER EM
0x041D,//  [?] [?]  CYRILLIC CAPITAL LETTER EN
0x041E,//  [?] [?]  CYRILLIC CAPITAL LETTER O
0x041F,//  [?] [?]  CYRILLIC CAPITAL LETTER PE
0x0420,//  [?] [?]  CYRILLIC CAPITAL LETTER ER
0x0421,//  [?] [?]  CYRILLIC CAPITAL LETTER ES
0x0422,//  [?] [?]  CYRILLIC CAPITAL LETTER TE
0x0423,//  [?] [?]  CYRILLIC CAPITAL LETTER U
0x0424,//  [?] [?]  CYRILLIC CAPITAL LETTER EF
0x0425,//  [?] [?]  CYRILLIC CAPITAL LETTER HA
0x0426,//  [?] [?]  CYRILLIC CAPITAL LETTER TSE
0x0427,//  [?] [?]  CYRILLIC CAPITAL LETTER CHE
0x0428,//  [?] [?]  CYRILLIC CAPITAL LETTER SHA
0x0429,//  [?] [?]  CYRILLIC CAPITAL LETTER SHCHA
0x042A,//  [?] [?]  CYRILLIC CAPITAL LETTER HARD SIGN
0x042B,//  [?] [?]  CYRILLIC CAPITAL LETTER YERU
0x042C,//  [?] [?]  CYRILLIC CAPITAL LETTER SOFT SIGN
0x042D,//  [?] [?]  CYRILLIC CAPITAL LETTER E
0x042E,//  [?] [?]  CYRILLIC CAPITAL LETTER YU
0x042F,//  [?] [?]  CYRILLIC CAPITAL LETTER YA
0x0430,//  [?] [?]  CYRILLIC SMALL LETTER A
0x0431,//  [?] [?]  CYRILLIC SMALL LETTER BE
0x0432,//  [?] [?]  CYRILLIC SMALL LETTER VE
0x0433,//  [?] [?]  CYRILLIC SMALL LETTER GHE
0x0434,//  [?] [?]  CYRILLIC SMALL LETTER DE
0x0435,//  [?] [?]  CYRILLIC SMALL LETTER IE
0x0436,//  [?] [?]  CYRILLIC SMALL LETTER ZHE
0x0437,//  [?] [?]  CYRILLIC SMALL LETTER ZE
0x0438,//  [?] [?]  CYRILLIC SMALL LETTER I
0x0439,//  [?] [?]  CYRILLIC SMALL LETTER SHORT I
0x043A,//  [?] [?]  CYRILLIC SMALL LETTER KA
0x043B,//  [?] [?]  CYRILLIC SMALL LETTER EL
0x043C,//  [?] [?]  CYRILLIC SMALL LETTER EM
0x043D,//  [?] [?]  CYRILLIC SMALL LETTER EN
0x043E,//  [?] [?]  CYRILLIC SMALL LETTER O
0x043F,//  [?] [?]  CYRILLIC SMALL LETTER PE
0x0440,//  [?] [?]  CYRILLIC SMALL LETTER ER
0x0441,//  [?] [?]  CYRILLIC SMALL LETTER ES
0x0442,//  [?] [?]  CYRILLIC SMALL LETTER TE
0x0443,//  [?] [?]  CYRILLIC SMALL LETTER U
0x0444,//  [?] [?]  CYRILLIC SMALL LETTER EF
0x0445,//  [?] [?]  CYRILLIC SMALL LETTER HA
0x0446,//  [?] [?]  CYRILLIC SMALL LETTER TSE
0x0447,//  [?] [?]  CYRILLIC SMALL LETTER CHE
0x0448,//  [?] [?]  CYRILLIC SMALL LETTER SHA
0x0449,//  [?] [?]  CYRILLIC SMALL LETTER SHCHA
0x044A,//  [?] [?]  CYRILLIC SMALL LETTER HARD SIGN
0x044B,//  [?] [?]  CYRILLIC SMALL LETTER YERU
0x044C,//  [?] [?]  CYRILLIC SMALL LETTER SOFT SIGN
0x044D,//  [?] [?]  CYRILLIC SMALL LETTER E
0x044E,//  [?] [?]  CYRILLIC SMALL LETTER YU
0x044F,//  [?] [?]  CYRILLIC SMALL LETTER YA
};

#pragma pack()
END_TERIMBER_NAMESPACE
