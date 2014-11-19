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

#ifndef _terimber_integer_hpp_
#define _terimber_integer_hpp_

#include "crypt/integer.h"
#include "base/list.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

inline
ub1_t
random_generator::get_byte()
{ 
	return generate_byte(); 
}

inline
ub4_t
random_generator::get_bit()
{ 
	return generate_bit(); 
}

inline
ub4_t
random_generator::get_dword(ub4_t a, ub4_t b) 
{ 
	return generate_dword(a, b); 
}

inline
ub2_t
random_generator::get_word(ub2_t a, ub2_t b) 
{ 
	return (ub2_t)generate_dword(a, b); 
}

inline
void
random_generator::get_block(ub1_t *output, ub4_t size) 
{ 
	generate_block(output, size); 
}

//////////////////////////////////////////////////////////////////////////
// integer
inline
integer& 
integer::operator=(const integer& x)
{
	if (this != &x)
	{
		_reg.resize(room_arithmetic::roundup_size(x.word_count()), true);
		_reg.copy(x._reg, _reg.size());
		_sign = x._sign;
	}
	return *this;
}

inline
bool
integer::is_zero() const
{
	return !*this;
}

inline
bool
integer::not_zero() const
{
	return !is_zero();
}

inline
bool
integer::is_negative() const
{ 
	return _sign; 
}

inline
bool
integer::not_negative() const
{
	return !_sign;
}

inline
bool
integer::is_positive() const
{
	return !_sign && !!*this;
}

inline
bool
integer::not_positive() const
{
	return _sign || !*this;
}

inline
bool
integer::is_event() const
{
	return !get_bit(0);
}

inline
bool
integer::is_odd() const
{
	return get_bit(0) == 1;
}

inline
integer&
integer::operator*=(const integer& x)
{
	return *this = times(x);
}

inline
integer&
integer::operator/=(const integer& x)
{
	return *this = divided(x);
}

inline
integer&
integer::operator%=(const integer& x)
{
	return *this = mod(x);
}

inline
integer&
integer::operator/=(ub4_t x)
{
	return *this = divided(x);
}

inline
integer&
integer::operator%=(ub4_t x)
{
	return *this = mod(x);
}

inline
void
integer::set_positive()
{ 
	_sign = false; 
}

inline
void
integer::set_negative()
{ 
	if (!!(*this)) 
		_sign = true; 
}

inline
integer
integer::operator+() const
{
	return *this;
}

inline
integer
integer::operator++(int)
{
	integer temp = *this; 
	++*this; 
	return temp;
}

inline
integer
integer::operator--(int)
{
	integer temp = *this;
	--*this; 
	return temp;
}

inline
integer
integer::operator>>(ub4_t n) const
{
	return integer(*this) >>= n;
}

inline
integer
integer::operator<<(ub4_t n) const
{
	return integer(*this) <<= n;
}

inline
integer& 
integer::operator+=(const integer& x)
{
	_reg.reserve(x._reg.size());
	if (not_negative())
		x.not_negative() ? _add(*this, *this, x) : _subtract(*this, *this, x);
	else if (x.not_negative())
		_subtract(*this, x, *this);
	else
	{
		_add(*this, *this, x);
		_sign = true;
	}

	return *this;
}

inline
integer& 
integer::operator-=(const integer& x)
{
	_reg.reserve(x._reg.size());
	if (not_negative())
		x.not_negative() ? _subtract(*this, *this, x) : _add(*this, *this, x);
	else if (x.not_negative())
	{
		_add(*this, *this, x);
		_sign = true;
	}
	else
		_subtract(*this, x, *this);

	return *this;
}

inline
integer& 
integer::operator<<=(ub4_t n)
{
	const ub4_t wordCount = word_count();
	const ub4_t shiftWords = n / word_bits;
	const ub4_t shiftBits = n % word_bits;

	_reg.reserve(room_arithmetic::roundup_size(wordCount + room_arithmetic::bits_to_words(n)));
	room_arithmetic::shl_words(_reg, wordCount + shiftWords, shiftWords);
	room_arithmetic::shl_bits(_reg+shiftWords, wordCount + room_arithmetic::bits_to_words(shiftBits), shiftBits);
	return *this;
}

