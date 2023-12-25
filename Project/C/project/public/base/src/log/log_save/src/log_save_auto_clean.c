/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#if defined(LOG_STACK_SERVER) || defined(LOG_STACK_CLIENT)
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_verno.h"
#include "log_log.h"

#define LOG_AUTO_CLEAN_TIMER "logautoclean"

static ub _log_reserved_days = 0;

static dave_bool
_log_save_auto_check_candidate_dir(s8 *candidate_dir)
{
	ub candidate_len = dave_strlen(candidate_dir);
	DateStruct current_date;
	ub current_data;

	// the dir_path is a log dir:20231125
	if(candidate_len != 8)
		return dave_false;
	if(t_is_all_digit((u8 *)candidate_dir, candidate_len) == dave_false)
		return dave_false;

	current_date = t_time_get_date(NULL);
	current_data = current_date.year*10000 + current_date.month*100 + current_date.day;

	/*
	 * If the time of the candidate directory is greater than the current system time,
	 * this candidate directory is retained.
	 */
	if(stringdigital(candidate_dir) > current_data)
		return dave_false;

	return dave_true;
}

static void
_log_save_auto_clean_del_dir(s8 *dir, s8 *candidate_dir, MBUF *reserve_list)
{
	dave_bool the_dir_must_be_reserve = dave_false;
	s8 current_data_str[128], dir_full[256];
	DateStruct current_date;

	if(_log_save_auto_check_candidate_dir(candidate_dir) == dave_false)
		return;

	while(reserve_list != NULL)
	{
		LOGDEBUG("the subdir:%s candidate_dir:%s the reserve dir:%s", dir, candidate_dir, ms8(reserve_list));

		if(dave_strcmp(candidate_dir, ms8(reserve_list)) == dave_true)
		{
			the_dir_must_be_reserve = dave_true;
			break;
		}
		reserve_list = reserve_list->next;
	}

	if(the_dir_must_be_reserve == dave_true)
		return;

	current_date = t_time_get_date(NULL);
	dave_snprintf(current_data_str, sizeof(current_data_str), "%04d%02d%02d",
		current_date.year, current_date.month, current_date.day);
	if(dave_strcmp(current_data_str, candidate_dir) == dave_true)
	{
		LOGLOG("Protection measures, do not delete the log of the day:%s!", candidate_dir);
		return;
	}

	dave_snprintf(dir_full, sizeof(dir_full), "%s/%s", dir, candidate_dir);

	if(dave_os_file_remove_dir(dir_full) == dave_true)
	{
		LOGLOG("auto clean log dir:%s date:%s", dir_full, datestr(&current_date));
	}
	else
	{
		LOGLOG("auto clean log dir:%s date:%s failed!", dir_full, datestr(&current_date));
	}
}

static MBUF *
_log_save_auto_add_reserve_list(MBUF *reserve_list, DateStruct current_date)
{
	MBUF *reserve_dir_name;

	reserve_dir_name = dave_mmalloc(128);
	dave_snprintf(ms8(reserve_dir_name), mlen(reserve_dir_name), "%04d%02d%02d",
		current_date.year, current_date.month, current_date.day);
	reserve_list = dave_mchain(reserve_list, reserve_dir_name);

	return reserve_list;
}


/*
 * Calculate the name of the log folder that needs to be retained
 */
static MBUF *
_log_save_auto_clean_reserve_list(void)
{
	MBUF *reserve_list = NULL;
	DateStruct current_date;
	ub current_second;
	ub progress_days;

	if(_log_reserved_days == 0)
		return reserve_list;

	current_date = t_time_get_date(NULL);	

	/*
	 * The t_time_struct_second function is calculated based on the 0 time zone and does not match the current system time. 
	 * In order to prevent accidental deletion of logs at the current time, 
	 * first add the current log folder to the reserve_list.
	 */
	reserve_list = _log_save_auto_add_reserve_list(reserve_list, current_date);
	current_second = t_time_struct_second(&current_date);

	for(progress_days=0; progress_days<_log_reserved_days; progress_days++)
	{
		current_date = t_time_second_struct(current_second);

		reserve_list = _log_save_auto_add_reserve_list(reserve_list, current_date);

		// advance one day
		current_second -= 86400;
	}

	return reserve_list;
}

static void
_log_save_auto_clean_dir(s8 *dir)
{
	MBUF *subdir_list = dave_os_dir_subdir_list(dir);
	MBUF *reserve_list = _log_save_auto_clean_reserve_list();
	MBUF *temp_list;

	temp_list = subdir_list;
	while(temp_list != NULL)
	{
		LOGDEBUG("the subdir:%s", ms8(temp_list));

		_log_save_auto_clean_del_dir(dir, ms8(temp_list), reserve_list);

		temp_list = temp_list->next;
	}

	dave_mfree(subdir_list);
	dave_mfree(reserve_list);
}

static void
_log_save_auto_clean_product(void)
{
	// the dir like this /dave/log/BASE/20230525
	//      or like this /dave/base/BASE/20230525
	s8 *home_dir = dave_os_file_home_dir();
	s8 full_dir[1024];
	MBUF *subdir_list, *temp_list;

	subdir_list = dave_os_dir_subdir_list(home_dir);

	LOGDEBUG("clean dir:%s", home_dir);

	temp_list = subdir_list;
	while(temp_list != NULL)
	{
		dave_snprintf(full_dir, sizeof(full_dir), "%s/%s", home_dir, ms8(temp_list));

		LOGDEBUG("subdir:%s", full_dir);

		_log_save_auto_clean_dir(full_dir);

		temp_list = temp_list->next;
	}

	dave_mfree(subdir_list);
}

static void
_log_save_auto_clean_timer(TIMERID timer_id, ub thread_index)
{
	_log_save_auto_clean_product();
}

// =====================================================================

void
log_save_auto_clean_init(ub log_reserved_days)
{
	if(log_reserved_days != 0)
	{
		_log_reserved_days = log_reserved_days;

		base_timer_creat(LOG_AUTO_CLEAN_TIMER, _log_save_auto_clean_timer, 3600 * 1000);

		_log_save_auto_clean_product();
	}
}

void
log_save_auto_clean_exit(void)
{
	base_timer_kill(LOG_AUTO_CLEAN_TIMER);
}

#endif

