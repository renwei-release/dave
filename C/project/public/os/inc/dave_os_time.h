/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OS_TIME_H__
#define __DAVE_OS_TIME_H__

ub dave_os_time_ns(void);

ub dave_os_time_us(void);

ub dave_os_time_ms(void);

ub dave_os_time_s(void);

void dave_os_utc_date(DateStruct *date);

void dave_os_timer_notify(unsigned long data);

dave_bool dave_os_start_hardware_timer(sync_notify_fun fun, ub alarm_ms);

void dave_os_stop_hardware_timer(void);

ErrCode dave_os_set_time(sw_uint16 year,sw_uint8 month,sw_uint8 day,sw_uint8 hour,sw_uint8 minute,sw_uint8 second);

ErrCode dave_os_get_time(sw_uint16 *year,sw_uint8 *month,sw_uint8 *day,sw_uint8 *hour,sw_uint8 *minute,sw_uint8 *second);

void dave_os_sleep(ub millisecond);

void dave_os_usleep(ub microseconds);

void dave_os_nsleep(ub nanosecond);

#endif

