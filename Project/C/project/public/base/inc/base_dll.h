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

typedef void (* dll_callback_fun)(void *msg);
typedef int (* dll_checkback_fun)(int);
typedef void (* dll_kv_timerout_fun)(void *kv, void *key);
typedef void (* dll_cfg_reg_fun)(void *name_ptr, int name_len, void *value_ptr, int value_len);
typedef int (* dll_dos_cmd_fun)(char *param_ptr, int param_len);

API void dave_dll_init(
	char *my_verno, char *work_mode,
	int thread_number,
	dll_callback_fun init_fun, dll_callback_fun main_fun, dll_callback_fun exit_fun,
	char *sync_domain);

API void dave_dll_running(void);

API void dave_dll_exit(void);

API int dave_dll_run_state(void);

API int dave_dll_self_check(char *string_data, int int_data, float float_data, dll_checkback_fun checkback);

API void dave_dll_log(char *func, int line, char *log_msg);

API char * dave_dll_verno(void);

API char * dave_dll_reset_verno(char *verno);

API void * dave_dll_mmalloc(int length, char *fun, int line);

API int dave_dll_mfree(void *m, char *fun, int line);

API void * dave_dll_mclone(void *m, char *fun, int line);

API char * dave_dll_self(void);

API void * dave_dll_thread_msg(int msg_len, char *fun, int line);

API void dave_dll_thread_msg_release(void *ptr, char *fun, int line);

API int dave_dll_thread_id_msg(unsigned long long dst_id, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API int dave_dll_thread_id_qmsg(unsigned long long dst_id, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API void * dave_dll_thread_id_co(unsigned long long dst_id, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line);

API void * dave_dll_thread_id_qco(unsigned long long dst_id, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line);

API int dave_dll_thread_name_msg(char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API int dave_dll_thread_name_qmsg(char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API void * dave_dll_thread_name_co(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line);

API void * dave_dll_thread_name_qco(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line);

API int dave_dll_thread_gid_msg(char *gid, char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API int dave_dll_thread_gid_qmsg(char *gid, char *dst_thread, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API void * dave_dll_thread_gid_co(char *gid, char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line);

API void * dave_dll_thread_gid_qco(char *gid, char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line);

API int dave_dll_thread_uid_msg(char *uid, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API int dave_dll_thread_uid_qmsg(char *uid, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API void * dave_dll_thread_uid_co(char *uid, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line);

API void * dave_dll_thread_uid_qco(char *uid, int req_id, int req_len, void *req_body, int rsp_id, char *fun, int line);

API void * dave_dll_thread_sync_msg(char *dst_thread, int req_id, int req_len, void *req_body, int rsp_id, int rsp_len, void *rsp_body, char *fun, int line);

API int dave_dll_thread_broadcast_msg(char *thread_name, int msg_id, int msg_len, void *msg_body, char *fun, int line);

API int dave_dll_cfg_set(char *cfg_name, char *cfg_value);

API int dave_dll_cfg_get(char *cfg_name, char *cfg_value_ptr, int cfg_value_len);

API int dave_dll_cfg_del(char *cfg_name);

API int dave_dll_cfg_reg(char *cfg_name, dll_cfg_reg_fun reg_fun);

API int dave_dll_cfg_remote_set(char *cfg_name, char *cfg_value, int ttl);

API int dave_dll_cfg_remote_get(char *cfg_name, char *cfg_value_ptr, int cfg_value_len);

API int dave_dll_cfg_remote_del(char *cfg_name);

API void dave_dll_poweroff(void);

API void * dave_dll_kv_malloc(char *name, int out_second, dll_kv_timerout_fun outback_fun);

API void dave_dll_kv_free(void *kv);

API int dave_dll_kv_add(void *kv, char *key, char *value);

API int dave_dll_kv_inq(void *kv, char *key, char *value_ptr, int value_len);

API int dave_dll_kv_del(void *kv, char *key);

API int dave_dll_dos_cmd_reg(const char *cmd, dll_dos_cmd_fun cmd_fun);

API void dave_dll_dos_print(char *msg);

API char * dave_dll_dos_get_user_input(char *give_user_msg, int wait_second);

#ifdef __cplusplus
}
#endif
#endif