inline
integer& 
integer::operator>>=(ub4_t n)
{
	const ub4_t wordCount = word_count();
	const ub4_t shiftWords = n / word_bits;
	const ub4_t shiftBits = n % word_bits;

	room_arithmetic::shr_words(_reg, wordCount, shiftWords);
	if (wordCount > shiftWords)
		room_arithmetic::shr_bits(_reg, wordCount-shiftWords, shiftBits);
	if (is_negative() && word_count() == 0)   // avoid -0
		*this = zero();
	return *this;
}

inline
integer
integer::doubled() const
{
	return plus(*this);
}

inline
integer
integer::squared() const
{
	return times(*this);
}
////////////////////////////////
//!
inline 
bool
operator==(const integer& a, const integer& b)
{
	return !a.compare(b);
}

//!
inline 
bool 
operator!=(const integer& a, const integer& b) 
{
	return a.compare(b) != 0;
}

//!
inline 
bool 
operator>(const integer& a, const integer& b)
{
	return a.compare(b) > 0;
}

//!
inline 
bool 
operator>=(const integer& a, const integer& b)
{
	return a.compare(b) >= 0;
}

//!
inline 
bool 
operator<(const integer& a, const integer& b)
{
	return a.compare(b) < 0;
}
//!
inline 
bool 
operator<=(const integer& a, const integer& b) 
{
	return a.compare(b) <= 0;
}

//!
inline 
integer 
operator+(const integer& a, const integer& b) 
{
	return a.plus(b);
}

//!
inline 
integer 
operator-(const integer& a, const integer& b)
{
	return a.minus(b);
}

//!
inline 
integer 
operator*(const integer& a, const integer& b)
{
	return a.times(b);
}

//!
inline 
integer 
operator/(const integer& a, const integer& b)
{
	return a.divided(b);
}

//!

inline 
integer 
operator%(const integer& a, const integer& b)
{
	return a.mod(b);
}


//!
inline 
integer 
operator/(const integer& a, ub4_t b) 
{
	return a.divided(b);
}

//!
inline 
ub4_t 
operator%(const integer& a, ub4_t b) 
{
	return a.mod(b);
}

inline 
void 
swap(integer& a, integer& b)
{
	a.swap(b);
}

//////////////////////////////////////////////////////////////////////////////
// ! abstract_group
template < class T >
abstract_group< T >::~abstract_group()
{
}

// virtual
template < class T >
bool 
abstract_group< T >::inversion_is_fast() const
{
	return false;
}

template < class T >
void 
abstract_group< T >::simultaneous_multiply(T* results, const T& base, const integer* expBegin, ub4_t expCount) const
{
	vector< vector< T > > buckets;
	buckets.resize(expCount);

	list< window_slider > exponents;
	ub4_t i;

	for (i = 0; i < expCount; ++i)
	{
		assert(expBegin->not_negative());
		window_slider& item = *exponents.push_back(window_slider(*expBegin++, inversion_is_fast(), 0));
		item.find_next_window();
		buckets[i].resize(1 << (item._window_size - 1), zero());
	}

	ub4_t expBitPosition = 0;
	T g = base;
	bool notDone = true;

	while (notDone)
	{
		notDone = false;
		list< window_slider >::iterator iter = exponents.begin();
		for (i = 0; i < expCount; ++i, ++iter)
		{
			if (!iter->_finished && expBitPosition == iter->_window_begin)
			{
				T& bucket = buckets[i][iter->_exp_window / 2];
				if (iter->_negate_next)
					accumulate(bucket, inverse(g));
				else
					accumulate(bucket, g);
				iter->find_next_window();
			}
			notDone = notDone || !iter->_finished;
		}

		if (notDone)
		{
			g = doubled(g);
			expBitPosition++;
		}
	}

	for (i = 0; i < expCount; ++i)
	{
		T& r = *results++;
		r = buckets[i][buckets[i].size()-1];
		if (buckets[i].size() > 1)
		{
			for (int j = (int)buckets[i].size() - 2; j >= 1; --j)
			{
				accumulate(buckets[i][j], buckets[i][j + 1]);
				accumulate(r, buckets[i][j]);
			}
			accumulate(buckets[i][0], buckets[i][1]);
			r = add(doubled(r), buckets[i][0]);
		}
	}
}


