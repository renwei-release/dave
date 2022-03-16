/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.03.10.
 * ================================================================================
 */

#ifndef __T_TIME_H__
#define __T_TIME_H__

DateStruct t_time_get_date(DateStruct *pDate);
ErrCode t_time_set_date(DateStruct *pDate);


// $$$$$$$$$$$$$$$$$$interface compatible$$$$$$$$$$$$$$$$$$$$
#define dave_timer_get_date t_time_get_date
#define dave_timer_set_date t_time_set_date


#endif

