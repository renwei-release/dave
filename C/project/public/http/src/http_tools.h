/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __HTTP_TOOLS_H__
#define __HTTP_TOOLS_H__

ub http_copy_uri(s8 *dst_uri, ub dst_uri_len, s8 *src_uri, ub src_uri_len);

void http_copy_head(HttpKeyValue *pDstHead, HttpKeyValue *pSrcHead);

s8 * http_load_content_type(HttpContentType type);

s8 * http_find_ramkv(HttpKeyValue *head_ptr, ub head_len, char *key);

dave_bool http_build_ramkv(HttpKeyValue *head_ptr, ub head_len, char *key, char *value);

#endif

