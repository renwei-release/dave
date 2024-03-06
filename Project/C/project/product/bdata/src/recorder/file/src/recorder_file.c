/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BDATA__
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_base.h"
#include "dave_3rdparty.h"
#include "bdata_log.h"

#define FILE_HOME_DIR (s8 *)"RECORDER"
#define FILE_NAME_MAX (256)
#define KEY_NAME_MAX (64)
#define RECORDER_FILE_MAGIC_DATA (0xcdbf1867)
#define FILE_NOTE_MAGIC_DATA (0xcdbf1234)
#define RECORDER_FILE_BUF_MAX (1024 * 32)
#define RECORDER_FILE_SERIAL_MAX (8192)
#define RECORDER_FILE_DATA_DEEP_MAX (128)
#define RECORDER_FILE_STR_MAX (65536)
#define RECORDER_FILE_VERSION (1)
#define FILE_TOTAL_VERSION (1)
#define FILE_LOCK_WAIT_MAX (81960)

typedef struct {
	ub version;
	ub total_store_serial;
} StatisticsData;

typedef struct {
	StatisticsData data;
	s8 reserve[1024 - sizeof(StatisticsData)];
} FileStatistics;

typedef struct {
	ub magic;

	s8 file_dir[FILE_NAME_MAX];

	s8 file_name[FILE_NAME_MAX];
	sb file_id;
	ub file_index;
	ub file_current_store_serial;
	DateStruct the_file_date;

	sb statistics_file_id;
	FileStatistics statistics;

	TLock opt_pv;
} FileStore;

typedef struct {
	ub magic;

	FileStore *pStore;
	void *pJson;	
} FileNote;

static TLock _file_pv;
static void *_file_kv = NULL;

static void
_recorder_file_reset(FileStore *pStore)
{
	pStore->magic = RECORDER_FILE_MAGIC_DATA;

	dave_memset(pStore->file_dir, 0x00, FILE_NAME_MAX);

	dave_memset(pStore->file_name, 0x00, FILE_NAME_MAX);
	pStore->file_id = -1;
	pStore->file_index = 0;
	pStore->file_current_store_serial = 0;

	pStore->statistics_file_id = -1;
	pStore->statistics.data.version = FILE_TOTAL_VERSION;
	pStore->statistics.data.total_store_serial = 0;

	t_lock_reset(&(pStore->opt_pv));
}

static inline void
_recorder_file_write_statistics(FileStore *pStore)
{
	if(pStore->statistics_file_id >= 0)
	{
		pStore->statistics.data.version = FILE_TOTAL_VERSION;

		if(dave_os_file_save(pStore->statistics_file_id, 0, sizeof(FileStatistics), (u8 *)(&(pStore->statistics))) != sizeof(FileStatistics))
		{
			BDABNOR("write<%s> total file error!", pStore->file_dir);
		}
	}
}

static inline void
_recorder_file_read_statistics(FileStore *pStore)
{
	if(pStore->statistics_file_id >= 0)
	{
		dave_os_file_load(pStore->statistics_file_id, 0, sizeof(FileStatistics), (u8 *)(&(pStore->statistics)));

		BDDEBUG("dir:%s version:%d store_serial:%d", pStore->file_dir, pStore->statistics.data.version, pStore->statistics.data.total_store_serial);

		if(pStore->statistics.data.version != FILE_TOTAL_VERSION)
		{
			pStore->statistics.data.version = FILE_TOTAL_VERSION;
			pStore->statistics.data.total_store_serial = 0;			

			_recorder_file_write_statistics(pStore);
		}
	}
	else
	{
		BDABNOR("can't get file of %s!", pStore->file_dir);

		pStore->statistics.data.version = FILE_TOTAL_VERSION;
		pStore->statistics.data.total_store_serial = 0;
	}
}

static inline ub
_recorder_file_add_total_serial(FileStore *pStore)
{
	ub total_serial;

	total_serial = pStore->statistics.data.total_store_serial ++;

	_recorder_file_write_statistics(pStore);

	return total_serial;
}

static inline sb
_recorder_file_open_statistics(FileStore *pStore)
{
	s8 file_name[256];
	sb file_id;

	dave_snprintf(file_name, sizeof(file_name), "%s/%s/.statistics", FILE_HOME_DIR, pStore->file_dir);

	file_id = dave_os_file_open(CREAT_WRITE_FLAG, file_name);

	if(file_id >= 0)
	{
		pStore->statistics_file_id = file_id;

		_recorder_file_read_statistics(pStore);
	}

	return file_id;
}

