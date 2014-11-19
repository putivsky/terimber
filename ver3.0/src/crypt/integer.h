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

#ifndef _terimber_integer_h_
#define _terimber_integer_h_

#include "base/vector.h"
#include "base/number.h"
#include "crypt/arithmet.h"


BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

class random_generator
{
public:
	random_generator();
	//!
	virtual ~random_generator() {}

	//! generates new random ub1_t and returns it
	virtual ub1_t generate_byte();

	//! generates new random bit and returns it
	virtual ub4_t generate_bit();

	//! generates a random 32 bit ub4_t in the range min to max, inclusive
	virtual ub4_t generate_dword(ub4_t a = 0, ub4_t b = 0xffffffffL);

	//! generates random array of bytes
	virtual void generate_block(ub1_t *output, ub4_t size);

	//! randomly shuffles the specified array. The resulting permutation is uniformly distributed
	template < class IT > void shuffle(IT begin, IT end)
	{
		for (; begin != end; ++begin)
			std::iter_swap(begin, begin + generate_dword(0, end - begin - 1));
	}

	inline ub1_t get_byte();
	inline ub4_t get_bit();
	inline ub4_t get_dword(ub4_t a = 0, ub4_t b = 0xffffffffL);
	inline ub2_t get_word(ub2_t a = 0, ub2_t b = 0xffff);
	inline void get_block(ub1_t *output, ub4_t size);
};

class integer
{
public:
	//! creates the zero integer
	integer();

	//! copy constructor
	integer(const integer& x);

	//! converts from int32_t
	integer(int value);

	//! converts from string
	/*! str can be in base 2, 8, 10, or 16.  Base is determined by a
		case insensitive suffix of 'h', 'o', or 'b'.  No suffix means base 10.
	*/
	integer(const char *str);

	//! converts from big-endian ub1_t array
	integer(const ub1_t *encodedInteger, ub4_t byteCount, bool s = false);

	//! creates a random integer
	/*! The random integer created is uniformly distributed over [0, 2**bitcount). */
	integer(random_generator &rng, ub4_t bitcount);

	//! avoid calling constructors for these frequently used integers
	static const integer& zero();
	//! avoid calling constructors for these frequently used integers
	static const integer& one();

	//! creates a random integer of special type
	/*! Ideally, the random integer created should be uniformly distributed
		over {x | min <= x <= max and x is of rnType and x % mod == equiv}.
		However the actual distribution may not be uniform because sequential
		search is used to find an appropriate number from a random starting
		point.
		May return (with very small probability) a pseudoprime when a prime
		is requested and max > lastSmallPrime*lastSmallPrime (lastSmallPrime
		is declared in nbtheory.h).
		\throw RandomNumberNotFound if the set is empty.
	*/
	integer(random_generator &rng, const integer& min, const integer& max, bool rnType = false, const integer& equiv = zero(), const integer& mod = one());

	//! return the integer 2**e
	static integer power2(ub4_t e);
//@}

//! \name ENCODE/DECODE
//@{
	//! minimum number of bytes to encode this integer
	/*! MinEncodedSize of 0 is 1 */
	ub4_t min_encoded_size(bool = false) const;
	//! encodes in big-endian format
	/*! unsigned means encode absolute value, signed means encode two's complement if negative.
		if outputLen < MinEncodedSize, the most significant bytes will be dropped
		if outputLen > MinEncodedSize, the most significant bytes will be padded
	*/
	ub4_t encode(ub1_t *output, ub4_t outputLen, bool = false) const;
	//!
	void decode(const ub1_t *input, ub4_t inputLen, bool = false);
	//! returns true if *this can be represented as a int32_t
	bool is_convertable_to_long() const;
	//! returns equivalent int if possible, otherwise undefined
	long convert_to_long() const;

	//! number of significant bits = floor(log2(abs(*this))) + 1
	ub4_t bit_count() const;
	//! number of significant bytes = ceiling(bit_count()/8)
	ub4_t byte_count() const;
	//! number of significant words = ceiling(byte_count()/sizeof(ub4_t))
	ub4_t word_count() const;

	//! returns the i-th bit, i=0 being the least significant bit
	bool get_bit(ub4_t i) const;
	//! returns the i-th ub1_t
	ub1_t get_byte(ub4_t i) const;
	//! returns n lowest bits of *this >> i
	ub4_t get_bits(ub4_t i, ub4_t n) const;

