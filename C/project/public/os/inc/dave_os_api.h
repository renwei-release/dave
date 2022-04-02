/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OS_API_H__
#define __DAVE_OS_API_H__

typedef void *(* dave_os_thread_fun)(void *arg);

typedef ErrCode (* sync_notify_fun)(ub notify_id);

void dave_os_init(void);

void dave_os_exit(void);

void *dave_os_malloc(ub len);

void dave_os_free(void *ptr);

ub dave_os_size(void *ptr);

void dave_os_init_thread(void);

void dave_os_exit_thread(void);

void *dave_os_create_thread(char *name, dave_os_thread_fun fun, void *arg);

void dave_os_release_thread(void *thread_id);

void *dave_os_thread_self(u64 *dave_thread_id);

dave_bool dave_os_thread_sleep(void *thread_id);

dave_bool dave_os_thread_wakeup(void *thread_id);

dave_bool dave_os_thread_canceled(void *thread_id);

void dave_os_thread_exit(void *thread_id);

sb dave_os_pv_lock(void);

void dave_os_pv_unlock(sb flag);

void dave_os_mutex_init(void *ptr);

void dave_os_mutex_destroy(void *ptr);

void dave_os_mutex_lock(void *ptr);

void dave_os_mutex_unlock(void *ptr);

dave_bool dave_os_mutex_trylock(void *ptr);

void dave_os_spin_lock_init(void *ptr);

void dave_os_spin_lock_destroy(void *ptr);

void dave_os_spin_lock(void *ptr);

void dave_os_spin_unlock(void *ptr);

void dave_os_spin_trylock(void *ptr);

void dave_os_rw_lock_init(void *ptr);

void dave_os_rw_lock_destroy(void *ptr);

dave_bool dave_os_rw_rlock(void *ptr);

dave_bool dave_os_rw_wlock(void *ptr);

dave_bool dave_os_rw_tryrlock(void *ptr);

dave_bool dave_os_rw_trywlock(void *ptr);

dave_bool dave_os_rw_unlock(void *ptr);

void dave_os_sleep(ub millisecond);

void dave_os_usleep(ub microseconds);

void dave_os_nsleep(ub nanosecond);

void dave_os_restart(sw_int8 *reason);

void dave_os_power_off(sw_int8 *reason);

ub dave_os_cpu_process_number(void);

dave_bool dave_os_start_hardware_timer(sync_notify_fun fun, ub alarm_ms);

void dave_os_stop_hardware_timer(void);

ErrCode dave_os_set_time(sw_uint16 year,sw_uint8 month,sw_uint8 day,sw_uint8 hour,sw_uint8 minute,sw_uint8 second);

ErrCode dave_os_get_time(sw_uint16 *year,sw_uint8 *month,sw_uint8 *day,sw_uint8 *hour,sw_uint8 *minute,sw_uint8 *second);

ub dave_os_time_ns(void);

ub dave_os_time_us(void);

ub dave_os_time_ms(void);

ub dave_os_time_s(void);

dave_bool dave_os_on_docker(void);

void dave_os_utc_date(DateStruct *date);

void dave_os_trace(TraceLevel level, sw_uint16 buf_len, sw_uint8 *buf);

sb dave_os_file_open(FileOptFlag flag, s8 *file_name);

ub dave_os_file_read(FileOptFlag flag, s8 *file_name, ub file_index, ub data_len, u8 *data);

dave_bool dave_os_file_write(FileOptFlag flag, s8 *file_name, ub file_index, ub data_len, u8 *data);

sb dave_os_file_load(sb file_id, ub pos, ub data_len, u8 *data);

sb dave_os_file_save(sb file_id, ub pos, ub data_len, u8 *data);

sb dave_os_file_len(s8 *file_name, sb file_id);

dave_bool dave_os_file_close(sb file_id);

dave_bool dave_os_file_delete(FileOptFlag flag, s8 * file_name);

void * dave_os_dir_open(s8 *dir_path, ub *file_number);

dave_bool dave_os_dir_valid(s8 *dir_path);

s8 * dave_os_dir_read(void *pDir, s8 *name_file, ub name_length);

void dave_os_dir_close(void *pDir);

MBUF * dave_os_dir_subdir_list(s8 *dir_path);

s8 * dave_os_file_home_dir(void);

void dave_os_file_creat_dir(s8 *dir);

MBUF * dave_os_file_read_mbuf(s8 *file_path);

dave_bool dave_os_process_exist(s8 *process_name);

ErrCode dave_os_load_imsi(s8 *imsi);

ErrCode dave_os_load_mac(u8 *mac);

ErrCode dave_os_load_ip(u8 ip_v4[DAVE_IP_V4_ADDR_LEN], u8 ip_v6[DAVE_IP_V6_ADDR_LEN]);

ErrCode dave_os_load_host_name(s8 *hostname, ub hostname_len);

dave_bool dave_os_tty_init(sync_notify_fun notify_fun);

void dave_os_tty_exit(void);

void dave_os_tty_write(u8 *data, ub data_len);

ub dave_os_tty_read(u8 *data, ub data_len);

dave_bool dave_os_system(char *cmdstring, char *result_ptr, int result_len);

sb dave_os_get_system_file_max(void);

dave_bool dave_os_set_system_file_max(sb file_max);

s8 * dave_os_errno(sb *ret_errno);

#endif

