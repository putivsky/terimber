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
 
#include "base/common.hpp"
#include "base/memory.hpp"
#include "base/vector.hpp"
#include "base/list.hpp"
#include "base/number.hpp"
#include "alg/algorith.hpp"
#include "crypt/integer.hpp"
#include "crypt/arithmet.hpp"

#include <time.h>

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

extern ub4_t small_prime_table_size;

integer::integer()
	: _reg(2), _sign(false)
{
	_reg[0] = _reg[1] = 0;
}

integer::integer(const integer& x)
: _reg(room_arithmetic::roundup_size(x.word_count())), _sign(x._sign)
{
	_reg.copy(x._reg, _reg.size());
}

integer::integer(int value)
	: _reg(2)
{
	if (value >= 0)
		_sign = false;
	else
	{
		_sign = true;
		value = -value;
	}
	_reg[0] = (ub4_t)value;
	//_reg[1] = sizeof(value) > word_size ? (ub4_t(value)>>word_bits) : 0;
	_reg[1] = 0;
}

bool 
integer::is_convertable_to_long() const
{
	if (byte_count() > sizeof(long))
		return false;

	long value = _reg[0];
	//value += sizeof(value)>word_size ? ((ub4_t)_reg[1]<<word_bits) : 0;

	return _sign ? -(long)value < 0 : (long)value >= 0;
}

long 
integer::convert_to_long() const
{
	long value = _reg[0];
	//value += sizeof(value)>word_size ? ((ub4_t)_reg[1]<<word_bits) : 0;
	return _sign ? -(long)value : value;
}

integer::integer(const ub1_t* encodedInteger, ub4_t byteCount, bool s) : 
	_reg(2)
{
	decode(encodedInteger, byteCount, s);
}

integer::integer(random_generator& rng, ub4_t bitcount) :
	_reg(2)
{
	randomize(rng, bitcount);
}

integer::integer(random_generator& rng, const integer& min, const integer& max, bool rnType, const integer& equiv, const integer& mod) :
	_reg(2)
{
	if (!randomize(rng, min, max, rnType, equiv, mod))
		exception::_throw("Random Number Not Found");
}

integer 
integer::power2(ub4_t e)
{
	integer r((ub4_t)0, room_arithmetic::bits_to_words(e + 1));
	r.set_bit(e);
	return r;
}

const integer&
integer::zero()
{
	static const integer zero;
	return zero;
}

const integer&
integer::one()
{
	static const integer one(1,2);
	return one;
}

bool 
integer::operator!() const
{
	return is_negative() ? false : (!_reg[0] && !word_count());
}

bool 
integer::get_bit(ub4_t n) const
{
	if (n/word_bits >= _reg.size())
		return false;
	else
		return bool((_reg[n/word_bits] >> (n % word_bits)) & 1);
}

void 
integer::set_bit(ub4_t n, bool value)
{
	if (value)
	{
		_reg.reserve(room_arithmetic::roundup_size(room_arithmetic::bits_to_words(n + 1)));
		_reg[n / word_bits] |= ((ub4_t)1 << (n % word_bits));
	}
	else
	{
		if (n / word_bits < _reg.size())
			_reg[n / word_bits] &= ~((ub4_t)1 << (n % word_bits));
	}
}

ub1_t 
integer::get_byte(ub4_t n) const
{
	if (n / word_size >= _reg.size())
		return 0;
	else
		return (ub1_t)(_reg[n / word_size] >> ((n % word_size)*8));
}

void 
integer::set_byte(ub4_t n, ub1_t value)
{
	_reg.reserve(room_arithmetic::roundup_size(room_arithmetic::bytes_to_words(n + 1)));
	_reg[n / word_size] &= ~((ub4_t)0xff << 8 * (n % word_size));
	_reg[n / word_size] |= ((ub4_t)value << 8 * (n % word_size));
}

ub4_t 
integer::get_bits(ub4_t i, ub4_t n) const
{
	assert(n <= sizeof(ub4_t) * 8);
	ub4_t v = 0;
	for (ub4_t j = 0; j < n; ++j)
		v |= get_bit(i + j) << j;
	return v;
}

integer 
integer::operator-() const
{
	integer result(*this);
	result.negate();
	return result;
}

integer 
integer::abs() const
{
	integer result(*this);
	result._sign = false;
	return result;
}

void 
integer::swap(integer& a)
{
	_reg.swap(a._reg);
	algorithm::swap(_sign, a._sign);
}

integer::integer(ub4_t value, ub4_t length)
	: _reg(room_arithmetic::roundup_size(length)), _sign(false)
{
	_reg[0] = value;
	room_arithmetic::set_words(_reg + 1, 0, (ub4_t)_reg.size() - 1);
}


integer::integer(const char *str)
	: _reg(2), _sign(false)
{
	ub4_t radix;
	ub4_t length = (ub4_t)strlen(str);

	room_arithmetic::set_words(_reg, 0, 2);

	if (length == 0)
		return;

	switch (str[length-1])
	{
		case 'h':
		case 'H':
			radix=16;
			break;
		case 'o':
		case 'O':
			radix=8;
			break;
		case 'b':
		case 'B':
			radix=2;
			break;
		default:
			radix=10;
	}

	if (strncmp("0x", str, 2) == 0)
		radix = 16;

	for (ub4_t i=0; i<length; i++)
	{
		ub4_t digit;

		if (str[i] >= '0' && str[i] <= '9')
			digit = str[i] - '0';
		else if (str[i] >= 'A' && str[i] <= 'F')
			digit = str[i] - 'A' + 10;
		else if (str[i] >= 'a' && str[i] <= 'f')
			digit = str[i] - 'a' + 10;
		else
			digit = radix;

		if (digit < radix)
		{
			*this *= radix;
			*this += digit;
		}
	}

	if (str[0] == '-')
		negate();
}

