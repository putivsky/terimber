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

#include "crypt/cryptimpl.h"
#include "crypt/crypt.hpp"
#include "crypt/base64.h"
#include "base/number.hpp"
#include "base/string.hpp"
#include "base/memory.hpp"

// crypt factory
terimber_cryptaccess::terimber_cryptaccess()
{
}

//
// destructor
//
terimber_cryptaccess::~terimber_cryptaccess()
{
}

// creates md5 hash objects
// caller is responsible for destroying it
terimber_hash_md5* 
terimber_cryptaccess::get_hash_md5()
{
	return new TERIMBER::hash_md5_impl();
}

// creates sha256 hash objects
// caller is responsible for destroying it
terimber_hash_sha256* 
terimber_cryptaccess::get_hash_sha256()
{
	return new TERIMBER::hash_sha256_impl();
}

// creates RC6 crypt objects
// caller is responsible for destroying it
terimber_crypt_rc6* 
terimber_cryptaccess::get_crypt_rc6(const unsigned char* pwd, size_t len, bool md5_or_sha256)
{
	return new TERIMBER::crypt_rc6_impl(pwd, len, md5_or_sha256 ? TERIMBER::MD5 : TERIMBER::SHA256);
}

// creates AES (RIJNDAEL) crypt objects
// caller is responsible for destroying it
terimber_crypt_aes* 
terimber_cryptaccess::get_crypt_aes(const unsigned char* pwd, size_t len, bool md5_or_sha256)
{
	return new TERIMBER::crypt_aes_impl(pwd, len, md5_or_sha256 ? TERIMBER::MD5 : TERIMBER::SHA256);
}

// creates RSA crypt objects
// caller is responsible for destroying it
terimber_crypt_rsa* 
terimber_cryptaccess::get_crypt_rsa()
{
	return new TERIMBER::crypt_rsa_impl();
}

// creates base64 crypt objects
// caller is responsible for destroying it
terimber_crypt_base64* 
terimber_cryptaccess::get_crypt_base64()
{
	return new TERIMBER::crypt_base64_impl();
}
//////////////////////////////////////
BEGIN_TERIMBER_NAMESPACE
#pragma pack(4)

// virtual 
bool 
hash_md5_impl::make_hash(const unsigned char* in, size_t len, unsigned char* out)
{
	if (!in || !out)
		return false;

	md5::calculate_digest(_stathash, in, len);
	memcpy(out, _stathash, 16);
	return true;
}

// virtual 
bool 
hash_sha256_impl::make_hash(const unsigned char* in, size_t len, unsigned char* out)
{
	if (!in || !out)
		return false;

	sha256::calculate_digest(_stathash, in, len);
	memcpy(out, _stathash, 32);
	return true;
}

//////////////////////////////////////////
crypt_rc6_impl::crypt_rc6_impl(const unsigned char* pwd, size_t len, crypt_hash hash_type) :
	crypt(pwd, len, hash_type)
{
}

// virtual 
bool 
crypt_rc6_impl::encrypt(unsigned char* inout, size_t& len)
{
	return encode(inout, len, RC6);
}

// virtual 
bool 
crypt_rc6_impl::decrypt(unsigned char* inout, size_t& len)
{
	return decode(inout, len, RC6);
}

crypt_aes_impl::crypt_aes_impl(const unsigned char* pwd, size_t len, crypt_hash hash_type) :
	crypt(pwd, len, hash_type)
{
}

// virtual 
bool 
crypt_aes_impl::encrypt(unsigned char* inout, size_t& len)
{
	return encode(inout, len, RIJNDAEL);
}

// virtual 
bool 
crypt_aes_impl::decrypt(unsigned char* inout, size_t& len)
{
	return decode(inout, len, RIJNDAEL);
}

//////////////////////////////////////////
crypt_rsa_impl::crypt_rsa_impl() : rsa(0, false)
{
}

// virtual 
bool 
crypt_rsa_impl::generate_keys(size_t min_length_in_bits)
{
	return produce_keys((ub4_t)min_length_in_bits);
}

// virtual 
size_t 
crypt_rsa_impl::get_d_key_length() const
{
	return _d.min_encoded_size();
}

// virtual 
size_t 
crypt_rsa_impl::get_e_key_length() const
{
	return _e.min_encoded_size();
}

// virtual 
size_t 
crypt_rsa_impl::get_n_key_length() const
{
	return _n.min_encoded_size();
}

// virtual 
bool 
crypt_rsa_impl::save_d_key_to_memory(unsigned char* buf, size_t& len) const
{
	if (len < _d.min_encoded_size())
		return false;

	len = _d.encode(buf, _d.min_encoded_size());
	return len != 0;
}

// virtual 
bool 
crypt_rsa_impl::save_e_key_to_memory(unsigned char* buf, size_t& len) const
{
	if (len < _e.min_encoded_size())
		return false;

	len = _e.encode(buf, _e.min_encoded_size());
	return len != 0;
}

// virtual 
bool 
crypt_rsa_impl::save_n_key_to_memory(unsigned char* buf, size_t& len) const
{
	if (len < _n.min_encoded_size())
		return false;

	len = _n.encode(buf, _n.min_encoded_size());
	return len != 0;
}

bool 
crypt_rsa_impl::write_file(room_array< ub1_t >& buffer, const char* file_name) const
{
	// tries to open file
	FILE* fdesc = ::fopen(file_name, "wb");

	if (!fdesc) 
	{
		_error = "Can not open file: ";
		_error += file_name;
		return false;
	}

	if (1 != ::fwrite(buffer, buffer.size(), 1, fdesc))
	{
		_error = "Can't write file: ";
		_error += file_name;
		::fclose(fdesc);
		return false;
	}

	::fclose(fdesc);
	return true;
}
//
// saves keys to the file
// @file_name - file name
//
// virtual 
bool 
crypt_rsa_impl::save_d_key_to_file(const char* file_name) const
{
	int len = _d.min_encoded_size();
	room_array< ub1_t > buffer(len);
	_d.encode(buffer, len);
	return write_file(buffer, file_name);
}