	//!
	inline bool is_zero() const;
	//!
	inline bool not_zero() const;
	//!
	inline bool is_negative() const;
	//!
	inline bool not_negative() const;
	//!
	inline bool is_positive() const;
	//!
	inline bool not_positive() const;
	//!
	inline bool is_event() const;
	//!
	inline bool is_odd() const;
//@}

//! \name MANIPULATORS
//@{
	//!
	inline integer& operator=(const integer& x);
	//!
	inline integer& operator+=(const integer& x);
	//!
	inline integer& operator-=(const integer& x);
	//!
	inline integer& operator*=(const integer& x);
	//!
	inline integer& operator/=(const integer& x);
	//!
	inline integer& operator%=(const integer& x);
	//!
	inline integer& operator/=(ub4_t x);
	//!
	inline integer& operator%=(ub4_t x);

	//!
	inline integer& operator<<=(ub4_t);
	//!
	inline integer& operator>>=(ub4_t);

	//!
	void randomize(random_generator &rng, ub4_t bitcount);
	//!
	void randomize(random_generator &rng, const integer& min, const integer& max);
	//! set this integer to a random element of {x | min <= x <= max and x is of rnType and x % mod == equiv}
	/*! returns false if the set is empty */
	bool randomize(random_generator &rng, const integer& min, const integer& max, bool rnType, const integer& equiv=zero(), const integer& mod=one());

	//! sets the n-th bit to value
	void set_bit(ub4_t n, bool value=1);
	//! sets the n-th ub1_t to value
	void set_byte(ub4_t n, ub1_t value);

	//!
	void negate();
	//!
	void set_positive();
	//!
	void set_negative();

	//!
	void swap(integer& a);
//@}

//! \name UNARY OPERATORS
//@{
	//!
	bool		operator!() const;
	//!
	integer 	operator+() const;
	//!
	integer 	operator-() const;
	//!
	integer&	operator++();
	//!
	integer&	operator--();
	//!
	integer 	operator++(int);
	//!
	integer 	operator--(int);
//@}

//! \name BINARY OPERATORS
//@{
	//! signed comparison
	/*! \retval -1 if *this < a
		\retval  0 if *this = a
		\retval  1 if *this > a
	*/
	int compare(const integer& a) const;

	//!
	integer plus(const integer& b) const;
	//!
	integer minus(const integer& b) const;
	//!
	integer times(const integer& b) const;
	//!
	integer divided(const integer& b) const;
	//!
	integer mod(const integer& b) const;
	//!
	integer divided(ub4_t b) const;
	//!
	ub4_t mod(ub4_t b) const;

	//!
	integer operator>>(ub4_t n) const;
	//!
	integer operator<<(ub4_t n) const;
//@}

//! \name OTHER ARITHMETIC FUNCTIONS
//@{
	//!
	integer abs() const;
	//!
	integer doubled() const;
	//!
	integer squared() const;
	//! extracts the square root, if negative returns 0, otherwise returns floor of square root
	integer sroot() const;
	//! returns whenever this integer is a perfect square
	bool is_square() const;

	//! is 1 or -1
	bool is_abs_one() const;
	//! returns inverse if 1 or -1, otherwise returns 0
	integer multiplicative_inverse() const;

	//! modular multiplication
	friend integer a_times_b_mod_c(const integer& x, const integer& y, const integer& m);
	//! modular exponentiation
	friend integer a_exp_b_mod_c(const integer& x, const integer& e, const integer& m);

	//! calculates r and q such that (a == d*q + r) && (0 <= r < abs(d))
	static void divide(integer& r, integer& q, const integer& a, const integer& d);
	//! uses a faster division algorithm when divisor is short
	static void divide(ub4_t &r, integer& q, const integer& a, ub4_t d);

	//! returns the same result as divide(r, q, a, power2(n)), but faster
	static void divide_power2(integer& r, integer& q, const integer& a, ub4_t n);

	//! greatest common divisor
	static integer gcd(const integer& a, const integer& n);
	//! calculates multiplicative inverse of *this mod n
	integer inverse_mod(const integer& n) const;
	//!
	ub4_t inverse_mod(ub4_t n) const;


//////////////////////////////////////////////////////
	static integer a_times_b_mod_c(const integer& x, const integer& y, const integer& m);
	static integer a_exp_b_mod_c(const integer& x, const integer& e, const integer& m);
	static ub4_t prime_search_interval(const integer& max);
	static integer CRT(const integer& xp, const integer& p, const integer& xq, const integer& q, const integer& u);
	static bool is_small_prime(const integer& p);
	static bool is_strong_probable_prime(const integer& n, const integer& b);
	static int jacobi(const integer& aIn, const integer& bIn);
	static integer lucas(const integer& e, const integer& pIn, const integer& n);
	static bool is_strong_lucas_probable_prime(const integer& n);
	static bool trial_division(const integer& p, ub4_t bound);
	static bool small_divisors_test(const integer& p);
	static bool is_prime(const integer& p);
	static inline bool fast_probable_prime_test(const integer& n)
	{ return is_strong_probable_prime(n , 2); }
	static bool first_prime(integer& p, const integer& max, const integer& equiv, const integer& mod);

//////////////////////////////////////////////////////

private:
	integer(ub4_t value, ub4_t length);