ub4_t 
integer::word_count() const
{
	return room_arithmetic::count_words(_reg, (ub4_t)_reg.size());
}

ub4_t 
integer::byte_count() const
{
	ub4_t wordCount = word_count();
	if (wordCount)
		return (wordCount - 1) * word_size + room_arithmetic::byte_precision(_reg[wordCount - 1]);
	else
		return 0;
}

ub4_t 
integer::bit_count() const
{
	ub4_t wordCount = word_count();
	if (wordCount)
		return (wordCount - 1) * word_bits + room_arithmetic::bit_precision(_reg[wordCount - 1]);
	else
		return 0;
}

void 
integer::decode(const ub1_t *input, ub4_t inputLen, bool s)
{
	ub1_t b = *input;

	_sign = (s && (b & 0x80)) ? true : false;

	while (inputLen > 0 && (!_sign ? b == 0 : b == 0xff))
	{
		++input;
		--inputLen;
		b = *input;
	}

	_reg.resize(room_arithmetic::roundup_size(room_arithmetic::bytes_to_words(inputLen)), true);

	for (ub4_t i = inputLen; i > 0; --i)
	{
		b = *input++;
		_reg[(i - 1) / word_size] |= b << ((i - 1) % word_size) * 8;
	}

	if (_sign)
	{
		for (ub4_t i = inputLen; i < _reg.size() * word_size; ++i)
			_reg[i / word_size] |= 0xff << (i % word_size) * 8;
		room_arithmetic::twos_complement(_reg, (ub4_t)_reg.size());
	}
}

ub4_t 
integer::min_encoded_size(bool s) const
{
	ub4_t outputLen = __max((ub4_t)1, byte_count());
	if (!s)
		return outputLen;
	if (not_negative() && (get_byte(outputLen - 1) & 0x80))
		++outputLen;
	if (is_negative() && *this < -power2(outputLen * 8 - 1))
		++outputLen;
	return outputLen;
}

ub4_t 
integer::encode(ub1_t *output, ub4_t outputLen, bool s) const
{
	if (!s || not_negative())
	{
		for (ub4_t i = outputLen; i > 0; --i)
			*output++ = get_byte(i - 1);
	}
	else
	{
		// takes two's complement of *this
		integer temp = integer::power2(8 * __max(byte_count(), outputLen)) + *this;
		for (ub4_t i = 0; i < outputLen; ++i)
			*output++ = temp.get_byte(outputLen - i - 1);
	}
	return outputLen;
}

void 
integer::randomize(random_generator &rng, ub4_t nbits)
{
	const ub4_t nbytes = nbits/8 + 1;
	room_array< ub1_t > buf(nbytes);
	rng.get_block(buf, nbytes);
	if (nbytes)
		buf[0] = (ub1_t)room_arithmetic::crop(buf[0], nbits % 8);
	decode(buf, nbytes, true);
}

void 
integer::randomize(random_generator &rng, const integer& min, const integer& max)
{
	assert(max >= min);

	integer range = max - min;
	const ub4_t nbits = range.bit_count();

	do
	{
		randomize(rng, nbits);
	}
	while (*this > range);

	*this += min;
}

bool 
integer::randomize(random_generator &rng, const integer& min, const integer& max, bool rnType, const integer& equiv, const integer& mod)
{
	assert(!equiv.is_negative() && equiv < mod);

	if (!rnType)
	{
		if (mod == one())
			randomize(rng, min, max);
		else
		{
			integer min1 = min + (equiv - min) % mod;
			if (max < min1)
				return false;
			randomize(rng, zero(), (max - min1) / mod);
			*this *= mod;
			*this += min1;
		}
		return true;
	}
	else
	{
		int i = 0;
		while (1)
		{
			if (++i == 16)
			{
				// checks if there are any suitable primes in [min, max]
				integer first = min;
				if (first_prime(first, max, equiv, mod))
				{
					// if there is only one suitable prime, we're done
					*this = first;
					if (!first_prime(first, max, equiv, mod))
						return true;
				}
				else
					return false;
			}

			randomize(rng, min, max);
			if (first_prime(*this, __min(*this + mod * prime_search_interval(max), max), equiv, mod))
				return true;
		}
	}
}


integer& 
integer::operator++()
{
	if (not_negative())
	{
		if (room_arithmetic::increment(_reg, (ub4_t)_reg.size()))
		{
			_reg.reserve(2 * _reg.size());
			_reg[_reg.size() / 2] = 1;
		}
	}
	else
	{
		ub4_t borrow = room_arithmetic::decrement(_reg, (ub4_t)_reg.size());
		assert(!borrow);
		if (word_count() == 0)
			*this = zero();
	}
	return *this;
}

