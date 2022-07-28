/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "log_stack.h"
#ifdef LOG_STACK_SERVER
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_os.h"
#include "log_lock.h"
#include "log_save_log.h"
#include "log_save_chain.h"
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

static LogFile *
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

static LogFile *
_log_save_file_id(s8 *file_name)
{
	LogFile *pLog = kv_inq_key_ptr(_log_file_kv, file_name);

	if(pLog == NULL)
	{
		pLog = _log_save_log_file_malloc(file_name);
	}

	return pLog;
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

// =====================================================================

void
log_save_init(void)
{
	_log_file_kv = kv_malloc("logsave", KvAttrib_list, LOG_FILE_CLOSE_TIME, _log_save_timer_out);

	log_save_chain_init();
}

void
log_save_exit(void)
{
	log_save_chain_exit();

	kv_free(_log_file_kv, _log_save_recycle);
}

void
log_save_log_file(s8 *file_name, TraceLevel level, s8 *content_ptr, ub content_len)
{
	LogFile *pLog = _log_save_file_id(file_name);

	if(pLog == NULL)
	{
		LOGABNOR("save level:%d content:%d/%s to %s failed!",
			level, content_len, content_ptr, file_name);
		return;
	}

	SAFECODEv1(pLog->pv, log_save_log(pLog->file_id, level, content_ptr, content_len); );
}

void
log_save_chain_file(s8 *file_name, s8 *device_info, s8 *service_verno, s8 *content_ptr, ub content_len)
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

#endif

