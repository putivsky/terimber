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

#include "allinc.h"
#include "base/memory.hpp"
#include "base/common.hpp"
#include "base/vector.hpp"
#include "base/number.hpp"
#include "crypt/crypt.hpp"
#include "crypt/integer.hpp"
#include "crypt/arithmet.hpp"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

#define RSA_PRODUCTION
//////////////////////////////////////////////////////////////////////
#define blk0(i) (W[i] = data[i])
#define blk1(i) (W[i&15] = rotlFixed(W[(i+13)&15]^W[(i+8)&15]^W[(i+2)&15]^W[i&15],1))
#define blk2(i) (W[i&15]+=s1(W[(i-2)&15])+W[(i-7)&15]+s0(W[(i-15)&15]))

#define Ch(x,y,z) (z^(x&(y^z)))
#define Maj(x,y,z) ((x&y)|(z&(x|y)))

#define a(i) T[(0-i)&7]
#define b(i) T[(1-i)&7]
#define c(i) T[(2-i)&7]
#define d(i) T[(3-i)&7]
#define e(i) T[(4-i)&7]
#define f(i) T[(5-i)&7]
#define g(i) T[(6-i)&7]
#define h(i) T[(7-i)&7]

#define R(i) h(i)+=S1(e(i))+Ch(e(i),f(i),g(i))+K[i+j]+(j?blk2(i):blk0(i));\
	d(i)+=h(i);h(i)+=S0(a(i))+Maj(a(i),b(i),c(i))

// for sha256
#define S0(x) (rotrFixed(x,2)^rotrFixed(x,13)^rotrFixed(x,22))
#define S1(x) (rotrFixed(x,6)^rotrFixed(x,11)^rotrFixed(x,25))
#define s0(x) (rotrFixed(x,7)^rotrFixed(x,18)^(x>>3))
#define s1(x) (rotrFixed(x,17)^rotrFixed(x,19)^(x>>10))

/////////////////////////////////////////////////////////////////
// ! block_transformer
// virtual
block_transformer::~block_transformer()
{
}
//////////////////////////////////////////////////////////
sha256::sha256() 
	: base_hash< true, 64, 32 >() 
{ 
	v_init(); 
}

// virtual
void
sha256::v_init()
{
#ifdef TERIMBER_LITTLE_ENDIAN
	_digest[0] = 0x6A09E667;
	_digest[1] = 0xBB67AE85;
	_digest[2] = 0x3C6EF372;
	_digest[3] = 0xA54FF53A;
	_digest[4] = 0x510E527F;
	_digest[5] = 0x9B05688C;
	_digest[6] = 0x1F83D9AB;
	_digest[7] = 0x5BE0CD19;
#else
	_digest[0] = 0x67E6096A;
	_digest[1] = 0x85AE67BB;
	_digest[2] = 0x72F36E3C;
	_digest[3] = 0x3AF54FA5;
	_digest[4] = 0x7F520E51;
	_digest[5] = 0x8C68059B;
	_digest[6] = 0xABD9831F;
	_digest[7] = 0x19CDE05B;
#endif
}

// virtual
void
sha256::v_transform(const ub4_t *data) 
{ 
	transform(_digest, data); 
}

void sha256::transform(ub4_t *state, const ub4_t *data)
{
	ub4_t W[16];
	ub4_t T[8];
    /* Copy context->state[] to working vars */
	memcpy(T, state, sizeof(T));
    /* 64 operations, partially loop unrolled */
	for (size_t j = 0; j < 64; j += 16)
	{
		R( 0); R( 1); R( 2); R( 3);
		R( 4); R( 5); R( 6); R( 7);
		R( 8); R( 9); R(10); R(11);
		R(12); R(13); R(14); R(15);
	}
    /* Adds the working vars back into context.state[] */
    state[0] += a(0);
    state[1] += b(0);
    state[2] += c(0);
    state[3] += d(0);
    state[4] += e(0);
    state[5] += f(0);
    state[6] += g(0);
    state[7] += h(0);
    /* Wipes variables */
	memset(W, 0, sizeof(W));
	memset(T, 0, sizeof(T));
}