// virtual 
bool 
crypt_rsa_impl::save_e_key_to_file(const char* file_name) const
{
	int len = _e.min_encoded_size();
	room_array< ub1_t > buffer(len);
	_e.encode(buffer, len);
	return write_file(buffer, file_name);
}

// virtual 
bool 
crypt_rsa_impl::save_n_key_to_file(const char* file_name) const
{
	int len = _n.min_encoded_size();
	room_array< ub1_t > buffer(len);
	_n.encode(buffer, len);
	return write_file(buffer, file_name);
}

// virtual 
bool 
crypt_rsa_impl::load_d_key_from_memory(const unsigned char* buf, size_t len)
{
	if (!buf || !len)
	{
		_error = "invalid buffer";
		return false;
	}

	_d.decode(buf, (ub4_t)len);
	return true;
}

// virtual 
bool 
crypt_rsa_impl::load_e_key_from_memory(const unsigned char* buf, size_t len)
{
	if (!buf || !len)
	{
		_error = "invalid buffer";
		return false;
	}

	_e.decode(buf, (ub4_t)len);
	return true;
}

// virtual 
bool 
crypt_rsa_impl::load_n_key_from_memory(const unsigned char* buf, size_t len)
{
	if (!buf || !len)
	{
		_error = "invalid buffer";
		return false;
	}

	_n.decode(buf, (ub4_t)len);
	return true;
}

bool 
crypt_rsa_impl::read_file(room_array< ub1_t >& buffer, const char* file_name) const
{
	// tries to open file
	FILE* fdesc = ::fopen(file_name, "rb");

	if (!fdesc) 
	{
		_error = "Can not open file: ";
		_error += file_name;
		return false;
	}

	// gets stats
	struct stat desc;
	::stat(file_name, &desc);
	int len = desc.st_size;

	if (!len)
	{
		_error = "File length is not available: ";
		_error += file_name;
		fclose(fdesc);
		return false;
	}

	// reads to memory
	buffer.resize(len);
	if (1 != ::fread(buffer, len, 1, fdesc))
	{
		_error = "Can't read file: ";
		_error += file_name;
		::fclose(fdesc);
		return false;
	}

	::fclose(fdesc);
	return true;
}

// virtual 
bool 
crypt_rsa_impl::load_d_key_from_file(const char* file_name)
{
	room_array< ub1_t > buffer(128);
	if (!read_file(buffer, file_name))
		return false;

	_d.decode(buffer, (ub4_t)buffer.size());
	return true;
}

// virtual 
bool 
crypt_rsa_impl::load_e_key_from_file(const char* file_name)
{
	room_array< ub1_t > buffer(128);
	if (!read_file(buffer, file_name))
		return false;

	_e.decode(buffer, (ub4_t)buffer.size());
	return true;
}

// virtual 
bool 
crypt_rsa_impl::load_n_key_from_file(const char* file_name)
{
	room_array< ub1_t > buffer(128);
	if (!read_file(buffer, file_name))
		return false;

	_n.decode(buffer, (ub4_t)buffer.size());
	return true;
}

// virtual 
bool 
crypt_rsa_impl::encode(const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len) const
{
	// creates integer
	if (!in || !in_len)
	{
		_error = "Invalid input buffer";
		return false;
	}

	if (!out)
	{
		_error = "Invalid output buffer";
		return false;
	}

	if (in_len > _n.min_encoded_size())
	{
		_error = "Too long input block";
		return false;
	}

	integer in_key;
	in_key.decode(in, (ub4_t)in_len);
	integer out_key;
	rsa::encode(in_key, out_key);
	size_t len = out_key.min_encoded_size();
	if (len > out_len)
	{
		out_len = len;
		_error = "Not enough size for output buffer";
		return false;
	}

	out_len = out_key.encode(out, (ub4_t)len);

	return true;
}

// virtual 
bool 
crypt_rsa_impl::decode(const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len) const
{
	// creates integer
	if (!in || !in_len)
	{
		_error = "Invalid input buffer";
		return false;
	}

	if (!out)
	{
		_error = "Invalid output buffer";
		return false;
	}

	if (in_len > _n.min_encoded_size())
	{
		_error = "Too long input block";
		return false;
	}

	integer in_key;
	in_key.decode(in, (ub4_t)in_len);
	integer out_key;
	rsa::decode(in_key, out_key);
	size_t len = out_key.min_encoded_size();
	if (len > out_len)
	{
		out_len = len;
		_error = "Not enough size for output buffer";
		return false;
	}

	out_len = out_key.encode(out, (ub4_t)len);

	return true;
}

// virtual 
const char* 
crypt_rsa_impl::get_last_error() const
{ 
	return _error; 
}

////////////////////////////////////////
crypt_base64_impl::crypt_base64_impl()
{
}

// virtual 
bool 
crypt_base64_impl::encode(const unsigned char* in, size_t in_len, unsigned char* out, size_t& out_len)
{
	if (!in || !out || !in_len)
		return false;
	// checks length
	if (out_len < (in_len / 3 + (in_len % 3 ? 1 : 0)) * 4)
		return false;

	out_len = in_len;
	encode_base64(out, in, out_len);
	return true;
}

// virtual 
bool 
crypt_base64_impl::decode(unsigned char* inout, size_t& len)
{
	if (!inout || !len)
		return false;

	decode_base64(inout, len);
	return true;
}



#pragma pack()
END_TERIMBER_NAMESPACE

