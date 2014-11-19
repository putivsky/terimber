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
#ifndef _terimber_fuzzyphonetic_hpp_
#define _terimber_fuzzyphonetic_hpp_

#include "fuzzyphonetic.h"
#include "base/common.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

namespace fuzzyphonetic
{
// 1. metaphonetic codes
// TH B  X  S  K  J  T  F  H  L  M  N  P  R  W  Y
// 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15

// Margarita Poutivskaia' copyright below
// metaphoneic codes as a music notes sequence
const ub1_t H_NOTE		= 0;
const ub1_t TH_NOTE		= 1;
const ub1_t F_NOTE		= 2;
const ub1_t P_NOTE		= 3;
const ub1_t T_NOTE		= 4;
const ub1_t K_NOTE		= 5;
const ub1_t S_NOTE		= 6;
const ub1_t X_NOTE		= 7;
const ub1_t B_NOTE		= 8;
const ub1_t N_NOTE		= 9;
const ub1_t L_NOTE		= 10;
const ub1_t M_NOTE		= 11;
const ub1_t W_NOTE		= 12;
const ub1_t Y_NOTE		= 13;
const ub1_t R_NOTE		= 14;
const ub1_t J_NOTE		= 15;
const ub1_t A_NOTE		= 16;


const ub1_t legal_mask = 0x10;
const ub1_t metaphoneic_mask = 0x0F;
const ub1_t lower_mask = 0x20;
const ub1_t vowel_mask = 0x40; // A, E, I, O, U, Y

static const ub1_t metaphoneic_rules[256] =
{
// 0-15
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 16-31
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 32-47	
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 48-63
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 64-79       A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
			0, 
				legal_mask + vowel_mask, // A
				legal_mask, // B
				legal_mask, // C
				legal_mask, // D
				legal_mask + vowel_mask, // E
				legal_mask, // F
				legal_mask, // G
				legal_mask, // H
				legal_mask + vowel_mask, // I
				legal_mask, // J
				legal_mask, // K
				legal_mask, // L
				legal_mask, // M
				legal_mask, // N
				legal_mask + vowel_mask, // O
// 80-95    P   Q   R   S   T   U   V   W   X   Y   Z
				legal_mask, // P 
				legal_mask, // Q
				legal_mask, // R
				legal_mask, // S
				legal_mask, // T
				legal_mask + vowel_mask, // U
				legal_mask, // V
				legal_mask, // W
				legal_mask, // X
				legal_mask + vowel_mask, // Y
				legal_mask, // Z
				0, 0, 0, 0, 0,
// 96-111      a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
			0, 
				legal_mask + vowel_mask + lower_mask, // a
				legal_mask + lower_mask, // b
				legal_mask + lower_mask, // c
				legal_mask + lower_mask, // d
				legal_mask + vowel_mask + lower_mask, // e
				legal_mask + lower_mask, // f
				legal_mask + lower_mask, // g
				legal_mask + lower_mask, // h
				legal_mask + vowel_mask + lower_mask, // i
				legal_mask + lower_mask, // j
				legal_mask + lower_mask, // k
				legal_mask + lower_mask, // l
				legal_mask + lower_mask, // m
				legal_mask + lower_mask, // n
				legal_mask + vowel_mask + lower_mask, // o
// 112-127  p   q   r   s   t   u   v   w   x   y   z
				legal_mask + lower_mask, // p 
				legal_mask + lower_mask, // q
				legal_mask + lower_mask, // r
				legal_mask + lower_mask, // s
				legal_mask + lower_mask, // t
				legal_mask + vowel_mask + lower_mask, // u
				legal_mask + lower_mask, // v
				legal_mask + lower_mask, // w
				legal_mask + lower_mask, // x
				legal_mask + vowel_mask + lower_mask, // y
				legal_mask + lower_mask, // z
				0, 0, 0, 0, 0,
// 128-143
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 144-159
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 160-175
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 176-191
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 192-207
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 208-223
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 224-239
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
// 240-255
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// set of mask to get information

// lowest 4 bits

// 2. legal char - 5th bit = 1 + 16

// 3. lower case - 6th bit = 1 + 32

// 4. vowel - 7th bit = 1 + 64 
 

inline bool is_legal_char(char ch) { return (metaphoneic_rules[(unsigned char)ch] & legal_mask) != 0; }
inline char to_upper(char ch) { return (metaphoneic_rules[(unsigned char)ch] & lower_mask) ? ch - 0x20 : ((metaphoneic_rules[(unsigned char)ch] & legal_mask)? ch : 0); }
inline bool is_vowel(char ch) { return (metaphoneic_rules[(unsigned char)ch] & vowel_mask) != 0; }
inline char to_lower(char ch) { return (metaphoneic_rules[(unsigned char)ch] & lower_mask) ? ch : ((metaphoneic_rules[(unsigned char)ch] & legal_mask) ? ch + 0x20 : ch); }


inline bool check_sub_string(const char* word, const char* pend, const char* ethalon, size_t len)
{
	if (!word)
		return false;

	while (word != pend && len-- && *ethalon)
	{
		if (to_upper(*word) != *ethalon)
			return false;

		++word, ++ethalon;
	}

	return word != pend;
}

inline bool is_slavo_germanic(const char* word, size_t len)
{
	while (len-- && *word)
	{
		switch (to_upper(*word))
		{
			case 'W':
				return true;
			case 'K':
				return true;
			case 'C':
				if (to_upper(*(word + 1)) == 'Z')
					return true;
				break;
		}
	
		++word;
	}

	return false;
}

inline
size_t 
find_metaphone_distance(const metaphone_key& x, const metaphone_key& y, byte_allocator& tmp, size_t max_penalty)
{
	return metaphone_distance((const char*)x._array, x._length, (const char*)y._array, y._length, tmp, max_penalty);
}


inline
size_t find_reflection_distance(const reflection_key& x, const reflection_key& y, byte_allocator& tmp, size_t max_penalty)
{
	return metaphone_distance((const char*)x._array, reflection_key::REFSIZE, (const char*)y._array, reflection_key::REFSIZE, tmp, max_penalty);
}

inline
size_t find_word_distance(const char* x, size_t xlen, const char* y, size_t ylen, byte_allocator& tmp, size_t max_penalty)
{
	return metaphone_distance(x, xlen, y, ylen, tmp, max_penalty);
}

template< class T >
inline
size_t
metaphone_distance(const T* ax, size_t x, const T* ay, size_t y, byte_allocator& tmp, size_t max_penalty)
{
	const size_t dim = sizeof(size_t) * (x + 1) * (y + 1);
	size_t* matrix = (size_t*)tmp.allocate(dim);
	if (!matrix)
		return 0;
	// it's supposed that matrix is a array of int_t at least (x + 1) * (y + 1) length
	memset(matrix, 0, dim);
	// set rows, and columns
	size_t row, col;
	for (row = 0; row <= x; ++row)
		matrix[row * (y + 1)] = row;
	for (col = 1; col <= y; ++col)
		matrix[col] = col;

	for (row = 1; row <= x; ++row)
	{
		register T x_row = ax[row - 1];
		for (col = 1; col <= y; ++col)
		{
			register T y_col = ay[col - 1];

			size_t cost;
			if (x_row == y_col)
			{
				cost = 0;
			}
			else
			{
				cost = 1;
			}

			register size_t above = matrix[(row - 1) * (y + 1) + col];
			register size_t left = matrix[row * (y + 1) + col - 1];
			register size_t diag = matrix[(row - 1) * (y + 1) + col - 1];
			register size_t cell = __min(above + 1, __min(left + 1, diag + cost));

			if (row > 2 && col > 2)
			{
				register size_t trans = matrix[(row - 2) * (y + 1) + col - 2] + 1;
				if (ax[row - 2] != y_col) 
					++trans;
				if (x_row != ay[col - 2])
					++trans;
				if (cell > trans)
					cell = trans;
			}

			matrix[row * (y + 1) + col] = cell;

			//if (cell >= max_penalty)
			//	return max_penalty;
		}
	}

	return matrix[x * (y + 1) + y];
}

inline
size_t 
find_key_distance(const metaphone_key& x, const metaphone_key& y, byte_allocator& tmp)
{
	return metaphone_distance((const size_t*)x._array, x._length, (const size_t*)y._array, y._length, tmp, 0xffffffff);
}

}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // 
