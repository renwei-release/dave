/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(ICONV_3RDPARTY)
#include "dave_tools.h"
#include "dave_os.h"
#include "iconv.h"
#include "party_log.h"

#define INVALID_ICONV 0xffffffffffffffff

static iconv_t _gbk_to_utf8_iconv = NULL;
static iconv_t _utf8_to_gbk_iconv = NULL;
static iconv_t _ascii_to_utf8_iconv = NULL;
static iconv_t _utf8_to_ucs2be_iconv = NULL;
static iconv_t _ucs2be_to_utf8_iconv = NULL;
static iconv_t _ucs2le_to_utf8_iconv = NULL;
static iconv_t _ucs2le_to_ascii_iconv = NULL;
static iconv_t _ucs2le_to_gbk_iconv = NULL;
static iconv_t _ucs4le_to_utf8_iconv = NULL;
static iconv_t _latin1_to_utf8_iconv = NULL;
static iconv_t _utf8_to_latin1_iconv = NULL;
static iconv_t _jis_x0208_1990_to_utf8_iconv = NULL;
static iconv_t _iso_8859_5_to_utf8_iconv = NULL;
static iconv_t _iso_8859_8_to_utf8_iconv = NULL;
static iconv_t _iso_2022_jp_to_utf8_iconv = NULL;
static iconv_t _jis_x0212_1990_to_utf8_iconv = NULL;
static iconv_t _ksc_5601_to_utf8_iconv = NULL;
static iconv_t _utf8_to_utf16be_iconv = NULL;
static iconv_t _utf16be_to_utf8_iconv = NULL;

static dave_bool
_iconv_ptr_check(iconv_t ptr)
{
	if(((ub)ptr == INVALID_ICONV) || (ptr == NULL))
		return dave_false;
	else
		return dave_true;
}

static void
_iconv_error(u8 *in, ub in_len)
{
	perror("iconv");

	switch(errno)
	{
		case E2BIG:
				PARTYABNOR("E2BiG");
			break;
		case EILSEQ:
				PARTYABNOR("EILSEQ");
			break;
		case EINVAL:
				PARTYABNOR("EINVAL");
			break;
		default:
				PARTYABNOR("errno:%d", errno);
			break;
	}
	dave_printf_hex("iconv_error", (u8 *)in, in_len);
}

static void
_iconv_fill_8_zero(char *out, ub out_len)
{
	if(out_len > 8)
	{
		out_len = 8;
	}
	else
	{
		PARTYLOG("Eight zeros are not filled and may be confused!");
	}

	dave_memset(out, 0x00, out_len);
}

static ub
_iconv_all(iconv_t iptr, s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	size_t in_len, out_len, iconv_size;
	char *bak_out_buf, **pin, **pout;

	if(_iconv_ptr_check(iptr) == dave_false)
	{
		PARTYABNOR("invalid iconv!");
		return 0;
	}

	in_len = (size_t)in_buf_len;

	out_len = (size_t)out_buf_len;

	bak_out_buf = (char *)out_buf;

	pin = (char **)&in_buf;

	pout = (char **)&out_buf;

	iconv_size = iconv(iptr, pin, (size_t *)(&in_len), pout, (size_t *)(&out_len));
	if(iconv_size == -1)
	{
		_iconv_error((u8 *)in_buf, in_buf_len);
	}

	if(((ub)out_buf - (ub)bak_out_buf) != (out_buf_len - out_len))
	{
		PARTYABNOR("Arithmetic error! %lx/%lx %d/%d",
			bak_out_buf, out_buf, out_buf_len, out_len);
	}

	if(out_len > 0)
	{
		_iconv_fill_8_zero(&(bak_out_buf[out_buf_len-out_len]), out_len);
	}
	else
	{
		PARTYLOG("maybe iconv out buffer overflow!");
	}

	if((in_buf_len > 0) && ((out_buf_len - out_len) == 0))
	{
		PARTYLOG("maybe iconv failed! %d", in_buf_len);
	}

	return (out_buf_len - out_len);
}

static iconv_t
_iconv_open(char *src_code, char *dst_code)
{
	iconv_t iconv_ptr;

	iconv_ptr = iconv_open(dst_code, src_code);

	if(_iconv_ptr_check(iconv_ptr) == dave_false)
	{
		PARTYABNOR("invalid %s->%s:%lx", src_code, dst_code, iconv_ptr);
	}

	return iconv_ptr;
}

static iconv_t
_iconv_close(iconv_t iconv_ptr)
{
	if(_iconv_ptr_check(iconv_ptr) == dave_true)
	{
		iconv_close(iconv_ptr);
	}

	return NULL;
}

// =====================================================================

