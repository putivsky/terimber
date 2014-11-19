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

#include "base/numeric.h"
#include "base/common.hpp"
#include "base/string.hpp"
#include "base/memory.hpp"
#include "base/number.hpp"
#include "base/list.hpp"
 
BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

inline 
size_t 
room_count(size_t x)
{
	size_t res;
	for (res = 1; x /= numeric::_radix; ++res);
	return res;
}


/////////////////////////////////////////////////////////////////////////////////
// static 
const numeric_radix	numeric::_radix = RADIX10;
////////////////
numeric::numeric(byte_allocator* allocator_) : 
	_vec(1, allocator_), _scale(0),  _precision(1), _sign(false)
{
	_vec[0] = 0;
}

numeric::~numeric()
{
}

numeric::numeric(ub1_t x, size_t length, byte_allocator* allocator_) :
	_vec(length, allocator_), _scale(0), _precision(length), _sign(false)
{
	_vec.fill(x, length);
}

numeric::numeric(const ub1_t* x, size_t length, byte_allocator* allocator_) :
	_vec(length, allocator_), _scale(0), _precision(length), _sign(false)
{
	_vec.copy(x, length);
}

numeric::numeric(sb4_t x, byte_allocator* allocator_) : 
	_vec(room_count(x), allocator_), _scale(0), _precision(0), _sign(x < 0)
{
	_precision = _vec.size();
	x *= _sign ? -1 : 1;
	
	for (size_t index = 0; (size_t)index < _precision; ++index)
	{
        _vec[index] = (ub1_t)(x % _radix);
		x /= _radix;
	}
}

numeric::numeric(sb8_t x, byte_allocator* allocator_) : 
	_vec(1, allocator_), _scale(0), _precision(1), _sign(false)
{
	_vec[0] = 0;
	char buf[128] = {0};
	size_t len = str_template::strprint(buf, 128, I64d, x);
	scan(buf, len, '.');
}

numeric::numeric(float x, byte_allocator* allocator_) : 
	_vec(1, allocator_), _scale(0), _precision(1), _sign(false)
{
	_vec[0] = 0;
	char buf[128] = {0};
	size_t len = str_template::strprint(buf, 128, "%hf", x);

	scan(buf, len, '.');
}

numeric::numeric(double x, byte_allocator* allocator_) : 
	_vec(1, allocator_), _scale(0), _precision(1), _sign(false)
{
	_vec[0] = 0;
	char buf[128] = {0};
	size_t len = str_template::strprint(buf, 128, "%f", x);

	scan(buf, len, '.');
}

numeric::numeric(const char *x, size_t len, numeric_radix radix, byte_allocator* allocator_) : 
	_vec(1, allocator_), _scale(0), _precision(0), _sign(false)
{

	_vec[0] = 0;

	if (!x || !*x)
		return;

	len = (len == os_minus_one) ? strlen(x) : len;

	if (strncmp("0x", x, 2) == 0)
		radix = RADIX16;

	for (size_t index = 0; index < len; ++index)
	{
		ub1_t digit = 0;
		if (!symbol_to_byte(digit, x[index], radix))
			continue;

		numeric res((sb4_t)digit, _vec.get_allocator());
		
		if (radix == RADIX10)
			*this << 1;
		else
			*this *= numeric((sb4_t)radix);

		*this += res;
	}

	_sign = x[0] == '-';
}

numeric::numeric(const char *x, size_t len, char delimeter, byte_allocator* allocator_) : 
	_vec(1, allocator_), _scale(0), _precision(0), _sign(false)
{
	scan(x, len, delimeter);
}

numeric::numeric(const wchar_t *x, size_t len, wchar_t delimeter, byte_allocator* allocator_) : 
	_vec(1, allocator_), _scale(0), _precision(0), _sign(false)
{
	scan(x, len, delimeter);
}

bool 
numeric::format(char* buf, char delimeter) const
{
	if (compare(*this, zero()) == 0)
	{
		*buf++ = '0';
		*buf = 0;
		return true;
	}

	if (_sign)
		*buf++ = '-';

	for (size_t index = _precision - 1; index >= 0; --index)
	{
		if (_scale != 0 && _scale == index + 1)
		{
			if (index == _precision - 1)
				*buf++ = '0';

			*buf++ = delimeter;
		}

		byte_to_symbol(*buf, _vec[index]); 
		++buf;
	}

	*buf = 0;
	return true;
}