	friend class modular_arithmetic;
	friend class montgomery_representation;
	int _compare(const integer& t) const;
	static void _add(integer& sum, const integer& a, const integer& b);
	static void _subtract(integer& diff, const integer& a, const integer& b);
	static void _multiply(integer& product, const integer& a, const integer& b);
	static void _divide(integer& remainder, integer& quotient, const integer& dividend, const integer& divisor);
	static void multiply(integer& product, const integer& a, const integer& b);

	room_array< ub4_t > _reg;
	bool _sign;
};

//!
inline bool operator==(const integer& a, const integer& b);
//!
inline bool operator!=(const integer& a, const integer& b);
//!
inline bool operator> (const integer& a, const integer& b);
//!
inline bool operator>=(const integer& a, const integer& b);
//!
inline bool operator< (const integer& a, const integer& b);
//!
inline bool operator<=(const integer& a, const integer& b);
//!
inline integer operator+(const integer& a, const integer& b);
//!
inline integer operator-(const integer& a, const integer& b);
//!
inline integer operator*(const integer& a, const integer& b);
//!
inline integer operator/(const integer& a, const integer& b);
//!
inline integer operator%(const integer& a, const integer& b);
//!
inline integer operator/(const integer& a, ub4_t b);
//!
inline ub4_t operator%(const integer& a, ub4_t b);

inline void swap(integer& a, integer& b);

//////////////////////////////////////////////////////////////////////////////
template < class T > 
class abstract_group
{
public:
	virtual ~abstract_group();

	virtual bool equal(const T& a, const T& b) const = 0;
	virtual const T& zero() const =0;
	virtual const T& add(const T& a, const T& b) const = 0;
	virtual const T& inverse(const T& a) const = 0;
	virtual bool inversion_is_fast() const;

	virtual const T& doubled(const T& a) const;
	virtual const T& subtract(const T& a, const T& b) const;
	virtual T& accumulate(T& a, const T& b) const;
	virtual T& reduce(T& a, const T& b) const;
	virtual T scalar_multiply(const T& a, const integer& e) const;
	virtual T cascade_scalar_multiply(const T& x, const integer& e1, const T& y, const integer& e2) const;
	virtual void simultaneous_multiply(T *results, const T& base, const integer *exponents, ub4_t exponentsCount) const;
};

//! Abstract Ring
template < class T > 
class abstract_ring : public abstract_group< T >
{
public:
	abstract_ring();
	abstract_ring(const abstract_ring &x);
	abstract_ring& operator=(const abstract_ring &sec);

	virtual bool is_abs_one(const T& a) const = 0;
	virtual const T& one() const = 0;
	virtual const T& multiply(const T& a, const T& b) const = 0;
	virtual const T& multiplicative_inverse(const T& a) const = 0;

	virtual const T& square(const T& a) const;
	virtual const T& divide(const T& a, const T& b) const;
	virtual T exponentiate(const T& a, const integer& e) const;
	virtual T cascade_exponentiate(const T& x, const integer& e1, const T& y, const integer& e2) const;
	virtual void simultaneous_exponentiate(T *results, const T& base, const integer *exponents, ub4_t exponentsCount) const;

	virtual const abstract_group< T >& multiplicative_group() const;

private:

	class multiplicative_group_t : public abstract_group< T >
	{
	public:
		const abstract_ring< T >& get_ring() const
		{
			return *_ring;
		}

		bool equal(const T& a, const T& b) const
		{
			return get_ring().equal(a, b);
		}

		const T& zero() const
		{
			return get_ring().one();
		}

		const T& add(const T& a, const T& b) const
		{
			return get_ring().multiply(a, b);
		}

		T& accumulate(T& a, const T& b) const
		{
			return a = get_ring().multiply(a, b);
		}

		const T& inverse(const T& a) const
		{
			return get_ring().multiplicative_inverse(a);
		}

		const T& subtract(const T& a, const T& b) const
		{
			return get_ring().divide(a, b);
		}

		T& reduce(T& a, const T& b) const
		{
			return a = get_ring().divide(a, b);
		}

		const T& doubled(const T& a) const
		{
			return get_ring().square(a);
		}

		T scalar_multiply(const T& a, const integer& e) const
		{
			return get_ring().exponentiate(a, e);
		}

		T cascade_scalar_multiply(const T& x, const integer& e1, const T& y, const integer& e2) const
		{
			return get_ring().cascade_exponentiate(x, e1, y, e2);
		}

		void simultaneous_multiply(T *results, const T& base, const integer *exponents, ub4_t exponentsCount) const
		{
			get_ring().simultaneous_exponentiate(results, base, exponents, exponentsCount);
		}

