#include "allinc.h"
#include "log.h"
#include "crypt/cryptaccess.h"
#include "crypt/crypt.h"
#include "base/common.hpp"

#define MAX_PACKET_SIZE 1024

int crypt_unittest(size_t wait, terimber_log* log)
{
	const size_t buf_len = 256;
	const char* in = "Hi!+junk @#$%^&*()_+1234567890=";
	size_t in_len = strlen(in);
	ub1_t out16[16];
	ub1_t out32[32];
	char hex32[33];
	terimber_cryptaccess cracc;
	terimber_hash_md5* obj_md5 = cracc.get_hash_md5();

	char big[1024*64 + 333];
	memset(big, 'a', sizeof(big) - 1);
	big[sizeof(big) - 1] = 0;

	//obj_md5->make_hash((const unsigned char*)in, in_len, out16);
	obj_md5->make_hash((const unsigned char*)big, sizeof(big) - 1, out16);

	for (int ii = 0; ii < 16; ++ii)
		TERIMBER::byte_to_hex(hex32 + ii * 2, out16[ii]);

	hex32[32] = 0;

	delete obj_md5;

	terimber_hash_sha256* obj_sha256 = cracc.get_hash_sha256();
	obj_sha256->make_hash((const unsigned char*)in, in_len, out32);
	char hex64[65];
	for (int iii = 0; iii < 32; ++iii)
		TERIMBER::byte_to_hex(hex64 + iii * 2, out32[iii]);

	hex64[64] = 0;
	delete obj_sha256;


	ub1_t out[buf_len];

	size_t out_len = in_len;
	memcpy(out, in, in_len);

	terimber_crypt_aes* obj_aes_md5 = cracc.get_crypt_aes((const unsigned char*)in, in_len, true);
	obj_aes_md5->encrypt(out, out_len);
	obj_aes_md5->decrypt(out, out_len);
	delete obj_aes_md5;

	out_len = in_len;
	memcpy(out, in, in_len);

	terimber_crypt_aes* obj_aes_sha256 = cracc.get_crypt_aes((const unsigned char*)in, in_len, false);
	obj_aes_sha256->encrypt(out, out_len);
	obj_aes_sha256->decrypt(out, out_len);
	delete obj_aes_sha256;

	out_len = in_len;
	memcpy(out, in, in_len);
	terimber_crypt_rc6* obj_rc6_md5 = cracc.get_crypt_rc6((const unsigned char*)in, in_len, true);
	obj_rc6_md5->encrypt(out, out_len);
	obj_rc6_md5->decrypt(out, out_len);
	delete obj_rc6_md5;

	out_len = in_len;
	memcpy(out, in, in_len);

	terimber_crypt_rc6* obj_rc6_sha256 = cracc.get_crypt_rc6((const unsigned char*)in, in_len, false);
	obj_rc6_sha256->encrypt(out, out_len);
	obj_rc6_sha256->decrypt(out, out_len);
	delete obj_rc6_sha256;

	terimber_crypt_rsa* obj_rsa = cracc.get_crypt_rsa();
	bool res = obj_rsa->generate_keys(buf_len);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}

	size_t l = obj_rsa->get_d_key_length();
	l = obj_rsa->get_e_key_length();
	l = obj_rsa->get_n_key_length();

	out_len = buf_len;
	res = obj_rsa->encode((const unsigned char*)in, in_len, out, out_len);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	unsigned char store[buf_len];
	size_t store_len = buf_len;
	memset(store, 0, store_len);
	res = obj_rsa->decode(out, out_len, store, store_len);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}

	unsigned char rsa_e_key_buf[buf_len], rsa_d_key_buf[buf_len], rsa_n_key_buf[buf_len];

	size_t e_l = buf_len;
	res = obj_rsa->save_d_key_to_memory(rsa_e_key_buf, e_l);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}

	size_t d_l = buf_len;
	res = obj_rsa->save_e_key_to_memory(rsa_d_key_buf, d_l);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}

	size_t n_l = buf_len;
	res = obj_rsa->save_n_key_to_memory(rsa_n_key_buf, n_l);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}

	res = obj_rsa->load_d_key_from_memory(rsa_e_key_buf, e_l);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	res = obj_rsa->load_e_key_from_memory(rsa_d_key_buf, d_l);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	res = obj_rsa->load_n_key_from_memory(rsa_n_key_buf, n_l);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}

	res = obj_rsa->save_d_key_to_file("./test_d_key.txt");
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	res = obj_rsa->save_e_key_to_file("./test_e_key.txt");
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	res = obj_rsa->save_n_key_to_file("./test_n_key.txt");
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}

	res = obj_rsa->load_d_key_from_file("./test_d_key.txt");
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	res = obj_rsa->load_e_key_from_file("./test_e_key.txt");
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	res = obj_rsa->load_n_key_from_file("./test_n_key.txt");
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}

	out_len = buf_len;
	res = obj_rsa->encode((const unsigned char*)in, in_len, out, out_len);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	store_len = buf_len;
	memset(store, 0, store_len);
	res = obj_rsa->decode(out, out_len, store, store_len);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	delete obj_rsa; 

	terimber_crypt_base64* obj_base64 = cracc.get_crypt_base64();
	out_len = buf_len;
	res = obj_base64->encode((const unsigned char*)in, in_len, out, out_len);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}
	res = obj_base64->decode(out, out_len);
	if (!res)
	{
		printf("rsa error: %s", obj_rsa->get_last_error());
		return -1;
	}

	delete obj_base64;


	return 0;
}