integer& 
integer::operator--()
{
	if (is_negative())
	{
		if (room_arithmetic::increment(_reg, (ub4_t)_reg.size()))
		{
			_reg.reserve(2 * _reg.size());
			_reg[_reg.size() / 2] = 1;
		}
	}
	else
	{
		if (room_arithmetic::decrement(_reg, (ub4_t)_reg.size()))
			*this = -one();
	}
	return *this;
}

void 
integer::_add(integer& sum, const integer& a, const integer& b)
{
	ub4_t carry;
	if (a._reg.size() == b._reg.size())
		carry = room_arithmetic::add(sum._reg, a._reg, b._reg, (ub4_t)a._reg.size());
	else if (a._reg.size() > b._reg.size())
	{
		carry = room_arithmetic::add(sum._reg, a._reg, b._reg, (ub4_t)b._reg.size());
		room_arithmetic::copy_words(sum._reg + b._reg.size(), a._reg + b._reg.size(), (ub4_t)a._reg.size() - (ub4_t)b._reg.size());
		carry = room_arithmetic::increment(sum._reg + b._reg.size(), (ub4_t)a._reg.size() - (ub4_t)b._reg.size(), carry);
	}
	else
	{
		carry = room_arithmetic::add(sum._reg, a._reg, b._reg, (ub4_t)a._reg.size());
		room_arithmetic::copy_words(sum._reg + a._reg.size(), b._reg + a._reg.size(), (ub4_t)b._reg.size() - (ub4_t)a._reg.size());
		carry = room_arithmetic::increment(sum._reg + a._reg.size(), (ub4_t)b._reg.size() - (ub4_t)a._reg.size(), carry);
	}

	if (carry)
	{
		sum._reg.reserve(2 * sum._reg.size());
		sum._reg[sum._reg.size() / 2] = 1;
	}

	sum._sign = false;
}

void 
integer::_subtract(integer& diff, const integer& a, const integer& b)
{
	ub4_t aSize = a.word_count();
	aSize += aSize % 2;
	ub4_t bSize = b.word_count();
	bSize += bSize % 2;

	if (aSize == bSize)
	{
		if (room_arithmetic::compare(a._reg, b._reg, aSize) >= 0)
		{
			room_arithmetic::subtract(diff._reg, a._reg, b._reg, aSize);
			diff._sign = false;
		}
		else
		{
			room_arithmetic::subtract(diff._reg, b._reg, a._reg, aSize);
			diff._sign = true;
		}
	}
	else if (aSize > bSize)
	{
		ub4_t borrow = room_arithmetic::subtract(diff._reg, a._reg, b._reg, bSize);
		room_arithmetic::copy_words(diff._reg + bSize, a._reg + bSize, aSize - bSize);
		borrow = room_arithmetic::decrement(diff._reg + bSize, aSize - bSize, borrow);
		assert(!borrow);
		diff._sign = false;
	}
	else
	{
		ub4_t borrow = room_arithmetic::subtract(diff._reg, b._reg, a._reg, aSize);
		room_arithmetic::copy_words(diff._reg + aSize, b._reg + aSize, bSize - aSize);
		borrow = room_arithmetic::decrement(diff._reg + aSize, bSize - aSize, borrow);
		assert(!borrow);
		diff._sign = true;
	}
}

integer 
integer::plus(const integer& b) const
{
	integer sum((ub4_t)0, (ub4_t)__max(_reg.size(), b._reg.size()));
	if (not_negative())
		b.not_negative() ? _add(sum, *this, b) : _subtract(sum, *this, b);
	else if (b.not_negative())
		_subtract(sum, b, *this);
	else
	{
		_add(sum, *this, b);
		sum._sign = true;
	}
	
	return sum;
}

integer 
integer::minus(const integer& x) const
{
	integer diff((ub4_t)0, (ub4_t)__max(_reg.size(), x._reg.size()));
	if (not_negative())
		x.not_negative() ? _subtract(diff, *this, x) : _add(diff, *this, x);
	else if (x.not_negative())
	{
		_add(diff, *this, x);
		diff._sign = true;
	}
	else
		_subtract(diff, x, *this);

	return diff;
}

void integer::_multiply(integer& product, const integer& a, const integer& b)
{
	ub4_t aSize = room_arithmetic::roundup_size(a.word_count());
	ub4_t bSize = room_arithmetic::roundup_size(b.word_count());

	product._reg.resize(room_arithmetic::roundup_size(aSize + bSize), true);
	product._sign = false;

	room_array< ub4_t > workspace(aSize + bSize);
	room_arithmetic::asymmetric_multiply(product._reg, workspace, a._reg, aSize, b._reg, bSize);
}

void integer::multiply(integer& product, const integer& a, const integer& b)
{
	_multiply(product, a, b);

	if (a.not_negative() != b.not_negative())
		product.negate();
}

integer integer::times(const integer& b) const
{
	integer product;
	multiply(product, *this, b);
	return product;
}

void integer::_divide(integer& remainder, integer& quotient,
				   const integer& a, const integer& b)
{
	ub4_t aSize = a.word_count();
	ub4_t bSize = b.word_count();

	if (!bSize)
		exception::_throw("Divide by zero");

	if (a._compare(b) == -1)
	{
		remainder = a;
		remainder._sign = false;
		quotient = integer::zero();
		return;
	}

	aSize += aSize % 2;	// rounds up to next even number
	bSize += bSize % 2;

	remainder._reg.resize(room_arithmetic::roundup_size(bSize), true);
	remainder._sign = false;
	quotient._reg.resize(room_arithmetic::roundup_size(aSize - bSize + 2), true);
	quotient._sign = false;

	room_array< ub4_t > T(aSize + 2*bSize + 4);
	room_arithmetic::divide(remainder._reg, quotient._reg, T, a._reg, aSize, b._reg, bSize);
}