bool 
numeric::persist(ub1_t* buf, size_t size) const
{
	// 1. x byte contains the sign
	// 2. next 4 bytes - scale
	// 3. next 4 bytes - precision
	// 4. the rest of bytes == precision - data
	size_t required = 1 + sizeof(size_t) * 2 + _precision;
	if (required > size)
		return false;

	// numeric keeps the reverse order
	buf[0] = _sign ? 1 : 0;
	memcpy(buf + 1, &_scale, sizeof(size_t));
	memcpy(buf + 1 + sizeof(size_t), &_precision, sizeof(size_t));

    for (size_t index = 0; index < _precision; ++index)
		buf[index + sizeof(size_t) * 2 + 1] = _vec[index];

	return true;
}

bool 
numeric::parse(const ub1_t* buf)
{
	// 1. x byte contains the sign
	// 2. next 4 bytes - scale
	// 3. next 4 bytes - precision
	// 4. the rest of bytes == precision - data
	if (!buf)
		return false;

	_sign = buf[0] != 0;
	memcpy(&_scale, buf + 1, sizeof(size_t));
	memcpy(&_precision, buf + 1 + sizeof(size_t), sizeof(size_t));
	
	if (_precision) 
		_vec.resize(_precision, true);

    for (size_t index = 0; index < _precision; ++index)
		_vec[index] = buf[index + sizeof(size_t) * 2 + 1];

	return true;
}

// oracle internal format
//
// oracle format
// [exponent]{base}[mantissa1][mantissa2]...[mantissaN]
// mantissa from 0 - 19 bytes
// examples:
// 0 -> [128]{0}
// 1 -> [193]{2}[0]
// 99 -> [193]{100}[0]
// 100 -> 01.0*pow(1) -> [194]{2}[0]
// 101 -> 01.01*pow(1) -> [194]{2}[2][0]
// 199 -> 01.99*pow(1) -> [194]{2}[100][0]
// 999 -> 09.99*pow(1) -> [194]{10}[100][0]
// 1000 -> 10*pow(1) ->[194]{11}[0]
// 1999 -> 19.99*pow(1) -> [194]{20}[100][0]
//
// -1 -> [62]{100}[102]
// -99 -> [62]{2}[102]
// -100 -> -1.0*pow(1) -> [61]{100}[102]
//

// 0.1 -> 0.10 -> 10*pow(-1) -> [192]{11}[0]
// 13.768 -> 13.7680 -> [193][14][77]{81}[0]
//
// -0.1 -> -0.10 ->10*pow(-1) -> [61]{91}[102]

// ebm1m2...mN
// e == 128 - just zero
// e >= 173 && e <= 213 - positive
// e >= 193 - >= 1
// e < 193 -> < 1
//
// algorithm 
// parser tmp numberic
// set it to zero x
// scans all oracle bytes and constructs mantissa - the exponent here does not matter
// sets scale and precision

bool 
numeric::parse_orcl(const ub1_t* buf)
{
	// checks pointer
	if (!buf)
		return false;
	
	bool negative = false;
	// checks exponent byte
	if (buf[0] == 128) // pure zero
	{
		*this = zero();
		return true;
	}
	else if (buf[0] >= 173 && buf[0] <= 213) // positive
		negative = false;
	else if (buf[0] >= 42 && buf[0] <= 82) // negative
		negative = true;
	else
		return false;


	sb1_t exponent = (negative ? ~buf[0] : buf[0]) - 193;
	// sets base here
	sb8_t base = negative ? 101 - buf[1] : buf[1] - 1;

	numeric tmp(base, _vec.get_allocator());

	size_t mantissa = 0;
	// looks for 0 tailing byte but no more than 20 bytes
	while (mantissa < 19 && 
		(!negative && buf[mantissa + 2] != 0 
		|| negative && buf[mantissa + 2] != 102)
		)
	{
		if (buf[mantissa + 2] > (100 + (negative ? 1 : 0)))
			return false;

		tmp << 2; // just move two magnitutes
		tmp += numeric(negative ? (sb4_t)(101 - buf[mantissa + 2]) : (sb4_t)(buf[mantissa + 2] - 1)); // add the current one
		++mantissa;
	}

	// adds exponent
	sb1_t shift = 0;
	if (exponent > 0 && (sb4_t)mantissa < exponent)
	{
		while (exponent > (sb4_t)mantissa)
		{
			tmp << 2;
			--exponent;
		}
	}
	else if (exponent < 0)
	{
		while (exponent < (sb4_t)(mantissa + shift))
		{
			tmp >> 2;
			--shift;
		}
	}

	// sets mantissa scale
	tmp._scale = (mantissa - exponent) * 2;
	if (tmp._precision < tmp._scale)
		tmp._precision = tmp._scale;

	if (negative)
		tmp.negative();

	tmp.cutjunk();
	*this = tmp;
	return true;
}

