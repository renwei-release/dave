/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __LOG_SAVE_MSG_TO_JSON_H__
#define __LOG_SAVE_MSG_TO_JSON_H__

#ifdef __cplusplus
extern "C"{
#endif

void log_save_msg_to_json_init(void);

void log_save_msg_to_json_exit(void);

void * log_save_msg_to_json(ub msg_id, ub msg_len, void *msg_body);

#ifdef __cplusplus
} //extern "C"
#endif

#endif

