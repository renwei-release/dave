/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_SAVE_TOOLS_H__
#define __LOG_SAVE_TOOLS_H__

#ifdef __cplusplus
extern "C"{
#endif

ub log_save_load_record(s8 *separator, s8 **record_ptr, ub *record_len, s8 *content_ptr, ub content_len);

ub log_save_load_key_value(s8 **key_start, s8 **key_end, s8 **value_start, s8 **value_end, s8 *content_ptr, ub content_len);

#ifdef __cplusplus
} //extern "C"
#endif

#endif

