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

#ifndef _terimber_cryptimpl_h_
#define _terimber_cryptimpl_h_

#include "crypt/crypt.h"
#include "crypt/cryptaccess.h"

BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// abstract interface for encryption/decryption and hash functionality
class hash_md5_impl : public terimber_hash_md5, public md5
{
public:
	// 
	// creates a hash function of internal buffer
	// exactly 16 bytes will be written to the output buffer
	//
	virtual bool make_hash(const unsigned char* in, size_t len, unsigned char* out);
private:
	ub1_t _stathash[16];
};

class hash_sha256_impl : public terimber_hash_sha256, public sha256
{
public:
	// 
	// creates a hash function of internal buffer
	// exactly 32 bytes will be written to the output buffer
	//
	virtual bool make_hash(const unsigned char* in, size_t len, unsigned char* out);
private:
	ub1_t _stathash[32];
};

class crypt_rc6_impl : public terimber_crypt_rc6, public crypt
{
public:
	//
	// constructor
	//
	crypt_rc6_impl(const unsigned char* pwd, size_t len, crypt_hash hash_type);
	// 
	// encrypts input buffer placing encrypted bytes into the same buffer
	// returns the actual length of encrypted bytes
	//
	virtual bool encrypt(unsigned char* inout, size_t& len);
	// 
	// decrypts input buffer placing decrypted bytes into the same buffer
	// returns the actual length of decrypted bytes
	//
	virtual bool decrypt(unsigned char* inout, size_t& len);
};

class crypt_aes_impl : public terimber_crypt_aes, public crypt
{
public:
	//
	// constructor
	//
	crypt_aes_impl(const unsigned char* pwd, size_t len, crypt_hash hash_type);
	// 
	// encrypts input buffer placing encrypted bytes into the same buffer
	// returns the actual length of encrypted bytes
	//
	virtual bool encrypt(unsigned char* inout, size_t& len);
	// 
	// decrypts input buffer placing decrypted bytes into the same buffer
	// returns the actual length of decrypted bytes
	//
	virtual bool decrypt(unsigned char* inout, size_t& len);
};

class crypt_rsa_impl : public terimber_crypt_rsa, public rsa
{
public:
	//
	// constructor
	//
	crypt_rsa_impl();
	// 
	// generate the public/private keys
	// actually, the public key is a pair of the d (decryption key) and the n (neutral key)
	// and the private key is a pair e (enctyption key) and n (neutral key)
    // store internally d, e, n keys
	// @min_length_in_bits - minimum length of keys in bits
	// NB! the free version will restrict the max length by 256 bits.
	//
	virtual bool generate_keys(size_t min_length_in_bits);

	//
	// estimates the required size for keys
	//
	virtual size_t get_d_key_length() const;
	virtual size_t get_e_key_length() const;
	virtual size_t get_n_key_length() const;

	//
	// saves keys to the provided memory
	// @buf - caller allocates the buffer and provides the length of it.
	// @len - input/output - takes the input buffer length and returns the actual number of bytes written
	//
	virtual bool save_d_key_to_memory(unsigned char* buf, size_t& len) const;
	virtual bool save_e_key_to_memory(unsigned char* buf, size_t& len) const;
	virtual bool save_n_key_to_memory(unsigned char* buf, size_t& len) const;

	//
	// saves keys to the file
	// @file_name - file name
	//
	virtual bool save_d_key_to_file(const char* file_name) const;
	virtual bool save_e_key_to_file(const char* file_name) const;
	virtual bool save_n_key_to_file(const char* file_name) const;

	//
	// loads correspondent key from memory
	//
	virtual bool load_d_key_from_memory(const unsigned char* buf, size_t len);
	virtual bool load_e_key_from_memory(const unsigned char* buf, size_t len);
	virtual bool load_n_key_from_memory(const unsigned char* buf, size_t len);

	//
	// loads correspondent key from file
	//
	virtual bool load_d_key_from_file(const char* file_name);
	virtual bool load_e_key_from_file(const char* file_name);
	virtual bool load_n_key_from_file(const char* file_name);

	//
	// encrypt block
	// the length of the block can not be greater than the key length
	// @in points to the input buffer with @in_len length
	// @out - is output buffer allocated by caller with lenth @out_len
	// @out_len will contains the actual number of bytes written
	virtual bool encode(const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len) const;

	//
	// decrypt block
	// the length of the block can not be greater than the key length
	// @in points to the input buffer with @in_len length
	// @out - is output buffer allocated by caller with lenth @out_len
	// @out_len will contains the actual number of bytes written
	virtual bool decode(const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len) const;
	//
	// returns error description
	//
	virtual const char* get_last_error() const;
private:
	bool read_file(room_array< ub1_t >& buffer, const char* file_name) const;
	bool write_file(room_array< ub1_t >& buffer, const char* file_name) const;
private:
	mutable string_t _error;
};

class crypt_base64_impl : public terimber_crypt_base64
{
public:
	crypt_base64_impl();

	// 
	// encode @in buffer 
    // out buffer is supposed to be larger then in buffer on 1/3
	//
	virtual bool encode(const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len);
	// 
	// decodes input buffer placing decoded bytes into the same buffer
	// returns the actual length of decoded bytes
	//
	virtual bool decode(unsigned char* inout, size_t& len);
};

#pragma pack()
END_TERIMBER_NAMESPACE

#endif // _terimber_cryptimpl_h_
