/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_CRYPTO_H__
#define __T_CRYPTO_H__

#define DAVE_AES_KEY_LEN (8)
#define DAVE_DES_KEY_LEN (8)
#define DAVE_MD5_HASH_LEN (16)
#define DAVE_MD5_HASH_STR_LEN (DAVE_MD5_HASH_LEN * 2 + 1)
#define DAVE_SHA1_HASH_LEN (20) 

ub __t_crypto_des_encode__(u8 *key, ub key_len, u8 *txt, ub txt_len, dave_bool pcks_flag, s8 *fun, ub line);
ub __t_crypto_des_decode__(u8 *key, ub key_len, u8 *txt, ub txt_len, dave_bool pcks_flag, s8 *fun, ub line);

u32 t_crypto_crc32(u8 *block, ub len);
u32 t_crypto_mcrc32(MBUF *m);

ub t_base64_encode(const u8 *in, ub inlen, s8 *out, ub outlen);
dave_bool t_base64_decode(const s8 *in, ub inlen, u8 *out, ub *outlen);

dave_bool t_crypto_md5(u8 md5_hash[DAVE_MD5_HASH_LEN], u8 *encode_ptr, ub encode_len);
dave_bool t_crypto_md5_str(s8 md5_str[DAVE_MD5_HASH_STR_LEN], u8 *encode_ptr, ub encode_len);

#define t_crypto_des_encode(key, key_len, txt, txt_len, pcks_flag) __t_crypto_des_encode__(key, key_len, txt, txt_len, pcks_flag, (s8 *)__func__, (ub)__LINE__)
#define t_crypto_des_decode(key, key_len, txt, txt_len, pcks_flag) __t_crypto_des_decode__(key, key_len, txt, txt_len, pcks_flag, (s8 *)__func__, (ub)__LINE__)


// $$$$$$$$$$$$$$$$$$interface compatible$$$$$$$$$$$$$$$$$$$$
#define dave_des_encode t_crypto_des_encode
#define dave_des_decode t_crypto_des_decode


#endif

