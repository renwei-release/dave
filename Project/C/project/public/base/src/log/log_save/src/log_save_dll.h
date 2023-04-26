/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_SAVE_DLL_H__
#define __LOG_SAVE_DLL_H__

#ifdef __cplusplus
extern "C"{
#endif

void log_save_dll_init(void);

void log_save_dll_exit(void);

void * log_save_msg_to_json(ub msg_id, ub msg_len, void *msg_body);

s8 * log_save_RPCMSG_str(RPCMSG enum_value);

#ifdef __cplusplus
} //extern "C"
#endif

#endif

