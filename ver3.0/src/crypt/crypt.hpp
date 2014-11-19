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

#ifndef _terimber_crypt_hpp_
#define _terimber_crypt_hpp_

#include "crypt/crypt.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

inline bool check_big_endianess(bool highFirst)
{
	return
#ifdef TERIMBER_LITTLE_ENDIAN
	!highFirst;
#else
	highFirst;
#endif
}


template <class T>
inline T rotlFixed(T x, ub4_t y)
{
	assert(y < sizeof(T) * 8);
	return (x << y) | (x >> (sizeof(T) * 8 - y));
}

template <class T>
inline T rotrFixed(T x, ub4_t y)
{
	assert(y < sizeof(T) * 8);
	return (x >> y) | (x << (sizeof(T) * 8 - y));
}

template <class T>
inline T rotlMod(T x, ub4_t y)
{
	y %= sizeof(T) * 8;
	return (x << y) | (x >> (sizeof(T) * 8 - y));
}

template <class T>
inline T rotrMod(T x, unsigned int y)
{
	y %= sizeof(T)*8;
	return (x >> y) | (x << (sizeof(T) * 8 - y));
}

template<>
inline ub4_t rotlFixed< ub4_t >(ub4_t x, ub4_t y)
{
	assert(y < 32);
#if OS_TYPE == OS_WIN32
	return y ? _lrotl(x, y) : x;
#else
	return (x << y) | (x >> (sizeof(ub4_t) * 8 - y));
#endif
}

template <>
inline ub4_t rotrFixed< ub4_t >(ub4_t x, ub4_t y)
{
	assert(y < 32);
#if OS_TYPE == OS_WIN32
	return y ? _lrotr(x, y) : x;
#else
	return (x >> y) | (x << (sizeof(ub4_t) * 8 - y));
#endif
}

template<>
inline ub4_t rotlMod< ub4_t >(ub4_t x, ub4_t y)
{
#if OS_TYPE == OS_WIN32
	return y ? _lrotl(x, y) : x;
#else
	return (x << y) | (x >> (sizeof(ub4_t) * 8 - y));
#endif
}

template<>
inline ub4_t rotrMod< ub4_t >(ub4_t x, unsigned int y)
{
#if OS_TYPE == OS_WIN32
	return y ? _lrotr(x, y) : x;
#else
	return (x >> y) | (x << (sizeof(ub4_t) * 8 - y));
#endif
}

//////////////////////////////////////////////
template < bool H, size_t S, size_t D >
base_hash< H, S, D>::base_hash() : _countLo(0), _countHi(0)
{
}

//virtual
template < bool H, size_t S, size_t D >
base_hash< H, S, D>::~base_hash()
{
}

template < bool H, size_t S, size_t D >
inline
size_t
base_hash< H, S, D>::digest_size() const
{
	return DIGESTSIZE;
};

template < bool H, size_t S, size_t D >
inline
void
base_hash< H, S, D>::calculate_digest(ub1_t *digest, const ub1_t *input, size_t length)
{
	update(input, length); final(digest);
}

template < bool H, size_t S, size_t D >
void
base_hash< H, S, D>::update(const ub1_t *input, size_t len)
{
	ub4_t tmp = _countLo;
	if ((_countLo = tmp + (ub4_t)len) < tmp)
		++_countHi;             // Carry from low to high
	_countHi += (ub4_t)len >> (8 * sizeof(ub4_t) - 3);

	assert((BLOCKSIZE & (BLOCKSIZE - 1)) == 0);	// BLOCKSIZE is a power of 2

	size_t num = (size_t)(tmp >> 3) & (BLOCKSIZE - 1);

	if (num != 0)
	{
		if ((num + len) >= BLOCKSIZE)
		{
			memcpy((ub1_t *)_data + num, input, BLOCKSIZE - num);
			hash_block(_data);
			input += (BLOCKSIZE - num);
			len -= (BLOCKSIZE - num);
			num = 0;
			// drop through and do the rest
		}
		else
		{
			memcpy((ub1_t *)_data + num, input, len);
			return;
		}
	}

	// we can now process the input data in blocks of blockSize
	// chars and save the leftovers to this->data.
	if (len >= BLOCKSIZE)
	{
		if ((size_t)input % sizeof(ub4_t) == 0)
		{
			size_t left_over = hash_multiple_blocks((ub4_t*)input, len);
			input += (len - left_over);
			len = left_over;
		}
		else
			do
			{   // copies the input first if it's not aligned correctly
				memcpy(_data, input, BLOCKSIZE);
				hash_block(_data);
				input += BLOCKSIZE;
				len -= BLOCKSIZE;
			}
			while (len >= BLOCKSIZE);
	}

	memcpy(_data, input, len);
}