void
dave_iconv_init(void)
{
	/* https://www.gnu.org/savannah-checkouts/gnu/libiconv/documentation/libiconv-1.15/iconv_open.3.html */
	_gbk_to_utf8_iconv = _iconv_open("gbk", "utf-8");
	_utf8_to_gbk_iconv = _iconv_open("utf-8", "gbk");
	_ascii_to_utf8_iconv = _iconv_open("ascii", "utf-8");
	_utf8_to_ucs2be_iconv = _iconv_open("utf-8", "ucs-2be");
	_ucs2be_to_utf8_iconv = _iconv_open("ucs-2be", "utf-8");
	_ucs2le_to_utf8_iconv = _iconv_open("ucs-2le", "utf-8");
	_ucs2le_to_ascii_iconv = _iconv_open("ucs-2le", "ascii");
	_ucs2le_to_gbk_iconv = _iconv_open("ucs-2le", "gbk");
	_ucs4le_to_utf8_iconv = _iconv_open("ucs-4le", "utf-8");
	_latin1_to_utf8_iconv = _iconv_open("latin1", "utf-8");
	_utf8_to_latin1_iconv = _iconv_open("utf-8", "latin1");
	_jis_x0208_1990_to_utf8_iconv = _iconv_open("jis_x0208-1990", "utf-8");
	_iso_8859_5_to_utf8_iconv = _iconv_open("iso_8859-5", "utf-8");
	_iso_8859_8_to_utf8_iconv = _iconv_open("iso_8859-8", "utf-8");
	_iso_2022_jp_to_utf8_iconv = _iconv_open("iso-2022-jp", "utf-8");
	_jis_x0212_1990_to_utf8_iconv = _iconv_open("jis_x0212-1990", "utf-8");
	_ksc_5601_to_utf8_iconv = _iconv_open("ksc_5601", "utf-8");
	_utf8_to_utf16be_iconv = _iconv_open("UTF-8", "UTF-16BE");
	_utf16be_to_utf8_iconv = _iconv_open("UTF-16BE", "UTF-8");
}

void
dave_iconv_exit(void)
{
	_iconv_close(_gbk_to_utf8_iconv);
	_iconv_close(_utf8_to_gbk_iconv);
	_iconv_close(_ascii_to_utf8_iconv);
	_iconv_close(_utf8_to_ucs2be_iconv);
	_iconv_close(_ucs2be_to_utf8_iconv);
	_iconv_close(_ucs2le_to_utf8_iconv);
	_iconv_close(_ucs2le_to_ascii_iconv);
	_iconv_close(_ucs2le_to_gbk_iconv);
	_iconv_close(_ucs4le_to_utf8_iconv);
	_iconv_close(_latin1_to_utf8_iconv);
	_iconv_close(_utf8_to_latin1_iconv);
	_iconv_close(_jis_x0208_1990_to_utf8_iconv);
	_iconv_close(_iso_8859_5_to_utf8_iconv);
	_iconv_close(_iso_8859_8_to_utf8_iconv);
	_iconv_close(_iso_2022_jp_to_utf8_iconv);
	_iconv_close(_jis_x0212_1990_to_utf8_iconv);
	_iconv_close(_ksc_5601_to_utf8_iconv);
	_iconv_close(_utf8_to_utf16be_iconv);
	_iconv_close(_utf16be_to_utf8_iconv);
}

ub
dave_iconv_gbk_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_gbk_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_utf8_to_gbk(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_utf8_to_gbk_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_ascii_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_ascii_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_utf8_to_ucs2be(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_utf8_to_ucs2be_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_ucs2be_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_ucs2be_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_ucs2le_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_ucs2le_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_ucs2le_to_ascii(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_ucs2le_to_ascii_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_ucs2le_to_gbk(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_ucs2le_to_gbk_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_ucs4le_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_ucs4le_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_latin1_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_latin1_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_utf8_to_latin1(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_utf8_to_latin1_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_jis_x0208_1990_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_jis_x0208_1990_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_iso_8859_5_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_iso_8859_5_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_iso_8859_8_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_iso_8859_8_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_iso_2022_jp_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_iso_2022_jp_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_jis_x0212_1990_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_jis_x0212_1990_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_ksc_5601_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_ksc_5601_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_utf8_to_utf16be(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_utf8_to_utf16be_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv_utf16be_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	return _iconv_all(_utf16be_to_utf8_iconv, in_buf, in_buf_len, out_buf, out_buf_len);
}

ub
dave_iconv(s8 *from_format, s8 *to_format, s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len)
{
	iconv_t iconv_ptr;
	ub out_len;

	iconv_ptr = iconv_open((const char *)to_format, (const char *)from_format);
	if(_iconv_ptr_check(iconv_ptr) == dave_false)
	{
		PARTYABNOR("invalid iconv:%s->%s!", from_format, to_format);
	}

	out_len = _iconv_all(iconv_ptr, in_buf, in_buf_len, out_buf, out_buf_len);

	if(_iconv_ptr_check(iconv_ptr) == dave_true)
	{
		iconv_close(iconv_ptr);
	}

	return out_len;
}

#endif

