/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"

static s8 *
_t_a2b_date_str(s8 *date_str, ub date_len, DateStruct *pDate)
{
	dave_snprintf(date_str, date_len, "%04d.%02d.%02d %02d:%02d:%02d",
		pDate->year, pDate->month, pDate->day,
		pDate->hour, pDate->minute, pDate->second);

	return date_str;
}

// =====================================================================

s8 *
t_a2b_date_str(DateStruct *pDate)
{
	static s8 date_str[32];

	return _t_a2b_date_str(date_str, sizeof(date_str), pDate);
}

s8 *
t_a2b_date_str_2(DateStruct *pDate)
{
	static s8 date_str[32];

	return _t_a2b_date_str(date_str, sizeof(date_str), pDate);
}

s8 *
t_a2b_date_str_3(DateStruct *pDate)
{
	static s8 date_str[32];

	return _t_a2b_date_str(date_str, sizeof(date_str), pDate);
}

s8 *
t_a2b_date_str_4(DateStruct *pDate)
{
	static s8 date_str[32];

	return _t_a2b_date_str(date_str, sizeof(date_str), pDate);
}

s8 *
t_a2b_date_str_5(DateStruct *pDate)
{
	static s8 date_str[32];

	return _t_a2b_date_str(date_str, sizeof(date_str), pDate);
}

s8 *
t_a2b_date_str_6(DateStruct *pDate)
{
	static s8 date_str[32];

	return _t_a2b_date_str(date_str, sizeof(date_str), pDate);
}