static inline void
_recorder_file_close_statistics(FileStore *pStore)
{
	if(pStore->statistics_file_id >= 0)
	{		
		_recorder_file_write_statistics(pStore);		

		dave_os_file_close(pStore->statistics_file_id);
	}
}

static inline FileStore *
_recorder_file_find(s8 *file_dir)
{
	FileStore *pStore;

	pStore = kv_inq_key_ptr(_file_kv, file_dir);
	if(pStore == NULL)
	{
		pStore = dave_ralloc(sizeof(FileStore));

		kv_add_key_ptr(_file_kv, file_dir, pStore);

		_recorder_file_reset(pStore);

		dave_strcpy(pStore->file_dir, file_dir, FILE_NAME_MAX);

		_recorder_file_open_statistics(pStore);
	}

	return pStore;
}

static inline dave_bool
_recorder_file_change_file_name(FileStore *pStore)
{
	DateStruct date;
	dave_bool ret;

	if((pStore->file_name[0] == '\0')
		|| (pStore->file_id < 0)
		|| (pStore->file_current_store_serial == 0)
		|| (pStore->file_current_store_serial >= RECORDER_FILE_SERIAL_MAX))
	{
		ret = dave_true;
	}
	else
	{
		t_time_get_date(&date);

		if((pStore->the_file_date.year != date.year)
			|| (pStore->the_file_date.month != date.month)
			|| (pStore->the_file_date.day != date.day))
		{
			ret = dave_true;
		}
		else
		{
			ret = dave_false;
		}
	}

	if(ret == dave_true)
	{
		pStore->file_current_store_serial = 0;
	}

	return ret;
}

static inline dave_bool
_recorder_file_is_valid(FileStore *pStore)
{
	if((pStore->file_name[0] != '\0')
		&& (pStore->file_id >= 0)
		&& (dave_os_file_valid(pStore->file_name) == dave_false))
	{
		BDABNOR("can't find the file:%s/%ld index:%ld serial:%ld",
			pStore->file_name, pStore->file_id,
			pStore->file_index, pStore->file_current_store_serial);
		return dave_false;
	}

	return dave_true;
}

static inline void
_recorder_file_new_file_name(FileStore *pStore)
{
	s8 old_file_name[FILE_NAME_MAX];

	t_time_get_date(&(pStore->the_file_date));

	dave_strcpy(old_file_name, pStore->file_name, sizeof(pStore->file_name));

	dave_sprintf(pStore->file_name, "%s/%s/%s/%04d/%02d/%02d/%02d%02d%02d.bdata",
		dave_os_file_home_dir(),
		FILE_HOME_DIR, pStore->file_dir,
		pStore->the_file_date.year, pStore->the_file_date.month, pStore->the_file_date.day,
		pStore->the_file_date.hour, pStore->the_file_date.minute, pStore->the_file_date.second);

	if(pStore->file_id >= 0)
	{
		BDLOG("update new file_name:%s->%s", old_file_name, pStore->file_name);
	}
	else
	{
		BDLOG("creat new file_name:%s->%s", old_file_name, pStore->file_name);
	}
}

static inline void
_recorder_file_json_start(FileNote *pNote)
{
	pNote->pJson = dave_json_malloc();

	if(pNote->pJson != NULL)
	{
		dave_json_add_str(pNote->pJson, "BDATA-FILE", pNote->pStore->file_dir);
	}
}

static inline ub
_recorder_file_head_format(s8 *head_buffer, ub head_len, FileStore *pStore)
{
	ub head_index;
	DateStruct date;

	t_time_get_date(&date);

	head_index = 0;

	head_index += (ub)dave_snprintf(&head_buffer[head_index], head_len-head_index, "[%02d]\t", RECORDER_FILE_VERSION);
	head_index += (ub)dave_snprintf(&head_buffer[head_index], head_len-head_index, "[%04d]\t", pStore->file_current_store_serial);
	head_index += (ub)dave_snprintf(&head_buffer[head_index], head_len-head_index, "[%ld]\t", _recorder_file_add_total_serial(pStore));
	head_index += (ub)dave_snprintf(&head_buffer[head_index], head_len-head_index,
					"[%04d.%02d.%02d %02d:%02d:%02d]\t",
						date.year, date.month, date.day,
						date.hour, date.minute, date.second);
	return head_index;
}