template < bool H, size_t S, size_t D >
ub4_t
base_hash< H, S, D>::hash_multiple_blocks(const ub4_t *input, size_t length)
{
	do
	{
		hash_block(input);
		input += BLOCKSIZE / sizeof(ub4_t);
		length -= BLOCKSIZE;
	}
	while (length >= BLOCKSIZE);
	return (ub4_t)length;
}

template < bool H, size_t S, size_t D >
void
base_hash< H, S, D>::final(ub1_t* x)
{
	pad_last_block(BLOCKSIZE - 2 * sizeof(ub4_t));
	correct_endianess(_data, _data, BLOCKSIZE - 2 * sizeof(ub4_t));

	_data[BLOCKSIZE / sizeof(ub4_t) - 2] = (HIGHFIRST ? _countHi : _countLo) << 3;
	_data[BLOCKSIZE / sizeof(ub4_t) - 1] = (HIGHFIRST ? _countLo : _countHi) << 3;

	v_transform(_data);
	correct_endianess(_digest, _digest, digest_size());
	memcpy(x, _digest, digest_size());

	reinit(); // reinit for next use
}

template < bool H, size_t S, size_t D >
void
base_hash< H, S, D>::pad_last_block(size_t lastBlockSize, ub1_t padFirst)
{
	size_t num = ((size_t)_countLo & (BLOCKSIZE - 1));
	assert(num < BLOCKSIZE);
	((ub1_t*)_data)[num++] = padFirst;

	if (num <= lastBlockSize)
		memset((ub1_t*)_data + num, 0, lastBlockSize - num);
	else
	{
		memset((ub1_t*)_data + num, 0, BLOCKSIZE - num);
		hash_block(_data);
		memset(_data, 0, lastBlockSize);
	}
}

template < bool H, size_t S, size_t D >
void
base_hash< H, S, D>::reinit()
{
	_countLo = _countHi = 0;
	v_init();
}


///////////////////////////////////////////////////////////////////////
// cript algorithm
inline ub8_t byteReverse(ub8_t value)
{
#if OS_TYPE == OS_WIN32
#if defined(_MSC_VER) && (_MSC_VER > 1200)
	value = ((value & 0xFF00FF00FF00FF00ULL) >> 8) | ((value & 0x00FF00FF00FF00FFULL) << 8);
	value = ((value & 0xFFFF0000FFFF0000ULL) >> 16) | ((value & 0x0000FFFF0000FFFFULL) << 16);
#else
	value = ((value & 0xFF00FF00FF00FF00UL) >> 8) | ((value & 0x00FF00FF00FF00FFUL) << 8);
	value = ((value & 0xFFFF0000FFFF0000UL) >> 16) | ((value & 0x0000FFFF0000FFFFUL) << 16);
#endif
#else
	value = ((value & 0xFF00FF00FF00FF00ULL) >> 8) | ((value & 0x00FF00FF00FF00FFULL) << 8);
	value = ((value & 0xFFFF0000FFFF0000ULL) >> 16) | ((value & 0x0000FFFF0000FFFFULL) << 16);
#endif

	return rotlFixed(value, 32U);
}

inline ub4_t byteReverse(ub4_t value)
{
	// 6 instructions with rotate instruction, 8 without
	value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
	return rotlFixed(value, 16U);
}

inline ub2_t byteReverse(ub2_t value)
{
	return rotlFixed(value, 8U);
}

template <class T>
void byteReverse(T *out, const T *in, size_t byteCount)
{
	size_t count = (byteCount + sizeof(T) - 1) / sizeof(T);
	for (size_t i = 0; i < count; ++i)
		out[i] = byteReverse(in[i]);
}

template < bool H, size_t S, size_t D >
// static
inline
void
base_hash< H, S, D>::correct_endianess(ub4_t* out, const ub4_t* in, size_t byteCount)
{
	if (!check_big_endianess(HIGHFIRST))
		byteReverse(out, in, byteCount);
	else if (in != out)
		memcpy(out, in, byteCount);
}

template < bool H, size_t S, size_t D >
void
base_hash< H, S, D>::hash_block(const ub4_t* input)
{
	if (!check_big_endianess(HIGHFIRST))
	{
		byteReverse(_data, input, BLOCKSIZE);
		v_transform(_data);
	}
	else
		v_transform(input);
}

