/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OS_THREAD_H__
#define __DAVE_OS_THREAD_H__

typedef void *(* dave_os_thread_fun)(void *arg);

void dave_os_init_thread(void);

void dave_os_exit_thread(void);

void *dave_os_create_thread(char *name, dave_os_thread_fun fun, void *arg);

void dave_os_release_thread(void *thread_id);

void *dave_os_thread_self(u64 *dave_thread_id);

ub dave_os_thread_id(void);

dave_bool dave_os_thread_sleep(void *thread_id);

dave_bool dave_os_thread_wakeup(void *thread_id);

dave_bool dave_os_thread_canceled(void *thread_id);

void dave_os_thread_exit(void *thread_id);

void dave_os_pv_lock(void);

void dave_os_pv_unlock(void);

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

#endif

