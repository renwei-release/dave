/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <sys/time.h>
#include <time.h>
#include "dave_base.h"
#include "dave_os.h"
#include "tools_log.h"

#define MAX_SECOND (157680000000)	// 60 * 60 * 24 * 365 * 5000
#define MAX_YEAR (5000)
#define YEAR_START (1900)

// =====================================================================

DateStruct
t_time_get_date(DateStruct *pDate)
{
	DateStruct local_date;

	dave_os_get_time(
		&(local_date.year), &(local_date.month), &(local_date.day),
		&(local_date.hour), &(local_date.minute), &(local_date.second),
		&(local_date.zone));

	if(pDate != NULL)
	{
		*pDate = local_date;
	}

	return local_date;
}

RetCode
t_time_set_date(DateStruct *pDate)
{
	if(pDate == NULL)
	{
		return RetCode_invalid_date;
	}

	return dave_os_set_time(
		pDate->year, pDate->month, pDate->day,
		pDate->hour, pDate->minute, pDate->second,
		pDate->zone);
}

DateStruct
t_time_second_struct(ub second_time)
{
	time_t tm_time;
	struct tm ptm = { 0 };
	struct tm *pTm;
	DateStruct date;

	if(second_time > MAX_SECOND)
	{
		TOOLSABNOR("too big second_time:%ld", second_time);
		second_time = 0;
	}

	tm_time = (time_t)second_time;

	pTm = gmtime_r(&tm_time, &ptm);

	date.year = YEAR_START + pTm->tm_year;
	date.month = pTm->tm_mon + 1;
	date.day = pTm->tm_mday;
	date.hour = pTm->tm_hour;
	date.minute = pTm->tm_min;
	date.second = pTm->tm_sec;

	return date;
}

ub
t_time_struct_second(DateStruct *pDate)
{
	struct tm ptm = { 0 };

	if(pDate->year > MAX_YEAR)
	{
		TOOLSLOG("invalid year:%d", pDate->year);
		pDate->year = MAX_YEAR;
	}
	else if(pDate->year < YEAR_START)
	{
		TOOLSLOG("invalid year:%d", pDate->year);
		t_time_get_date(pDate);
	}

	if((pDate->month == 0) || (pDate->month > 12))
	{
		TOOLSLOG("invalid month:%d", pDate->month);
		pDate->month = 2;
	}

	if((pDate->day == 0) || (pDate->day >= 32))
	{
		TOOLSLOG("invalid day:%d", pDate->day);
		pDate->day = 1;
	}

	ptm.tm_year = pDate->year - YEAR_START;
	ptm.tm_mon = pDate->month - 1;
	ptm.tm_mday = pDate->day;
	ptm.tm_hour = pDate->hour;
	ptm.tm_min = pDate->minute;
	ptm.tm_sec = pDate->second;

	return (ub)mktime(&ptm);
}