template < class T > 
T 
abstract_group< T >::scalar_multiply(const T& base, const integer& exponent) const
{
	T result;
	simultaneous_multiply(&result, base, &exponent, 1);
	return result;
}

template < class T >
const T& 
abstract_group< T >::doubled(const T& a) const
{
	return add(a, a);
}

template < class T > 
const T& 
abstract_group< T >::subtract(const T& a, const T& b) const
{
	// makes a copy of a in case Inverse() overwrites it
	T a1(a);
	return add(a1, inverse(b));
}

template < class T >
T& 
abstract_group< T >::accumulate(T& a, const T& b) const
{
	return a = add(a, b);
}

template < class T >
T& 
abstract_group< T >::reduce(T& a, const T& b) const
{
	return a = subtract(a, b);
}

template < class T >
T 
abstract_group< T >::cascade_scalar_multiply(const T& x, const integer& e1, const T& y, const integer& e2) const
{
	const ub4_t expLen = __max(e1.bit_count(), e2.bit_count());
	if (expLen == 0)
		return zero();

	const ub4_t w = (expLen <= 46 ? 1 : (expLen <= 260 ? 2 : 3));
	const ub4_t tableSize = 1 << w;
	
	vector< T > powerTable;
	powerTable.resize(tableSize << w);

	powerTable[1] = x;
	powerTable[tableSize] = y;
	if (w == 1)
		powerTable[3] = add(x,y);
	else
	{
		powerTable[2] = doubled(x);
		powerTable[2 * tableSize] = doubled(y);

		ub4_t i, j;

		for (i = 3; i < tableSize; i += 2)
			powerTable[i] = add(powerTable[i - 2], powerTable[2]);
		for (i = 1; i < tableSize; i += 2)
			for (j = i + tableSize; j < (tableSize << w); j += tableSize)
				powerTable[j] = add(powerTable[j - tableSize], y);

		for (i = 3 * tableSize; i < (tableSize << w); i += 2 * tableSize)
			powerTable[i] = add(powerTable[i - 2 * tableSize], powerTable[2 * tableSize]);
		for (i = tableSize; i < (tableSize << w); i += 2 * tableSize)
			for (j = i + 2; j < i + tableSize; j += 2)
				powerTable[j] = add(powerTable[j - 1], x);
	}

	T result;
	ub4_t power1 = 0, power2 = 0, prevPosition = expLen - 1;
	bool firstTime = true;

	for (int i = expLen - 1; i >= 0; i--)
	{
		power1 = 2 * power1 + e1.get_bit(i);
		power2 = 2 * power2 + e2.get_bit(i);

		if (i == 0 || 2 * power1 >= tableSize || 2 * power2 >= tableSize)
		{
			ub4_t squaresBefore = prevPosition - i;
			ub4_t squaresAfter = 0;
			prevPosition = i;
			while ((power1 || power2) && power1 % 2 == 0 && power2 % 2==0)
			{
				power1 /= 2;
				power2 /= 2;
				squaresBefore--;
				squaresAfter++;
			}
			if (firstTime)
			{
				result = powerTable[(power2 << w) + power1];
				firstTime = false;
			}
			else
			{
				while (squaresBefore--)
					result = doubled(result);
				if (power1 || power2)
					accumulate(result, powerTable[(power2 << w) + power1]);
			}
			while (squaresAfter--)
				result = doubled(result);
			power1 = power2 = 0;
		}
	}
	return result;
}
////////////////////////////////////
//! abstract_ring
template < class T >
abstract_ring< T >::abstract_ring()
{
	_mg._ring = this;
}

template < class T >
abstract_ring< T >::abstract_ring(const abstract_ring &x) 
{
	_mg._ring = this;
}

