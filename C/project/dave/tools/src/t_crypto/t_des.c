/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "GLOBAL.H"
#include "DES.H"
#include "dave_tools.h"
#include "tools_log.h"

#define KEY_LEN (8)

/* input buffer must have extra space for padding */
#define DES_WORK_BUF_LEN	(input_len + 8)

static u8 _iv[KEY_LEN] = "davedes";

static ub
_PKCS7(u8 *buf, ub buf_len)
{
	ub pad_char;
	ub index;
	
	pad_char = 8 - buf_len % 8;
	
	for (index=0; index<pad_char; ++index)
	{
		buf[buf_len + index] = pad_char;
	}

	return buf_len + pad_char;
}

static ub
_reverse_PKCS7(u8 *buf, ub buf_len, s8 *fun, ub line)
{
	/* no checking is performed for now */
	ub pad_char;
	ub index;

	pad_char = buf[buf_len - 1];
	
	if(((buf_len % 8) != 0) || (pad_char == 0) || (pad_char > 8))
	{
		return buf_len;
	}

	for(index=0; index<pad_char; index++)
	{
		if(buf[buf_len - 1 - index] != pad_char)
		{
			TOOLSLOG("des check pad char fail! pad=%d check=%d index:%d <%s:%d>",
				pad_char, buf[buf_len - 1 - index], index, fun, line);

			return buf_len;
		}
	}

	return buf_len - pad_char;
}

static ub
_des(dave_bool encrypt, u8 *input, ub input_len, u8 *key, dave_bool pcks_flag, s8 *fun, ub line)
{
	DES_CBC_CTX context;
	ub len;
	u8 *work_buf;

	work_buf = dave_malloc(DES_WORK_BUF_LEN);

	DES_CBCInit(&context, key, (sw_uint8 *)_iv, encrypt);

	if((encrypt == dave_true) && (pcks_flag == dave_true))
	{
		len = _PKCS7(input, input_len);
	}
	else
	{
		len = input_len;
	}

	DES_CBCUpdate(&context, work_buf, input, (unsigned int)len);

	if((encrypt == dave_false) && (pcks_flag == dave_true))
	{
		len = _reverse_PKCS7(work_buf, input_len, fun, line);
	}

	if(encrypt == dave_true)
	{
		if(len > (input_len + 8))
		{
			TOOLSLOG("invalid encrypt:%d len:%d input_len:%d <%s:%d>",
				encrypt, len, input_len, fun, line);

			len = input_len;
		}
	}
	else
	{
		if(len > input_len)
		{
			TOOLSLOG("invalid encrypt:%d len:%d input_len:%d <%s:%d>",
				encrypt, len, input_len, fun, line);

			len = input_len;
		}
	}

	dave_memcpy(input, work_buf, len);

	dave_free(work_buf);

	return len;
}

// =====================================================================

ub
__t_crypto_des_encode__(u8 *key, ub key_len, u8 *txt, ub txt_len, dave_bool pcks_flag, s8 *fun, ub line)
{
	ub len;

	if(key_len != KEY_LEN)
	{
		len = 0;
	}
	else
	{
		len = _des(dave_true, txt, txt_len, key, pcks_flag, fun, line);
	}

	return len;
}

ub
__t_crypto_des_decode__(u8 *key, ub key_len, u8 *txt, ub txt_len, dave_bool pcks_flag, s8 *fun, ub line)
{
	ub len;

	if(key_len != KEY_LEN)
	{
		TOOLSABNOR("des has invalid key len:%d <%s:%d>", key_len, fun, line);
		len = 0;
	}
	else
	{
		len = _des(dave_false, txt, txt_len, key, pcks_flag, fun, line);
	}

	return len;
}