// static 
int 
numeric::compare_orcl(const ub1_t* x, const ub1_t* y)
{
	if (!x)
		return (y != 0 ? -1 : 0);
	else if (!y)
		return 1;

	// compares
	int xsign = 0, ysign = 0;

	// checks exponent byte
	if (x[0] == 128) // pure zero
		xsign = 0;
	else if (x[0] >= 173 && x[0] <= 213) // positive
		xsign = 1;
	else if (x[0] >= 42 && x[0] <= 82) // negative
		xsign = -1;
	else
	{
		assert(false);
	}

	if (y[0] == 128) // pure zero
		ysign = 0;
	else if (y[0] >= 173 && y[0] <= 213) // positive
		ysign = 1;
	else if (y[0] >= 42 && y[0] <= 82) // negative
		ysign = -1;
	else
	{
		assert(false);
	}

	// makes simple comparision here
	if (!xsign && !ysign)
		return 0;
	else if (xsign < ysign)
		return -1;
	else if (ysign < xsign)
		return 1;

	sb1_t xexponent = (xsign < 0 ? ~x[0] : x[0]) - 193;
	sb1_t yexponent = (ysign < 0 ? ~y[0] : y[0]) - 193;

	if (xexponent < yexponent)
		return xsign > 0 ? -1 : 1;
	else if (yexponent < xexponent)
		return xsign < 0 ? -1 : 1;

	// sets base here
	sb8_t xbase = xsign < 0 ? 101 - x[1] : x[1] - 1;
	sb8_t ybase = ysign < 0 ? 101 - y[1] : y[1] - 1;

	if (xbase < ybase)
		return xsign > 0 ? -1 : 1;
	else if (ybase < xbase)
		return xsign < 0 ? -1 : 1;

	size_t xmantissa = 0, ymantissa = 0;
	// looks for 0 tailing byte but no more than 20 bytes
	while (xmantissa < 19 && 
		(xsign > 0 && x[xmantissa + 2] != 0 
		|| xsign < 0 && x[xmantissa + 2] != 102)

		&& 
		ymantissa < 19 && 
		(ysign > 0 && y[ymantissa + 2] != 0 
		|| ysign < 0 && y[ymantissa + 2] != 102)
		)
	{
		assert(x[xmantissa + 2] <= (100 + (xsign < 0 ? 1 : 0)));
		assert(y[ymantissa + 2] <= (100 + (ysign < 0 ? 1 : 0)));

		sb4_t xnext = xsign < 0 ? (sb4_t)(101 - x[xmantissa + 2]) : (sb4_t)(x[xmantissa + 2] - 1); // add the current one
		sb4_t ynext = ysign < 0 ? (sb4_t)(101 - y[ymantissa + 2]) : (sb4_t)(y[ymantissa + 2] - 1); // add the current one

		if (xnext < ynext)
			return xsign > 0 ? -1 : 1;
		else if (ynext < xnext)
			return xsign < 0 ? -1 : 1;

		++xmantissa;
		++ymantissa;
	}

	if (ymantissa < 19 && 
		(ysign > 0 && y[ymantissa + 2] != 0 
		|| ysign < 0 && y[ymantissa + 2] != 102)
		)
		return -1;

	if (xmantissa < 19 && 
		(xsign > 0 && x[xmantissa + 2] != 0 
		|| xsign < 0 && x[xmantissa + 2] != 102)
		)
		return 1;

	return 0;
}

// internal numeric format
// reverse byte order
// mantissa m
// precision p
// scale s
// sign n
// 0 -> [0], p=1, s=0, n = 0 --> [128][0]
// 1 -> [1], p=1, s=0, n = 0 --> [193][2][0]
// 99 -> [9][9], p=2, s=0, n = 0 --> [193][100][0]
// 100 -> [0][0][1], p=3, s=0, n = 0 --> [194][2][0]
// 101 -> [1][0][1], p=3, s=0, n = 0 --> [194][2][2][0]
// 199 -> [9][9][1], p=3, s=0, n = 0 --> [194][2][100][0] 
// 999 -> [9][9][9], p=3, s=0, n = 0 --> [194][10][100][0]
// 1000 -> [0][0][0][1], p=4, s=0, n = 0 --> [194][11][0]
// 1999 -> [9][9][9][1], p=4, s=0, n = 0 --> [194][20[100][0]
//
// 
// -1 -> m[1], p=1, s=0, n = 1 --> [62][100][102]
// -99 -> m[9][9], p=2, s=0, n = 1 --> [62][2][102]