void integer::divide(integer& remainder, integer& quotient, const integer& dividend, const integer& divisor)
{
	_divide(remainder, quotient, dividend, divisor);

	if (dividend.is_negative())
	{
		quotient.negate();
		if (!remainder.is_zero())
		{
			--quotient;
			remainder = divisor.abs() - remainder;
		}
	}

	if (divisor.is_negative())
		quotient.negate();
}

void integer::divide_power2(integer& r, integer& q, const integer& a, ub4_t n)
{
	q = a;
	q >>= n;

	const ub4_t wordCount = room_arithmetic::bits_to_words(n);
	if (wordCount <= a.word_count())
	{
		r._reg.resize(room_arithmetic::roundup_size(wordCount));
		r._reg.copy(a._reg, wordCount);
		if (n % word_bits != 0)
			r._reg[wordCount - 1] %= (1 << (n % word_bits));
	}
	else
	{
		r._reg.resize(room_arithmetic::roundup_size(a.word_count()));
		r._reg.copy(a._reg, r._reg.size());
	}
	r._sign = false;

	if (a.is_negative() && !r.is_zero())
	{
		--q;
		r = power2(n) - r;
	}
}

integer 
integer::divided(const integer& b) const
{
	integer remainder, quotient;
	divide(remainder, quotient, *this, b);
	return quotient;
}

integer 
integer::mod(const integer& b) const
{
	integer remainder, quotient;
	divide(remainder, quotient, *this, b);
	return remainder;
}

void integer::divide(ub4_t &remainder, integer& quotient, const integer& dividend, ub4_t divisor)
{
	if (!divisor)
		exception::_throw("Divide by zero");

	assert(divisor);

	if ((divisor & (divisor - 1)) == 0)	// divisor is a power of 2
	{
		quotient = dividend >> (room_arithmetic::bit_precision(divisor) - 1);
		remainder = dividend._reg[0] & (divisor - 1);
		return;
	}

	ub4_t i = dividend.word_count();
	quotient._reg.resize(room_arithmetic::roundup_size(i), true);
	remainder = 0;
	while (i--)
	{
		quotient._reg[i] = (ub4_t)(make_ub8(dividend._reg[i], remainder) / divisor);
		remainder = (ub4_t)(make_ub8(dividend._reg[i], remainder) % divisor);
	}

	if (dividend.not_negative())
		quotient._sign = false;
	else
	{
		quotient._sign = true;
		if (remainder)
		{
			--quotient;
			remainder = divisor - remainder;
		}
	}
}

integer integer::divided(ub4_t b) const
{
	ub4_t remainder;
	integer quotient;
	divide(remainder, quotient, *this, b);
	return quotient;
}

ub4_t integer::mod(ub4_t divisor) const
{
	if (!divisor)
		exception::_throw("Divide by zero");

	assert(divisor);

	ub4_t remainder;

	if ((divisor & (divisor - 1)) == 0)	// divisor is a power of 2
		remainder = _reg[0] & (divisor - 1);
	else
	{
		ub4_t i = word_count();

		if (divisor <= 5)
		{
			ub8_t sum = 0;
			while (i--)
				sum += _reg[i];
			remainder = ub4_t(sum % divisor);
		}
		else
		{
			remainder = 0;
			while (i--)
				remainder = ub4_t(make_ub8(_reg[i], remainder) % divisor);
		}
	}

	if (is_negative() && remainder)
		remainder = divisor - remainder;

	return remainder;
}

void integer::negate()
{
	if (!!(*this))	// donesn't flip _sign if *this==0
		_sign = !_sign;
}

int integer::_compare(const integer& x) const
{
	ub4_t size = word_count(), srcSize = x.word_count();

	if (size == srcSize)
		return room_arithmetic::compare(_reg, x._reg, size);
	else
		return size > srcSize ? 1 : -1;
}

int 
integer::compare(const integer& x) const
{
	if (not_negative())
		return x.not_negative() ? _compare(x) : 1;
	else
		return x.not_negative() ? -1 : -_compare(x);
}

integer integer::sroot() const
{
	if (!is_positive())
		return zero();

	// overestimates square root
	integer x, y = power2((bit_count() + 1) / 2);
	assert(y * y >= *this);

	do
	{
		x = y;
		y = (x + *this / x) >> 1;
	} 
	while (y < x);

	return x;
}

bool integer::is_square() const
{
	integer r = sroot();
	return *this == r.squared();
}

bool integer::is_abs_one() const
{
	return (word_count() == 1) && (_reg[0] == 1);
}

integer integer::multiplicative_inverse() const
{
	return is_abs_one() ? *this : zero();
}

integer integer::gcd(const integer& a, const integer& b)
{
	return euclidean_domain_of< integer >().gcd(a, b);
}