static inline void
_recorder_file_recorder_file(FileStore *pStore, s8 *file_buffer, ub file_len)
{
	s8 head_buffer[1024];
	ub head_len;
	dave_bool flag;

	if((_recorder_file_change_file_name(pStore) == dave_true)
		|| _recorder_file_is_valid(pStore) == dave_false)
	{
		_recorder_file_new_file_name(pStore);

		if(pStore->file_id >= 0)
		{
			dave_os_file_close(pStore->file_id);
			pStore->file_id = -1;
		}

		pStore->file_id = dave_os_file_open(DIRECT_FLAG|CREAT_WRITE_FLAG, pStore->file_name);
		
		pStore->file_index = 0;
	}

	if(pStore->file_id >= 0)
	{
		head_len = _recorder_file_head_format(head_buffer, sizeof(head_buffer), pStore);

		flag = dave_false;

		if(dave_os_file_save(pStore->file_id, pStore->file_index, head_len, (u8 *)head_buffer) == head_len)
		{
			pStore->file_index += head_len;

			if(dave_os_file_save(pStore->file_id, pStore->file_index, file_len, (u8 *)file_buffer) == file_len)
			{
				pStore->file_index += file_len;

				if(dave_os_file_save(pStore->file_id, pStore->file_index, 1, (u8 *)"\n") == 1)
				{
					pStore->file_index += 1;

					pStore->file_current_store_serial ++;

					flag = dave_true;
				}
				else
				{
					BDLOG("file:%s/%s file end write failed!", pStore->file_dir, pStore->file_name);
				}
			}
			else
			{
				BDLOG("file:%s/%s file_len:%ld write failed!", pStore->file_dir, pStore->file_name, file_len);
			}
		}
		else
		{
			BDLOG("file:%s/%s head_len:%ld write failed!", pStore->file_dir, pStore->file_name, head_len);
		}

		if(flag == dave_false)
		{
			BDABNOR("save file:%s<%d,%d> failed!", pStore->file_name, file_len, pStore->file_index);
		}
	}
	else
	{
		BDABNOR("file<%s><%s> id is empty!", pStore->file_dir, pStore->file_name);
	}
}

static inline s8 *
_recorder_file_string_(FileNote *pNote, ub *string_len)
{
	ub string_len_temp;

	if(string_len == NULL)
	{
		string_len = &string_len_temp;
	}

	return (s8 *)json_object_to_json_string_length((struct json_object *)(pNote->pJson), JSON_C_TO_STRING_PLAIN, string_len);
}

static inline FileNote *
_recorder_file_open_(FileStore *pStore)
{
	FileNote *pNote;

	pNote = dave_malloc(sizeof(FileNote));

	pNote->magic = FILE_NOTE_MAGIC_DATA;

	pNote->pStore = pStore;

	pNote->pJson = NULL;

	_recorder_file_json_start(pNote);

	return pNote;
}

static inline void
_recorder_file_close_(FileNote *pNote)
{
	FileStore *pStore;
	s8 *string_ptr;
	ub string_len;

	if(pNote->magic != FILE_NOTE_MAGIC_DATA)
	{
		BDABNOR("invalid magic:%x", pNote->magic);
		return;
	}

	pStore = pNote->pStore;

	if(pStore->magic == RECORDER_FILE_MAGIC_DATA)
	{
		if(pNote->pJson != NULL)
		{
			string_ptr = _recorder_file_string_(pNote, &string_len);
			
			SAFECODEv1(pStore->opt_pv, {

				_recorder_file_recorder_file(pStore, string_ptr, string_len);

			});

			dave_json_free(pNote->pJson);

			pNote->pJson = NULL;
		}
		else
		{
			BDABNOR("Empty pJson!");
		}
	}
	else
	{
		BDABNOR("invalid magic:%x", pStore->magic);
	}

	dave_free(pNote);
}

static inline dave_bool
_recorder_file_str_(FileNote *pNote, char *key_name, s8 *str_data, ub str_len)
{
	if(str_data == NULL)
	{
		BDABNOR("key_name:%s has empty data!", key_name);
		return dave_false;
	}

	if(str_len == 0)
	{
		str_len = dave_strlen(str_data);
	}

	return dave_json_add_str_len(pNote->pJson, key_name, str_data, str_len);
}