// 0.001 [1][0][0], p = 3, s=3, n=0 --> 0.0010 --> [192][11][0]
// 0.0001 [1][0][0][0] p = 4, s=4, n=0 --> 0.0001 --> [192][2][0]
// 1.001 [1][0][0][1] p = 4, s=3, n=0 --> 01.0010 --> [193][2][1][11][0]
// 10.0001 [1][0][0][0][0][1] p = 6, s=4, n=0 -> 10.0001 --> [193][11][1][2][0]

// 1. move decimal point if precision == scale --> 0.0001
// 2. move decimal point, skipping tens, hundreds, and so on --> 1000


bool 
numeric::persist_orcl(ub1_t* buf) const
{
	// buf should have 22 bytes
	// the last byte will be set to sezo
	if (_precision > 38)
		return false;

	if (is_zero())
	{
		buf[0] = 128;
		buf[1] = 0;
		return true;
	}

	// oracle 100-based format
	// NUMBER = base.mantissa * pow(100, exponent);
	// where 0 < base < 100, mantissa has even digits
	// 0.00001 --> 10.00 * pow(100, -3);
	// 1234.1233 --> 12.341233 * pow(100, 1);
	// 1234.12337 --> 1234.123370 --> 12.34123370 * pow(100, 1);
	// 1222820 --> 1.222820 * pow(100, 3);
	// 12228207 --> 12.228207 * pow(100, 3);
	// notation
	// e(?) - exponent [-38, 38]
	// b{?} - base		[1, 99]
	// m[?] - mantissa  [1, 99]
	// t<?> - tail byte {0, 102}
	// byte sequence is e(?)b{?}m[?]m[?]m[?]m[?]m[?]t<?>

	size_t scale_moved = 0;
	size_t tail_zero = 0;
	size_t precision_moved = 0;

	if (!_scale) // 12300
	{
		// no changes
		precision_moved = _precision;
	}
	else if (_precision != _scale)// 1120.00123
	{
		tail_zero = _scale % 2;
		scale_moved = _scale + tail_zero;
		// no changes
		precision_moved = _precision;
	}
	else // 0.001234
	{
		assert(_precision == _scale);

		tail_zero = _scale % 2;	
		scale_moved = _scale + tail_zero;

		// skips zeros after decimal point
		precision_moved = _precision;
		while (!_vec[precision_moved - 1] && precision_moved)
		{
			--precision_moved;
		}
	}

	bool odd_base = (precision_moved + tail_zero) % 2 != 0;
	
	assert(scale_moved % 2 == 0);
	// exponent finder
	sb1_t exponent = (sb1_t)(precision_moved + tail_zero - (odd_base ? 0 : 1) - scale_moved) / 2;

	// assigns base
	sb1_t base = odd_base ? _vec[precision_moved - 1] :
			_vec[precision_moved - 1] *10 + _vec[precision_moved - 2];

	if (sign())
		buf[1] = 101 - base;
	else
		buf[1] = base + 1;

	// sets exponent
	buf[0] = sign() ? (~exponent - 193) : (exponent + 193);

	// place to stop writing mantissa
	size_t mantissa_upper_index = precision_moved - (odd_base ? 1 : 2);
	size_t mantissa_lower_index = tail_zero;

	assert((mantissa_upper_index - mantissa_lower_index) % 2 == 0);

	size_t length_mantissa = (mantissa_upper_index - mantissa_lower_index) / 2;

	// mantissa is always even
	for (size_t index = 0; index < length_mantissa; ++index)
	{
		ub1_t result = _vec[2*index + mantissa_lower_index] + _vec[2*index + 1 + mantissa_lower_index] * 10;				
		buf[length_mantissa - index  + 1] = (sign() ? 101 - result : result + 1);
	}

	if (tail_zero)
	{
		++length_mantissa;
		ub1_t result = _vec[0] * 10;				
		buf[length_mantissa + 1] = (sign() ? 101 - result : result + 1);
	}

	if (length_mantissa < 19)
		buf[length_mantissa + 2] = (sign() ? 102 : 0); // set tailing byte

	return true;
}

size_t 
numeric::orcl_len() const
{
	if (is_zero())
	{
		return 2;
	}
	else
	{
		bool odd_scale = _scale % 2 != 0;
		size_t scale = _scale + (odd_scale ? 1 : 0);
		size_t precision = _precision + (odd_scale ? 1 : 0);
		bool odd_precision = precision % 2 != 0; 	
		size_t mantissa = precision / 2;
		return mantissa + 2 + (odd_precision ? 1 : 0);
	}
}