integer integer::inverse_mod(const integer& m) const
{
	assert(m.not_negative());

	if (is_negative() || *this >= m)
		return (*this % m).inverse_mod(m);

	if (m.is_event())
	{
		if (!m || is_event())
			return zero();	// no inverse
		if (*this == one())
			return one();

		integer u = m.inverse_mod(*this);
		return !u ? zero() : (m * (*this - u) + 1) / (*this);
	}

	room_array< ub4_t > T(m._reg.size() * 4);
	integer r((ub4_t)0, (ub4_t)m._reg.size());
	ub4_t k = room_arithmetic::almost_inverse(r._reg, T, _reg, (ub4_t)_reg.size(), m._reg, (ub4_t)m._reg.size());
	room_arithmetic::divide_power2_mod(r._reg, r._reg, k, m._reg, (ub4_t)m._reg.size());
	return r;
}

ub4_t integer::inverse_mod(const ub4_t mod) const
{
	ub4_t g0 = mod, g1 = *this % mod;
	ub4_t v0 = 0, v1 = 1;
	ub4_t y;

	while (g1)
	{
		if (g1 == 1)
			return v1;
		y = g0 / g1;
		g0 = g0 % g1;
		v0 += y * v1;

		if (!g0)
			break;
		if (g0 == 1)
			return mod - v0;
		y = g1 / g0;
		g1 = g1 % g0;
		v1 += y * v0;
	}
	return 0;
}

//static 
integer 
integer::a_times_b_mod_c(const integer& x, const integer& y, const integer& m)
{
	return x * y % m;
}

//static 
integer 
integer::a_exp_b_mod_c(const integer& x, const integer& e, const integer& m)
{
	modular_arithmetic mr(m);
	return mr.exponentiate(x, e);
}

//static 
ub4_t 
integer::prime_search_interval(const integer& max)
{
	return max.bit_count();
}

//static 
integer 
integer::CRT(const integer& xp, const integer& p, const integer& xq, const integer& q, const integer& u)
{
	// isn't the operator overloading great?
	return p * (u * (xq - xp) % q) + xp;
}

//static 
bool 
integer::is_small_prime(const integer& p)
{
	room_arithmetic::build_prime_table();

	if (p.is_positive() && p <= small_prime_table[small_prime_table_size - 1])
		return std::binary_search(small_prime_table, small_prime_table + small_prime_table_size, (ub4_t)p.convert_to_long());
	else
		return false;
}

//static 
bool 
integer::is_strong_probable_prime(const integer& n, const integer& b)
{
	if (n <= 3)
		return n == 2 || n == 3;

	assert(n > 3 && b > 1 && b < n - 1);

	if ((n.is_event() && n != 2) || integer::gcd(b, n) != 1)
		return false;

	integer nminus1 = n - 1;
	ub4_t a;

	// calculates a = largest power of 2 that divides (n-1)
	for (a = 0; ; ++a)
		if (nminus1.get_bit(a))
			break;

	integer m = nminus1 >> a;

	integer z = a_exp_b_mod_c(b, m, n);
	//if (z == 1 || z == nminus1)
	if (z == one() || z == nminus1)
		return true;
	for (ub4_t i = 1; i < a; ++i)
	{
		z = z.squared() % n;
		if (z == nminus1)
			return true;
		//if (z == 1)
		if (z == one())
			return false;
	}
	return false;
}

//static 
int 
integer::jacobi(const integer& aIn, const integer& bIn)
{
	assert(bIn.is_odd());

	integer b = bIn, a = aIn % bIn;
	int result = 1;

	while (!!a)
	{
		ub4_t i=0;
		while (a.get_bit(i) == 0)
			i++;
		a >>= i;

		if (i % 2 == 1 && (b % 8 == 3 || b % 8 == 5))
			result = -result;

		if (a % 4 == 3 && b % 4 == 3)
			result = -result;

		algorithm::swap(a, b);
		a %= b;
	}

	return b == one() ? result : 0;
}

//static 
integer 
integer::lucas(const integer& e, const integer& pIn, const integer& n)
{
	ub4_t i = e.bit_count();
	if (i == 0)
		return 2;

	montgomery_representation m(n);
	integer p = m.convert_in(pIn % n), two = m.convert_in(2);
	integer v = p, v1 = m.subtract(m.square(p), two);

	i--;
	while (i--)
	{
		if (e.get_bit(i))
		{
			// v = (v*v1 - p) % m;
			v = m.subtract(m.multiply(v, v1), p);
			// v1 = (v1*v1 - 2) % m;
			v1 = m.subtract(m.square(v1), two);
		}
		else
		{
			// v1 = (v*v1 - p) % m;
			v1 = m.subtract(m.multiply(v, v1), p);
			// v = (v*v - 2) % m;
			v = m.subtract(m.square(v), two);
		}
	}
	return m.convert_out(v);
}

//static 
bool 
integer::is_strong_lucas_probable_prime(const integer& n)
{
	if (n <= 1)
		return false;

	if (n.is_event())
		return n == 2;

	assert(n > 2);

	integer b = 3;
	ub4_t i = 0;
	int j;

	while ((j = jacobi(b.squared()-4, n)) == 1)
	{
		if (++i==64 && n.is_square())	// avoid infinite loop if n is a square
			return false;
		++b; ++b;
	}

	if (j == 0) 
		return false;

	integer n1 = n + 1;
	ub4_t a;

	// calculates a = largest power of 2 that divides n1
	for (a=0; ; ++a)
		if (n1.get_bit(a))
			break;
	integer m = n1 >> a;

	integer z = lucas(m, b, n);
	if (z == 2 || z == n - 2)
		return true;
	for (i = 1; i < a; ++i)
	{
		z = (z.squared() - 2) % n;
		if (z == n - 2)
			return true;
		if (z == 2)
			return false;
	}
	return false;
}

