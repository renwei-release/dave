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
#define YEAR_START (1900)

// =====================================================================

DateStruct
t_time_get_date(DateStruct *pDate)
{
	DateStruct local_date;

	dave_os_get_time(&(local_date.year), &(local_date.month), &(local_date.day), &(local_date.hour), &(local_date.minute), &(local_date.second));

	local_date.week = 0;

	if(pDate != NULL)
		*pDate = local_date;

	return local_date;
}

RetCode
t_time_set_date(DateStruct *pDate)
{
	if(pDate == NULL)
		return RetCode_invalid_date;

	pDate->week = 0;

	return dave_os_set_time(pDate->year, pDate->month, pDate->day, pDate->hour, pDate->minute, pDate->second);
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