bool 
numeric::persist_sql(ub1_t& sign, ub1_t* buf, ub1_t& precision, sb1_t& scale) const
{
	// lowers ending heximal representation
	numeric value = *this;
	value << (int)_scale;
	numeric divider((sb4_t)256);

	size_t index = 0;
	while (!value.is_zero())
	{
		numeric res = value % divider;
		buf[index] = res._vec[0] + 10 * res._vec[1] + 100 * res._vec[2];
		value /= divider;
		++index;
	}

	sign = _sign ? 0 : 1;
	precision = (ub1_t)_precision;
	scale = (ub1_t)_scale;

	return true;
}

bool 
numeric::parse_sql(ub1_t sign, const ub1_t* buf, ub1_t precision, sb1_t scale)
{
	// lowers ending heximal representation
	size_t index = 0;
	numeric tmp = zero();
	numeric power((sb4_t)1);
	numeric multiplier((sb4_t)256);

	while (buf[index])
	{
		tmp += power * numeric((sb4_t)buf[index]);
		power *= multiplier;
		++index;
	}

	//tmp._precision = precision;
	tmp._scale = scale;
	tmp._sign = sign == 0;

	*this = tmp;
	return true;
}


numeric& 
numeric::negative()
{
	_sign = true;
	return *this;
}

bool 
numeric::sign() const
{
	return _sign;
}

size_t 
numeric::scale() const
{
	return _scale;
}

size_t 
numeric::precision() const
{
	return _precision;
}

// static 
const numeric& 
numeric::zero()
{
	const static numeric s_zero;
	return s_zero;
}

// static 
const numeric& 
numeric::one()
{
	const static numeric s_one((sb4_t)1);
	return s_one;
}

bool 
numeric::is_zero() const
{
	for (size_t index  = 0; index < _precision; ++index)
		if (_vec[index])
			return false;

	return true;
}

numeric& 
numeric::operator++()
{
	return *this += one();
}

numeric 
numeric::operator++(int)
{
	numeric tmp(*this);
	*this += one();
	return tmp;
}

numeric& 
numeric::operator--()
{
	return *this -= one();
}

numeric 
numeric::operator--(int)
{
	numeric tmp(*this);
	*this -= one();
	return tmp;
}

numeric& 
numeric::operator+=(const numeric& x)
{
	numeric res(_vec.get_allocator());
	if (!_sign) // this >= 0
	{
		if (!x._sign) // x >= 0
			*this = plus(res, *this, x);
		else // x < 0
			if (compare(*this, x) < 0) // this < x
			{
				*this = minus(res, x, *this);
				_sign = true; // this < 0
			}
			else // this >= x
				*this = minus(res, *this, x); // this >= 0;
	}
	else // this < 0
	{
		if (x._sign) // x < 0
			*this = plus(res, *this, x);
		else // x >= 0
			if (compare(*this, x) <= 0) // this <= x
			{
				*this = minus(res, x, *this);
				_sign = false; // this >= 0
			}
			else // this > x
				*this = minus(res, *this, x); // this < 0;	
	}

	return cutjunk();
}

numeric& 
numeric::operator-=(const numeric& x)
{
	numeric res(_vec.get_allocator());
	if (!_sign) // this >= 0
	{
		if (x._sign) // x < 0
			*this = plus(res, *this, x);
		else // x >= 0
			if (compare(*this, x) < 0) // this < x
			{
				*this = minus(res, x, *this);
				_sign = true; // this < 0
			}
			else // this >= x
				*this = minus(res, *this, x); // this >= 0;
	}
	else // this < 0
	{
		if (!x._sign) // x >= 0
			*this = plus(res, *this, x);
		else // x < 0
			if (compare(*this, x) <= 0) // this <= x
			{
				*this = minus(res, x, *this);
				_sign = false; // this >= 0
			}
			else // this > x
				*this = minus(res, *this, x); // this < 0;	
	}

	return cutjunk();
}

numeric& 
numeric::operator*=(const numeric& x)
{
	numeric res(_vec.get_allocator());
	*this = multiply(res, *this, x);
	_sign ^= x._sign;
	return cutjunk();
}

numeric& 
numeric::operator/=(const numeric& x)
{
	numeric res(_vec.get_allocator());
	numeric reminder(_vec.get_allocator());
	*this = divide(res, reminder, *this, x);
	_sign ^= x._sign;
	return cutjunk();
}

numeric& 
numeric::operator%=(const numeric& x)
{
	numeric res(_vec.get_allocator());
	numeric reminder(_vec.get_allocator());
	divide(res, reminder, *this, x);
	*this = reminder;
	_sign ^= x._sign;
	return cutjunk();
}

