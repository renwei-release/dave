/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_LIBICONV_H__
#define __DAVE_LIBICONV_H__

void dave_iconv_init(void);

void dave_iconv_exit(void);

ub dave_iconv_gbk_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_utf8_to_gbk(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_ascii_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_utf8_to_ucs2be(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_ucs2be_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_ucs2le_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_ucs2le_to_ascii(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_ucs2le_to_gbk(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_ucs4le_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_latin1_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_utf8_to_latin1(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_jis_x0208_1990_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_iso_8859_5_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_iso_8859_8_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_iso_2022_jp_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_jis_x0212_1990_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_ksc_5601_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_utf8_to_utf16be(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv_utf16be_to_utf8(s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

ub dave_iconv(s8 *from_format, s8 *to_format, s8 *in_buf, ub in_buf_len, s8 *out_buf, ub out_buf_len);

#endif

