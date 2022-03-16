/*
 * AES functions
 * Copyright (c) 2003-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef __DAVE_AES_H__
#define __DAVE_AES_H__
#include "dave_base.h"

#define AES_BLOCK_SIZE 16

void * dave_aes_encrypt_init(u8 *key, ub len);

void dave_aes_encrypt(void *ctx, u8 *plain, u8 *crypt);

void dave_aes_encrypt_deinit(void *ctx);

void * dave_aes_decrypt_init(u8 *key, ub len);

void dave_aes_decrypt(void *ctx, u8 *crypt, u8 *plain);

void dave_aes_decrypt_deinit(void *ctx);

#endif /* AES_H */