numeric::numeric(const numeric& x) :
	_vec(x._vec), _scale(x._scale), _precision(x._precision), _sign(x._sign) 
{
}

numeric& 
numeric::operator=(const numeric& x)
{
	if (this != &x)
	{
		_vec = x._vec;
		_sign = x._sign;
		_precision = x._precision;
		_scale = x._scale;
	}
	return *this;
}

numeric& 
numeric::cutjunk()
{
	// corrects tailing zeros
	size_t head = (_precision) ? _precision - 1 : 0;
	for (; head >= _scale;)
		if (_vec[head])
			break;
		else
            --_precision, --head;

	size_t tail = 0;
	for (; tail < _scale;)
		if (_vec[tail])
			break;
		else
			++tail;

	if (tail)
	{
		_vec >> tail;
		_precision -= tail;
		_scale -= tail;
	}

	return *this;
}

numeric& 
numeric::operator<<(int shift)
{
	if (shift)
	{
		if (shift < 0)
			return *this >> -shift;

		int decr = __min(shift, (int)_scale);
		shift -= decr;
		_scale -= decr;

		if (shift)
		{
			_precision += shift;
			_vec.reserve(_precision);
			_vec << shift;
		}
	}

	return *this;
}

numeric& 
numeric::operator>>(int shift)
{
	if (shift)
	{
		if (shift < 0)
			return *this << shift;


		_scale += (size_t)shift;

		if (_precision < _scale)
			_precision = _scale;


		_vec.reserve(_precision);
	}

	return *this;
}

bool 
numeric::operator==(const numeric& x)
{
	return _sign == x._sign && compare(*this, x) == 0;
}

bool 
numeric::operator!=(const numeric& x)
{
	return !(*this == x);
}

bool 
numeric::operator<(const numeric& x)
{
	return _sign && !x._sign || !(_sign ^ x._sign) && compare(_sign ? x : *this, _sign ? *this : x) < 0;
}

bool 
numeric::operator<=(const numeric& x)
{
	return _sign && !x._sign || !(_sign ^ x._sign) && compare(_sign ? x : *this, _sign ? *this : x) <= 0;
}

bool 
numeric::operator>(const numeric& x)
{
	return !(*this <= x);
}

bool 
numeric::operator>=(const numeric& x)
{
	return !(*this < x);
}

// static functions
////////////////////////////////////////////////////////////////////////
// static 
numeric&
numeric::plus(numeric& res, const numeric& x, const numeric& y)
{
	size_t x_whole = x._precision - x._scale;
	size_t y_whole = y._precision - y._scale;

	int scale = (int)__max(x._scale, y._scale);
	size_t precision = scale + __max(x_whole, y_whole);
	
	res._vec.reserve(precision + 1); // room for carry
	res._precision = precision;
	res._scale = scale;

	ub1_t carry = 0;

	// sum scale
	int shift = (int)x._scale - (int)y._scale;
	for (int iscale = 0; iscale < (int)scale; ++iscale)
	{
		ub1_t sum = carry;
		if (shift > 0) // x scale is bigger
			sum += x._vec[iscale] + (iscale >= shift ? y._vec[iscale - shift] : 0);
		else if (shift < 0) // y scale is bigger
			sum += y._vec[iscale] + (iscale >= -shift ? x._vec[iscale + shift] : 0);
		else // equal scales
			sum += x._vec[iscale] + y._vec[iscale];

		res._vec[iscale] = sum % _radix;
		carry = sum / _radix;
	}

	// sum whole part
	for (int iprecision = scale; iprecision < (int)precision; ++iprecision)
	{
		ub1_t sum = carry;
		if (iprecision - (int)scale < (int)(x._precision - x._scale))
			sum += x._vec[iprecision - scale + x._scale];
		if (iprecision - (int)scale < (int)(y._precision - y._scale))
			sum += y._vec[iprecision - scale + y._scale];

		res._vec[iprecision] = sum % _radix;
		carry = sum / _radix;
	}

	if (carry)
	{
		res._vec[precision] = carry;
		++res._precision;
	}

	return res;
}

