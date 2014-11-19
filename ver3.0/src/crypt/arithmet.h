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

#ifndef _terimber_arithmetic_h_
#define _terimber_arithmetic_h_

#include "allinc.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//const ub4_t word_size = sizeof(ub4_t);
//const ub4_t word_bits = word_size * 8;
#define word_size 4
#define word_bits 32

const ub4_t max_small_prime_table_size = 3511;	// last prime 32719
const ub4_t last_small_prime = 32719;

extern ub4_t small_prime_table_size;
extern ub4_t small_prime_table[max_small_prime_table_size];

inline ub8_t make_ub8(ub4_t low_word, ub4_t high_word);
inline ub4_t get_low_ub4(ub8_t x);
inline ub4_t get_high_ub4(ub8_t x);

namespace room_arithmetic
{
inline ub4_t count_words(const ub4_t* x, ub4_t n);
inline void set_words(ub4_t* r, ub4_t a, ub4_t n);
inline void copy_words(ub4_t* r, const ub4_t* a, ub4_t n);
inline void xor_words(ub4_t* r, const ub4_t* a, const ub4_t* b, ub4_t n);
inline void xor_words(ub4_t* r, const ub4_t* a, ub4_t n);
inline void and_words(ub4_t* r, const ub4_t* a, const ub4_t* b, ub4_t n);
inline void and_words(ub4_t* r, const ub4_t* a, ub4_t n);
inline ub4_t shl_bits(ub4_t* r, ub4_t n, ub4_t shift);
inline ub4_t shr_bits(ub4_t* r, ub4_t n, ub4_t shift);
inline void shl_words(ub4_t* r, ub4_t n, ub4_t shift);
inline void shr_words(ub4_t* r, ub4_t n, ub4_t shift);
inline ub4_t parity(ub4_t n);
inline ub4_t byte_precision(ub4_t n);
inline ub4_t bit_precision(ub4_t n);
inline ub4_t crop(ub4_t n, ub4_t size);
inline ub4_t bits_to_bytes(ub4_t bitCount);
inline ub4_t bytes_to_words(ub4_t byteCount);
inline ub4_t bits_to_words(ub4_t bitCount);
inline ub4_t add(ub4_t* r, const ub4_t* a, const ub4_t* b, ub4_t n);
inline ub4_t subtract(ub4_t* r, const ub4_t* a, const ub4_t* b, ub4_t n);
inline int compare(const ub4_t* a, const ub4_t* b, ub4_t n);
inline ub4_t increment(ub4_t* a, ub4_t n, ub4_t b = 1);
inline ub4_t decrement(ub4_t* a, ub4_t n, ub4_t b = 1);
inline void twos_complement(ub4_t* a, ub4_t n);
inline ub4_t linear_multiply(ub4_t* r, const ub4_t* a, ub4_t b, ub4_t n);
inline void atomic_multiply(ub4_t* r, const ub4_t* a, const ub4_t* b);
inline ub4_t atomic_multiply_add(ub4_t* r, const ub4_t* a, const ub4_t* b);
inline void atomic_multiply_bottom(ub4_t* r, const ub4_t* a, const ub4_t* b);
inline void mul_acc(ub4_t a, ub4_t b, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e);
inline void save_mul_acc(ub4_t& r, ub4_t a, ub4_t b, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e);
inline void mul_acc1(ub4_t a, ub4_t aa, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e);
inline void save_mul_acc1(ub4_t& r, ub4_t a, ub4_t aa,ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e);
inline void squ_acc(ub4_t a, ub4_t aa, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e);
inline void aave_squ_acc(ub4_t& r, ub4_t a, ub4_t aa, ub8_t& p, ub4_t& c, ub4_t& d, ub4_t& e);
inline void comba_square4(ub4_t* r, const ub4_t* a);
inline void comba_multiply4(ub4_t* r, const ub4_t* a, const ub4_t* b);
inline void comba_multiply8(ub4_t* r, const ub4_t* a, const ub4_t* b);
inline void comba_multiply_bottom4(ub4_t* r, const ub4_t* a, const ub4_t* b);
inline void comba_multiply_bottom8(ub4_t* r, const ub4_t* a, const ub4_t* b);
inline void atomic_inverse_mod_power2(ub4_t* r, ub4_t a, ub4_t b);

void build_prime_table();
void recursive_multiply(ub4_t* r, ub4_t* t, const ub4_t* a, const ub4_t* b, ub4_t n);
void recursive_square(ub4_t* r, ub4_t* t, const ub4_t* a, ub4_t n);
void recursive_multiply_bottom(ub4_t* r, ub4_t* t, const ub4_t* a, const ub4_t* b, ub4_t n);
void recursive_multiply_top(ub4_t* r, ub4_t* t, const ub4_t* l, const ub4_t* a, const ub4_t* b, ub4_t n);
void asymmetric_multiply(ub4_t* r, ub4_t* t, const ub4_t* a, ub4_t na, const ub4_t* b, ub4_t nb);
void recursive_inverse_mod_power2(ub4_t* r, ub4_t* t, const ub4_t* a, ub4_t n);
void montgomery_reduce(ub4_t* r, ub4_t* t, const ub4_t* x, const ub4_t* m, const ub4_t* u, ub4_t n);
void half_montgomery_reduce(ub4_t* r, ub4_t* t, const ub4_t* x, const ub4_t* m, const ub4_t* u, const ub4_t* v, ub4_t n);
ub4_t subatomic_divide(ub4_t* a, ub4_t b0, ub4_t b1);
void atomic_divide(ub4_t* q, const ub4_t* a, const ub4_t* b);
void correct_quotient_estimate(ub4_t* r, ub4_t* t, ub4_t* q, const ub4_t* b, ub4_t n);
void divide(ub4_t* r, ub4_t* q, ub4_t* t, const ub4_t* a, ub4_t na, const ub4_t* b, ub4_t nb);

inline ub4_t even_word_count(const ub4_t* x, ub4_t n);

ub4_t almost_inverse(ub4_t* r, ub4_t* t, const ub4_t* a, ub4_t na, const ub4_t* m, ub4_t n);
void divide_power2_mod(ub4_t* r, const ub4_t* a, ub4_t k, const ub4_t* m, ub4_t n);
void multiply_power2_mod(ub4_t* r, const ub4_t* a, ub4_t k, const ub4_t* m, ub4_t n);

inline ub4_t roundup_size(ub4_t n);

extern const ub4_t roundup_size_table[];
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_arithmetic_h_

