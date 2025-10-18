/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#if defined(__DAVE_CYGWIN__) || defined(__DAVE_LINUX__)
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <sys/resource.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "os_log.h"

typedef struct {
	volatile dave_bool start;
	pthread_t thread;
	ub interval_ms;
} HWTIMER;

static HWTIMER _hw_timer;
static sync_notify_fun _linux_timer_notify = NULL;

static inline void
_timespec_add_ms(struct timespec *ts, ub ms)
{
	ts->tv_sec  += (time_t)(ms / 1000);
	long add_ns  = (long)((ms % 1000) * 1000000L);
	ts->tv_nsec += add_ns;
	if (ts->tv_nsec >= 1000000000L) {
		ts->tv_sec += 1;
		ts->tv_nsec -= 1000000000L;
	}
}

static void *
_timer_thread_fn(void *arg)
{
	(void)arg;

	if (_hw_timer.interval_ms == 0) {
		_hw_timer.interval_ms = 1;
	}

	struct timespec next;
	clock_gettime(CLOCK_MONOTONIC, &next);
	_timespec_add_ms(&next, _hw_timer.interval_ms);

	while (_hw_timer.start == dave_true) {
		int rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
		if (rc == EINTR) {
			continue;
		}

		sync_notify_fun cb = _linux_timer_notify;
		if (cb != NULL) {
			cb(0);
		}

		_timespec_add_ms(&next, _hw_timer.interval_ms);
	}

	return NULL;
}

static inline void
_time_set_tz(int tz)
{	
	char tzstr[256] = {0};		
	int tzhour = -tz;

	snprintf(tzstr, sizeof(tzstr), "GMT%+02d", tzhour);

	if(setenv("TZ", tzstr, 1)!=0)
	{
		printf("setenv TZ:%s failed\n", tzstr);
	}

	tzset();
}

static inline int
_time_get_tz(time_t *time_utc, struct tm *tm_local)
{
	struct tm tm_gmt;

	// Change it to GMT tm
	gmtime_r(time_utc, &tm_gmt);
 
	int time_zone = tm_local->tm_hour - tm_gmt.tm_hour;
	if (time_zone < -12) {
		time_zone += 24; 
	} else if (time_zone > 12) {
		time_zone -= 24;
	}

	return time_zone;
}

// =====================================================================

ub 
dave_os_time_ns(void)
{
	struct timespec time={0, 0};
		
	clock_gettime(CLOCK_REALTIME, &time);

	return (1000000000 * time.tv_sec) + time.tv_nsec;
}

ub
dave_os_time_us(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (ub)((tv.tv_sec * 1000000) + tv.tv_usec);
}

ub
dave_os_time_ms(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (ub)((tv.tv_sec * 1000) + tv.tv_usec / 1000);
}

ub
dave_os_time_s(void)
{
	return (ub)time(NULL);
}

void
dave_os_utc_date(DateStruct *date)
{
	time_t timep;
	struct tm *p;

	time(&timep);

	p=gmtime(&timep);

	date->year = 1900+p->tm_year;
	date->month = 1+p->tm_mon;
	date->day = p->tm_mday;

	date->hour = p->tm_hour;
	date->minute = p->tm_min;
	date->second = (p->tm_sec < 60 ? p->tm_sec : 59);
}

dave_bool
dave_os_start_hardware_timer(sync_notify_fun fun, ub alarm_ms)
{
	dave_os_stop_hardware_timer();

	_linux_timer_notify = fun;
	_hw_timer.interval_ms = alarm_ms;
	_hw_timer.start = dave_true;

	pthread_attr_t attr;
	if(pthread_attr_init(&attr) != 0)
	{
		_hw_timer.start = dave_false;
		return dave_false;
	}

	int rc = pthread_create(&(_hw_timer.thread), &attr, _timer_thread_fn, NULL);
	pthread_attr_destroy(&attr);
	if(rc != 0) {
		_hw_timer.start = dave_false;
		OSABNOR("timer thread create failed:%d<%s>!", rc, strerror(rc));
		return dave_false;
	}

	return dave_true;
}

void
dave_os_stop_hardware_timer(void)
{
	if(_hw_timer.start == dave_true)
	{
		_hw_timer.start = dave_false;
		if (_hw_timer.thread != (pthread_t)0) {
			pthread_join(_hw_timer.thread, NULL);
			_hw_timer.thread = (pthread_t)0;
		}
	}
	_linux_timer_notify = NULL;
}

RetCode
dave_os_set_time(u16 year, u8 month, u8 day, u8 hour, u8 minute, u8 second, s8 zone)
{
	struct timeval tv;
	struct tm tnow;

	tnow.tm_year = year - 1900;
	tnow.tm_mon = month - 1;
	tnow.tm_mday = day;
	tnow.tm_hour = hour;
	tnow.tm_min = minute;
	tnow.tm_sec = second;
	_time_set_tz((int)zone);

	tv.tv_sec = mktime(&tnow);
	tv.tv_usec = 0;

	if(settimeofday((const struct timeval *)&tv, NULL) == 0)
		return RetCode_OK;
	else
		return RetCode_Invalid_parameter;
}

RetCode
dave_os_get_time(u16 *year, u8 *month, u8 *day, u8 *hour, u8 *minute, u8 *second, s8 *zone)
{
	time_t time_utc = time(NULL);
	struct tm tm_local = { 0 };
	struct tm *tnow;

	tnow = localtime_r(&time_utc, &tm_local);

	*year = 1900+tnow->tm_year;
	*month = tnow->tm_mon + 1;
	*day = tnow->tm_mday;
	*hour = tnow->tm_hour;
	*minute = tnow->tm_min;
	*second = tnow->tm_sec;
	*zone = (s8)_time_get_tz(&time_utc, &tm_local);

	return RetCode_OK;
}

void
dave_os_sleep(ub millisecond)
{
	struct timespec req;

	if(millisecond == 0)
	{
		req.tv_sec = 0;
		req.tv_nsec = (500000L);	// 0.5ms
	}
	else
	{
		req.tv_sec = millisecond / 1000;
		req.tv_nsec = ((unsigned long)(millisecond % 1000)) * 1000000L;
	}

	while ((nanosleep (&req, &req) == -1) && (errno == EINTR));
}

void
dave_os_usleep(ub microseconds)
{
	struct timespec req;

	if(microseconds == 0)
	{
		req.tv_sec = 0;
		req.tv_nsec = (100000L);	// 0.1ms
	}
	else
	{
		req.tv_sec = microseconds / 1000000L;
		req.tv_nsec = ((unsigned long)(microseconds % 1000000L)) * 1000;
	}

	while ((nanosleep (&req, &req) == -1) && (errno == EINTR));
}

void
dave_os_nsleep(ub nanosecond)
{
	struct timespec req;

	if(nanosecond == 0)
	{
		req.tv_sec = 0;
		req.tv_nsec = (100000L);	// 0.1ms
	}
	else
	{
		req.tv_sec = 0;
		req.tv_nsec = ((unsigned long)(nanosecond));
	}

	while ((nanosleep (&req, &req) == -1) && (errno == EINTR));
}

#endif

