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

#ifndef _terimber_cryptaccess_h_
#define _terimber_cryptaccess_h_

// class provides MD5 hash functionality
class terimber_hash_md5
{
public:
	//
	// destructor
	//
	virtual ~terimber_hash_md5() {}

	// 
	// creates a hash function of input buffer @in with length @len
	// exactly 16 bytes will be written to the output buffer @out
	// caller is responsible to allocate @out buffer
	//
	virtual bool make_hash(const unsigned char* in, size_t len, unsigned char* out) = 0;
};

// class provides SAH256 hash functionality
class terimber_hash_sha256
{
public:
	//
	// destructor
	//
	virtual ~terimber_hash_sha256() {}

	// 
	// creates a hash function of input buffer @in with length @len
	// exactly 32 bytes will be written to the output buffer @out
	// caller is responsible to allocate @out buffer
	//
	virtual bool make_hash(const unsigned char* in, size_t len, unsigned char* out) = 0;
};

// class provides RC6 encryption/decryption functionality
class terimber_crypt_rc6
{
public:
	//
	// destructor
	//
	virtual ~terimber_crypt_rc6() {}

	// 
	// encrypts input buffer placing encrypted bytes to the same buffer @inout
	// returns the actual length of encrypted bytes
	// NB! encryption algorithm processes 16 bytes blocks, 
	// caller is responsible to allocate inout buffer to the @len rounded to 16 bytes blocks
	// even though input length (@len % 16 != 0)
	virtual bool encrypt(unsigned char* inout, size_t& len) = 0;
	// 
	// decrypts input buffer placing decrypted bytes to the same buffer
	// returns the actual length of decrypted bytes
	// the length of @inout buffer is suppose to be (@len % 16 == 0)
	//
	virtual bool decrypt(unsigned char* inout, size_t& len) = 0;
};

// class provides AES (RIJNDAEL) encryption/decryption functionality
class terimber_crypt_aes
{
public:
	//
	// destructor
	//
	virtual ~terimber_crypt_aes() {}

	// 
	// encrypt inputs buffer placing encrypted bytes to the same buffer @inout
	// returns the actual length of encrypted bytes
	// NB! encryption algorithm processes 16 bytes blocks, 
	// caller is responsible to allocate inout buffer to the @len rounded to 16 bytes blocks
	// even though input length (@len % 16 != 0)
	virtual bool encrypt(unsigned char* inout, size_t& len) = 0;

	// 
	// decrypts input buffer placing decrypted bytes to the same buffer
	// returns the actual length of decrypted bytes
	// the length of @inout buffer is suppose to be (@len % 16 == 0)
	//
	virtual bool decrypt(unsigned char* inout, size_t& len) = 0;
};

// class provides RSA private/public keys encryption/decryption functionality
class terimber_crypt_rsa
{
public:
	//
	// destructor
	//
	virtual ~terimber_crypt_rsa() {}

	// 
	// generates the public/private keys
	// actually public key is a pair of d (decryption key) and n (modulus)
	// and private key is a pair e (encryption key) and n (modulus)
    // store internally d, e, n keys
	// @min_length_in_bits - minimum length of keys in bits
	// NB! min can't be less then 16 bits 
	// NB! the free version will restrict the max length by 256 bits.
	// production version doesn't have a restriction
	// can produce 1024 bits length keys in less than a second on 1GHz CPU
	virtual bool generate_keys(size_t min_length_in_bits) = 0;

	//
	// estimates the required size for keys in bytes
	//
	virtual size_t get_d_key_length() const = 0;
	virtual size_t get_e_key_length() const = 0;
	virtual size_t get_n_key_length() const = 0;

	//
	// saves keys to the provided memory
	// @buf - caller allocates the buffer and provides the length of it.
	// @len - input/output - takes the input buffer length and returns the actual number of bytes written
	//
	virtual bool save_d_key_to_memory(unsigned char* buf, size_t& len) const = 0;
	virtual bool save_e_key_to_memory(unsigned char* buf, size_t& len) const = 0;
	virtual bool save_n_key_to_memory(unsigned char* buf, size_t& len) const = 0;