template < class T >
abstract_ring< T >&
abstract_ring< T >::operator=(const abstract_ring &x) 
{
	return *this;
}

// virtual 
template < class T >
const abstract_group< T >& 
abstract_ring< T >::multiplicative_group() const
{
	return _mg;
}

template < class T > 
const T& 
abstract_ring< T >::square(const T& a) const
{
	return multiply(a, a);
}

template < class T >
const T& 
abstract_ring< T >::divide(const T& a, const T& b) const
{
	// makes a copy of a in case MultiplicativeInverse() overwrites it
	T a1(a);
	return multiply(a1, multiplicative_inverse(b));
}

template < class T > 
T 
abstract_ring< T >::exponentiate(const T& base, const integer& exponent) const
{
	T result;
	simultaneous_exponentiate(&result, base, &exponent, 1);
	return result;
}

template < class T >
T 
abstract_ring< T >::cascade_exponentiate(const T& x, const integer& e1, const T& y, const integer& e2) const
{
	return multiplicative_group().abstract_group<T>::cascade_scalar_multiply(x, e1, y, e2);
}

template < class T >
void 
abstract_ring< T >::simultaneous_exponentiate(T* results, const T& base, const integer* exponents, ub4_t expCount) const
{
	multiplicative_group().abstract_group<T>::simultaneous_multiply(results, base, exponents, expCount);
}

//////////////////////////////////////////////////////////////
inline 
const integer&
modular_arithmetic::get_modulus() const
{ 
	return _modulus; 
}

inline 
void
modular_arithmetic::set_modulus(const integer& modulus)
{ 
	_modulus = modulus; 
	_result._reg.resize(_modulus._reg.size()); 
}

inline 
bool 
modular_arithmetic::equal(const integer& a, const integer& b) const
{ 
	return a == b; 
}

inline 
const integer&
modular_arithmetic::zero() const
{ 
	return integer::zero(); 
}

inline 
const integer&
modular_arithmetic::doubled(const integer& a) const
{ 
	return add(a, a); 
}

inline 
const integer&
modular_arithmetic::one() const
{ 
	return integer::one(); 
}

inline 
const integer&
modular_arithmetic::multiply(const integer& a, const integer& b) const
{ 
	return _result1 = a * b % _modulus; 
}

inline 
const integer&
modular_arithmetic::square(const integer& a) const
{ 
	return _result1 = a.squared() % _modulus; 
}

inline 
bool
modular_arithmetic::is_abs_one(const integer& a) const
{ 
	return integer::gcd(a, _modulus).is_abs_one(); 
}

inline 
const integer&
modular_arithmetic::multiplicative_inverse(const integer& a) const
{ 
	return _result1 = a.inverse_mod(_modulus); 
}

inline 
const integer&
modular_arithmetic::divide(const integer& a, const integer& b) const
{ 
	return multiply(a, multiplicative_inverse(b)); 
}

inline 
ub4_t
modular_arithmetic::bit_count() const
{ 
	return (_modulus - 1).bit_count(); 
}

inline 
ub4_t
modular_arithmetic::byte_count() const
{ 
	return (_modulus - 1).byte_count(); 
}

inline 
integer
modular_arithmetic::random_element(random_generator &rng , const int &ignore_for_now) const
{ 
	return integer(rng, integer(0) , _modulus - integer(1)); 
}

inline 
integer
montgomery_representation::convert_in(const integer& a) const
{
	return (a << (word_bits * (ub4_t)_modulus._reg.size())) % _modulus;
}

inline 
const integer&
montgomery_representation::one() const
{ 
	return _result1 = integer::power2(word_bits * (ub4_t)_modulus._reg.size()) % _modulus; 
}

inline
integer
montgomery_representation::cascade_exponentiate(const integer& x, const integer& e1, const integer& y, const integer& e2) const
{ 
	return abstract_ring< integer >::cascade_exponentiate(x, e1, y, e2); 
}

