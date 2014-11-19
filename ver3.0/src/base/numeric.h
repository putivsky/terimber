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

#ifndef _terimber_numeric_h_
#define _terimber_numeric_h_

#include "base/common.h"
#include "base/number.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//! \class numeric 
//! \brief float number without restrictions on precision and scale
class numeric
{
public:
	//! \brief default constructor
	numeric(		byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief destructor
	~numeric();
	//! \brief constructor
	explicit 
	numeric(		sb8_t x,								//!< 64 bits integer
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief constructor
	explicit 
	numeric(		sb4_t x,								//!< 32 bits integer
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief constructor
	explicit 
	numeric(		float x,								//!< 32 bits float
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief constructor
	explicit 
	numeric(		double x,								//!< 64 bits float
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief constructor
	explicit 
	numeric(		const char* x,						//!< input string, no delimeters
					size_t len,								//!< string length
					numeric_radix radix = RADIX10,			//!< radix
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief constructor
	explicit 
	numeric(		const char* x,						//!< input string
					size_t len,								//!< string length
					char delimeter,							//!< delimeter '.', ',', etc
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief constructor
	explicit 
	numeric(		const wchar_t* x,						//!< input wide string
					size_t len,								//!< string length
					wchar_t delimeter,						//!< delimeter L'.', L',', etc
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief copy constructor
	numeric(const numeric& x);
	//! \brief assign operator
	numeric& operator=(const numeric& x);
	//! \brief operator++
	numeric& operator++();
	//! \brief operator++(int)
	numeric operator++(int);
	//! \brief operator--
	numeric& operator--();
	//! \brief operator--(int)
	numeric operator--(int);
	//! \brief operator+=
	numeric& operator+=(const numeric& x);
	//! \brief operator-=
	numeric& operator-=(const numeric& x);
	//! \brief operator *=
	numeric& operator*=(const numeric& x);
	//! \brief operator /=
	numeric& operator/=(const numeric& x);
	//! \brief operator %=
	numeric& operator%=(const numeric& x);
	//! \brief equal operator
	bool operator==(const numeric& x);
	//! \brief non equal operator
	bool operator!=(const numeric& x);
	//! \brief operator<
	bool operator<(const numeric& x);
	//! \brief operator <=
	bool operator<=(const numeric& x);
	//! \brief operator>
	bool operator>(const numeric& x);
	//! \brief operator >=
	bool operator>=(const numeric& x);
	//! \brief get sign, return true for negative numbers
	bool sign() const;
	//! \brief get scale
	size_t scale() const;
	//! \brief check if number is zero
	bool is_zero() const;
	//! \brief get precision
	size_t precision() const;
	//! \brief output numeric using provided buffer and delimeter
	//! buffer has to have enough room to accomodate all bytes
	//! precision + 1 (delimeter) + 1 (sign) + 1 (leading zero)
	bool format(char* buf, char delimeter = '.') const; 
	//! \brief output numeric in binary format
	bool persist(ub1_t* buf, size_t size) const;
	//! \brief parses binary buffer extracting numeric
	bool parse(const ub1_t* buf); 
	//! \brief outputs numeric in binary oracle like format NUMERIC type
	bool persist_orcl(ub1_t* buf) const;
	//! \brief parses external oracle like binary buffer extracting numeric
	bool parse_orcl(const ub1_t* buf);
	//! \brief outputs numeric in binary ms sql like format SQL_NUMERIC type
	bool persist_sql(ub1_t& sign, ub1_t* buf, ub1_t& precision, sb1_t& scale) const;
	//! \brief parses external ms sql like binary buffer extracting numeric
	bool parse_sql(ub1_t sign, const ub1_t* buf, ub1_t precision, sb1_t scale);
	//! \brief gets the minimum length for oracle output
	size_t orcl_len() const;
	//! \brief operator+
	friend numeric operator+(const numeric& x, const numeric& y);
	//! \brief operator-
	friend numeric operator-(const numeric& x, const numeric& y);
	//! \brief operator*
	friend numeric operator*(const numeric& x, const numeric& y);
	//! \brief operator/
	friend numeric operator/(const numeric& x, const numeric& y);
	//! \brief operator%
	friend numeric operator%(const numeric& x, const numeric& y);
	//! \brief static zero
	static 
	const numeric& 
	zero();
	//! \brief static one
	static 
	const 
	numeric& 
	one();

	//! \brief compares binaries in oracle format
	static 
	int 
	compare_orcl(	const ub1_t* x,							//!< first argument
					const ub1_t* y							//!< second argument
					);

private:
	//! \brief returns x+y
	static 
	numeric& 
	plus(			numeric& res,							//!< result
					const numeric& x,						//!< first argument
					const numeric& y						//!< second argument
					);
	//! \brief returns x-y
	static 
	numeric& 
	minus(			numeric& res,							//!< result
					const numeric& x,						//!< first argument
					const numeric& y						//!< second argument
					);
	//! \brief returns x*y
	static 
	numeric& 
	multiply(		numeric& res,							//!< result
					const numeric& x,						//!< first argument
					const numeric& y						//!< second argument
					);
	//! \brief returns x/y
	static 
	numeric& 
	divide(			numeric& res,							//!< result
					numeric& reminder,						//!< reminder
					const numeric& dividend,				//!< dividend
					const numeric& divider					//!< divider
					);
	//! \brief compares x, y
	static 
	int 
	compare(		const numeric& x,						//!< first argument
					const numeric& y						//!< second argument
					);
	//! \brief private constructor
	numeric(		ub1_t x,								//!< input value
					size_t length,							//!< reserved length
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief private constructor
	numeric(		const ub1_t* x,						//!< input buffer
					size_t length,							//!< buffer length
					byte_allocator* allocator_ = 0			//!< optional external allocator
					);
	//! \brief makes numeric negative
	numeric& 
	negative();
	//! \brief removes unused tailing or leading zeroes
	numeric& 
	cutjunk();
	//! \brief rounds up to the scale
	numeric& 
	round(			size_t scale_							//!< scale
					);
	//! \brief shifts buffer to the left
	numeric& 
	operator<<(int shift);
	//! \brief shifts buffer to the right
	numeric& 
	operator>>(int shift);
	//! \brief scans external string with specified delimeter
	numeric& 
	scan(			const char *x,						//!< input string
					size_t len,								//!< string length
					char delimeter							//!< delimeter
					);
	//! \brief scans external wide string with specified delimeter
	numeric& 
	scan(			const wchar_t *x,						//!< input wide string
					size_t len,								//!< string length
					wchar_t delimeter						//!< delimeter
					);
public:
    static const numeric_radix			_radix;				//!< radix 10

private:
	room_array< ub1_t >					_vec;				//!< internal buffer
	size_t								_scale;				//!< scale
	size_t								_precision;			//!< precision
	bool								_sign;				//!< sign, true means negative
};

//! \brief global operator+
numeric 
operator+(			const numeric& x,						//!< first argument
					const numeric& y						//!< second argument
					);
//! \brief global operator-
numeric 
operator-(			const numeric& x,						//!< first argument
					const numeric& y						//!< second argument
					);
//! \brief global operator*
numeric 
operator*(			const numeric& x,						//!< first argument
					const numeric& y						//!< second argument
					);
//! \brief global operator/
numeric 
operator/(			const numeric& x,						//!< first argument
					const numeric& y						//!< second argument
					);
//! \brief global operator%
numeric 
operator%(			const numeric& x,						//!< first argument
					const numeric& y						//!< second argument
					);


#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_numeric_h_

