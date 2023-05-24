/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#if defined(LOG_STACK_SERVER) || defined(LOG_STACK_CLIENT)
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_os.h"
#include "log_lock.h"
#include "log_save_json.h"
#include "log_save_txt.h"
#include "log_save_cfg.h"
#include "log_save_chain.h"
#include "log_save_auto_clean.h"
#include "log_log.h"

#define LOG_FILE_CLOSE_TIME 86400 * 2

typedef struct {
	s8 file_name[128];
	sb file_id;
	/*
	 * 设置资源回收时间是LOG_FILE_CLOSE_TIME（两天），
	 * 这个时候已经不会有日志写到这里。
	 * 可以安全释放锁。
	 */
	TLock pv;
} LogFile;

static void *_log_file_kv = NULL;

static inline LogFile *
_log_save_log_file_malloc(s8 *file_name)
{
	LogFile *pLog;

	log_lock();

	pLog = kv_inq_key_ptr(_log_file_kv, file_name);
	if(pLog == NULL)
	{
		pLog = dave_malloc(sizeof(LogFile));

		t_lock_reset(&(pLog->pv));

		dave_strcpy(pLog->file_name, file_name, sizeof(pLog->file_name));
		pLog->file_id = dave_os_file_open(CREAT_READ_WRITE_FLAG, pLog->file_name);
		if(pLog->file_id >= 0)
		{
			kv_add_key_ptr(_log_file_kv, pLog->file_name, pLog);
		}
		else
		{
			LOGABNOR("open %s failed!", pLog->file_name);
			dave_free(pLog);
			pLog = NULL;
		}
	}

	log_unlock();

	return pLog;
}

static void
_log_save_log_file_free(s8 *file_name)
{
	LogFile *pLog;

	log_lock();

	pLog = kv_del_key_ptr(_log_file_kv, file_name);
	if(pLog != NULL)
	{
		if(pLog->file_id >= 0)
		{
			dave_os_file_close(pLog->file_id);
		}
	
		dave_free(pLog);
	}

	log_unlock();
}

static RetCode
_log_save_recycle(void *ramkv, s8 *key)
{
	LogFile *pLog = kv_inq_key_ptr(ramkv, key);

	if(pLog == NULL)
	{
		return RetCode_empty_data;
	}

	_log_save_log_file_free(pLog->file_name);

	return RetCode_OK;
}

static void
_log_save_timer_out(void *ramkv, s8 *key)	
{
	LogFile *pLog = kv_inq_key_ptr(ramkv, key);

	if(pLog == NULL)
	{
		return;
	}

	_log_save_log_file_free(pLog->file_name);
}

static inline LogFile *
_log_save_file_id(s8 *file_name)
{
	LogFile *pLog = kv_inq_key_ptr(_log_file_kv, file_name);

	if(pLog == NULL)
	{
		pLog = _log_save_log_file_malloc(file_name);
	}

	return pLog;
}

static inline void
_log_save_to_json_file(s8 *file_name, TraceLevel level, s8 *content_ptr, ub content_len)
{
	LogFile *pLog = _log_save_file_id(file_name);

	if(pLog == NULL)
	{
		LOGABNOR("save level:%d content:%d/%s to %s failed!",
			level, content_len, content_ptr, file_name);
		return;
	}

	SAFECODEv1(pLog->pv, log_save_json(pLog->file_id, level, content_ptr, content_len); );
}

static inline void
_log_save_to_txt_file(s8 *file_name, TraceLevel level, s8 *content_ptr, ub content_len)
{
	LogFile *pLog = _log_save_file_id(file_name);

	if(pLog == NULL)
	{
		LOGABNOR("save level:%d content:%d/%s to %s failed!",
			level, content_len, content_ptr, file_name);
		return;
	}

	SAFECODEv1(pLog->pv, log_save_txt(pLog->file_id, level, content_ptr, content_len); );
}

#ifdef LOG_STACK_SERVER

static inline void
_log_save_to_chain_file(s8 *file_name, s8 *device_info, s8 *service_verno, s8 *content_ptr, ub content_len)
{
	LogFile *pLog = _log_save_file_id(file_name);
	if(pLog == NULL)
	{
		LOGABNOR("save content:%d/%s to %s failed!",
			content_len, content_ptr, file_name);
		return;
	}

	SAFECODEv1(pLog->pv, log_save_chain(pLog->file_id, device_info, service_verno, content_ptr, content_len); );
}

static inline void
_log_save_build_chain_file_name(s8 *file_name_ptr, ub file_name_len, s8 *chain_name)
{
	DateStruct date = t_time_get_date(NULL);

	dave_snprintf(file_name_ptr, sizeof(file_name_ptr), "%s/%04d%02d%02d/%02d/%02d",
		chain_name,
		date.year, date.month, date.day,
		date.hour, date.minute);
}

#endif

static inline void
_log_save_build_log_file_name(s8 *file_name_ptr, ub file_name_len, s8 *project_name, s8 *device_info, s8 *extension)
{
	DateStruct date = t_time_get_date(NULL);

	dave_snprintf(file_name_ptr, file_name_len, "%s/%04d%02d%02d/%s.%s",
		project_name,
		date.year, date.month, date.day,
		device_info,
		extension);
}

// =====================================================================

void
log_save_init(void)
{
	log_save_cfg_init();

	_log_file_kv = kv_malloc("logsave", LOG_FILE_CLOSE_TIME, _log_save_timer_out);

#ifdef LOG_STACK_SERVER
	log_save_chain_init();
#endif

	log_save_auto_clean_init(log_save_days());
}

void
log_save_exit(void)
{
#ifdef LOG_STACK_SERVER
	log_save_chain_exit();
#endif

	kv_free(_log_file_kv, _log_save_recycle);

	log_save_cfg_exit();

	log_save_auto_clean_exit();
}

void
log_save_json_file(s8 *project_name, s8 *device_info, TraceLevel level, s8 *content_ptr, ub content_len)
{
	if(log_save_json_enable() == dave_true)
	{
		s8 file_ptr[256];

		_log_save_build_log_file_name(file_ptr, sizeof(file_ptr), project_name, device_info, "json");
	
		_log_save_to_json_file(file_ptr, level, content_ptr, content_len);
	}
}

void
log_save_txt_file(s8 *project_name, s8 *device_info, TraceLevel level, s8 *content_ptr, ub content_len)
{
	if(log_save_txt_enable() == dave_true)
	{
		s8 file_ptr[256];

		_log_save_build_log_file_name(file_ptr, sizeof(file_ptr), project_name, device_info, "txt");

		_log_save_to_txt_file(file_ptr, level, content_ptr, content_len);
	}
}

#ifdef LOG_STACK_SERVER

void
log_save_chain_file(s8 *chain_name, s8 *device_info, s8 *service_verno, s8 *content_ptr, ub content_len)
{
	s8 file_ptr[256];

	_log_save_build_chain_file_name(file_ptr, sizeof(file_ptr), chain_name);

	_log_save_to_chain_file(file_ptr, device_info, service_verno, content_ptr, content_len);
}

#endif

#endif