inline
void
montgomery_representation::simultaneous_exponentiate(integer *results, const integer& base, const integer *exponents, ub4_t exponentsCount) const
{
	abstract_ring<integer>::simultaneous_exponentiate(results, base, exponents, exponentsCount); 
}

//////////////////////////////////////////////////////////////////////
// window_slider
inline
void
window_slider::find_next_window()
{
	ub4_t expLen = _exp.word_count() * word_bits;
	ub4_t skipCount = _first_time ? 0 : _window_size;
	_first_time = false;
	while (!_exp.get_bit(skipCount))
	{
		if (skipCount >= expLen)
		{
			_finished = true;
			return;
		}
		skipCount++;
	}

	_exp >>= skipCount;
	_window_begin += skipCount;
	_exp_window = _exp % (1 << _window_size);

	if (_fast_negate && _exp.get_bit(_window_size))
	{
		_negate_next = true;
		_exp_window = (1 << _window_size) - _exp_window;
		_exp += _window_modulus;
	}
	else
		_negate_next = false;
}

//////////////////////////////////////////////////////////////////////////////////////
//! euclidean_domain_of
template < class T >
euclidean_domain_of< T >::euclidean_domain_of() 
{
}

template < class T >
bool
euclidean_domain_of< T >::equal(const T& a, const T& b) const 
{ 
	return a == b; 
}

template < class T >
const T&
euclidean_domain_of< T >::zero() const 
{ 
	return T::zero(); 
}

template < class T >
const T& 
euclidean_domain_of< T >::add(const T& a, const T& b) const 
{ 
	return _result = a + b; 
}

template < class T >
T& 
euclidean_domain_of< T >::accumulate(T& a, const T& b) const 
{ 
	return a += b; 
}

template < class T >
const T& 
euclidean_domain_of< T >::inverse(const T& a) const 
{ 
	return _result = -a; 
}

template < class T >
const T& 
euclidean_domain_of< T >::subtract(const T& a, const T& b) const 
{ 
	return _result = a - b; 
}

template < class T >
T& 
euclidean_domain_of< T >::reduce(T& a, const T& b) const 
{ 
	return a -= b; 
}

template < class T >
const T&
euclidean_domain_of< T >::doubled(const T& a) const 
{ 
	return _result = a.doubled(); 
}

template < class T >
const T& 
euclidean_domain_of< T >::one() const 
{ 
	return T::one(); 
}

template < class T >
const T& 
euclidean_domain_of< T >::multiply(const T& a, const T& b) const 
{ 
	return _result = a * b; 
}

template < class T >
const T& 
euclidean_domain_of< T >::square(const T& a) const 
{ 
	return _result = a.squared(); 
}

template < class T >
bool 
euclidean_domain_of< T >::is_abs_one(const T& a) const 
{ 
	return a.is_abs_one(); 
}

template < class T >
const T& 
euclidean_domain_of< T >::multiplicative_inverse(const T& a) const 
{ 
	return _result = a.multiplicative_inverse(); 
}

template < class T >
const T& 
euclidean_domain_of< T >::divide(const T& a, const T& b) const 
{ 
	return _result = a / b; 
}

template < class T >
const T& 
euclidean_domain_of< T >::mod(const T& a, const T& b) const 
{ 
	return _result = a % b; 
}

template < class T >
void 
euclidean_domain_of< T >::division_algorithm(T& r, T& q, const T& a, const T& d) const 
{ 
	T::divide(r, q, a, d); 
}

///////////////////////////////////////////////////////
template < class T > 
const T& 
abstract_euclidean_domain< T >::mod(const T& a, const T& b) const
{
	T q;
	division_algorithm(_result, q, a, b);
	return _result;
}

template < class T >
const T& 
abstract_euclidean_domain< T >::gcd(const T& a, const T& b) const
{
	T g[3] = {b, a};
	ub4_t i0 = 0, i1 = 1, i2 = 2;

	while (!equal(g[i1], this->zero()))
	{
		g[i2] = mod(g[i0], g[i1]);
		ub4_t t = i0; i0 = i1; i1 = i2; i2 = t;
	}

	return _result = g[i0];
}


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_integer_hpp_

