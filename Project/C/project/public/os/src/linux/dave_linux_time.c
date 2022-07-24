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
#include "dave_linux.h"
#include "os_log.h"

typedef struct {
	dave_bool start;
	timer_t id;
} HWTIMER;

static HWTIMER _hw_timer;

static sync_notify_fun _linux_timer_notify = NULL;

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

void
dave_os_timer_notify(unsigned long data)
{
	if(_linux_timer_notify != NULL)
	{
		_linux_timer_notify((ub)data);
	}
}

dave_bool
dave_os_start_hardware_timer(sync_notify_fun fun, ub alarm_ms)
{
	struct sigevent sev;
	struct itimerspec its;

	dave_os_stop_hardware_timer();

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = TIMER_SIG;
	sev.sigev_value.sival_ptr = (void *)fun;

	_linux_timer_notify = fun;

	its.it_value.tv_sec = alarm_ms / 1000;
	its.it_value.tv_nsec = (alarm_ms % 1000) * 1000;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if (timer_create(CLOCK_REALTIME, &sev, &(_hw_timer.id)) == -1)
	{
		OSABNOR("timer create failed:%d<%s>!", errno, strerror(errno));
		return dave_false;
	}

	_hw_timer.start = dave_true;

	if (timer_settime(_hw_timer.id, 0, &its, NULL) == -1)
	{
		OSABNOR("timer set failed:%d<%s>!", errno, strerror(errno));
		return dave_false;
	}

	return dave_true;
}

void
dave_os_stop_hardware_timer(void)
{
	if(_hw_timer.start == dave_true)
	{
		timer_delete(_hw_timer.id);
		_hw_timer.start = dave_false;
	}
	_linux_timer_notify = NULL;
}

RetCode
dave_os_set_time(sw_uint16 year,sw_uint8 month,sw_uint8 day,sw_uint8 hour,sw_uint8 minute,sw_uint8 second)
{
	struct timeval tv;
	struct timezone tz;
	struct tm tnow;
	
	gettimeofday(&tv, &tz);
	
	tnow.tm_year = year - 1900;
	tnow.tm_mon = month - 1;
	tnow.tm_mday = day;
	tnow.tm_hour = hour;
	tnow.tm_min = minute;
	tnow.tm_sec = second;
	tv.tv_sec = mktime(&tnow);

	if(settimeofday((const struct timeval *)&tv, NULL) == 0)
		return RetCode_OK;
	else
		return RetCode_Invalid_parameter;
}

RetCode
dave_os_get_time(u16 *year, u8 *month, u8 *day, u8 *hour, u8 *minute, u8 *second)
{
	time_t now;
	struct tm *tnow;
	struct tm ptm = { 0 };

	now = time(NULL);

	tnow = localtime_r(&now, &ptm);

	*year = 1900+tnow->tm_year;
	*month = tnow->tm_mon + 1;
	*day = tnow->tm_mday;
	*hour = tnow->tm_hour;
	*minute = tnow->tm_min;
	*second = tnow->tm_sec;

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