	//
	// save keys to the file
	// @file_name - file name
	//
	virtual bool save_d_key_to_file(const char* file_name) const = 0;
	virtual bool save_e_key_to_file(const char* file_name) const = 0;
	virtual bool save_n_key_to_file(const char* file_name) const = 0;

	//
	// loads correspondent key from memory
	//
	virtual bool load_d_key_from_memory(const unsigned char* buf, size_t len) = 0;
	virtual bool load_e_key_from_memory(const unsigned char* buf, size_t len) = 0;
	virtual bool load_n_key_from_memory(const unsigned char* buf, size_t len) = 0;

	//
	// loads correspondent key from file
	//
	virtual bool load_d_key_from_file(const char* file_name) = 0;
	virtual bool load_e_key_from_file(const char* file_name) = 0;
	virtual bool load_n_key_from_file(const char* file_name) = 0;

	//
	// encryption/decryption and digital signature
	// the next two method can be used for encryption/decryption
	// and digital signatures as well
	//

	//
	// encrypt block
	// the length of block can not be greater then the key length
	// @in points to the input buffer with @in_len length
	// @out - is output buffer allocated by caller with lenth @out_len
	// @out_len will contains the actual number of bytes written
	// the applied algorithm as follow
	// out = pow(in, e) % n;
	// where in is an input plain message, out is output a ciphertext
	// or in is an encrypted digital signature, out is an output plain signature

	// NB! the length of input block can't be longer then the generated modulus n - key in bytes
	virtual bool encode(const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len) const = 0;

	//
	// decrypt block
	// the length of block can not be greater then key length
	// @in points to the input buffer with @in_len length
	// @out - is output buffer allocated by caller with lenth @out_len
	// @out_len will contains the actual number of bytes written
	// the applied algorithm as follow
	// out = pow(in, d) % n;
	// where in is a ciphertext plain message, out is an input plain message
	// or in an input plain signature, out is an encrypted digital signature

	// NB! the length of input block can't be longer then the generated modulus n - key in bytes
	virtual bool decode(const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len) const = 0;

	//
	// returns error description
	//
	virtual const char* get_last_error() const = 0;
};

class terimber_crypt_base64
{
public:
	//
	// destructor
	//
	virtual ~terimber_crypt_base64() {}

	// 
	// encode @in buffer 
    // @out buffer is supposed to be larger then @in buffer (rounded 3 bytes blocks) on 1/4
	// out_len >= (in_len / 3 + (in_len % 3 ? 1 : 0)) * 4
	//
	virtual bool encode(const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len) = 0;
	// 
	// decodes input buffer placing decoded bytes to the same buffer
	// returns the actual length of decoded bytes
	//
	virtual bool decode(unsigned char* inout, size_t& len) = 0;
};

// crypt factory
class terimber_cryptaccess
{
public:
	//
	// constructor
	//
	terimber_cryptaccess();

	//
	// destructor
	//
	~terimber_cryptaccess();

	// creates md5 hash objects
	// caller is responsible for destroying it
    terimber_hash_md5* get_hash_md5();

	// creates sha256 hash objects
	// caller is responsible for destroying it
	terimber_hash_sha256* get_hash_sha256();

	// create RC6 crypt objects
	// caller is responsible for destroying it
	// @pwd is a password
	// @len is the password length
	// @md5_or_sha256 - true for md5, false for sha256 hash alogorithm 
	// will be applied to the password before encrypt/decrypt
	terimber_crypt_rc6* get_crypt_rc6(const unsigned char* pwd, size_t len, bool md5_or_sha256);

	// create AES (RIJNDAEL) crypt objects
	// caller is responsible for destroying it
	// @pwd is a password
	// @len is the password length
	// @md5_or_sha256 - true for md5, false for sha256 hash alogorithm 
	// will be applied to the password before encrypt/decrypt
	terimber_crypt_aes* get_crypt_aes(const unsigned char* pwd, size_t len, bool md5_or_sha256);
	
	// creates RSA crypt objects
	// caller is responsible for destroying it
	terimber_crypt_rsa* get_crypt_rsa();

	// creates base64 crypt objects
	// caller is responsible for destroying it
	terimber_crypt_base64* get_crypt_base64();
};

#endif // _terimber_cryptaccess_h_