////////////////////////////////////////////////////////////////////////////////
// md5
#define f1_md5(x, y, z) (z ^ (x & (y ^ z)))
#define f2_md5(x, y, z) f1_md5(z, x, y)
#define f3_md5(x, y, z) (x ^ y ^ z)
#define f4_md5(x, y, z) (y ^ (x | ~z))

#define md5_step(f, w, x, y, z, data, s) \
	w = rotlFixed(w + f(x, y, z) + data, s) + x

////////////////////////////////////////////////////////
// ! md5
md5::md5() 
: base_hash< false, 64, 16 >() 
{ 
	v_init();
}

void 
md5::v_init()
{
#ifdef TERIMBER_LITTLE_ENDIAN
	_digest[0] = 0x67452301L;
	_digest[1] = 0xEFCDAB89L;
	_digest[2] = 0x98BADCFEL;
	_digest[3] = 0x10325476L;
#else
	_digest[0] = 0x01234567L;
	_digest[1] = 0x89ABCDEFL;
	_digest[2] = 0xFEDCBA98L;
	_digest[3] = 0x76543210L;
#endif
}

void 
md5::v_transform(const ub4_t *data) 
{ 
	transform(_digest, data);
}

void md5::transform (ub4_t *digest, const ub4_t *in)
{
    ub4_t a, b, c, d;

	a=digest[0];
	b=digest[1];
	c=digest[2];
	d=digest[3];

    md5_step(f1_md5, a, b, c, d, in[0] + 0xd76aa478, 7);
    md5_step(f1_md5, d, a, b, c, in[1] + 0xe8c7b756, 12);
    md5_step(f1_md5, c, d, a, b, in[2] + 0x242070db, 17);
    md5_step(f1_md5, b, c, d, a, in[3] + 0xc1bdceee, 22);
    md5_step(f1_md5, a, b, c, d, in[4] + 0xf57c0faf, 7);
    md5_step(f1_md5, d, a, b, c, in[5] + 0x4787c62a, 12);
    md5_step(f1_md5, c, d, a, b, in[6] + 0xa8304613, 17);
    md5_step(f1_md5, b, c, d, a, in[7] + 0xfd469501, 22);
    md5_step(f1_md5, a, b, c, d, in[8] + 0x698098d8, 7);
    md5_step(f1_md5, d, a, b, c, in[9] + 0x8b44f7af, 12);
    md5_step(f1_md5, c, d, a, b, in[10] + 0xffff5bb1, 17);
    md5_step(f1_md5, b, c, d, a, in[11] + 0x895cd7be, 22);
    md5_step(f1_md5, a, b, c, d, in[12] + 0x6b901122, 7);
    md5_step(f1_md5, d, a, b, c, in[13] + 0xfd987193, 12);
    md5_step(f1_md5, c, d, a, b, in[14] + 0xa679438e, 17);
    md5_step(f1_md5, b, c, d, a, in[15] + 0x49b40821, 22);

    md5_step(f2_md5, a, b, c, d, in[1] + 0xf61e2562, 5);
    md5_step(f2_md5, d, a, b, c, in[6] + 0xc040b340, 9);
    md5_step(f2_md5, c, d, a, b, in[11] + 0x265e5a51, 14);
    md5_step(f2_md5, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    md5_step(f2_md5, a, b, c, d, in[5] + 0xd62f105d, 5);
    md5_step(f2_md5, d, a, b, c, in[10] + 0x02441453, 9);
    md5_step(f2_md5, c, d, a, b, in[15] + 0xd8a1e681, 14);
    md5_step(f2_md5, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    md5_step(f2_md5, a, b, c, d, in[9] + 0x21e1cde6, 5);
    md5_step(f2_md5, d, a, b, c, in[14] + 0xc33707d6, 9);
    md5_step(f2_md5, c, d, a, b, in[3] + 0xf4d50d87, 14);
    md5_step(f2_md5, b, c, d, a, in[8] + 0x455a14ed, 20);
    md5_step(f2_md5, a, b, c, d, in[13] + 0xa9e3e905, 5);
    md5_step(f2_md5, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    md5_step(f2_md5, c, d, a, b, in[7] + 0x676f02d9, 14);
    md5_step(f2_md5, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    md5_step(f3_md5, a, b, c, d, in[5] + 0xfffa3942, 4);
    md5_step(f3_md5, d, a, b, c, in[8] + 0x8771f681, 11);
    md5_step(f3_md5, c, d, a, b, in[11] + 0x6d9d6122, 16);
    md5_step(f3_md5, b, c, d, a, in[14] + 0xfde5380c, 23);
    md5_step(f3_md5, a, b, c, d, in[1] + 0xa4beea44, 4);
    md5_step(f3_md5, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    md5_step(f3_md5, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    md5_step(f3_md5, b, c, d, a, in[10] + 0xbebfbc70, 23);
    md5_step(f3_md5, a, b, c, d, in[13] + 0x289b7ec6, 4);
    md5_step(f3_md5, d, a, b, c, in[0] + 0xeaa127fa, 11);
    md5_step(f3_md5, c, d, a, b, in[3] + 0xd4ef3085, 16);
    md5_step(f3_md5, b, c, d, a, in[6] + 0x04881d05, 23);
    md5_step(f3_md5, a, b, c, d, in[9] + 0xd9d4d039, 4);
    md5_step(f3_md5, d, a, b, c, in[12] + 0xe6db99e5, 11);
    md5_step(f3_md5, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    md5_step(f3_md5, b, c, d, a, in[2] + 0xc4ac5665, 23);

    md5_step(f4_md5, a, b, c, d, in[0] + 0xf4292244, 6);
    md5_step(f4_md5, d, a, b, c, in[7] + 0x432aff97, 10);
    md5_step(f4_md5, c, d, a, b, in[14] + 0xab9423a7, 15);
    md5_step(f4_md5, b, c, d, a, in[5] + 0xfc93a039, 21);
    md5_step(f4_md5, a, b, c, d, in[12] + 0x655b59c3, 6);
    md5_step(f4_md5, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    md5_step(f4_md5, c, d, a, b, in[10] + 0xffeff47d, 15);
    md5_step(f4_md5, b, c, d, a, in[1] + 0x85845dd1, 21);
    md5_step(f4_md5, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    md5_step(f4_md5, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    md5_step(f4_md5, c, d, a, b, in[6] + 0xa3014314, 15);
    md5_step(f4_md5, b, c, d, a, in[13] + 0x4e0811a1, 21);
    md5_step(f4_md5, a, b, c, d, in[4] + 0xf7537e82, 6);
    md5_step(f4_md5, d, a, b, c, in[11] + 0xbd3af235, 10);
    md5_step(f4_md5, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    md5_step(f4_md5, b, c, d, a, in[9] + 0xeb86d391, 21);

	digest[0]+=a;
	digest[1]+=b;
	digest[2]+=c;
	digest[3]+=d;
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////
/////////////////////////////////////////////////////////////////
// ! rc6_encrypt
rc6_encrypt::rc6_encrypt(const ub1_t* key, size_t keyLen, size_t rounds)
	: rc6(key, keyLen, rounds) 
{
}

//////////////////////////////////////////////////////////////////
// ! rc6_decrypt
rc6_decrypt::rc6_decrypt(const ub1_t* key, size_t keyLen, size_t rounds)
	: rc6(key, keyLen, rounds) 
{
}

// crypt algorithm
rc6::rc6(const ub1_t* k, size_t keylen, size_t rounds) :
	_rounds(rounds), 
	_table((2 * rounds) + 4)
{
	assert(keylen == key_length(keylen));
	static const ub4_t MAGIC_P = 0xb7e15163L;    // magic constant P for wordsize
	static const ub4_t MAGIC_Q = 0x9e3779b9L;    // magic constant Q for wordsize
	//static const ub4_t U = sizeof(ub4_t);

	const size_t ws_len = __max((size_t)((keylen + sizeof(ub4_t) - 1) / sizeof(ub4_t)), (size_t)1);
	room_array< ub4_t > workspace(ws_len);
	get_user_key_little_endian((ub4_t*)workspace, ws_len, k, keylen);

	_table[0] = MAGIC_P;
	for (size_t j = 1; j < _table.size(); ++j)
		_table[j] = _table[j - 1] + MAGIC_Q;

	ub4_t a = 0, b = 0;
	const size_t n = 3 * __max(_table.size(), ws_len);

	for (size_t h = 0; h < n; ++h)
	{
		a = _table[h % _table.size()] = rotlFixed((_table[h % _table.size()] + a + b), 3);
		b = workspace[h % ws_len] = rotlMod((workspace[h % ws_len] + a + b), (a + b));
	}
}

void 
rc6_encrypt::process_block(const ub1_t *in, ub1_t *out) const
{
	const ub4_t *sptr = _table;
	ub4_t a, b, c, d, t, u;

	GetBlockLittleEndian(in, a, b, c, d);
	b += sptr[0];
	d += sptr[1];
	sptr += 2;

	for(size_t i = 0; i < _rounds; ++i)
	{
		t = rotlFixed(b*(2*b+1), 5);
		u = rotlFixed(d*(2*d+1), 5);
		a = rotlMod(a^t,u) + sptr[0];
		c = rotlMod(c^u,t) + sptr[1];
		t = a; a = b; b = c; c = d; d = t;
		sptr += 2;
	}

	a += sptr[0];
	c += sptr[1];

	PutBlockLittleEndian(out, a, b, c, d);
}

void 
rc6_decrypt::process_block(const ub1_t *in, ub1_t *out) const
{
	const ub4_t *sptr = _table + _table.size();
	ub4_t a, b, c, d, t, u;

	GetBlockLittleEndian(in, a, b, c, d);

	sptr -= 2;
	c -= sptr[1];
	a -= sptr[0];

	for (size_t i = 0; i < _rounds; ++i)
	{
		sptr -= 2;
		t = a; a = d; d = c; c = b; b = t;
		u = rotlFixed(d*(2*d+1), 5);
		t = rotlFixed(b*(2*b+1), 5);
		c = rotrMod(c-sptr[1], t) ^ u;
		a = rotrMod(a-sptr[0], u) ^ t;
	}

	sptr -= 2;
	d -= _table[1];
	b -= _table[0];

	PutBlockLittleEndian(out, a, b, c, d);
}

///////////////////////////////////////////////////////////////////////////////
rijndael::rijndael(const ub1_t* userKey, size_t keylen) :
	_rounds(keylen / 4 + 6), _key(4 * (_rounds + 1))
{
	assert(keylen == key_length(keylen));

	ub4_t temp, *rk = _key;
	ub4_t i=0;

	get_user_key_big_endian(rk, keylen / 4, userKey, keylen);

	switch(keylen)
	{
		case 16:
		for (;;)
		{
			temp  = rk[3];
			rk[4] = rk[0] ^
				(Te4[(temp >> 16) & 0x000000ff] & 0xff000000) ^
				(Te4[(temp >>  8) & 0x000000ff] & 0x00ff0000) ^
				(Te4[(temp      ) & 0x000000ff] & 0x0000ff00) ^
				(Te4[(temp >> 24)       ] & 0x000000ff) ^
				rcon[i];
			rk[5] = rk[1] ^ rk[4];
			rk[6] = rk[2] ^ rk[5];
			rk[7] = rk[3] ^ rk[6];
			if (++i == 10)
				return;
			rk += 4;
		}
		case 24:
		for (;;) 
		{
			temp = rk[ 5];
			rk[ 6] = rk[ 0] ^
				(Te4[(temp >> 16) & 0x000000ff] & 0xff000000) ^
				(Te4[(temp >>  8) & 0x000000ff] & 0x00ff0000) ^
				(Te4[(temp      ) & 0x000000ff] & 0x0000ff00) ^
				(Te4[(temp >> 24)       ] & 0x000000ff) ^
				rcon[i];
			rk[ 7] = rk[ 1] ^ rk[ 6];
			rk[ 8] = rk[ 2] ^ rk[ 7];
			rk[ 9] = rk[ 3] ^ rk[ 8];
			if (++i == 8)
				return;
			rk[10] = rk[ 4] ^ rk[ 9];
			rk[11] = rk[ 5] ^ rk[10];
			rk += 6;
		}
		case 32:
		for (;;) 
		{
        	temp = rk[ 7];
        	rk[ 8] = rk[ 0] ^
        		(Te4[(temp >> 16) & 0x000000ff] & 0xff000000) ^
        		(Te4[(temp >>  8) & 0x000000ff] & 0x00ff0000) ^
        		(Te4[(temp      ) & 0x000000ff] & 0x0000ff00) ^
        		(Te4[(temp >> 24)       ] & 0x000000ff) ^
        		rcon[i];
        	rk[ 9] = rk[ 1] ^ rk[ 8];
        	rk[10] = rk[ 2] ^ rk[ 9];
        	rk[11] = rk[ 3] ^ rk[10];
			if (++i == 7)
				return;
        	temp = rk[11];
        	rk[12] = rk[ 4] ^
        		(Te4[(temp >> 24)       ] & 0xff000000) ^
        		(Te4[(temp >> 16) & 0x000000ff] & 0x00ff0000) ^
        		(Te4[(temp >>  8) & 0x000000ff] & 0x0000ff00) ^
        		(Te4[(temp      ) & 0x000000ff] & 0x000000ff);
        	rk[13] = rk[ 5] ^ rk[12];
        	rk[14] = rk[ 6] ^ rk[13];
        	rk[15] = rk[ 7] ^ rk[14];

			rk += 8;
		}
	}
}

//////////////////////////////////////////////////////////////////
// ! rijndael_encrypt
rijndael_encrypt::rijndael_encrypt(const ub1_t* userKey, size_t keylength)
	: rijndael(userKey, keylength) 
{
}

void 
rijndael_encrypt::process_block(const ub1_t *in, ub1_t *out) const
{
	ub4_t s0, s1, s2, s3, t0, t1, t2, t3;
	const ub4_t *rk = _key;

    /*
	 * map ub1_t array block to cipher state
	 * and adds initial round key:
	 */
	GetBlockBigEndian(in, s0, s1, s2, s3);
	s0 ^= rk[0];
	s1 ^= rk[1];
	s2 ^= rk[2];
	s3 ^= rk[3];
    /*
	 * Nr - 1 full rounds:
	 */
    ub4_t r = (ub4_t)_rounds >> 1;
    for (;;) 
	{
        t0 =
            Te0[(s0 >> 24)       ] ^
            Te1[(s1 >> 16) & 0x000000ff] ^
            Te2[(s2 >>  8) & 0x000000ff] ^
            Te3[(s3      ) & 0x000000ff] ^
            rk[4];
        t1 =
            Te0[(s1 >> 24)       ] ^
            Te1[(s2 >> 16) & 0x000000ff] ^
            Te2[(s3 >>  8) & 0x000000ff] ^
            Te3[(s0      ) & 0x000000ff] ^
            rk[5];
        t2 =
            Te0[(s2 >> 24)       ] ^
            Te1[(s3 >> 16) & 0x000000ff] ^
            Te2[(s0 >>  8) & 0x000000ff] ^
            Te3[(s1      ) & 0x000000ff] ^
            rk[6];
        t3 =
            Te0[(s3 >> 24)       ] ^
            Te1[(s0 >> 16) & 0x000000ff] ^
            Te2[(s1 >>  8) & 0x000000ff] ^
            Te3[(s2      ) & 0x000000ff] ^
            rk[7];

        rk += 8;
        if (--r == 0) 
            break;

        s0 =
            Te0[(t0 >> 24)       ] ^
            Te1[(t1 >> 16) & 0x000000ff] ^
            Te2[(t2 >>  8) & 0x000000ff] ^
            Te3[(t3      ) & 0x000000ff] ^
            rk[0];
        s1 =
            Te0[(t1 >> 24)       ] ^
            Te1[(t2 >> 16) & 0x000000ff] ^
            Te2[(t3 >>  8) & 0x000000ff] ^
            Te3[(t0      ) & 0x000000ff] ^
            rk[1];
        s2 =
            Te0[(t2 >> 24)       ] ^
            Te1[(t3 >> 16) & 0x000000ff] ^
            Te2[(t0 >>  8) & 0x000000ff] ^
            Te3[(t1      ) & 0x000000ff] ^
            rk[2];
        s3 =
            Te0[(t3 >> 24)       ] ^
            Te1[(t0 >> 16) & 0x000000ff] ^
            Te2[(t1 >>  8) & 0x000000ff] ^
            Te3[(t2      ) & 0x000000ff] ^
            rk[3];
    }
    /*
	 * applies the last round and
	 * maps cipher state to ub1_t array block:
	 */
	s0 =
		(Te4[(t0 >> 24)       ] & 0xff000000) ^
		(Te4[(t1 >> 16) & 0x000000ff] & 0x00ff0000) ^
		(Te4[(t2 >>  8) & 0x000000ff] & 0x0000ff00) ^
		(Te4[(t3      ) & 0x000000ff] & 0x000000ff) ^
		rk[0];
	s1 =
		(Te4[(t1 >> 24)       ] & 0xff000000) ^
		(Te4[(t2 >> 16) & 0x000000ff] & 0x00ff0000) ^
		(Te4[(t3 >>  8) & 0x000000ff] & 0x0000ff00) ^
		(Te4[(t0      ) & 0x000000ff] & 0x000000ff) ^
		rk[1];
	s2 =
		(Te4[(t2 >> 24)       ] & 0xff000000) ^
		(Te4[(t3 >> 16) & 0x000000ff] & 0x00ff0000) ^
		(Te4[(t0 >>  8) & 0x000000ff] & 0x0000ff00) ^
		(Te4[(t1      ) & 0x000000ff] & 0x000000ff) ^
		rk[2];
	s3 =
		(Te4[(t3 >> 24)       ] & 0xff000000) ^
		(Te4[(t0 >> 16) & 0x000000ff] & 0x00ff0000) ^
		(Te4[(t1 >>  8) & 0x000000ff] & 0x0000ff00) ^
		(Te4[(t2      ) & 0x000000ff] & 0x000000ff) ^
		rk[3];

	PutBlockBigEndian(out, s0, s1, s2, s3);
}

rijndael_decrypt::rijndael_decrypt(const ub1_t* userKey, size_t keylength)
	: rijndael(userKey, keylength)
{
	size_t i, j;
	ub4_t temp, *rk = _key;

	/* inverts the order of the round keys: */
	for (i = 0, j = 4 * _rounds; i < j; i += 4, j -= 4) 
	{
		temp = rk[i    ]; rk[i    ] = rk[j    ]; rk[j    ] = temp;
		temp = rk[i + 1]; rk[i + 1] = rk[j + 1]; rk[j + 1] = temp;
		temp = rk[i + 2]; rk[i + 2] = rk[j + 2]; rk[j + 2] = temp;
		temp = rk[i + 3]; rk[i + 3] = rk[j + 3]; rk[j + 3] = temp;
	}
	/* applies the inverse MixColumn transform to all round keys but the first and the last: */
	for (i = 1; i < _rounds; ++i) 
	{
		rk += 4;
		rk[0] =
			Td0[Te4[(rk[0] >> 24)       ] & 0x000000ff] ^
			Td1[Te4[(rk[0] >> 16) & 0x000000ff] & 0x000000ff] ^
			Td2[Te4[(rk[0] >>  8) & 0x000000ff] & 0x000000ff] ^
			Td3[Te4[(rk[0]      ) & 0x000000ff] & 0x000000ff];
		rk[1] =
			Td0[Te4[(rk[1] >> 24)       ] & 0x000000ff] ^
			Td1[Te4[(rk[1] >> 16) & 0x000000ff] & 0x000000ff] ^
			Td2[Te4[(rk[1] >>  8) & 0x000000ff] & 0x000000ff] ^
			Td3[Te4[(rk[1]      ) & 0x000000ff] & 0x000000ff];
		rk[2] =
			Td0[Te4[(rk[2] >> 24)       ] & 0x000000ff] ^
			Td1[Te4[(rk[2] >> 16) & 0x000000ff] & 0x000000ff] ^
			Td2[Te4[(rk[2] >>  8) & 0x000000ff] & 0x000000ff] ^
			Td3[Te4[(rk[2]      ) & 0x000000ff] & 0x000000ff];
		rk[3] =
			Td0[Te4[(rk[3] >> 24)       ] & 0x000000ff] ^
			Td1[Te4[(rk[3] >> 16) & 0x000000ff] & 0x000000ff] ^
			Td2[Te4[(rk[3] >>  8) & 0x000000ff] & 0x000000ff] ^
			Td3[Te4[(rk[3]      ) & 0x000000ff] & 0x000000ff];
	}
}

void 
rijndael_decrypt::process_block(const ub1_t* in, ub1_t* out) const
{
	ub4_t s0, s1, s2, s3, t0, t1, t2, t3;
    const ub4_t *rk = _key;

    /*
	 * maps ub1_t array block to cipher state
	 * and adds initial round key:
	 */
	GetBlockBigEndian(in, s0, s1, s2, s3);
	s0 ^= rk[0];
	s1 ^= rk[1];
	s2 ^= rk[2];
	s3 ^= rk[3];
    /*
     * Nr - 1 full rounds:
     */
    ub4_t r = (ub4_t)_rounds >> 1;
    for (;;) 
	{
        t0 =
            Td0[(s0 >> 24)       ] ^
            Td1[(s3 >> 16) & 0x000000ff] ^
            Td2[(s2 >>  8) & 0x000000ff] ^
            Td3[(s1      ) & 0x000000ff] ^
            rk[4];
        t1 =
            Td0[(s1 >> 24)       ] ^
            Td1[(s0 >> 16) & 0x000000ff] ^
            Td2[(s3 >>  8) & 0x000000ff] ^
            Td3[(s2      ) & 0x000000ff] ^
            rk[5];
        t2 =
            Td0[(s2 >> 24)       ] ^
            Td1[(s1 >> 16) & 0x000000ff] ^
            Td2[(s0 >>  8) & 0x000000ff] ^
            Td3[(s3      ) & 0x000000ff] ^
            rk[6];
        t3 =
            Td0[(s3 >> 24)       ] ^
            Td1[(s2 >> 16) & 0x000000ff] ^
            Td2[(s1 >>  8) & 0x000000ff] ^
            Td3[(s0      ) & 0x000000ff] ^
            rk[7];

        rk += 8;
        if (--r == 0)
            break;

        s0 =
            Td0[(t0 >> 24)       ] ^
            Td1[(t3 >> 16) & 0x000000ff] ^
            Td2[(t2 >>  8) & 0x000000ff] ^
            Td3[(t1      ) & 0x000000ff] ^
            rk[0];
        s1 =
            Td0[(t1 >> 24)       ] ^
            Td1[(t0 >> 16) & 0x000000ff] ^
            Td2[(t3 >>  8) & 0x000000ff] ^
            Td3[(t2      ) & 0x000000ff] ^
            rk[1];
        s2 =
            Td0[(t2 >> 24)       ] ^
            Td1[(t1 >> 16) & 0x000000ff] ^
            Td2[(t0 >>  8) & 0x000000ff] ^
            Td3[(t3      ) & 0x000000ff] ^
            rk[2];
        s3 =
            Td0[(t3 >> 24)       ] ^
            Td1[(t2 >> 16) & 0x000000ff] ^
            Td2[(t1 >>  8) & 0x000000ff] ^
            Td3[(t0      ) & 0x000000ff] ^
            rk[3];
    }
    /*
	 * applies last round and
	 * maps cipher state to ub1_t array block:
	 */
   	s0 =
   		(Td4[(t0 >> 24)       ] & 0xff000000) ^
   		(Td4[(t3 >> 16) & 0x000000ff] & 0x00ff0000) ^
   		(Td4[(t2 >>  8) & 0x000000ff] & 0x0000ff00) ^
   		(Td4[(t1      ) & 0x000000ff] & 0x000000ff) ^
   		rk[0];
   	s1 =
   		(Td4[(t1 >> 24)       ] & 0xff000000) ^
   		(Td4[(t0 >> 16) & 0x000000ff] & 0x00ff0000) ^
   		(Td4[(t3 >>  8) & 0x000000ff] & 0x0000ff00) ^
   		(Td4[(t2      ) & 0x000000ff] & 0x000000ff) ^
   		rk[1];
   	s2 =
   		(Td4[(t2 >> 24)       ] & 0xff000000) ^
   		(Td4[(t1 >> 16) & 0x000000ff] & 0x00ff0000) ^
   		(Td4[(t0 >>  8) & 0x000000ff] & 0x0000ff00) ^
   		(Td4[(t3      ) & 0x000000ff] & 0x000000ff) ^
   		rk[2];
   	s3 =
   		(Td4[(t3 >> 24)       ] & 0xff000000) ^
   		(Td4[(t2 >> 16) & 0x000000ff] & 0x00ff0000) ^
   		(Td4[(t1 >>  8) & 0x000000ff] & 0x0000ff00) ^
   		(Td4[(t0      ) & 0x000000ff] & 0x000000ff) ^
   		rk[3];

	PutBlockBigEndian(out, s0, s1, s2, s3);
}

//////////////////////////////////////////////////////////////////
crypt::crypt(const ub1_t* key, size_t len, crypt_hash hash) : _size(0)
{
	assert(len);
	// creates hash
	switch (hash)
	{
		case MD5:
			{
				md5 md;
				md.calculate_digest(_hash, key, len);
				_size = md5::DIGESTSIZE;
			}
			break;
		case SHA256:
			{
				sha256 sha;
				sha.calculate_digest(_hash, key, len); 
				_size = sha256::DIGESTSIZE;
			}
			break;
		default:
			assert(false);
	}
}

bool
crypt::encode(ub1_t* buffer, size_t& len, crypt_algorithm alogithm) const
{
	// decrypts data
	switch (alogithm)
	{
		case RC6:
			{
				rc6_encrypt cipher(_hash, _size);
				cipher.do_work(buffer, len);
			}
			break;
		case RIJNDAEL:
			{
				rijndael_encrypt cipher(_hash, _size);
				cipher.do_work(buffer, len);
			}
			break;
		default:
			assert(false);
			return false;
	}

	return true;
}

bool 
crypt::decode(ub1_t* buffer, size_t& len, crypt_algorithm alogithm) const
{
	// decrypts data
	switch (alogithm)
	{
		case RC6:
			{
				rc6_decrypt cipher(_hash, _size);
				cipher.do_work(buffer, len);
			}
			break;
		case RIJNDAEL:
			{
				rijndael_decrypt cipher(_hash, _size);
				cipher.do_work(buffer, len);
			}
			break;
		default:
			assert(false);
			return false;
	}

	return true;
}

size_t 
crypt::block_size(crypt_algorithm alogithm) const
{
	switch (alogithm)
	{
		case RC6:
			return rc6::BLOCKSIZE;
		case RIJNDAEL:
			return rijndael::BLOCKSIZE;
		default:
			assert(false);
			return 0;
	}
	
	return 0;
}


inline integer LCM(const integer &a, const integer &b)
{ return a / integer::gcd(a, b) * b; }

rsa::rsa(ub4_t keybits, bool gen_keys)
{
	if (!gen_keys)
		return;

	produce_keys(keybits);
}

rsa::rsa(const integer& ed, const integer& n) :
	_e(ed), _d(ed), _n(n)
{
}

bool 
rsa::produce_keys(ub4_t keybits)
{
	if (keybits < 16
#ifndef RSA_PRODUCTION
		|| keybits > 1024
#else
#endif
		)
	{ 
		return false;
	}

	integer eStart(_rng, keybits / 16); 

	if (keybits == 0)
		return false;

	assert(keybits >= 16);

	integer p, q;

	// generates 2 random primes of suitable size
	if (keybits % 2 == 0)
	{
		const integer minP = integer(182) << (keybits / 2 - 8);
		const integer maxP = integer::power2(keybits / 2) - 1;
		p.randomize(_rng, minP, maxP, true);
		q.randomize(_rng, minP, maxP, true);
	}
	else
	{
		const integer minP = integer::power2((keybits - 1) / 2);
		const integer maxP = integer(181) << ((keybits + 1) / 2 - 8);
		p.randomize(_rng, minP, maxP, true);
		q.randomize(_rng, minP, maxP, true);
	}

	// pre-calculates some other data for faster speed
	const integer lcm = LCM(p-1, q-1);
	// makes sure that e starts odd
	for (_e = eStart + ( 1 - eStart % 2); integer::gcd(_e, lcm) != 1; ++_e, ++_e);

	_d = _e.inverse_mod(lcm);
	_n = p * q;
	assert(_n.bit_count() == keybits);
	return true;
}

void 
rsa::encode(const integer& in, integer& out) const
{
	out = integer::a_exp_b_mod_c(in, _e, _n);
}

void 
rsa::decode(const integer& in, integer& out) const
{
	out = integer::a_exp_b_mod_c(in, _d, _n);
}

#pragma pack()
END_TERIMBER_NAMESPACE