static RetCode
_recorder_file_recycle(void *ramkv, s8 *key)
{
	FileStore *pStore = kv_del_key_ptr(_file_kv, key);

	if(pStore == NULL)
		return RetCode_empty_data;

	SAFECODEv1(pStore->opt_pv, {
		if(pStore->file_id >= 0)
		{
			dave_os_file_close(pStore->file_id);
		}
	} );
	
	_recorder_file_close_statistics(pStore);

	dave_free(pStore);

	return RetCode_OK;
}

// ====================================================================

void
recorder_file_init(void)
{
	t_lock_reset(&_file_pv);

	_file_kv = kv_malloc("recorder-file", 0, NULL);
}

void
recorder_file_exit(void)
{
	SAFECODEv1(_file_pv, {
		kv_free(_file_kv, _recorder_file_recycle);
	} );
}

void *
recorder_file_open(s8 *file_dir)
{
	FileStore *pStore = NULL;

	if((file_dir == NULL) || (file_dir[0] == '\0'))
	{
		BDABNOR("file name is empty:%s!", file_dir);
		return NULL;
	}

	if(t_is_all_show_char((u8 *)file_dir, dave_strlen(file_dir)) == dave_false)
	{
		BDABNOR("has invalid characters!");
		return NULL;
	}

	SAFECODEv1(_file_pv, { pStore = _recorder_file_find(file_dir); } );

	if(pStore == NULL)
	{
		BDABNOR("file_dir:%s open failed!", file_dir);
		return NULL;
	}

	return _recorder_file_open_(pStore);
}

void
recorder_file_close(void *ptr)
{
	FileNote *pNote = (FileNote *)ptr;

	if((pNote == NULL) || (pNote->magic != FILE_NOTE_MAGIC_DATA))
	{
		BDABNOR("invalid ptr:%x", ptr);
		return;
	}

	_recorder_file_close_(pNote);
}

void *
recorder_file_json(void *ptr)
{
	FileNote *pNote = (FileNote *)ptr;

	return pNote->pJson;
}

dave_bool
recorder_file_str(void *ptr, char *key_name, s8 *str_data, ub str_len)
{
	FileNote *pNote = (FileNote *)ptr;

	if((pNote == NULL) || (pNote->magic != FILE_NOTE_MAGIC_DATA))
	{
		BDABNOR("invalid ptr:%x", ptr);
		return dave_false;
	}

	if(key_name == NULL)
	{
		BDABNOR("key_name is NULL!");
		return dave_false;
	}

	if(str_data == NULL)
	{
		str_data = "NULL";
	}

	return _recorder_file_str_(pNote, key_name, str_data, str_len);
}

dave_bool
recorder_file_bin(void *ptr, char *key_name, u8 *bin_data, ub bin_len)
{
	ub bin_str_length = bin_len * 2 + 1;
	s8 *bin_str_buffer;
	dave_bool ret;

	bin_str_buffer = dave_malloc(bin_str_length);

	bin_str_length = t_crypto_base64_encode(bin_data, bin_len, bin_str_buffer, bin_str_length - 1);

	bin_str_buffer[bin_str_length] = '\0';

	ret = recorder_file_str(ptr, key_name, bin_str_buffer, bin_str_length);

	dave_free(bin_str_buffer);

	return ret;
}

dave_bool
recorder_file_obj(void *ptr, char *key_name, s8 *str_data, ub str_len)
{
	if((str_len > 2) && (str_data[0] == '{') && (str_data[str_len - 1] == '}'))
	{
		FileNote *pNote = (FileNote *)ptr;
		void *obj_json = dave_string_to_json(str_data, str_len);

		if(obj_json != NULL)
		{

			return dave_json_add_object(pNote->pJson, key_name, obj_json);
		}
		else
		{
			BDLOG("file:%s key:%s is not json data, write to string:%ld/%s",
				pNote->pStore->file_name, key_name,
				str_len, str_data);
		}
	}

	return recorder_file_str(ptr, key_name, str_data, str_len);
}

ub
recorder_file_info(s8 *info_ptr, ub info_len)
{
	ub info_index, index;
	FileStore *pStore;

	info_index = dave_snprintf(info_ptr, info_len, "RECORDER FILE INFO:\n");

	for(index=0; index<102400; index++)
	{
		pStore = kv_index_key_ptr(_file_kv, index);
		if(pStore == NULL)
			break;

		if(index > 0)
		{
			info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "\n");
		}

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
			" name:%s/%ld len:%ld serial:%ld date:%s",
			pStore->file_name, pStore->file_id, pStore->file_index,
			pStore->file_current_store_serial, datestr(&(pStore->the_file_date)));
	}

	return info_index;
}

#endif

