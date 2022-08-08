/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_DLL_H__
#define __BASE_DLL_H__

#ifdef __cplusplus
extern "C"{
#endif

#define API __attribute__((visibility("default")))

typedef void (* dll_callback_fun)(void *);
typedef int (* dll_checkback_fun)(int);

API void dave_dll_init(
	char *my_verno, char *mode,
	int thread_number,
	dll_callback_fun init_fun, dll_callback_fun main_fun, dll_callback_fun exit_fun);

API void dave_dll_running(void);

API void dave_dll_exit(void);

API void dave_dll_wait_dll_exit(void);

API int dave_dll_self_check(char *string_data, int int_data, float float_data, dll_checkback_fun checkback);

API void dave_dll_log(char *func, int line, char *log_msg);

API char * dave_dll_verno(void);

API void * dave_dll_mmalloc(int length, char *func, int line);

API int dave_dll_mfree(void *m, char *func, int line);

API void * dave_dll_thread_msg(int msg_len, char *fun, int line);

API void dave_dll_thread_msg_release(void *ptr, char *fun, int line);

API int dave_dll_thread_id_msg(unsigned long long dst_id, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API int dave_dll_thread_name_msg(char *thread_name, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API int dave_dll_thread_gid_msg(char *gid, char *thread_name, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API void * dave_dll_thread_sync_msg(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, int rsp_len, void *rsp_body, char *fun, int line);

API int dave_dll_thread_broadcast_msg(char *thread_name, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API int dave_dll_cfg_set(char *cfg_name, char *cfg_ptr);

API int dave_dll_cfg_get(char *cfg_name, char *cfg_ptr, int cfg_len);

API int dave_dll_cfg_remote_set(char *cfg_name, char *cfg_ptr);

API int dave_dll_cfg_remote_get(char *cfg_name, char *cfg_ptr, int cfg_len);

API void dave_dll_poweroff(void);

#ifdef __cplusplus
}
#endif
#endif