//static 
bool 
integer::trial_division(const integer& p, ub4_t bound)
{
	assert(small_prime_table[small_prime_table_size - 1] >= bound);

	ub4_t i;
	for (i = 0; small_prime_table[i] < bound; ++i)
		if ((p % small_prime_table[i]) == 0)
			return true;

	if (bound == small_prime_table[i])
		return (p % bound == 0);
	else
		return false;
}

//static 
bool 
integer::small_divisors_test(const integer& p)
{
	room_arithmetic::build_prime_table();
	return !trial_division(p, small_prime_table[small_prime_table_size - 1]);
}

//static 
bool 
integer::is_prime(const integer& p)
{
	static const integer lastSmallPrimeSquared = integer(last_small_prime).squared();

	if (p <= last_small_prime)
		return is_small_prime(p);
	else if (p <= lastSmallPrimeSquared)
		return small_divisors_test(p);
	else
		return small_divisors_test(p) && is_strong_probable_prime(p, 3) && is_strong_lucas_probable_prime(p);
}

//static 
bool 
integer::first_prime(integer& p, const integer& max, const integer& equiv, const integer& mod)
{
	assert(!equiv.is_negative() && equiv < mod);

	integer gcd = integer::gcd(equiv, mod);
	if (gcd != integer::one())
	{
		// the only possible prime p such that p%mod==equiv where GCD(mod,equiv)!=1 is GCD(mod,equiv)
		if (p <= gcd && gcd <= max && is_prime(gcd))
		{
			p = gcd;
			return true;
		}
		else
			return false;
	}

	room_arithmetic::build_prime_table();

	if (p <= small_prime_table[small_prime_table_size - 1])
	{
		ub4_t *pItr;

		--p;
		if (p.is_positive())
			pItr = std::upper_bound(small_prime_table, small_prime_table + small_prime_table_size, (ub4_t)p.convert_to_long());
		else
			pItr = small_prime_table;

		while (pItr < small_prime_table+small_prime_table_size && *pItr%mod != equiv)
			++pItr;

		if (pItr < small_prime_table+small_prime_table_size)
		{
			p = *pItr;
			return p <= max;
		}

		p = small_prime_table[small_prime_table_size-1]+1;
	}

	assert(p > small_prime_table[small_prime_table_size-1]);

	if (mod.is_odd())
		return first_prime(p, max, CRT(equiv, mod, 1, 2, 1), mod << 1);

	p += (equiv - p) % mod;

	if (p > max)
		return false;

	prime_sieve sieve(p, max, mod);

	while (sieve.next_candidate(p))
	{
		if (fast_probable_prime_test(p) && is_prime(p))
			return true;
	}

	return false;
}

//////////////////////////////////////////////
random_generator::random_generator()
{
	srand((unsigned int)time(0));
}

//virtual 
ub4_t 
random_generator::generate_bit()
{ 
	return room_arithmetic::parity(get_byte()); 
}

// virtual 
ub4_t 
random_generator::generate_dword(ub4_t min, ub4_t max)
{
	ub4_t range = max - min;
	const int maxBytes = room_arithmetic::byte_precision(range);
	const int maxBits = room_arithmetic::bit_precision(range);

	ub4_t value;

	do
	{
		value = 0;
		for (int i = 0; i < maxBytes; ++i)
			value = (value << 8) | get_byte();

		value = room_arithmetic::crop(value, maxBits);
	} 
	while (value > range);

	return value + min;
}

// virtual 
void random_generator::generate_block(ub1_t *output, ub4_t size)
{
	while (size--)
		*output++ = get_byte();
}

// virtual 
ub1_t 
random_generator::generate_byte()
{
	return rand();
}

////////////////////////////////////////////////////
montgomery_representation::montgomery_representation(const integer& m)	// _modulus must be odd
	: modular_arithmetic(m),
	  _u((ub4_t)0, (ub4_t)_modulus._reg.size()),
	  _workspace(5 * (ub4_t)_modulus._reg.size())
{
	assert(_modulus.is_odd());
	room_arithmetic::recursive_inverse_mod_power2(_u._reg, _workspace, _modulus._reg, (ub4_t)_modulus._reg.size());
}

const integer& 
montgomery_representation::multiply(const integer& a, const integer& b) const
{
	ub4_t* const T = _workspace;
	ub4_t* const R = _result._reg;
	const ub4_t N = (ub4_t)_modulus._reg.size();
	assert(a._reg.size() <= N && b._reg.size() <= N);

	room_arithmetic::asymmetric_multiply(T, T + 2 * N, a._reg, (ub4_t)a._reg.size(), b._reg, (ub4_t)b._reg.size());
	room_arithmetic::set_words(T + a._reg.size() + b._reg.size(), 0, 2 * N - (ub4_t)a._reg.size() - (ub4_t)b._reg.size());
	room_arithmetic::montgomery_reduce(R, T + 2 * N, T, _modulus._reg, _u._reg, N);
	return _result;
}