		const abstract_ring< T > *_ring;
	};

	multiplicative_group_t _mg;
};

class modular_arithmetic : public abstract_ring< integer >
{
public:
	modular_arithmetic(const integer& modulus = integer::one());
	modular_arithmetic(const modular_arithmetic &ma);
	inline const integer& get_modulus() const;
	inline void set_modulus(const integer& modulus);

	virtual integer convert_in(const integer& a) const;
	virtual integer convert_out(const integer& a) const;

	const integer& half(const integer& a) const;
	inline bool equal(const integer& a, const integer& b) const;
	inline const integer& zero() const;
	const integer& add(const integer& a, const integer& b) const;
	integer& accumulate(integer& a, const integer& b) const;
	const integer& inverse(const integer& a) const;
	const integer& subtract(const integer& a, const integer& b) const;
	integer& reduce(integer& a, const integer& b) const;

	inline const integer& doubled(const integer& a) const;
	inline const integer& one() const;
	inline const integer& multiply(const integer& a, const integer& b) const;
	inline const integer& square(const integer& a) const;
	inline bool is_abs_one(const integer& a) const;
	inline const integer& multiplicative_inverse(const integer& a) const;
	inline const integer& divide(const integer& a, const integer& b) const;

	integer cascade_exponentiate(const integer& x, const integer& e1, const integer& y, const integer& e2) const;
	void simultaneous_exponentiate(integer *results, const integer& base, const integer *exponents, ub4_t exponentsCount) const;

	inline ub4_t bit_count() const;
	inline ub4_t byte_count() const;
	inline integer random_element(random_generator &rng , const int &ignore_for_now = 0) const;

	static const int _randomization_parameter;

protected:
	integer _modulus;
	mutable integer _result, _result1;
};

class montgomery_representation : public modular_arithmetic
{
public:
	montgomery_representation(const integer& modulus);	// modulus must be odd

	inline integer convert_in(const integer& a) const;

	integer convert_out(const integer& a) const;

	inline const integer& one() const;

	const integer& multiply(const integer& a, const integer& b) const;

	const integer& square(const integer& a) const;

	const integer& multiplicative_inverse(const integer& a) const;

	inline integer cascade_exponentiate(const integer& x, const integer& e1, const integer& y, const integer& e2) const;

	inline void simultaneous_exponentiate(integer *results, const integer& base, const integer *exponents, ub4_t exponentsCount) const;

private:
	integer							_u;
	mutable room_array< ub4_t >	_workspace;
};


//////////////////////////////////////////////////////////////////////
class window_slider
{
public:
	window_slider(const integer& exp, bool fast_negate, ub4_t window_size = 0);
	inline void find_next_window();

	integer _exp, _window_modulus;
	ub4_t _window_size, _window_begin, _exp_window;
	bool _fast_negate, _negate_next, _first_time, _finished;
};

template < class T > class abstract_euclidean_domain : public abstract_ring< T >
{
public:
	virtual void division_algorithm(T& r, T& q, const T& a, const T& d) const = 0;
	virtual const T& mod(const T& a, const T& b) const = 0;

	virtual const T& gcd(const T& a, const T& b) const;

protected:
	mutable T _result;
};

// ********************************************************

//! euclidean_domain_of
template < class T > class euclidean_domain_of : public abstract_euclidean_domain< T >
{
public:
	euclidean_domain_of();

	inline bool equal(const T& a, const T& b) const;
	inline const T& zero() const;
	inline const T& add(const T& a, const T& b) const;
	inline T& accumulate(T& a, const T& b) const;
	inline const T& inverse(const T& a) const;
	inline const T& subtract(const T& a, const T& b) const;
	inline T& reduce(T& a, const T& b) const;
	inline const T& doubled(const T& a) const;
	inline const T& one() const;
	inline const T& multiply(const T& a, const T& b) const;
	inline const T& square(const T& a) const;
	inline bool is_abs_one(const T& a) const;
	inline const T& multiplicative_inverse(const T& a) const;
	inline const T& divide(const T& a, const T& b) const;
	inline const T& mod(const T& a, const T& b) const;
	inline void division_algorithm(T& r, T& q, const T& a, const T& d) const;

private:
	mutable T _result;
};


//////////////////////////////////////////////
class prime_sieve
{
public:
	// delta == 1 or -1 means double sieve with p = 2*q + delta
	prime_sieve(const integer& first, const integer& last, const integer& step, sb4_t delta = 0);
	bool next_candidate(integer& c);

	void do_sieve();
	static void sieve_single(vector< bool >& sieve, ub4_t p, const integer& first, const integer& step, ub4_t stepInv);

	integer _first, _last, _step;
	sb4_t _delta;
	ub4_t _next;
	vector< bool > _sieve;
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_integer_h_


