/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_tools.h"
#include "rxtx_secure.h"
#include "rxtx_tools.h"
#include "rxtx_log.h"

static u8 _stack_simple_secure_key[DAVE_DES_KEY_LEN] = {0x78, 0x11, 0x9c, 0x75, 0xee, 0xac, 0xbd, 0xd1};

// =====================================================================

MBUF *
rxtx_simple_encode_request(MBUF *data)
{
	MBUF *encode_package;

	encode_package = dave_mmalloc(data->tot_len + 128);

	encode_package->len = encode_package->tot_len = t_a2b_mbuf_to_buf(data, dave_mptr(encode_package), encode_package->len);

	encode_package->len = encode_package->tot_len = rxtx_build_crc(dave_mptr(encode_package), encode_package->len);

	encode_package->len = encode_package->tot_len =
		t_crypto_des_encode(_stack_simple_secure_key, DAVE_DES_KEY_LEN, dave_mptr(encode_package), encode_package->len, dave_true);

	return encode_package;
}

void
rxtx_simple_encode_release(MBUF *encode_package)
{
	dave_mfree(encode_package);
}

u8 *
rxtx_simple_decode_request(u8 *decode_package, ub decode_package_len, ub *package_len)
{
	ub decode_length;

	decode_length = t_crypto_des_decode(_stack_simple_secure_key, DAVE_DES_KEY_LEN, decode_package, decode_package_len, dave_true);

	decode_length = rxtx_check_crc(decode_package, decode_length);

	if(package_len != NULL)
	{
		*package_len = decode_length;
	}

	if(decode_length == 0)
	{
		return NULL;
	}
	else
	{
		return decode_package;
	}
}

void
rxtx_simple_decode_release(u8 *package)
{

}

#endif