// static 
numeric& 
numeric::minus(numeric& res, const numeric& x, const numeric& y)
{
	size_t x_whole = x._precision - x._scale;
	size_t y_whole = y._precision - y._scale;

	size_t scale = __max(x._scale, y._scale);
	assert(x_whole >= y_whole);
	size_t precision = x_whole + scale;
	
	res._vec.reserve(precision); // room for carry
	res._precision = precision;
	res._scale = scale;

	ub1_t borrow = 0;

	// sub scale
	int shift = (int)x._scale - (int)y._scale;
	for (int iscale = 0; iscale < (int)scale; ++iscale)
	{
		sb1_t sub = 0;
		if (shift > 0) // x scale is bigger
			sub = x._vec[iscale] - (iscale >= shift ? y._vec[iscale - shift] : 0) - borrow;
		else if (shift < 0) // y scale is bigger
			sub = (iscale >= -shift ? x._vec[iscale + shift] : 0) - y._vec[iscale] - borrow;
		else // equal scales
			sub = x._vec[iscale] - y._vec[iscale] - borrow;

		res._vec[iscale] = sub >= 0 ? sub : _radix + sub;
		borrow = sub >= 0 ? 0 : 1;
	}

	// sub whole part
	for (int iprecision = (int)scale; iprecision < (int)precision; ++iprecision)
	{
		sb1_t sub = 0;
		if (iprecision - (int)scale < (int)(x._precision - x._scale))
			sub += x._vec[iprecision - scale + x._scale];
		if (iprecision - (int)scale < (int)(y._precision - y._scale))
			sub -= y._vec[iprecision - scale + y._scale];

		sub -= borrow;

		res._vec[iprecision] = sub >= 0 ? sub : _radix + sub;
		borrow = sub >= 0 ? 0 : 1;
	}


	assert(borrow == 0);
	return res;
}

// static 
numeric& 
numeric::multiply(numeric& res, const numeric& x, const numeric& y)
{
	// estimates the max size for result
	size_t max_room = x._precision * y._precision + 1;
	// allocates room for results
	res._vec.reserve(max_room);

	ub1_t mult = 0;
	res._vec.fill(0, max_room);
	res._precision = 0;
	res._scale = 0;

	// workspace for row mupliplication
	numeric row((ub1_t)0, max_room, res._vec.get_allocator()); // room for carry

	for (int iy = 0; iy < (int)y._precision; ++iy, ++mult)
	{
		// clears each time
		row._vec.fill(0, max_room);
		row._precision = x._precision;
		row._scale = 0;
		ub1_t carry = 0;
        
		for (int ix = 0; ix < (int)x._precision; ++ix)
		{
			// multiplies digits
			ub1_t sum = x._vec[ix] * y._vec[iy];
			// adds to the temp result
			sum += carry;
			// assigns value
			row._vec[ix] = sum % _radix;
			// saves carry
			carry = sum / _radix;
		}

		if (carry)
		{
			// sets carry
			row._vec[x._precision] = carry;
			// increases precision
			++row._precision;
		}

		// correction for power
		row._vec << mult;
		row._precision += mult;
		// adds to final result
		res += row;
	}

	// sets the final precision and scale
	res._scale = x._scale + y._scale;
	if (res._precision < res._scale)
		res._precision = res._scale;
	return res;
}

// the result will keep the scale as a max of dividend and divisor scales
// user can extend the scale explitily
// static 
numeric& 
numeric::divide(numeric& res, numeric& reminder, const numeric& dividend, const numeric& divider)
{
	reminder = zero();

	if (divider.is_zero())
		exception::_throw("Divided by zero");

	if (dividend.is_zero())
	{
		res = zero();
		return res;
	}


	// normalize
	// xxx.yyyyyyyy
	// zzzzzzzzz.www
	// 1. make x > z
	// xxxyyyyyy.yy
	// zzzzzzzzz.www
	// 2. skip scale
	// xxxyyyyyyyy0.
	// zzzzzzzzzwww.
	// 3. add scale max(z.scale, x.scale)
	// xxxyyyyyyyy000000000.
	// zzzzzzzzzwww.


	int initial_scale = 0;

	numeric dividend_(dividend);
	numeric divider_(divider);
	dividend_.cutjunk();
	divider_.cutjunk();

	int dividend_scale = (int)dividend_._scale;
	int divider_scale = (int)divider_._scale;
	int max_scale = __max(dividend_scale, divider_scale);

	dividend_ << 2 * max_scale - dividend_scale;
	divider_ << max_scale - divider_scale;

	// moves divider
	while (compare(dividend_, divider_) > 0)
	{
		if (divider_._scale)
			--divider_._scale;
		else
			divider_._vec.reserve(++divider_._precision) << 1;

		--initial_scale;
	}

	// moves dividend
	if (initial_scale < 0 && compare(dividend_, divider_) < 0)
	{
		if (divider_._scale)
		{
			++divider_._scale;
			++initial_scale;
		}
		else
			divider_._vec.reserve(--divider_._precision) >> 1;
	}


	res = zero();

	// divides as integers
	int rounds = initial_scale;
	numeric circles(res._vec.get_allocator());

	while (rounds < 0)
	{
		res._vec.reserve(++res._precision) << 1;

		circles = zero();
		while (compare(dividend_, divider_) >= 0)
		{
			++circles;
			dividend_ -= divider_;
		}

		res += circles;
		divider_._vec >> 1;
		--divider_._precision;
		++rounds;
	}

	reminder = dividend_.cutjunk();
	res._vec.reserve(res._scale = (max_scale - dividend_scale + divider_scale));
	if (res._precision < res._scale)
		res._precision = res._scale;
	return res;
}

