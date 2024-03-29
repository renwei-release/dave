/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OS_API_H__
#define __DAVE_OS_API_H__

typedef RetCode (* sync_notify_fun)(ub notify_id);

void *dave_os_malloc(size_t len);

void dave_os_free(void *ptr);

ub dave_os_size(void *ptr);

void dave_os_restart(sw_int8 *reason);

void dave_os_power_off(sw_int8 *reason);

dave_bool dave_os_on_docker(void);

dave_bool dave_os_process_exist(s8 *process_name);

dave_bool dave_os_system(char *cmdstring, char *result_ptr, int result_len);

sb dave_os_get_system_file_max(void);

dave_bool dave_os_set_system_file_max(sb file_max);

s8 * dave_os_errno(sb *ret_errno);

#endif

