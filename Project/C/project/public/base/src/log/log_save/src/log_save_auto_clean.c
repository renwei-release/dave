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

static void
_log_save_auto_clean_del_dir(s8 *dir, s8 *candidate_dir, MBUF *allowed_list)
{
	ub candidate_len = dave_strlen(candidate_dir);
	dave_bool remove_flag = dave_true;
	s8 dir_full[256];

	// the dir_path is a log dir:20XXXXXX
	if(candidate_len != 8)
		return;
	if(t_is_all_digit((u8 *)candidate_dir, candidate_len) == dave_false)
		return;
	if((candidate_dir[0] != '2') || (candidate_dir[1] != '0'))
		return;
	if(candidate_dir[4] > '1')
		return;
	if(candidate_dir[6] > '3')
		return;

	while(allowed_list != NULL)
	{
		if(dave_strcmp(candidate_dir, ms8(allowed_list)) == dave_true)
		{
			remove_flag = dave_false;
			break;
		}
		allowed_list = allowed_list->next;
	}

	if(remove_flag == dave_false)
		return;

	dave_snprintf(dir_full, sizeof(dir_full), "%s/%s", dir, candidate_dir);

	if(dave_os_file_remove_dir(dir_full) == dave_true)
	{
		LOGLOG("auto clean log dir %s", dir_full);
	}
	else
	{
		LOGLOG("auto clean log dir %s failed!", dir_full);
	}
}

static MBUF *
_log_save_auto_clean_allowed_list(void)
{
	MBUF *allowed_list = NULL, *allowed_date;
	DateStruct current_date;
	ub current_second;
	ub progress_days;

	if(_log_reserved_days == 0)
		return allowed_list;

	current_date = t_time_get_date(NULL);
	current_second = t_time_struct_second(&current_date);

	for(progress_days=0; progress_days<_log_reserved_days; progress_days++)
	{
		current_date = t_time_second_struct(current_second);

		allowed_date = dave_mmalloc(128);
		dave_snprintf(ms8(allowed_date), mlen(allowed_date), "%04d%02d%02d",
			current_date.year, current_date.month, current_date.day);
		LOGDEBUG("allowed_date:%s", ms8(allowed_date));
		allowed_list = dave_mchain(allowed_list, allowed_date);

		current_second -= 86400;
	}

	return allowed_list;
}

static void
_log_save_auto_clean_dir(s8 *dir)
{
	MBUF *subdir_list = dave_os_dir_subdir_list(dir);
	MBUF *allowed_list = _log_save_auto_clean_allowed_list();
	MBUF *temp_list;

	temp_list = subdir_list;
	while(temp_list != NULL)
	{
		LOGDEBUG("subdir:%s", ms8(temp_list));

		_log_save_auto_clean_del_dir(dir, ms8(temp_list), allowed_list);

		temp_list = temp_list->next;
	}

	dave_mfree(subdir_list);
	dave_mfree(allowed_list);
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

