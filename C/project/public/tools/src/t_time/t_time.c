/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_os.h"
#include "tools_log.h"

// =====================================================================

DateStruct
t_time_get_date(DateStruct *pDate)
{
	DateStruct local_date;

	dave_os_get_time(&(local_date.year), &(local_date.month), &(local_date.day), &(local_date.hour), &(local_date.minute), &(local_date.second));

	if(pDate != NULL)
		*pDate = local_date;

	return local_date;
}

ErrCode
t_time_set_date(DateStruct *pDate)
{
	if(pDate == NULL)
		return ERRCODE_invalid_date;

	return dave_os_set_time(pDate->year, pDate->month, pDate->day, pDate->hour, pDate->minute, pDate->second);
}
