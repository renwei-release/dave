/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_MAIN_H__
#define __BASE_MAIN_H__

void base_init(void *main_thread_id, s8 *sync_domain);
dave_bool base_running(dave_bool platform_schedule);
void base_exit(void);
void base_restart(const char *args, ...);
dave_bool base_power_state(void);
void base_power_off(s8 *reason);

#endif