const integer& 
montgomery_representation::square(const integer& a) const
{
	ub4_t* const T = _workspace;
	ub4_t* const R = _result._reg;
	const ub4_t N = (ub4_t)_modulus._reg.size();
	assert(a._reg.size() <= N);

	room_arithmetic::recursive_square(T, T + 2 * N, a._reg, (ub4_t)a._reg.size());
	room_arithmetic::set_words(T + 2 * a._reg.size(), 0, 2 * N - 2 * (ub4_t)a._reg.size());
	room_arithmetic::montgomery_reduce(R, T + 2 * N, T, _modulus._reg, _u._reg, N);
	return _result;
}

integer 
montgomery_representation::convert_out(const integer& a) const
{
	ub4_t* const T = _workspace;
	ub4_t* const R = _result._reg;
	const ub4_t N = (ub4_t)_modulus._reg.size();
	assert(a._reg.size() <= N);

	room_arithmetic::copy_words(T, a._reg, (ub4_t)a._reg.size());
	room_arithmetic::set_words(T + a._reg.size(), 0, 2*N-(ub4_t)a._reg.size());
	room_arithmetic::montgomery_reduce(R, T+2*N, T, _modulus._reg, _u._reg, N);
	return _result;
}

const integer& 
montgomery_representation::multiplicative_inverse(const integer& a) const
{
	ub4_t* const T = _workspace;
	ub4_t* const R = _result._reg;
	const ub4_t N = (ub4_t)_modulus._reg.size();
	assert(a._reg.size() <= N);

	room_arithmetic::copy_words(T, a._reg, (ub4_t)a._reg.size());
	room_arithmetic::set_words(T + a._reg.size(), 0, 2 * N - (ub4_t)a._reg.size());
	room_arithmetic::montgomery_reduce(R, T + 2 * N, T, _modulus._reg, _u._reg, N);
	ub4_t k = room_arithmetic::almost_inverse(R, T, R, N, _modulus._reg, N);

	k > N * word_bits ? room_arithmetic::divide_power2_mod(R, R, k - N * word_bits, _modulus._reg, N) :
						room_arithmetic::multiply_power2_mod(R, R, N * word_bits - k, _modulus._reg, N);

	return _result;
}

//////////////////////////////////////////////////////
modular_arithmetic::modular_arithmetic(const integer& modulus)
	: _modulus(modulus), _result((ub4_t)0, (ub4_t)_modulus._reg.size()) 
{
}

modular_arithmetic::modular_arithmetic(const modular_arithmetic &ma)
	: _modulus(ma._modulus), _result((ub4_t)0, (ub4_t)_modulus._reg.size()) 
{
}

// virtual 
integer
modular_arithmetic::convert_in(const integer& a) const
{ 
	return a % _modulus; 
}

// virtual 
integer
modular_arithmetic::convert_out(const integer& a) const
{
	return a; 
}

const integer& 
modular_arithmetic::half(const integer& a) const
{
	if (a._reg.size() == _modulus._reg.size())
	{
		room_arithmetic::divide_power2_mod(_result._reg, a._reg, 1, _modulus._reg, (ub4_t)a._reg.size());
		return _result;
	}
	else
		return _result1 = (a.is_event() ? (a >> 1) : ((a + _modulus) >> 1));
}

const integer& 
modular_arithmetic::add(const integer& a, const integer& b) const
{
	if (a._reg.size()==_modulus._reg.size() && b._reg.size()==_modulus._reg.size())
	{
		if (room_arithmetic::add(_result._reg, a._reg, b._reg, (ub4_t)a._reg.size())
			|| room_arithmetic::compare(_result._reg, _modulus._reg, (ub4_t)a._reg.size()) >= 0)
		{
			room_arithmetic::subtract(_result._reg, _result._reg, _modulus._reg, (ub4_t)a._reg.size());
		}
		return _result;
	}
	else
	{
		_result1 = a + b;
		if (_result1 >= _modulus)
			_result1 -= _modulus;

		return _result1;
	}
}

integer& 
modular_arithmetic::accumulate(integer& a, const integer& b) const
{
	if (a._reg.size() == _modulus._reg.size() && b._reg.size() == _modulus._reg.size())
	{
		if (room_arithmetic::add(a._reg, a._reg, b._reg, (ub4_t)a._reg.size())
			|| room_arithmetic::compare(a._reg, _modulus._reg, (ub4_t)a._reg.size()) >= 0)
		{
			room_arithmetic::subtract(a._reg, a._reg, _modulus._reg, (ub4_t)a._reg.size());
		}
	}
	else
	{
		a += b;
		if (a >= _modulus)
			a -= _modulus;
	}

	return a;
}

const integer& 
modular_arithmetic::subtract(const integer& a, const integer& b) const
{
	if (a._reg.size() == _modulus._reg.size() && b._reg.size() == _modulus._reg.size())
	{
		if (room_arithmetic::subtract(_result._reg, a._reg, b._reg, (ub4_t)a._reg.size()))
			room_arithmetic::add(_result._reg, _result._reg, _modulus._reg, (ub4_t)a._reg.size());
		return _result;
	}
	else
	{
		_result1 = a - b;
		if (_result1.is_negative())
			_result1 += _modulus;
		return _result1;
	}
}