// static 
int 
numeric::compare(const numeric& x, const numeric& y)
{
	int x_whole = (int)x._precision - (int)x._scale;
	int y_whole = (int)y._precision - (int)y._scale;

	if (x_whole > y_whole)
		return 1;
	else if (x_whole < y_whole)
		return -1;
	else
	{
		for (int index = x_whole - 1; index >= 0; --index)
		{
			if (x._vec[index + x._scale] == y._vec[index + y._scale])
				continue;
			else 
				return x._vec[index + x._scale] > y._vec[index + y._scale] ? 1 : -1;
		}
	}

	
	// whole parts are equal
	int min_fraction = (int)__min(x._scale, y._scale);
	
	for (int index = min_fraction - 1; index >= 0; --index)
	{
		if (x._vec[index + x._scale - min_fraction] == y._vec[index + y._scale - min_fraction])
			continue;
		else 
			return x._vec[index + x._scale - min_fraction] > y._vec[index + y._scale - min_fraction] ? 1 : -1;
	}

	if (x._scale == y._scale)
		return 0;
	else
		return x._scale > y._scale ? 1 : -1;
}


///////////////////////////////////////////////////////////////////
numeric operator+(const numeric& x, const numeric& y)
{
	numeric res(x);
	res += y;
	return res.cutjunk();
}

numeric operator-(const numeric& x, const numeric& y)
{
	numeric res(x);
	res -= y;
	return res.cutjunk();
}

numeric operator*(const numeric& x, const numeric& y)
{
	numeric res(x);
	res *= y;
	return res.cutjunk();
}

numeric operator/(const numeric& x, const numeric& y)
{
	numeric res(x);
	res /= y;
	return res.cutjunk();
}

numeric operator%(const numeric& x, const numeric& y)
{
	numeric res(x);
	res %= y;
	return res.cutjunk();
}

//////////////////////////////////////////////
numeric&
numeric::scan(const char* x, size_t len, char delimeter)
{
	_vec[0] = 0;
	_sign = false;
	_precision = 1;
	_scale = 0;
	
	if (!x || !*x)
		return *this;;

	bool after_delimeter = false;
	numeric divider((sb4_t)1, _vec.get_allocator());

	for (size_t index = 0; index < len; ++index)
	{
		ub1_t digit = 0;

		if (symbol_to_byte(digit, x[index], RADIX16))
			;
		else if (x[index] == delimeter && !after_delimeter)
		{
			after_delimeter = true;
			continue;
		}
		else
			continue;

		if (!after_delimeter)
		{
			*this << 1;
			*this += numeric((sb4_t)digit);
		}
		else // scale
		{
			if (divider._scale == divider._precision)
				divider._vec.reserve(++divider._precision);		
			++divider._scale;

			numeric fraction((sb4_t)digit, _vec.get_allocator());
			fraction *= divider;
			*this += fraction;
		}
	}

	_sign = (ub1_t)x[0] == '-';
	cutjunk();
	return *this;
}

numeric&
numeric::scan(const wchar_t* x, size_t len, wchar_t delimeter)
{
	_vec[0] = 0;
	_sign = false;
	_precision = 1;
	_scale = 0;
	
	if (!x || !*x || !len)
		return *this;

	bool after_delimeter = false;
	numeric divider((sb4_t)1, _vec.get_allocator());

	for (size_t index = 0; index < len; ++index)
	{
		ub1_t digit = 0;

		if (symbol_to_byte(digit, x[index], RADIX16))
			;
		else if (x[index] == delimeter && !after_delimeter)
		{
			after_delimeter = true;
			continue;
		}
		else
			continue;

		if (!after_delimeter)
		{
			*this << 1;
			*this += numeric((sb4_t)digit);
		}
		else // scale
		{
			if (divider._scale == divider._precision)
				divider._vec.reserve(++divider._precision);		
			++divider._scale;

			numeric fraction((sb4_t)digit, _vec.get_allocator());
			fraction *= divider;
			*this += fraction;
		}
	}

	_sign = (ub1_t)x[0] == '-';
	return *this;
}


#pragma pack()
END_TERIMBER_NAMESPACE

