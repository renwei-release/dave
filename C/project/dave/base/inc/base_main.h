/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.03.13.
 * ================================================================================
 */

#ifndef __BASE_MAIN_H__
#define __BASE_MAIN_H__

void base_init(void *main_thread_id);
dave_bool base_running(dave_bool platform_schedule);
void base_exit(void);
void base_restart(const char *args, ...);
dave_bool base_power_state(void);
void base_power_off(s8 *reason);


// $$$$$$$$$$$$$$$$$$interface compatible$$$$$$$$$$$$$$$$$$$$
#define dave_restart base_restart
#define dave_power_state base_power_state


#endif

