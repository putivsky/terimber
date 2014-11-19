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

#ifndef _terimber_crypt_h_
#define _terimber_crypt_h_

#include "crypt/integer.h"
#include "base/string.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

//////////////////////////////////////////////
template < bool H, size_t S, size_t D >
class base_hash
{
public:
	enum {HIGHFIRST = H, BLOCKSIZE = S, DIGESTSIZE = D};

	base_hash();
	virtual ~base_hash();
	
	inline size_t digest_size() const;
	//! use this if your input is short and you don't want to call Update() and Final() seperately
	inline void calculate_digest(ub1_t *digest, const ub1_t *input, size_t length);

protected:
	void update(const ub1_t *input, size_t len);
	ub4_t hash_multiple_blocks(const ub4_t *input, size_t length);
	void final(ub1_t *hash);
	void pad_last_block(size_t lastBlockSize, ub1_t padFirst = 0x80);
	void reinit();
	inline static void correct_endianess(ub4_t* out, const ub4_t* in, size_t byteCount);
	void hash_block(const ub4_t* input);
	
	virtual void v_transform(const ub4_t* data) = 0;
	virtual void v_init() = 0;

	ub4_t _countLo, _countHi;	// 64-bit bit count
	ub4_t _data[BLOCKSIZE / sizeof(ub4_t)];	// Data buffer
	ub4_t _digest[DIGESTSIZE / sizeof(ub4_t)];	// Message digest
};

class sha256 : public base_hash< true, 64, 32 >
{
public:
	sha256();

protected:

	static void transform(ub4_t *digest, const ub4_t *data);
	virtual void v_init();
	virtual void v_transform(const ub4_t *data);

	const static ub4_t K[64];
};

class md5 : public base_hash< false, 64, 16 >
{
public:
	md5();
	static void transform(ub4_t *digest, const ub4_t *data);

protected:
	virtual void v_init();
	virtual void v_transform(const ub4_t *data);
};

////////////////////////////////////////////////////////
// ! block_transformer
class block_transformer
{
public:
	//!
	virtual ~block_transformer();

	//! encrypts or decrypts one block, may assume in != out
	//! \pre size of in and out == block_size()
	virtual void process_block(const ub1_t *in, ub1_t *out) const = 0;
};

//! provides an implementation of BlockSize()
template < size_t S, size_t D, size_t Mn, size_t Mx, size_t Q = 1 >
class fixed_block_transformer : public block_transformer
{
public:
	enum { BLOCKSIZE = S, MIN_KEYLENGTH = Mn, MAX_KEYLENGTH = Mx, DEFAULT_KEYLENGTH = D, KEYLENGTH_MULTIPLE = Q };

	fixed_block_transformer();
	static size_t key_length(size_t n);
	// do real work
	void do_work(ub1_t* inout, size_t& lenght);
protected:
	ub1_t _data[BLOCKSIZE];	// Data buffer
};

/// base class, do not use directly
class rc6 : public fixed_block_transformer< 16, 16, 0, 255 >
{
public:
	enum { DEFAULT_ROUNDS = 20 };
protected:
	rc6(const ub1_t* key, size_t keyLen, size_t rounds);

	const size_t			_rounds;  // number of rounds
	room_array< ub4_t >	_table;  // expanded key table
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#RC6">RC6</a>
class rc6_encrypt : public rc6
{
public:
	rc6_encrypt(const ub1_t* key, size_t keyLen = DEFAULT_KEYLENGTH, size_t rounds = DEFAULT_ROUNDS);
protected:
	virtual void process_block(const ub1_t* in, ub1_t* out) const;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#RC6">RC6</a>
class rc6_decrypt : public rc6
{
public:
	rc6_decrypt(const ub1_t* key, size_t keyLen = DEFAULT_KEYLENGTH, size_t rounds = DEFAULT_ROUNDS);
protected:
	virtual void process_block(const ub1_t* in, ub1_t* out) const;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Rijndael">Rijndael</a>
class rijndael : public fixed_block_transformer< 16, 16, 16, 32, 8 >
{
protected:
	rijndael(const ub1_t* userKey, size_t keylength);

	static const ub4_t Te0[256];
	static const ub4_t Te1[256];
	static const ub4_t Te2[256];
	static const ub4_t Te3[256];
	static const ub4_t Te4[256];

	static const ub4_t Td0[256];
	static const ub4_t Td1[256];
	static const ub4_t Td2[256];
	static const ub4_t Td3[256];
	static const ub4_t Td4[256];

	static const ub4_t rcon[];

	size_t					_rounds;
	room_array< ub4_t >	_key;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Rijndael">Rijndael</a>
class rijndael_encrypt : public rijndael
{
public:
	rijndael_encrypt(const ub1_t* userKey, size_t keylength = DEFAULT_KEYLENGTH);
	virtual void process_block(const ub1_t* in, ub1_t* out) const;
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Rijndael">Rijndael</a>
class rijndael_decrypt : public rijndael
{
public:
	rijndael_decrypt(const ub1_t* userKey, size_t keylength = DEFAULT_KEYLENGTH);
	virtual void process_block(const ub1_t* in, ub1_t* out) const;
};

enum crypt_algorithm
{
	RC6,
	RIJNDAEL
};

enum crypt_hash
{
	MD5,
	SHA256
};

class crypt
{
public:
	crypt(const ub1_t* key, size_t len, crypt_hash hash = MD5);
	bool encode(ub1_t* buffer, size_t& len, crypt_algorithm alogithm = RC6) const;
	bool decode(ub1_t* buffer, size_t& len, crypt_algorithm alogithm = RC6) const;
	size_t block_size(crypt_algorithm alogithm = RC6) const;
private:
	ub1_t				_hash[64];
	size_t				_size;
};

class rsa
{
public:
	// generates all keys
	rsa(ub4_t keybits, bool gen_keys = true);

	// for encoding/decoding
	rsa(const integer& ed, 
		const integer& n
		);

	void encode(const integer& in, integer& out) const;
	void decode(const integer& in, integer& out) const;

	inline const integer& get_e() const { return _e; }
	inline const integer& get_n() const { return _n; }
	inline const integer& get_d() const { return _d; }
protected:
	bool produce_keys(ub4_t keybits);
protected:
	integer _e, _n, _d;
	random_generator _rng;
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_crypt_h_
