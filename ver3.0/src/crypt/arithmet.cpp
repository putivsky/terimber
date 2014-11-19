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

#include "crypt/arithmet.hpp"
#include "alg/algorith.hpp"



BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

ub4_t small_prime_table_size = 552;

namespace room_arithmetic
{

const ub4_t 
roundup_size_table[] = {2, 2, 2, 4, 4, 8, 8, 8, 8};


void 
build_prime_table()
{
	size_t p = small_prime_table[small_prime_table_size - 1];
	for (int i = small_prime_table_size; i < max_small_prime_table_size; ++i)
	{
		int j;
		do
		{
			p += 2;
			for (j = 1; j < 54; ++j)
				if (p % small_prime_table[j] == 0)
					break;
		} 
		while (j != 54);
		small_prime_table[i] = (ub4_t)p;
	}
	small_prime_table_size = max_small_prime_table_size;
	assert(small_prime_table[small_prime_table_size - 1] == last_small_prime);
}


// ********************************************************
// r[2*n] - result = a*b
// t[2*n] - temporary work space
// a[n] --- multiplier
// b[n] --- multiplicant

void 
recursive_multiply(ub4_t* r, ub4_t* t, const ub4_t* a, const ub4_t* b, ub4_t n)
{
	assert(n >= 2 && n % 2 == 0);

	if (n == 2)
		atomic_multiply(r, a, b);
	else if (n == 4)
		comba_multiply4(r, a, b);
	else if (n == 8)
		comba_multiply8(r, a, b);
	else
	{
		const ub4_t n2 = n / 2;
		int carry;

		int aComp = compare(a, a + n2, n2);
		int bComp = compare(b, b + n2, n2);

		switch (2*aComp + aComp + bComp)
		{
		case -4:
			subtract(r, a + n2, a, n2);
			subtract(r + n2, b, b + n2, n2);
			recursive_multiply(t, t + n, r, r + n2, n2);
			subtract(t + n2, t + n2, r, n2);
			carry = -1;
			break;
		case -2:
			subtract(r, a + n2, a, n2);
			subtract(r + n2, b, b + n2, n2);
			recursive_multiply(t, t + n, r, r + n2, n2);
			carry = 0;
			break;
		case 2:
			subtract(r, a, a + n2, n2);
			subtract(r + n2, b + n2, b, n2);
			recursive_multiply(t, t + n, r, r + n2, n2);
			carry = 0;
			break;
		case 4:
			subtract(r, a + n2, a, n2);
			subtract(r + n2, b, b + n2, n2);
			recursive_multiply(t, t + n, r, r + n2, n2);
			subtract(t + n2, t + n2, r + n2, n2);
			carry = -1;
			break;
		default:
			set_words(t, 0, n);
			carry = 0;
		}

		recursive_multiply(r, t + n, a, b, n2);
		recursive_multiply(r + n, t + n, a + n2, b + n2, n2);

		// now t[01] holds (a + n2 -a)*(b-(b+n2)), r[01] holds a*b, r[23] holds (a + n2)*(b + n2)

		carry += add(t, t, r, n);
		carry += add(t, t, r + n, n);
		carry += add(r + n2, r + n2, t, n);

		assert (carry >= 0 && carry <= 2);
		increment(r + n + n2, n2, carry);
	}
}

// r[2*n] - result = a*a
// t[2*n] - temporary work space
// a[n] --- number to be squared

void 
recursive_square(ub4_t* r, ub4_t* t, const ub4_t* a, ub4_t n)
{
	assert(n && n % 2 == 0);
 
	if (n == 2)
		atomic_multiply(r, a, a);
	else if (n == 4)
	{
		// VC60 workaround: MSVC 6.0 has an optimization bug that makes
		// (ub8_t)a*b where either a or b has been cast to a ub8_t before
		// very expensive. Revisit a comba_square4() function when this
		// bug is fixed.
		comba_multiply4(r, a, a);
	}
	else
	{
		const ub4_t n2 = n / 2;

		recursive_square(r, t + n, a, n2);
		recursive_square(r + n, t + n, a + n2, n2);
		recursive_multiply(t, t + n, a, a + n2, n2);

		ub4_t carry = add(r + n2, r + n2, t, n);
		carry += add(r + n2, r + n2, t, n);
		increment(r + n + n2, n2, carry);
	}
}

// r[n] - bottom half of a*b
// t[n] - temporary work space
// a[n] - multiplier
// b[n] - multiplicant

void 
recursive_multiply_bottom(ub4_t* r, ub4_t* t, const ub4_t* a, const ub4_t* b, ub4_t n)
{
	assert(n >= 2 && n % 2 == 0);

	if (n == 2)
		atomic_multiply_bottom(r, a, b);
	else if (n == 4)
		comba_multiply_bottom4(r, a, b);
	else if (n == 8)
		comba_multiply_bottom8(r, a, b);
	else
	{
		const ub4_t n2 = n / 2;

		recursive_multiply(r, t, a, b, n2);
		recursive_multiply_bottom(t, t + n2, a + n2, b, n2);
		add(r + n2, r + n2, t, n2);
		recursive_multiply_bottom(t, t + n2, a, b + n2, n2);
		add(r + n2, r + n2, t, n2);
	}
}

// r[n] --- upper half of a*b
// t[2*n] - temporary work space
// L[n] --- lower half of a*b
// a[n] --- multiplier
// b[n] --- multiplicant

void 
recursive_multiply_top(ub4_t* r, ub4_t* t, const ub4_t* L, const ub4_t* a, const ub4_t* b, ub4_t n)
{
	assert(n >= 2 && n % 2 == 0);

	if (n == 2)
	{
		atomic_multiply(t, a, b);
		((ub8_t *)r)[0] = ((ub8_t *)t)[1];
	}
	else if (n == 4)
	{
		comba_multiply4(t, a, b);
		((ub8_t *)r)[0] = ((ub8_t *)t)[2];
		((ub8_t *)r)[1] = ((ub8_t *)t)[3];
	}
	else
	{
		const ub4_t n2 = n / 2;
		int carry;

		int aComp = compare(a, a + n2, n2);
		int bComp = compare(b, b + n2, n2);

		switch (2*aComp + aComp + bComp)
		{
		case -4:
			subtract(r, a + n2, a, n2);
			subtract(r + n2, b, b + n2, n2);
			recursive_multiply(t, t + n, r, r + n2, n2);
			subtract(t + n2, t + n2, r, n2);
			carry = -1;
			break;
		case -2:
			subtract(r, a + n2, a, n2);
			subtract(r + n2, b, b + n2, n2);
			recursive_multiply(t, t + n, r, r + n2, n2);
			carry = 0;
			break;
		case 2:
			subtract(r, a, a + n2, n2);
			subtract(r + n2, b + n2, b, n2);
			recursive_multiply(t, t + n, r, r + n2, n2);
			carry = 0;
			break;
		case 4:
			subtract(r, a + n2, a, n2);
			subtract(r + n2, b, b + n2, n2);
			recursive_multiply(t, t + n, r, r + n2, n2);
			subtract(t + n2, t + n2, r + n2, n2);
			carry = -1;
			break;
		default:
			set_words(t, 0, n);
			carry = 0;
		}

		recursive_multiply(t + n, r, a + n2, b + n2, n2);

		// now t[01] holds ((a+n2)-a)*(b-(b+n2)), t[23] holds (a+n2)*(b+n2)

		copy_words(r, L + n2, n2);
		ub4_t c2 = subtract(r, r, L, n2);
		c2 += subtract(r, r, t, n2);
		ub4_t tc = (compare(r, t + n, n2) == -1);

		carry += tc;
		carry += increment(r, n2, c2 + tc);
		carry += add(r, r, t + n2, n2);
		carry += add(r, r, t + n + n2, n2);

		copy_words(r + n2, t + n + n2, n2);
		assert (carry >= 0 && carry <= 2);
		increment(r + n2, n2, carry);
	}
}

// r[na+nb] - result = a*b
// t[na+nb] - temporary work space
// a[na] ---- multiplier
// b[nb] ---- multiplicant

void 
asymmetric_multiply(ub4_t* r, ub4_t* t, const ub4_t* a, ub4_t na, const ub4_t* b, ub4_t nb)
{
	if (na == nb)
	{
		if (a == b)
			recursive_square(r, t, a, na);
		else
			recursive_multiply(r, t, a, b, na);

		return;
	}

	if (na > nb)
	{
		algorithm::swap(a, b);
		algorithm::swap(na, nb);
	}

	assert(nb % na == 0);
	assert((nb / na) % 2 == 0); 	// nb is an even multiple of na

	if (na == 2 && !a[1])
	{
		switch (a[0])
		{
		case 0:
			set_words(r, 0, nb + 2);
			return;
		case 1:
			copy_words(r, b, nb);
			r[nb] = r[nb + 1] = 0;
			return;
		default:
			r[nb] = linear_multiply(r, b, a[0], nb);
			r[nb+1] = 0;
			return;
		}
	}

	recursive_multiply(r, t, a, b, na);
	copy_words(t + 2 * na, r + na, na);

	ub4_t i;

	for (i = 2 * na; i < nb; i += 2 * na)
		recursive_multiply(t + na + i, t, a, b + i, na);
	for (i = na; i < nb; i += 2 * na)
		recursive_multiply(r + i, t, a, b + i, na);

	if (add(r + na, r + na, t + 2 * na, nb - na))
		increment(r + nb, na);
}

// r[n] ----- result = a inverse mod 2**(word_bits*n)
// t[3*n/2] - temporary work space
// a[n] ----- an odd number as input

void 
recursive_inverse_mod_power2(ub4_t* r, ub4_t* t, const ub4_t* a, ub4_t n)
{
	if (n == 2)
		atomic_inverse_mod_power2(r, a[0], a[1]);
	else
	{
		const ub4_t n2 = n / 2;
		recursive_inverse_mod_power2(r, t, a, n2);
		t[0] = 1;
		set_words(t + 1, 0, n2 - 1);
		recursive_multiply_top(r + n2, t + n2, t, r, a, n2);
		recursive_multiply_bottom(t, t + n2, r, a + n2, n2);
		add(t, r + n2, t, n2);
		twos_complement(t, n2);
		recursive_multiply_bottom(r + n2, t + n2, r, t, n2);
	}
}

// r[n] --- result = X/(2**(word_bits*n)) mod m
// t[3*n] - temporary work space
// X[2*n] - number to be reduced
// m[n] --- _modulus
// U[n] --- multiplicative inverse of m mod 2**(word_bits*n)

void 
montgomery_reduce(ub4_t* r, ub4_t* t, const ub4_t* X, const ub4_t* m, const ub4_t* U, ub4_t n)
{
	recursive_multiply_bottom(r, t, X, U, n);
	recursive_multiply_top(t, t + n, X, r, m, n);
	if (subtract(r, X + n, t, n))
	{
		ub4_t carry = add(r, r, m, n);
		assert(carry);
	}
}

// r[n] --- result = X/(2**(word_bits*n/2)) mod m
// t[2*n] - temporary work space
// X[2*n] - number to be reduced
// m[n] --- _modulus
// U[n/2] - multiplicative inverse of m mod 2**(word_bits*n/2)
// V[n] --- 2**(word_bits*3*n/2) mod m

void 
half_montgomery_reduce(ub4_t* r, ub4_t* t, const ub4_t* X, const ub4_t* m, const ub4_t* U, const ub4_t* V, ub4_t n)
{
	assert(n % 2 == 0 && n >= 4);

	const ub4_t n2 = n / 2;
	recursive_multiply(t, t + n, V, X + n + n2, n2);
	int c2 = add(t, t, X, n);
	recursive_multiply_bottom(t + n + n2, t + n, t, U, n2);
	recursive_multiply_top(t + n, r, t, t + n + n2, m, n2);
	c2 -= subtract(t + n, t + n2, t + n, n2);
	recursive_multiply(t, r, t + n + n2, m + n2, n2);
	c2 -= subtract(t, t + n, t, n2);
	int c3 = -(int)subtract(t + n2, X + n, t + n2, n2);
	recursive_multiply(r, t + n, V + n2, X + n + n2, n2);
	c3 += add(r, r, t, n);

	if (c2 > 0)
		c3 += increment(r + n2, n2);
	else if (c2<0)
		c3 -= decrement(r + n2, n2, -c2);

	assert(c3 >= -1 && c3 <= 1);
	if (c3 > 0)
		subtract(r, r, m, n);
	else if (c3 < 0)
		add(r, r, m, n);
}

// does a 3 ub4_t by 2 ub4_t divide, returns quotient and leaves remainder in a
ub4_t 
subatomic_divide(ub4_t* a, ub4_t b, ub4_t b1)
{
	// asserts {a[2],a[1]} < {B1,b}, so quotient can fit in a ub4_t
	assert(a[2] < b1 || (a[2] == b1 && a[1] < b));

	ub4_t q;

	// estimates the quotient: do a 2 ub4_t by 1 ub4_t divide
	if (b1+1 == 0)
		q = a[2];
	else
		q = ub4_t(make_ub8(a[1], a[2]) / (b1 + 1));

	// subtracts q*b from a
	ub8_t p = (ub8_t) b*q;
	ub8_t u = (ub8_t) a[0] - get_low_ub4(p);
	a[0] = get_low_ub4(u);
	u = (ub8_t) a[1] - get_high_ub4(p) - (ub8_t)(0 - get_high_ub4(u)) - (ub8_t)b1*q;
	a[1] = get_low_ub4(u);
	a[2] += get_high_ub4(u);

	// q <= actual quotient, so fix it
	while (a[2] || a[1] > b1 || (a[1] == b1 && a[0] >= b))
	{
		u = (ub8_t) a[0] - b;
		a[0] = get_low_ub4(u);
		u = (ub8_t) a[1] - b1 - (ub8_t)(0-get_high_ub4(u));
		a[1] = get_low_ub4(u);
		a[2] += get_high_ub4(u);
		q++;
		assert(q);	// shouldn't overflow
	}

	return q;
}

// does a 4 ub4_t by 2 ub4_t divide, returns 2 ub4_t quotient in Q0 and Q1
void 
atomic_divide(ub4_t* q, const ub4_t* a, const ub4_t* b)
{
	if (!b[0] && !b[1]) // if divisor is 0, we assume divisor==2**(2*word_bits)
	{
		q[0] = a[2];
		q[1] = a[3];
	}
	else
	{
		ub4_t t[4];
		t[0] = a[0]; t[1] = a[1]; t[2] = a[2]; t[3] = a[3];
		q[1] = subatomic_divide(t + 1, b[0], b[1]);
		q[0] = subatomic_divide(t, b[0], b[1]);

		// multiplies the quotient and the divisor and adds the remainder, makes sure it equals the dividend
		assert(!t[2] && !t[3] && (t[1] < b[1] || (t[1] == b[1] && t[0] < b[0])));
		ub4_t P[4];
		atomic_multiply(P, q, b);
		add(P, P, t, 4);
		assert(memcmp(P, a, 4 * word_size) == 0);
	}
}

// for use by divide(), corrects the underestimated quotient {Q1,Q0}
void 
correct_quotient_estimate(ub4_t* r, ub4_t* t, ub4_t* q, const ub4_t* b, ub4_t n)
{
	assert(n && n % 2 == 0);

	if (q[1])
	{
		t[n] = t[n + 1] = 0;
		ub4_t i;
		for (i = 0; i < n; i += 4)
			atomic_multiply(t + i, q, b + i);
		for (i = 2; i < n; i += 4)
			if (atomic_multiply_add(t + i, q, b + i))
				t[i + 5] += (++t[i + 4] == 0);
	}
	else
	{
		t[n] = linear_multiply(t, b, q[0], n);
		t[n + 1] = 0;
	}

	ub4_t borrow = subtract(r, r, t, n + 2);
	assert(!borrow && !r[n + 1]);

	while (r[n] || compare(r, b, n) >= 0)
	{
		r[n] -= subtract(r, r, b, n);
		q[1] += (++q[0] == 0);
		assert(q[0] || q[1]); // no overflow
	}
}

// r[nb] -------- remainder = a%b
// q[na-nb+2] --- quotient	= a/b
// t[na+2*nb+4] - temp work space
// a[na] -------- dividend
// b[nb] -------- divisor

void 
divide(ub4_t* r, ub4_t* q, ub4_t* t, const ub4_t* a, ub4_t na, const ub4_t* b, ub4_t nb)
{
	assert(na && nb && na % 2 == 0 && nb % 2 == 0);
	assert(b[nb - 1] || b[nb - 2]);
	assert(nb <= na);

	// sets up temporary work space
	ub4_t *const ta=t;
	ub4_t *const tb=t + na + 2;
	ub4_t *const TP=t + na + 2 + nb;

	// copies b into tb and normalize it so that tb has highest bit set to 1
	ub4_t shiftWords = (b[nb - 1]==0);
	tb[0] = tb[nb - 1] = 0;
	copy_words(tb + shiftWords, b, nb - shiftWords);
	ub4_t shiftBits = word_bits - bit_precision(tb[nb - 1]);
	assert(shiftBits < word_bits);
	shl_bits(tb, nb, shiftBits);

	// copies a into ta and normalize it
	ta[0] = ta[na] = ta[na + 1] = 0;
	copy_words(ta + shiftWords, a, na);
	shl_bits(ta, na + 2, shiftBits);

	if (ta[na + 1]==0 && ta[na] <= 1)
	{
		q[na - nb + 1] = q[na - nb] = 0;
		while (ta[na] || compare(ta + na - nb, tb, nb) >= 0)
		{
			ta[na] -= subtract(ta + na - nb, ta + na - nb, tb, nb);
			++q[na - nb];
		}
	}
	else
	{
		na+=2;
		assert(compare(ta + na - nb, tb, nb) < 0);
	}

	ub4_t bt[2];
	bt[0] = tb[nb - 2] + 1;
	bt[1] = tb[nb - 1] + (bt[0]==0);

	// starts reducing ta mod tb, 2 words at a time
	for (ub4_t i = na - 2; i >= nb; i -= 2)
	{
		atomic_divide(q + i - nb, ta + i - 2, bt);
		correct_quotient_estimate(ta + i - nb, TP, q + i - nb, tb, nb);
	}

	// copies ta into r, and denormalize it
	copy_words(r, ta + shiftWords, nb);
	shr_bits(r, nb, shiftBits);
}

// returns k
// r[n] --- result = a^(-1) * 2^k mod m
// t[4*n] - temporary work space
// a[na] -- number to take inverse of
// m[n] --- modulus

ub4_t 
almost_inverse(ub4_t* r, ub4_t* t, const ub4_t* a, ub4_t na, const ub4_t* m, ub4_t n)
{
	assert(na <= n && n && n % 2 == 0);

	ub4_t* b = t;
	ub4_t* c = t + n;
	ub4_t* f = t + 2 * n;
	ub4_t* g = t + 3 * n;
	ub4_t bcLen = 2, fgLen = even_word_count(m, n);
	ub4_t k = 0, s = 0;

	set_words(t, 0, 3 * n);
	b[0] = 1;
	copy_words(f, a, na);
	copy_words(g, m, n);

	while (1)
	{
		ub4_t t = f[0];
		while (!t)
		{
			if (even_word_count(f, fgLen) == 0)
			{
				set_words(r, 0, n);
				return 0;
			}

			shr_words(f, fgLen, 1);
			if (c[bcLen - 1]) bcLen += 2;
			assert(bcLen <= n);
			shl_words(c, bcLen, 1);
			k += word_bits;
			t = f[0];
		}

		ub4_t i = 0;
		while (t % 2 == 0)
		{
			t >>= 1;
			++i;
		}
		k += i;

		if (t == 1 && f[1]==0 && even_word_count(f, fgLen) == 2)
		{
			if (s % 2 == 0)
				copy_words(r, b, n);
			else
				subtract(r, m, b, n);
			return k;
		}

		shr_bits(f, fgLen, i);
		t = shl_bits(c, bcLen, i);
		if (t)
		{
			c[bcLen] = t;
			bcLen += 2;
			assert(bcLen <= n);
		}

		if (f[fgLen - 2] == 0 && g[fgLen - 2] == 0 && f[fgLen - 1] == 0 && g[fgLen - 1] == 0)
			fgLen-=2;

		if (compare(f, g, fgLen) == -1)
		{
			algorithm::swap(f, g);
			algorithm::swap(b, c);
			++s;
		}

		subtract(f, f, g, fgLen);

		if (add(b, b, c, bcLen))
		{
			b[bcLen] = 1;
			bcLen += 2;
			assert(bcLen <= n);
		}
	}
}

// r[n] - result = a/(2^k) mod m
// a[n] - input
// m[n] - modulus

void 
divide_power2_mod(ub4_t* r, const ub4_t* a, ub4_t k, const ub4_t* m, ub4_t n)
{
	copy_words(r, a, n);

	while (k--)
	{
		if (r[0] % 2 == 0)
			shr_bits(r, n, 1);
		else
		{
			ub4_t carry = add(r, r, m, n);
			shr_bits(r, n, 1);
			r[n - 1] += carry << (word_bits-1);
		}
	}
}

// r[n] - result = a*(2^k) mod m
// a[n] - input
// m[n] - _modulus

void 
multiply_power2_mod(ub4_t* r, const ub4_t* a, ub4_t k, const ub4_t* m, ub4_t n)
{
	copy_words(r, a, n);

	while (k--)
		if (shl_bits(r, n, 1) || compare(r, m, n) >= 0)
			subtract(r, r, m, n);
}

}
////////////////////////////////////////////////////////
ub4_t small_prime_table[max_small_prime_table_size] =
{
	2, 3, 5, 7, 11, 13, 17, 19,
	23, 29, 31, 37, 41, 43, 47, 53,
	59, 61, 67, 71, 73, 79, 83, 89,
	97, 101, 103, 107, 109, 113, 127, 131,
	137, 139, 149, 151, 157, 163, 167, 173,
	179, 181, 191, 193, 197, 199, 211, 223,
	227, 229, 233, 239, 241, 251, 257, 263,
	269, 271, 277, 281, 283, 293, 307, 311,
	313, 317, 331, 337, 347, 349, 353, 359,
	367, 373, 379, 383, 389, 397, 401, 409,
	419, 421, 431, 433, 439, 443, 449, 457,
	461, 463, 467, 479, 487, 491, 499, 503,
	509, 521, 523, 541, 547, 557, 563, 569,
	571, 577, 587, 593, 599, 601, 607, 613,
	617, 619, 631, 641, 643, 647, 653, 659,
	661, 673, 677, 683, 691, 701, 709, 719,
	727, 733, 739, 743, 751, 757, 761, 769,
	773, 787, 797, 809, 811, 821, 823, 827,
	829, 839, 853, 857, 859, 863, 877, 881,
	883, 887, 907, 911, 919, 929, 937, 941,
	947, 953, 967, 971, 977, 983, 991, 997,
	1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049,
	1051, 1061, 1063, 1069, 1087, 1091, 1093, 1097,
	1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
	1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
	1229, 1231, 1237, 1249, 1259, 1277, 1279, 1283,
	1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321,
	1327, 1361, 1367, 1373, 1381, 1399, 1409, 1423,
	1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459,
	1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511,
	1523, 1531, 1543, 1549, 1553, 1559, 1567, 1571,
	1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619,
	1621, 1627, 1637, 1657, 1663, 1667, 1669, 1693,
	1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747,
	1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811,
	1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877,
	1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949,
	1951, 1973, 1979, 1987, 1993, 1997, 1999, 2003,
	2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069,
	2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129,
	2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203,
	2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267,
	2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311,
	2333, 2339, 2341, 2347, 2351, 2357, 2371, 2377,
	2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423,
	2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503,
	2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579,
	2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657,
	2659, 2663, 2671, 2677, 2683, 2687, 2689, 2693,
	2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741,
	2749, 2753, 2767, 2777, 2789, 2791, 2797, 2801,
	2803, 2819, 2833, 2837, 2843, 2851, 2857, 2861,
	2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939,
	2953, 2957, 2963, 2969, 2971, 2999, 3001, 3011,
	3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079,
	3083, 3089, 3109, 3119, 3121, 3137, 3163, 3167,
	3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221,
	3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301,
	3307, 3313, 3319, 3323, 3329, 3331, 3343, 3347,
	3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413,
	3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491,
	3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541,
	3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607,
	3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671,
	3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727,
	3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797,
	3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863,
	3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923,
	3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003
};

#pragma pack()
END_TERIMBER_NAMESPACE