integer& 
modular_arithmetic::reduce(integer& a, const integer& b) const
{
	if (a._reg.size() == _modulus._reg.size() && b._reg.size() == _modulus._reg.size())
	{
		if (room_arithmetic::subtract(a._reg, a._reg, b._reg, (ub4_t)a._reg.size()))
			room_arithmetic::add(a._reg, a._reg, _modulus._reg, (ub4_t)a._reg.size());
	}
	else
	{
		a -= b;
		if (a.is_negative())
			a += _modulus;
	}

	return a;
}

const integer& 
modular_arithmetic::inverse(const integer& a) const
{
	if (!a)
		return a;

	_result._reg.copy(_modulus._reg, _modulus._reg.size());
	if (room_arithmetic::subtract(_result._reg, _result._reg, a._reg, (ub4_t)a._reg.size()))
		room_arithmetic::decrement(_result._reg + a._reg.size(), 1, (ub4_t)_modulus._reg.size() - (ub4_t)a._reg.size());

	return _result;
}

integer 
modular_arithmetic::cascade_exponentiate(const integer& x, const integer& e1, const integer& y, const integer& e2) const
{
	if (_modulus.is_odd())
	{
		montgomery_representation dr(_modulus);
		return dr.convert_out(dr.cascade_exponentiate(dr.convert_in(x), e1, dr.convert_in(y), e2));
	}
	else
		return abstract_ring<integer>::cascade_exponentiate(x, e1, y, e2);
}

void 
modular_arithmetic::simultaneous_exponentiate(integer *results, const integer& base, const integer *exponents, ub4_t exponentsCount) const
{
	if (_modulus.is_odd())
	{
		montgomery_representation dr(_modulus);
		dr.simultaneous_exponentiate(results, dr.convert_in(base), exponents, exponentsCount);
		for (ub4_t i = 0; i < exponentsCount; ++i)
			results[i] = dr.convert_out(results[i]);
	}
	else
		abstract_ring<integer>::simultaneous_exponentiate(results, base, exponents, exponentsCount);
}

///////////////////////////////////////////////////////////////////////////////
window_slider::window_slider(const integer& exp, bool fast_negate, ub4_t window_size)
		: _exp(exp), _window_modulus(integer::one()), _window_size(window_size), _window_begin(0), _fast_negate(fast_negate), _first_time(true), _finished(false)
{
	if (!_window_size)
	{
		ub4_t expLen = _exp.bit_count();
		_window_size = expLen <= 17 ? 1 : (expLen <= 24 ? 2 : (expLen <= 70 ? 3 : (expLen <= 197 ? 4 : (expLen <= 539 ? 5 : (expLen <= 1434 ? 6 : 7)))));
	}
	
	_window_modulus <<= _window_size;
}
///////////////////////////////////////////////////////////////////////////////
prime_sieve::prime_sieve(const integer& first, const integer& last, const integer& step, sb4_t delta)
	: _first(first), _last(last), _step(step), _delta(delta), _next(0)
{
	do_sieve();
}

void prime_sieve::do_sieve()
{
	room_arithmetic::build_prime_table();

	const ub4_t maxSieveSize = 32768;
	ub4_t sieveSize = __min(integer(maxSieveSize), (_last - _first) / _step + 1).convert_to_long();

	_sieve.clear();
	_sieve.resize(sieveSize, false);

	if (_delta == 0)
	{
		for (ub4_t i = 0; i < small_prime_table_size; ++i)
			sieve_single(_sieve, small_prime_table[i], _first, _step, _step.inverse_mod(small_prime_table[i]));
	}
	else
	{
		assert(_step % 2 == 0);
		integer qFirst = (_first - _delta) >> 1;
		integer halfStep = _step >> 1;
		for (ub4_t i = 0; i < small_prime_table_size; ++i)
		{
			ub4_t p = small_prime_table[i];
			ub4_t stepInv = _step.inverse_mod(p);
			sieve_single(_sieve, p, _first, _step, stepInv);

			ub4_t halfStepInv = 2 * stepInv < p ? 2 * stepInv : 2 * stepInv - p;
			sieve_single(_sieve, p, qFirst, halfStep, halfStepInv);
		}
	}
}

bool prime_sieve::next_candidate(integer& c)
{
	vector< bool >::iterator first = _sieve.begin() + _next;
	vector< bool >::iterator last = _sieve.end();

	for (; first != last; ++first, ++_next)
		if (!*first)
			break;

	if (_next == _sieve.size())
	{
		_first += (ub4_t)_sieve.size() * _step;
		if (_first > _last)
			return false;
		else
		{
			_next = 0;
			do_sieve();
			return next_candidate(c);
		}
	}
	else
	{
		c = _first + _next * _step;
		++_next;
		return true;
	}
}

void prime_sieve::sieve_single(vector< bool > &sieve, ub4_t p, const integer& first, const integer& step, ub4_t stepInv)
{
	if (stepInv)
	{
		ub4_t sieveSize = (ub4_t)sieve.size();
		ub4_t j = (ub4_t)((ub8_t(p - (first % p)) * stepInv) % p);
		// if the first multiple of p is p, skips it
		if (first.word_count() <= 1 && first + step * j == p)
			j += p;
		for (; j < sieveSize; j += p)
			sieve[j] = true;
	}
}

////////////////////////////////////////////////////////////////////////////////
#pragma pack()
END_TERIMBER_NAMESPACE

