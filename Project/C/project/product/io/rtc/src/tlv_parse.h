/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TLV_PARSE_H__
#define __TLV_PARSE_H__
#include "rtc_param.h"

ub tlv_parse_find_end(RTCClient *pClient);

dave_bool tlv_parse_get_token(s8 *token_ptr, ub token_len, s8 *tlv_ptr, ub tlv_len);
dave_bool tlv_parse_get_id(s8 *id_ptr, ub id_len, s8 *tlv_ptr, ub tlv_len);
dave_bool tlv_parse_get_app_data(s8 **value_ptr, ub *value_len, s8 *tlv_ptr, ub tlv_len);
dave_bool tlv_parse_get_app_format(s8 **value_ptr, ub *value_len, s8 *tlv_ptr, ub tlv_len);

MBUF * tlv_parse_set_close(void);
MBUF * tlv_parse_set_app_data(s8 *token, s8 *data_ptr, ub data_len, s8 *format_ptr, ub format_len);

#endif