template <class T>
inline void get_user_key_big_endian(T* out, size_t outlen, const ub1_t* in, size_t inlen)
{
	const unsigned int U = sizeof(T);
	assert(inlen <= outlen * U);
	memcpy(out, in, inlen);
	memset((ub1_t*)out + inlen, 0, outlen * U - inlen);
#ifdef TERIMBER_LITTLE_ENDIAN
	byteReverse(out, out, inlen);
#endif
}

template <class T>
inline void get_user_key_little_endian(T* out, size_t outlen, const ub1_t* in, size_t inlen)
{
	const unsigned int U = sizeof(T);
	assert(inlen <= outlen * U);
	memcpy(out, in, inlen);
	memset((ub1_t*)out + inlen, 0, outlen * U - inlen);
#ifndef TERIMBER_LITTLE_ENDIAN
	byteReverse(out, out, inlen);
#endif
}

template <class T>
inline void GetBlockLittleEndian(const ub1_t *block, T &a, T &b, T &c, T &d)
{
#ifdef TERIMBER_LITTLE_ENDIAN
	a = ((T*)block)[0];
	b = ((T*)block)[1];
	c = ((T*)block)[2];
	d = ((T*)block)[3];
#else
	a = byteReverse(((T*)block)[0]);
	b = byteReverse(((T*)block)[1]);
	c = byteReverse(((T*)block)[2]);
	d = byteReverse(((T*)block)[3]);
#endif
}

template <class T>
inline void PutBlockLittleEndian(ub1_t* block, T a, T b, T c, T d)
{
#ifdef TERIMBER_LITTLE_ENDIAN
	((T*)block)[0] = a;
	((T*)block)[1] = b;
	((T*)block)[2] = c;
	((T*)block)[3] = d;
#else
	((T*)block)[0] = byteReverse(a);
	((T*)block)[1] = byteReverse(b);
	((T*)block)[2] = byteReverse(c);
	((T*)block)[3] = byteReverse(d);
#endif
}

template <class T>
inline void GetBlockBigEndian(const ub1_t *block, T &a, T &b, T &c, T &d)
{
#ifndef TERIMBER_LITTLE_ENDIAN
	a = ((T*)block)[0];
	b = ((T*)block)[1];
	c = ((T*)block)[2];
	d = ((T*)block)[3];
#else
	a = byteReverse(((T*)block)[0]);
	b = byteReverse(((T*)block)[1]);
	c = byteReverse(((T*)block)[2]);
	d = byteReverse(((T*)block)[3]);
#endif
}

template <class T>
inline
void
PutBlockBigEndian(ub1_t *block, T a, T b, T c, T d)
{
#ifndef TERIMBER_LITTLE_ENDIAN
	((T*)block)[0] = a;
	((T*)block)[1] = b;
	((T*)block)[2] = c;
	((T*)block)[3] = d;
#else
	((T*)block)[0] = byteReverse(a);
	((T*)block)[1] = byteReverse(b);
	((T*)block)[2] = byteReverse(c);
	((T*)block)[3] = byteReverse(d);
#endif
}

inline
ub4_t
round_down_of(ub4_t n, ub4_t m)
{
	return n - n % m;
}

inline
ub4_t
round_up_of(ub4_t n, ub4_t m)
{
	return round_down_of(n + m - 1, m);
}


///////////////////////////////////////////////////////////
//! provides an implementation of BlockSize()
template < size_t S, size_t D, size_t Mn, size_t Mx, size_t Q >
fixed_block_transformer< S, D, Mn, Mx, Q >::fixed_block_transformer()
{
}

template < size_t S, size_t D, size_t Mn, size_t Mx, size_t Q >
// static
size_t
fixed_block_transformer< S, D, Mn, Mx, Q >::key_length(size_t n)
{
	if (n < MIN_KEYLENGTH)
		return MIN_KEYLENGTH;
	else if (n > MAX_KEYLENGTH)
		return MAX_KEYLENGTH;
	else
		return round_up_of((ub4_t)n, KEYLENGTH_MULTIPLE);
}

template < size_t S, size_t D, size_t Mn, size_t Mx, size_t Q >
void
fixed_block_transformer< S, D, Mn, Mx, Q >::do_work(ub1_t* inout, size_t& length)
{
	size_t shift = 0;
	int _length = (int)length;

	while (_length > 0)
	{
		memset((ub1_t*)_data, 0, BLOCKSIZE);
		memcpy((ub1_t*)_data, inout + shift, __min(_length, (int)BLOCKSIZE));
		process_block(_data, inout + shift);
		_length -= BLOCKSIZE;
		shift += BLOCKSIZE;
	}

	length -= _length;
}

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_crypt_hpp_
