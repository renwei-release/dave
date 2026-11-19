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
	dave_bool start;
	dave_bool thread_created;
	pthread_t thread;
	ub interval_ms;
	pthread_mutex_t mutex;
	pthread_cond_t condition;
} HWTIMER;

static HWTIMER _hw_timer = { .mutex = PTHREAD_MUTEX_INITIALIZER };
// Serialize lifecycle operations without holding the worker's mutex during join.
static pthread_mutex_t _timer_control_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t _timer_condition_once = PTHREAD_ONCE_INIT;
static int _timer_condition_error;
static sync_notify_fun _linux_timer_notify = NULL;

static void
_timer_condition_init(void)
{
	pthread_condattr_t attr;
	int rc = pthread_condattr_init(&attr);

	if(rc == 0)
	{
		rc = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
		if(rc == 0)
			rc = pthread_cond_init(&_hw_timer.condition, &attr);
		int destroy_rc = pthread_condattr_destroy(&attr);
		if(destroy_rc != 0)
			OSABNOR("timer condition attribute destroy failed:%d<%s>!", destroy_rc, strerror(destroy_rc));
	}
	_timer_condition_error = rc;
}

static dave_bool
_timer_in_callback(dave_bool request_stop)
{
	dave_bool in_callback;

	pthread_mutex_lock(&_hw_timer.mutex);
	in_callback = (_hw_timer.thread_created == dave_true)
		&& pthread_equal(pthread_self(), _hw_timer.thread);
	if((in_callback == dave_true) && (request_stop == dave_true))
	{
		_hw_timer.start = dave_false;
		_linux_timer_notify = NULL;
	}
	pthread_mutex_unlock(&_hw_timer.mutex);

	return in_callback;
}

static dave_bool
_timer_stop(void)
{
	int rc;
	pthread_t thread;

	pthread_mutex_lock(&_hw_timer.mutex);
	_hw_timer.start = dave_false;
	if(_hw_timer.thread_created == dave_false)
	{
		_linux_timer_notify = NULL;
		pthread_mutex_unlock(&_hw_timer.mutex);
		return dave_true;
	}
	thread = _hw_timer.thread;
	rc = pthread_cond_signal(&_hw_timer.condition);
	pthread_mutex_unlock(&_hw_timer.mutex);
	if(rc != 0)
	{
		OSABNOR("timer condition signal failed:%d<%s>!", rc, strerror(rc));
		return dave_false;
	}

	rc = pthread_join(thread, NULL);
	if(rc != 0)
	{
		OSABNOR("timer thread join failed:%d<%s>!", rc, strerror(rc));
		return dave_false;
	}
	pthread_mutex_lock(&_hw_timer.mutex);
	_hw_timer.thread_created = dave_false;
	_linux_timer_notify = NULL;
	pthread_mutex_unlock(&_hw_timer.mutex);

	return dave_true;
}

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
	struct timespec next;
	int rc;

	(void)arg;

	pthread_mutex_lock(&_hw_timer.mutex);
	if(clock_gettime(CLOCK_MONOTONIC, &next) != 0)
	{
		OSABNOR("timer clock read failed:%d<%s>!", errno, strerror(errno));
		_hw_timer.start = dave_false;
		pthread_mutex_unlock(&_hw_timer.mutex);
		return NULL;
	}
	_timespec_add_ms(&next, _hw_timer.interval_ms);

	while(_hw_timer.start == dave_true)
	{
		rc = pthread_cond_timedwait(&_hw_timer.condition, &_hw_timer.mutex, &next);
		if(_hw_timer.start == dave_false)
			break;
		if(rc == 0)
			continue;
		if(rc != ETIMEDOUT)
		{
			OSABNOR("timer condition wait failed:%d<%s>!", rc, strerror(rc));
			_hw_timer.start = dave_false;
			break;
		}

		sync_notify_fun cb = _linux_timer_notify;
		pthread_mutex_unlock(&_hw_timer.mutex);
		if(cb != NULL)
			cb(0);
		pthread_mutex_lock(&_hw_timer.mutex);

		_timespec_add_ms(&next, _hw_timer.interval_ms);
	}
	pthread_mutex_unlock(&_hw_timer.mutex);

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
	int rc;

	if(_timer_in_callback(dave_false) == dave_true)
	{
		OSABNOR("can't restart hardware timer from its callback!");
		return dave_false;
	}
	pthread_mutex_lock(&_timer_control_mutex);
	if(_timer_stop() == dave_false)
	{
		pthread_mutex_unlock(&_timer_control_mutex);
		return dave_false;
	}

	rc = pthread_once(&_timer_condition_once, _timer_condition_init);
	if(rc == 0)
		rc = _timer_condition_error;
	if(rc != 0)
	{
		OSABNOR("timer condition init failed:%d<%s>!", rc, strerror(rc));
		pthread_mutex_unlock(&_timer_control_mutex);
		return dave_false;
	}

	pthread_mutex_lock(&_hw_timer.mutex);
	_linux_timer_notify = fun;
	_hw_timer.interval_ms = (alarm_ms == 0) ? 1 : alarm_ms;
	_hw_timer.start = dave_true;
	rc = pthread_create(&_hw_timer.thread, NULL, _timer_thread_fn, NULL);
	if(rc == 0)
	{
		_hw_timer.thread_created = dave_true;
	}
	else
	{
		_hw_timer.start = dave_false;
		_linux_timer_notify = NULL;
		OSABNOR("timer thread create failed:%d<%s>!", rc, strerror(rc));
	}
	pthread_mutex_unlock(&_hw_timer.mutex);
	pthread_mutex_unlock(&_timer_control_mutex);

	return (rc == 0) ? dave_true : dave_false;
}

void
dave_os_stop_hardware_timer(void)
{
	// A callback can request stop, but another thread must reap it.
	if(_timer_in_callback(dave_true) == dave_true)
		return;

	pthread_mutex_lock(&_timer_control_mutex);
	_timer_stop();
	pthread_mutex_unlock(&_timer_control_mutex);
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

