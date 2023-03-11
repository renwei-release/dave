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

RetCode dave_os_set_time(u16 year, u8 month, u8 day, u8 hour, u8 minute, u8 second, s8 zone);

RetCode dave_os_get_time(u16 *year, u8 *month, u8 *day, u8 *hour, u8 *minute, u8 *second, s8 *zone);

void dave_os_sleep(ub millisecond);

void dave_os_usleep(ub microseconds);

void dave_os_nsleep(ub nanosecond);

#endif

