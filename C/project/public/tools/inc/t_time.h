/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_TIME_H__
#define __T_TIME_H__

DateStruct t_time_get_date(DateStruct *pDate);
RetCode t_time_set_date(DateStruct *pDate);


// $$$$$$$$$$$$$$$$$$interface compatible$$$$$$$$$$$$$$$$$$$$
#define dave_timer_get_date t_time_get_date
#define dave_timer_set_date t_time_set_date


#endif

