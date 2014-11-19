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

#ifndef _terimber_arithmetic_hpp_
#define _terimber_arithmetic_hpp_

#include "crypt/arithmet.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

inline ub8_t make_ub8(ub4_t low_word, ub4_t high_word)
{
	return (ub8_t(high_word) << word_bits) | low_word;
}

inline ub4_t get_low_ub4(ub8_t x)
{
	return (ub4_t)(x & 0x00000000ffffffff);
}

inline ub4_t get_high_ub4(ub8_t x)
{
	return (ub4_t)((x >> 32) & 0x00000000ffffffff);
}



namespace room_arithmetic
{
inline 
ub4_t 
count_words(const ub4_t* x, ub4_t n)
{ 
	while (n && x[n-1] == 0) 
		--n; 
	return n; 
}

inline 
void 
set_words(ub4_t* r, ub4_t a, ub4_t n)
{ 
	for (ub4_t i = 0; i < n; ++i) 
		r[i] = a; 
}

inline
void
copy_words(ub4_t* r, const ub4_t *a, ub4_t n)
{ 
	for (ub4_t i = 0; i < n; ++i) 
		r[i] = a[i]; 
}

inline 
void 
xor_words(ub4_t* r, const ub4_t* a, const ub4_t* b, ub4_t n)
{ 
	for (ub4_t i = 0; i < n; ++i) 
		r[i] = a[i] ^ b[i]; 
}

inline 
void 
xor_words(ub4_t* r, const ub4_t* a, ub4_t n)
{ 
	for (ub4_t i = 0; i < n; ++i) 
		r[i] ^= a[i]; 
}

inline 
void 
and_words(ub4_t* r, const ub4_t* a, const ub4_t* b, ub4_t n)
{ 
	for (ub4_t i = 0; i < n; ++i) 
		r[i] = a[i] & b[i]; 
}

inline 
void 
and_words(ub4_t* r, const ub4_t* a, ub4_t n)
{ 
	for (ub4_t i = 0; i < n; ++i) 
		r[i] &= a[i]; 
}

inline 
ub4_t 
shl_bits(ub4_t* r, ub4_t n, ub4_t shift)
{
	assert(shift < word_bits);
	ub4_t u, carry=0;
	if (shift)
		for (ub4_t i = 0; i < n; ++i)
		{
			u = r[i];
			r[i] = (u << shift) | carry;
			carry = u >> (word_bits - shift);
		}
	return carry;
}

inline 
ub4_t 
shr_bits(ub4_t* r, ub4_t n, ub4_t shift)
{
	assert(shift < word_bits);
	ub4_t u, carry=0;
	if (shift)
	{
		for (int i = n - 1; i >= 0; --i)
		{
			u = r[i];
			r[i] = (u >> shift) | carry;
			carry = u << (word_bits - shift);
		}
	}
	return carry;
}

inline 
void 
shl_words(ub4_t* r, ub4_t n, ub4_t shift)
{
	shift = __min(shift, n);
	if (shift)
	{
		for (ub4_t i = n - 1; i >= shift; --i)
			r[i] = r[i - shift];
		set_words(r, 0, shift);
	}
}

inline 
void 
shr_words(ub4_t* r, ub4_t n, ub4_t shift)
{
	shift = __min(shift, n);
	if (shift)
	{
		for (ub4_t i = 0; i + shift < n; ++i)
			r[i] = r[i + shift];
		set_words(r + n - shift, 0, shift);
	}
}

inline 
ub4_t 
parity(ub4_t n)
{
	for (ub4_t i = 8 * sizeof(n) / 2; i > 0; i /= 2)
		n ^= n >> i;
	return (ub4_t)n & 1;
}

inline 
ub4_t 
byte_precision(ub4_t n)
{
	ub4_t i;
	for (i = sizeof(n); i; --i)
		if (n >> (i - 1) * 8)
			break;

	return i;
}

inline 
ub4_t 
bit_precision(ub4_t n)
{
	if (!n)
		return 0;

	ub4_t l = 0, h = 8 * sizeof(n);

	while (h - l > 1)
	{
		ub4_t t = (l + h)/2;
		if (n >> t)
			l = t;
		else
			h = t;
	}

	return h;
}

inline 
ub4_t 
crop(ub4_t n, ub4_t size)
{ 
	return (size < 8 * sizeof(n)) ? n & ((1L << size) - 1) : n; 
}

inline 
ub4_t 
bits_to_bytes(ub4_t bitCount)
{ 
	return (bitCount + 7)/8; 
}

inline
ub4_t
bytes_to_words(ub4_t byteCount)
{ 
	return (byteCount + word_size - 1)/word_size; 
}

inline
ub4_t
bits_to_words(ub4_t bitCount)
{ 
	return (bitCount + word_bits - 1)/word_bits; 
}

inline 
ub4_t 
add(ub4_t* r, const ub4_t* a, const ub4_t* b, ub4_t n)
{
	assert (n % 2 == 0);

	ub4_t carry = 0;
	for (ub4_t i = 0; i < n; i += 2)
	{
		ub8_t u = (ub8_t) carry + a[i] + b[i];
		r[i] = get_low_ub4(u);
		u = (ub8_t) get_high_ub4(u) + a[i+1] + b[i+1];
		r[i+1] = get_low_ub4(u);
		carry = get_high_ub4(u);
	}
	return carry;
}

inline 
ub4_t
subtract(ub4_t* r, const ub4_t* a, const ub4_t* b, ub4_t n)
{
	assert (n % 2 == 0);

	ub4_t borrow = 0;
	for (ub4_t i = 0; i < n; i += 2)
	{
		ub8_t u = (ub8_t)a[i] - b[i] - borrow;
		r[i] = get_low_ub4(u);
		u = (ub8_t)a[i+1] - b[i+1] - (ub8_t)(0 - get_high_ub4(u));
		r[i+1] = get_low_ub4(u);
		borrow = 0 - get_high_ub4(u);
	}
	return borrow;
}

inline 
int
compare(const ub4_t* a, const ub4_t* b, ub4_t n)
{
	while (n--)
		if (a[n] > b[n])
			return 1;
		else if (a[n] < b[n])
			return -1;

	return 0;
}

inline 
ub4_t 
increment(ub4_t* a, ub4_t n, ub4_t b)
{
	assert(n);
	ub4_t t = a[0];
	a[0] = t + b;
	if (a[0] >= t)
		return 0;
	for (ub4_t i = 1; i < n; ++i)
		if (++a[i])
			return 0;
	return 1;
}

inline 
ub4_t 
decrement(ub4_t* a, ub4_t n, ub4_t b)
{
	assert(n);
	ub4_t t = a[0];
	a[0] = t - b;
	if (a[0] <= t)
		return 0;
	for (ub4_t i = 1; i < n; ++i)
		if (a[i]--)
			return 0;
	return 1;
}

inline
void
twos_complement(ub4_t* a, ub4_t n)
{
	decrement(a, n);
	for (ub4_t i = 0; i < n; ++i)
		a[i] = ~a[i];
}

inline
ub4_t 
linear_multiply(ub4_t* r, const ub4_t* a, ub4_t b, ub4_t n)
{
	ub4_t carry = 0;
	for(ub4_t i = 0; i < n; ++i)
	{
		ub8_t p = (ub8_t)a[i] * b + carry;
		r[i] = get_low_ub4(p);
		carry = get_high_ub4(p);
	}
	return carry;
}

inline
void 
atomic_multiply(ub4_t* r, const ub4_t* a, const ub4_t* b)
{
	// this segment is the branchless equivalent of above
	ub4_t d[4] = {a[1] - a[0], a[0] - a[1], b[0] - b[1], b[1] - b[0]};
	ub4_t ai = a[1] < a[0];
	ub4_t bi = b[0] < b[1];
	ub4_t di = ai & bi;
	ub8_t ddi = (ub8_t)d[di] * d[di + 2];
	d[1] = d[3] = 0;
	ub4_t si = ai + !bi;
	ub4_t s = d[si];

	ub8_t a0b0 = (ub8_t)a[0] * b[0];
	r[0] = get_low_ub4(a0b0);

	ub8_t a1b1 = (ub8_t)a[1] * b[1];
	ub8_t t = (ub8_t) get_high_ub4(a0b0) + get_low_ub4(a0b0) + get_low_ub4(ddi) + get_low_ub4(a1b1);
	r[1] = get_low_ub4(t);

	t = a1b1 + get_high_ub4(t) + get_high_ub4(a0b0) + get_high_ub4(ddi) + get_high_ub4(a1b1) - s;
	r[2] = get_low_ub4(t);
	r[3] = get_high_ub4(t);
}

inline 
ub4_t 
atomic_multiply_add(ub4_t* r, const ub4_t* a, const ub4_t* b)
{
	ub4_t D[4] = {a[1]-a[0], a[0]-a[1], b[0]-b[1], b[1]-b[0]};
	ub4_t ai = a[1] < a[0];
	ub4_t bi = b[0] < b[1];
	ub4_t di = ai & bi;
	ub8_t d = (ub8_t)D[di]*D[di+2];
	D[1] = D[3] = 0;
	ub4_t si = ai + !bi;
	ub4_t s = D[si];

	ub8_t a0b0 = (ub8_t)a[0]*b[0];
	ub8_t t = a0b0 + r[0];
	r[0] = get_low_ub4(t);

	ub8_t a1b1 = (ub8_t)a[1]*b[1];
	t = (ub8_t) get_high_ub4(t) + get_low_ub4(a0b0) + get_low_ub4(d) + get_low_ub4(a1b1) + r[1];
	r[1] = get_low_ub4(t);

	t = (ub8_t) get_high_ub4(t) + get_low_ub4(a1b1) + get_high_ub4(a0b0) + get_high_ub4(d) + get_high_ub4(a1b1) - s + r[2];
	r[2] = get_low_ub4(t);

	t = (ub8_t) get_high_ub4(t) + get_high_ub4(a1b1) + r[3];
	r[3] = get_low_ub4(t);
	return get_high_ub4(t);
}

inline 
void 
atomic_multiply_bottom(ub4_t* r, const ub4_t* a, const ub4_t* b)
{
	ub8_t t = (ub8_t)a[0]*b[0];
	r[0] = get_low_ub4(t);
	r[1] = get_high_ub4(t) + a[0]*b[1] + a[1]*b[0];
}

inline
void
mul_acc(ub4_t a, ub4_t b, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e)
{
    p = (ub8_t)a * b + c;
	c = get_low_ub4(p);
	p = (ub8_t)d + get_high_ub4(p);
	d = get_low_ub4(p);
	e += get_high_ub4(p);
}

inline
void
save_mul_acc(ub4_t& r, ub4_t a, ub4_t b, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e)
{
	r = c;
	p = (ub8_t)a * b + d;
	c = get_low_ub4(p);
	p = (ub8_t)e + get_high_ub4(p);
	d = get_low_ub4(p);
	e = get_high_ub4(p);
}

inline 
void
mul_acc1(ub4_t a, ub4_t aa, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e)
{
    p = (ub8_t)a * aa + c;
	c = get_low_ub4(p);
	p = (ub8_t)d + get_high_ub4(p);
	d = get_low_ub4(p);
	e += get_high_ub4(p);
}

inline
void
save_mul_acc1(ub4_t& r, ub4_t a, ub4_t aa,ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e)
{
	r = c;
	p = (ub8_t)a * aa + d;
	c = get_low_ub4(p);
	p = (ub8_t)e + get_high_ub4(p);
	d = get_low_ub4(p);
	e = get_high_ub4(p);
}

inline 
void
squ_acc(ub4_t a, ub4_t aa, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e)
{
	p = (ub8_t)a * aa;
	p = p + p + c;
	c = get_low_ub4(p);
	p = (ub8_t)d + get_high_ub4(p);
	d = get_low_ub4(p);
	e += get_high_ub4(p);
}

inline 
void
aave_squ_acc(ub4_t& r, ub4_t a, ub4_t aa, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e)
{
	r = c;
	p = (ub8_t)a * aa;
	p = p + p + d;
	c = get_low_ub4(p);
	p = (ub8_t)e + get_high_ub4(p);
	d = get_low_ub4(p);
	e = get_high_ub4(p);
}

inline 
void 
comba_square4(ub4_t* r, const ub4_t* a)
{
	ub8_t p = (ub8_t)a[0] * a[0];
	r[0] = get_low_ub4(p);
	ub4_t c = get_high_ub4(p);

	ub4_t d = 0, e = 0;
	squ_acc(a[0], a[1], p, c, d, e);

	aave_squ_acc(r[1], a[2], a[0], p, c, d, e);
	mul_acc1(a[1], a[1], p, c, d, e);

	aave_squ_acc(r[2], a[0], a[3], p, c, d, e);
	squ_acc(a[1], a[2], p, c, d, e);

	aave_squ_acc(r[3], a[3], a[1], p, c, d, e);
	mul_acc1(a[2], a[2], p, c, d, e);

	aave_squ_acc(r[4], a[2], a[3], p, c, d, e);

	r[5] = c;
	p = (ub8_t)a[3] * a[3] + d;
	r[6] = get_low_ub4(p);
	r[7] = e + get_high_ub4(p);
}

inline 
void 
comba_multiply4(ub4_t* r, const ub4_t* a, const ub4_t* b)
{
	ub8_t p = (ub8_t)a[0] * b[0];
	r[0] = get_low_ub4(p);
	ub4_t c = get_high_ub4(p);

	ub4_t d = 0, e = 0;

	mul_acc(a[0], b[1], p, c, d, e);
	mul_acc(a[1], b[0], p, c, d, e);

	save_mul_acc(r[1], a[2], b[0], p, c, d, e);
	mul_acc(a[1], b[1], p, c, d, e);
	mul_acc(a[0], b[2], p, c, d, e);

	save_mul_acc(r[2], a[0], b[3], p, c, d, e);
	mul_acc(a[1], b[2], p, c, d, e);
	mul_acc(a[2], b[1], p, c, d, e);
	mul_acc(a[3], b[0], p, c, d, e);

	save_mul_acc(r[3], a[3], b[1], p, c, d, e);
	mul_acc(a[2], b[2], p, c, d, e);
	mul_acc(a[1], b[3], p, c, d, e);

	save_mul_acc(r[4], a[2], b[3], p, c, d, e);
	mul_acc(a[3], b[2], p, c, d, e);

	r[5] = c;
	p = (ub8_t)a[3] * b[3] + d;
	r[6] = get_low_ub4(p);
	r[7] = e + get_high_ub4(p);
}

inline 
void 
comba_multiply8(ub4_t* r, const ub4_t* a, const ub4_t* b)
{
	ub8_t p = (ub8_t)a[0] * b[0];
	r[0] = get_low_ub4(p);
	ub4_t c = get_high_ub4(p);

	ub4_t d = 0, e = 0;
	mul_acc(a[0], b[1], p, c, d, e);
	mul_acc(a[1], b[0], p, c, d, e);

	save_mul_acc(r[1], a[2], b[0], p, c, d, e);
	mul_acc(a[1], b[1], p, c, d, e);
	mul_acc(a[0], b[2], p, c, d, e);

	save_mul_acc(r[2], a[0], b[3], p, c, d, e);
	mul_acc(a[1], b[2], p, c, d, e);
	mul_acc(a[2], b[1], p, c, d, e);
	mul_acc(a[3], b[0], p, c, d, e);

	save_mul_acc(r[3], a[0], b[4], p, c, d, e);
	mul_acc(a[1], b[3], p, c, d, e);
	mul_acc(a[2], b[2], p, c, d, e);
	mul_acc(a[3], b[1], p, c, d, e);
	mul_acc(a[4], b[0], p, c, d, e);

	save_mul_acc(r[4], a[0], b[5], p, c, d, e);
	mul_acc(a[1], b[4], p, c, d, e);
	mul_acc(a[2], b[3], p, c, d, e);
	mul_acc(a[3], b[2], p, c, d, e);
	mul_acc(a[4], b[1], p, c, d, e);
	mul_acc(a[5], b[0], p, c, d, e);

	save_mul_acc(r[5], a[0], b[6], p, c, d, e);
	mul_acc(a[1], b[5], p, c, d, e);
	mul_acc(a[2], b[4], p, c, d, e);
	mul_acc(a[3], b[3], p, c, d, e);
	mul_acc(a[4], b[2], p, c, d, e);
	mul_acc(a[5], b[1], p, c, d, e);
	mul_acc(a[6], b[0], p, c, d, e);

	save_mul_acc(r[6], a[0], b[7], p, c, d, e);
	mul_acc(a[1], b[6], p, c, d, e);
	mul_acc(a[2], b[5], p, c, d, e);
	mul_acc(a[3], b[4], p, c, d, e);
	mul_acc(a[4], b[3], p, c, d, e);
	mul_acc(a[5], b[2], p, c, d, e);
	mul_acc(a[6], b[1], p, c, d, e);
	mul_acc(a[7], b[0], p, c, d, e);

	save_mul_acc(r[7], a[1], b[7], p, c, d, e);
	mul_acc(a[2], b[6], p, c, d, e);
	mul_acc(a[3], b[5], p, c, d, e);
	mul_acc(a[4], b[4], p, c, d, e);
	mul_acc(a[5], b[3], p, c, d, e);
	mul_acc(a[6], b[2], p, c, d, e);
	mul_acc(a[7], b[1], p, c, d, e);

	save_mul_acc(r[8], a[2], b[7], p, c, d, e);
	mul_acc(a[3], b[6], p, c, d, e);
	mul_acc(a[4], b[5], p, c, d, e);
	mul_acc(a[5], b[4], p, c, d, e);
	mul_acc(a[6], b[3], p, c, d, e);
	mul_acc(a[7], b[2], p, c, d, e);

	save_mul_acc(r[9], a[3], b[7], p, c, d, e);
	mul_acc(a[4], b[6], p, c, d, e);
	mul_acc(a[5], b[5], p, c, d, e);
	mul_acc(a[6], b[4], p, c, d, e);
	mul_acc(a[7], b[3], p, c, d, e);

	save_mul_acc(r[10], a[4], b[7], p, c, d, e);
	mul_acc(a[5], b[6], p, c, d, e);
	mul_acc(a[6], b[5], p, c, d, e);
	mul_acc(a[7], b[4], p, c, d, e);

	save_mul_acc(r[11], a[5], b[7], p, c, d, e);
	mul_acc(a[6], b[6], p, c, d, e);
	mul_acc(a[7], b[5], p, c, d, e);

	save_mul_acc(r[12], a[6], b[7], p, c, d, e);
	mul_acc(a[7], b[6], p, c, d, e);

	r[13] = c;
	p = (ub8_t)a[7] * b[7] + d;
	r[14] = get_low_ub4(p);
	r[15] = e + get_high_ub4(p);
}

inline 
void 
comba_multiply_bottom4(ub4_t* r, const ub4_t* a, const ub4_t* b)
{
	ub8_t p = (ub8_t)a[0] * b[0];
	r[0] = get_low_ub4(p);
	ub4_t c = get_high_ub4(p);

	ub4_t d = 0, e = 0;
	mul_acc(a[0], b[1], p, c, d, e);
	mul_acc(a[1], b[0], p, c, d, e);

	save_mul_acc(r[1], a[2], b[0], p, c, d, e);
	mul_acc(a[1], b[1], p, c, d, e);
	mul_acc(a[0], b[2], p, c, d, e);

	r[2] = c;
	r[3] = d + a[0] * b[3] + a[1] * b[2] + a[2] * b[1] + a[3] * b[0];
}

inline 
void 
comba_multiply_bottom8(ub4_t* r, const ub4_t* a, const ub4_t* b)
{
	ub8_t p = (ub8_t)a[0] * b[0];
	r[0] = get_low_ub4(p);
	ub4_t c = get_high_ub4(p);

	ub4_t d = 0, e = 0;
	mul_acc(a[0], b[1], p, c, d, e);
	mul_acc(a[1], b[0], p, c, d, e);

	save_mul_acc(r[1], a[2], b[0], p, c, d, e);
	mul_acc(a[1], b[1], p, c, d, e);
	mul_acc(a[0], b[2], p, c, d, e);

	save_mul_acc(r[2], a[0], b[3], p, c, d, e);
	mul_acc(a[1], b[2], p, c, d, e);
	mul_acc(a[2], b[1], p, c, d, e);
	mul_acc(a[3], b[0], p, c, d, e);

	save_mul_acc(r[3], a[0], b[4], p, c, d, e);
	mul_acc(a[1], b[3], p, c, d, e);
	mul_acc(a[2], b[2], p, c, d, e);
	mul_acc(a[3], b[1], p, c, d, e);
	mul_acc(a[4], b[0], p, c, d, e);

	save_mul_acc(r[4], a[0], b[5], p, c, d, e);
	mul_acc(a[1], b[4], p, c, d, e);
	mul_acc(a[2], b[3], p, c, d, e);
	mul_acc(a[3], b[2], p, c, d, e);
	mul_acc(a[4], b[1], p, c, d, e);
	mul_acc(a[5], b[0], p, c, d, e);

	save_mul_acc(r[5], a[0], b[6], p, c, d, e);
	mul_acc(a[1], b[5], p, c, d, e);
	mul_acc(a[2], b[4], p, c, d, e);
	mul_acc(a[3], b[3], p, c, d, e);
	mul_acc(a[4], b[2], p, c, d, e);
	mul_acc(a[5], b[1], p, c, d, e);
	mul_acc(a[6], b[0], p, c, d, e);

	r[6] = c;
	r[7] = d + a[0] * b[7] + a[1] * b[6] + a[2] * b[5] + a[3] * b[4] +
				a[4] * b[3] + a[5] * b[2] + a[6] * b[1] + a[7] * b[0];
}

inline 
void 
atomic_inverse_mod_power2(ub4_t* r, ub4_t a, ub4_t b)
{
	assert(a % 2 == 1);

	ub8_t dw = make_ub8(a, b), rmd = a % 8;

	for (ub4_t i = 3; i < 2 * word_bits; i *= 2)
		rmd = rmd * (2 - rmd * dw);

	assert(rmd * dw == 1);

	r[0] = get_low_ub4(rmd);
	r[1] = get_high_ub4(rmd);
}

inline 
ub4_t 
even_word_count(const ub4_t* x, ub4_t n)
{
	while (n && x[n - 2] == 0 && x[n - 1] == 0)
		n -= 2;
	return n;
}

inline 
ub4_t 
roundup_size(ub4_t n)
{
	if (n <= 8)
		return roundup_size_table[n];
	else if (n <= 16)
		return 16;
	else if (n <= 32)
		return 32;
	else if (n <= 64)
		return 64;
	else 
		return 1U << bit_precision(n - 1);
}

}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_arithmetic_hpp_

