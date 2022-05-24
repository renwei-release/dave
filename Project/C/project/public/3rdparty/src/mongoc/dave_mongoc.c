/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(MONGO_3RDPARTY)
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/sysinfo.h>
#include <dlfcn.h>
#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_mongoc.h"
#include "dave_3rdparty.h"
#include "party_log.h"

#if defined(LEVEL_PRODUCT_alpha)
// #define MONGOC_DEBUG_ENABLE
#endif

#define ENABLE_MONGOC_AUTH

#define MONGDB_RANGE_MEMORY 10
#define MONGDB_MAX_MEMORY 5

#ifdef MONGOC_DEBUG_ENABLE
#define MONGOCSHOWBSON(msg, bson) _mongoc_show_bson_data((char *)msg, (bson_t *)bson)
#else
#define MONGOCSHOWBSON(msg, bson)
#endif 

#define MONGOC_DEFAULT_VALUE_NAME (const char *)"_value"
#define MONGOC_DEFAULT_DATE_NAME (const char *)"_date"

#define MONGODB_BIN_DIR (s8 *)"/dave/tools/mongodb/bin"
#define MONGODB_BIN_NAME (s8 *)"mongod"
#define MONGODB_BIN		(s8 *)"mongo"
#define MONGODB_CFG_NAME (s8 *)"mongodb.conf"
#define MONGODB_CREAT_ADMIN (s8 *)" --eval \"db.getSiblingDB(\'admin\').createUser({user:\'admin\', pwd:\'admin\', roles:[{role:\'userAdminAnyDatabase\', db:\'admin\'}]})\""
#define MONGODB_DOWN_SERVER (s8 *)"--eval \"db.getSiblingDB(\'admin\').shutdownServer()\""
#define MONGODB_CREAT_USER0 (s8 *)" --eval \"db.getSiblingDB(\'"
#define MONGODB_CREAT_USER1 (s8 *)"\').createUser({user:\'"
#define MONGODB_CREAT_USER2 (s8 *)"\', pwd:\'"
#define MONGODB_CREAT_USER3 (s8 *)"\', roles:[{role:\'readWrite\', db:\'"
#define MONGODB_CREAT_USER4 (s8 *)"\'}]})\""

static volatile dave_bool _mongoc_init = dave_false;
static float _mongoc_CacheSizeGB = 0;
static void *_bson_dll_handle = NULL;
static void *_mongoc_dll_handle = NULL;

static char * (* so_bson_as_json)(const bson_t *bson, size_t *length) = NULL;
static void (* so_bson_free)(void *mem) = NULL;
static void (* so_bson_oid_init)(bson_oid_t *oid, bson_context_t *context) = NULL;
static void (* so_bson_oid_to_string)(const bson_oid_t *oid, char str[25]) = NULL;
static void (* so_bson_oid_init_from_string)(bson_oid_t *oid, const char *str) = NULL;
static bool (* so_bson_append_oid)(bson_t *bson, const char *key, int key_length, const bson_oid_t *oid) = NULL;
static bool (* so_bson_append_now_utc)(bson_t *bson, const char *key, int key_length) = NULL;
static bool (* so_bson_iter_init)(bson_iter_t *iter, const bson_t *bson) = NULL;
static bool (* so_bson_iter_find)(bson_iter_t *iter, const char *key) = NULL;
static void (* so_bson_iter_binary)(const bson_iter_t *iter, bson_subtype_t *subtype, uint32_t *binary_len, const uint8_t **binary) = NULL;
static bool (* so_bson_append_binary)(bson_t *bson, const char *key, int key_length, bson_subtype_t subtype, const uint8_t *binary, uint32_t length) = NULL;
static void (* so_bson_destroy)(bson_t *bson) = NULL;
static bson_t * (* so_bson_new)(void) = NULL;
static bson_t * (* so_bson_new_from_json)(const uint8_t *data, ssize_t len, bson_error_t *error) = NULL;
static bool (* so_bson_append_utf8)(bson_t *bson, const char *key, int key_length, const char *value, int length) = NULL;

static void (* so_mongoc_init)(void) = NULL;
static void (* so_mongoc_cleanup)(void) = NULL;
static mongoc_client_t * (* so_mongoc_client_new)(const char *uri_string) = NULL;
static void (* so_mongoc_client_destroy)(mongoc_client_t *client) = NULL;
static mongoc_database_t * (* so_mongoc_client_get_database)(mongoc_client_t *client, const char *name) = NULL;
static void (* so_mongoc_database_destroy)(mongoc_database_t *database) = NULL;
static mongoc_cursor_t * (* so_mongoc_collection_find_with_opts)(mongoc_collection_t *collection, const bson_t *filter, const bson_t *opts, const mongoc_read_prefs_t *read_prefs) = NULL;
static bool (* so_mongoc_cursor_next)(mongoc_cursor_t *cursor, const bson_t **bson) = NULL;
static void (* so_mongoc_cursor_destroy)(mongoc_cursor_t *cursor) = NULL;
static void (* so_mongoc_log_set_handler)(mongoc_log_func_t log_func, void *user_data) = NULL;
static mongoc_collection_t * (* so_mongoc_client_get_collection)(mongoc_client_t *client, const char *db, const char *collection) = NULL;
static void (* so_mongoc_collection_destroy)(mongoc_collection_t *collection) = NULL;
static bool (* so_mongoc_collection_insert)(mongoc_collection_t *collection, mongoc_insert_flags_t flags, const bson_t *document, const mongoc_write_concern_t *write_concern, bson_error_t *error) = NULL;
static bool (* so_mongoc_collection_update_one)(mongoc_collection_t *collection, const bson_t *selector, const bson_t *update, const bson_t *opts, bson_t *reply, bson_error_t *error) = NULL;
static bool (* so_mongoc_collection_insert_one)(mongoc_collection_t *collection, const bson_t *document, const bson_t *opts, bson_t *reply, bson_error_t *error);
static bool (* so_mongoc_collection_update)(mongoc_collection_t *collection, mongoc_update_flags_t flags, const bson_t *selector, const bson_t *update, const mongoc_write_concern_t *write_concern, bson_error_t *error) = NULL;

#ifdef ENABLE_MONGOC_AUTH
static void _mongoc_enable_auth(ub port, s8 *db_name, s8 *file_name, s8 *user, s8 *password, ub CacheSizeGB);
#endif

static dave_bool
_bson_dll_init(void)
{
	static const char dll_file_table[][256] = {
		{"/dave/tools/mongodb-c/lib64/libbson-1.0.so"},
		{""}
	};
	sb dll_file_table_index;

	if(_bson_dll_handle == NULL)
	{
		for(dll_file_table_index=0; (_bson_dll_handle==NULL)&&(dll_file_table_index<16); dll_file_table_index++)
		{
			if(dave_strlen(dll_file_table[dll_file_table_index]) == 0)
				break;

			_bson_dll_handle = dlopen(dll_file_table[dll_file_table_index], RTLD_LAZY);
		}
	}

	if(_bson_dll_handle == NULL)
	{
		PARTYABNOR("can not find any dll lib!!");
		return dave_false;
	}

	so_bson_as_json = dlsym(_bson_dll_handle, "bson_as_json");
	if(so_bson_as_json == NULL)
		return dave_false;
	so_bson_free = dlsym(_bson_dll_handle, "bson_free");
	if(so_bson_free == NULL)
		return dave_false;
	so_bson_oid_init = dlsym(_bson_dll_handle, "bson_oid_init");
	if(so_bson_oid_init == NULL)
		return dave_false;
	so_bson_oid_to_string = dlsym(_bson_dll_handle, "bson_oid_to_string");
	if(so_bson_oid_to_string == NULL)
		return dave_false;
	so_bson_oid_init_from_string = dlsym(_bson_dll_handle, "bson_oid_init_from_string");
	if(so_bson_oid_init_from_string == NULL)
		return dave_false;
	so_bson_append_oid = dlsym(_bson_dll_handle, "bson_append_oid");
	if(so_bson_append_oid == NULL)
		return dave_false;
	so_bson_append_now_utc = dlsym(_bson_dll_handle, "bson_append_now_utc");
	if(so_bson_append_now_utc == NULL)
		return dave_false;
	so_bson_iter_init = dlsym(_bson_dll_handle, "bson_iter_init");
	if(so_bson_iter_init == NULL)
		return dave_false;
	so_bson_iter_find = dlsym(_bson_dll_handle, "bson_iter_find");
	if(so_bson_iter_find == NULL)
		return dave_false;
	so_bson_iter_binary = dlsym(_bson_dll_handle, "bson_iter_binary");
	if(so_bson_iter_binary == NULL)
		return dave_false;
	so_bson_append_binary = dlsym(_bson_dll_handle, "bson_append_binary");
	if(so_bson_append_binary == NULL)
		return dave_false;
	so_bson_destroy = dlsym(_bson_dll_handle, "bson_destroy");
	if(so_bson_destroy == NULL)
		return dave_false;
	so_bson_new = dlsym(_bson_dll_handle, "bson_new");
	if(so_bson_new == NULL)
		return dave_false;
	so_bson_new_from_json = dlsym(_bson_dll_handle, "bson_new_from_json");
	if(so_bson_new_from_json == NULL)
		return dave_false;
	so_bson_append_utf8 = dlsym(_bson_dll_handle, "bson_append_utf8");
	if(so_bson_append_utf8 == NULL)
		return dave_false;

	return dave_true;
}

static void
_bson_dll_exit(void)
{
	if(_bson_dll_handle != NULL)
	{
		dlclose(_bson_dll_handle);
		_bson_dll_handle = NULL;

		so_bson_as_json = NULL;
		so_bson_free = NULL;
		so_bson_oid_init = NULL;
		so_bson_oid_to_string = NULL;
		so_bson_oid_init_from_string = NULL;
		so_bson_append_oid = NULL;
		so_bson_append_now_utc = NULL;
		so_bson_iter_init = NULL;
		so_bson_iter_find = NULL;
		so_bson_iter_binary = NULL;
		so_bson_append_binary = NULL;
		so_bson_destroy  = NULL;
		so_bson_new = NULL;
		so_bson_new_from_json = NULL;
	}
}

static dave_bool
_mongoc_dll_init(void)
{
	static const char dll_file_table[][256] = {
		{"/dave/tools/mongodb-c/lib64/libmongoc-1.0.so"},
		{""}
	};
	sb dll_file_table_index;

	if(_mongoc_dll_handle == NULL)
	{
		for(dll_file_table_index=0; (_mongoc_dll_handle==NULL)&&(dll_file_table_index<16); dll_file_table_index++)
		{
			if(dave_strlen(dll_file_table[dll_file_table_index]) == 0)
				break;

			_mongoc_dll_handle = dlopen(dll_file_table[dll_file_table_index], RTLD_LAZY);
		}
	}

	if(_mongoc_dll_handle == NULL)
	{
		PARTYABNOR("can not find any dll lib!!");
		return dave_false;
	}

	so_mongoc_init = dlsym(_bson_dll_handle, "mongoc_init");
	if(so_mongoc_init == NULL)
		return dave_false;
	so_mongoc_cleanup = dlsym(_bson_dll_handle, "mongoc_cleanup");
	if(so_mongoc_cleanup == NULL)
		return dave_false;
	so_mongoc_client_new = dlsym(_bson_dll_handle, "mongoc_client_new");
	if(so_mongoc_client_new == NULL)
		return dave_false;
	so_mongoc_client_destroy = dlsym(_bson_dll_handle, "mongoc_client_destroy");
	if(so_mongoc_client_destroy == NULL)
		return dave_false;
	so_mongoc_client_get_database = dlsym(_bson_dll_handle, "mongoc_client_get_database");
	if(so_mongoc_client_get_database == NULL)
		return dave_false;
	so_mongoc_database_destroy = dlsym(_bson_dll_handle, "mongoc_database_destroy");
	if(so_mongoc_database_destroy == NULL)
		return dave_false;
	so_mongoc_collection_find_with_opts = dlsym(_bson_dll_handle, "mongoc_collection_find_with_opts");
	if(so_mongoc_collection_find_with_opts == NULL)
		return dave_false;
	so_mongoc_cursor_next = dlsym(_bson_dll_handle, "mongoc_cursor_next");
	if(so_mongoc_cursor_next == NULL)
		return dave_false;
	so_mongoc_cursor_destroy = dlsym(_bson_dll_handle, "mongoc_cursor_destroy");
	if(so_mongoc_cursor_destroy == NULL)
		return dave_false;
	so_mongoc_log_set_handler = dlsym(_bson_dll_handle, "mongoc_log_set_handler");
	if(so_mongoc_log_set_handler == NULL)
		return dave_false;
	so_mongoc_client_get_collection = dlsym(_bson_dll_handle, "mongoc_client_get_collection");
	if(so_mongoc_client_get_collection == NULL)
		return dave_false;
	so_mongoc_collection_destroy = dlsym(_bson_dll_handle, "mongoc_collection_destroy");
	if(so_mongoc_collection_destroy == NULL)
		return dave_false;
	so_mongoc_collection_insert = dlsym(_bson_dll_handle, "mongoc_collection_insert");
	if(so_mongoc_collection_insert == NULL)
		return dave_false;
	so_mongoc_collection_update_one = dlsym(_bson_dll_handle, "mongoc_collection_update_one");
	if(so_mongoc_collection_update_one == NULL)
		return dave_false;
	so_mongoc_collection_insert_one = dlsym(_bson_dll_handle, "mongoc_collection_insert_one");
	if(so_mongoc_collection_insert_one == NULL)
		return dave_false;
	so_mongoc_collection_update = dlsym(_bson_dll_handle, "mongoc_collection_update");
	if(so_mongoc_collection_update == NULL)
		return dave_false;

	return dave_true;
}

static void
_mongoc_dll_exit(void)
{
	if(_mongoc_dll_handle != NULL)
	{
		dlclose(_mongoc_dll_handle);
		_mongoc_dll_handle = NULL;

		so_mongoc_init = NULL;
		so_mongoc_cleanup = NULL;
		so_mongoc_client_new = NULL;
		so_mongoc_client_destroy = NULL;
		so_mongoc_client_get_database = NULL;
		so_mongoc_database_destroy = NULL;
		so_mongoc_collection_find_with_opts = NULL;
		so_mongoc_cursor_next = NULL;
		so_mongoc_cursor_destroy = NULL;
		so_mongoc_log_set_handler = NULL;
		so_mongoc_client_get_collection = NULL;
		so_mongoc_collection_destroy = NULL;
		so_mongoc_collection_insert = NULL;
		so_mongoc_collection_update_one = NULL;
		so_mongoc_collection_insert_one = NULL;
		so_mongoc_collection_update = NULL;
	}
}

#ifdef MONGOC_DEBUG_ENABLE

static void
_mongoc_show_bson_data(char *msg, bson_t *pBson)
{
	char *str;

	str = so_bson_as_json(pBson, NULL);

	PARTYLOG("msg:\"%s\" bson:%s", msg, str);

	so_bson_free(str);
}

static void
_mongoc_log_handler(mongoc_log_level_t  log_level, const char* log_domain, const char* message, void* user_data)
{
	PARTYLOG("[MONGOC]%s", message);
}

#endif /*MONGOC_DEBUG_ENABLE*/

static void
_mongoc_build_conf_file(ub port, s8 *file_name, dave_bool auth)
{
	s8 *dbpath = (s8 *)"/dave/dba/mongodb/data/db/";
	s8 *logfile = (s8 *)"/dave/dba/mongodb/data/logs";
	s8 *conf_buf;
	ub conf_buf_len = 2048;
	ub conf_buf_index;

	dave_os_file_creat_dir(dbpath);
	dave_os_file_creat_dir(logfile);

	dave_os_file_delete(DIRECT_FLAG, file_name);

	conf_buf = dave_malloc(conf_buf_len);
	conf_buf_index = 0;

	conf_buf_index += dave_snprintf(&conf_buf[conf_buf_index], conf_buf_len-conf_buf_index, "bind_ip = 0.0.0.0\n");
	conf_buf_index += dave_snprintf(&conf_buf[conf_buf_index], conf_buf_len-conf_buf_index, "port=%d\n", port);
	conf_buf_index += dave_snprintf(&conf_buf[conf_buf_index], conf_buf_len-conf_buf_index, "dbpath=%s\n", dbpath);
	conf_buf_index += dave_snprintf(&conf_buf[conf_buf_index], conf_buf_len-conf_buf_index, "logappend=true\n");
	conf_buf_index += dave_snprintf(&conf_buf[conf_buf_index], conf_buf_len-conf_buf_index, "fork=true\n");
	conf_buf_index += dave_snprintf(&conf_buf[conf_buf_index], conf_buf_len-conf_buf_index, "logpath=%s\n", logfile);
	conf_buf_index += dave_snprintf(&conf_buf[conf_buf_index], conf_buf_len-conf_buf_index, "journal=true\n");

	if (dave_true == auth)
	{
		conf_buf_index += dave_snprintf(&conf_buf[conf_buf_index], conf_buf_len-conf_buf_index, "auth=true\n");
	}	
	else
	{
		conf_buf_index += dave_snprintf(&conf_buf[conf_buf_index], conf_buf_len-conf_buf_index, "auth=false\n");
	}
	dave_os_file_write(DIRECT_FLAG|CREAT_WRITE_FLAG, file_name, 0, conf_buf_index, (u8 *)conf_buf);

	dave_free(conf_buf);
}

static float
_mongoc_server_system_memory_range(void)
{
	struct sysinfo si;
	float total_mem_gbyte;

	sysinfo(&si);

	total_mem_gbyte = ((float)(si.freeram / (1024 * 1024 * 1024)) / MONGDB_RANGE_MEMORY);
	if(total_mem_gbyte > MONGDB_MAX_MEMORY)
	{
		total_mem_gbyte = MONGDB_MAX_MEMORY;
	}

	PARTYDEBUG("set up mongdb cache on %fGB", total_mem_gbyte);

	return total_mem_gbyte;
}

static void
_mongoc_server_stop(void)
{
	s8 kill_cmd[128];

	dave_sprintf(kill_cmd, "killall -2 %s", MONGODB_BIN_NAME);

	system((const char *)kill_cmd);
}

static dave_bool  
__mongoc_server_start__(s8 *file_name, float CacheSizeGB)
{
	s8 start_cmd[512];
	int system_ret;

	dave_snprintf(start_cmd, sizeof(start_cmd), "%s/%s --wiredTigerCacheSizeGB %f -f %s", MONGODB_BIN_DIR, MONGODB_BIN_NAME, CacheSizeGB, file_name);

	system_ret = system((const char *)start_cmd);
	if(system_ret != 0)
	{
		PARTYABNOR("system run %s failed:%d", start_cmd, system_ret);
		return dave_false;
	}

	return dave_true;
}

static dave_bool
_mongoc_server_start(ub port, s8 *db_name, s8 *user, s8 *password, float CacheSizeGB)
{
	s8 file_name[128];

	if(dave_os_process_exist(MONGODB_BIN_NAME) == dave_true)
	{
		_mongoc_server_stop();
	}

	dave_snprintf(file_name, sizeof(file_name), "%sconfig/%s", dave_os_file_home_dir(), MONGODB_CFG_NAME);

	dave_os_file_creat_dir(file_name);

#ifdef ENABLE_MONGOC_AUTH
	_mongoc_enable_auth(port, db_name, file_name, user, password, CacheSizeGB);
#else
	_mongoc_build_conf_file(port, file_name, dave_false);

	__mongoc_server_start__(file_name, CacheSizeGB);
#endif

	return dave_true;
}

static void
_mongoc_client_stop(void)
{
	if(_mongoc_init == dave_true)
	{
		so_mongoc_cleanup();
		_mongoc_init = dave_false;
	}
}

static dave_bool
_mongoc_client_start(void)
{
	if(dave_false == _mongoc_init)
	{
		so_mongoc_init();
		_mongoc_init = dave_true;
	}

	return _mongoc_init;
}

#ifdef ENABLE_MONGOC_AUTH

static void
_mongoc_enable_auth(ub port, s8 *db_name, s8 *file_name, s8 *user, s8 *password, ub CacheSizeGB)
{
	s8 command[1024];
	int ret;
	
	_mongoc_server_stop();
	
	dave_os_sleep(2000);

	_mongoc_build_conf_file(port, file_name, dave_false);

	__mongoc_server_start__(file_name, CacheSizeGB);

	dave_os_sleep(2000);
	
	dave_snprintf(command, sizeof(command), "%s/%s %s", MONGODB_BIN_DIR, MONGODB_BIN, MONGODB_CREAT_ADMIN);
	ret = system((const char *)command);
	if(ret < 0)
	{
		PARTYLOG("ret:%d command:%s", ret, command);
	}

	dave_snprintf(command, sizeof(command), "%s/%s  --authenticationDatabase \"admin\" %s", MONGODB_BIN_DIR, MONGODB_BIN, MONGODB_DOWN_SERVER);
	ret = system((const char *)command);
	if(ret < 0)
	{
		PARTYLOG("ret:%d command:%s", ret, command);
	}

	_mongoc_build_conf_file(port, file_name, dave_true);

	__mongoc_server_start__(file_name, CacheSizeGB);

	dave_os_sleep(2000);

	dave_snprintf(command, sizeof(command), "%s/%s -u \"admin\" -p \"admin\" --authenticationDatabase \"admin\" %s%s%s%s%s%s%s%s%s",\
					MONGODB_BIN_DIR, MONGODB_BIN, MONGODB_CREAT_USER0, db_name,\
					MONGODB_CREAT_USER1,user, MONGODB_CREAT_USER2, password,\
					MONGODB_CREAT_USER3, db_name, MONGODB_CREAT_USER4);
	
	ret = system((const char *)command);
	if(ret < 0)
	{
		PARTYLOG("ret:%d command:%s", ret, command);
	}
}

#endif

static mongoc_client_t *
_mongoc_client_connect(s8 *ip_addr, ub port, s8* db_name, s8 *user, s8 *password)
{
	s8 url[512];

	if((NULL==ip_addr) || (NULL==db_name))
	{
		PARTYABNOR("Invalid parameter!");
		return NULL;
	}
	if(('\0'==user[0]) || ('\0'==password[0]))
	{
		dave_snprintf(url, sizeof(url), "mongodb://%s:%d/?authSource=%s", ip_addr, port, db_name);
	}
	else
	{
		dave_snprintf(url, sizeof(url), "mongodb://%s:%s@%s:%d/?authSource=%s", user, password, ip_addr, port, db_name);
	}

	return so_mongoc_client_new((const char *)url);
}

static void
_mongoc_client_disconnect(mongoc_client_t *pClient)
{
	if(pClient != NULL)
	{
		so_mongoc_client_destroy(pClient);
	}
}

static mongoc_database_t *
_mongoc_database_connect(mongoc_client_t *pClient, s8 *db_name)
{
	return so_mongoc_client_get_database(pClient, (const char *)db_name);
}

static void
_mongoc_database_disconnect(mongoc_database_t *pDatabase)
{
	if(pDatabase != NULL)
	{
		so_mongoc_database_destroy(pDatabase);
	}
}

static dave_bool
_mongoc_build_oid(u8 *key, s8 *key_str)
{
	bson_oid_t oid;

	so_bson_oid_init(&oid, NULL);

	if(key != NULL)
	{
		dave_memcpy(key, oid.bytes, DAVE_NOSQL_KEY_MAX);
	}
	else if(key_str != NULL)
	{
		so_bson_oid_to_string(&oid, (char *)key_str);
	}

	return dave_true;
}

static dave_bool
_mongoc_inside_oid(bson_t *pBson, u8 *key, s8 *key_str)
{
	bson_oid_t oid;

	if(key != NULL)
	{
		dave_memcpy(oid.bytes, key, 12);
	}
	else if(key_str != NULL)
	{
		so_bson_oid_init_from_string(&oid, (const char *)key_str);
	}

	if(!so_bson_append_oid(pBson, "_id", strlen("_id"), &oid))
	{
		return dave_false;
	}

	return dave_true;
}

static void
_mongoc_inside_time(bson_t *pBson)
{
	so_bson_append_now_utc(pBson, MONGOC_DEFAULT_DATE_NAME, 5);
}

static dave_bool
_mongoc_database_query(void *pCollection, bson_t *query, u8 *value, ub *value_len, dave_bool det)
{
	mongoc_cursor_t *cursor;
	const bson_t *doc;
	bson_iter_t iter;
	bson_subtype_t subtype;
	uint32_t binary_len;
	const uint8_t *binary;
	dave_bool ret = dave_false;

	cursor = so_mongoc_collection_find_with_opts((mongoc_collection_t *)pCollection, (const bson_t *)query, NULL, NULL);

	if(cursor != NULL)
	{
		ret = dave_false;
	
		while((ret == dave_false) && (so_mongoc_cursor_next(cursor, &doc)))
		{
			MONGOCSHOWBSON("inq key value", doc);

			if(so_bson_iter_init(&iter, doc))
			{
				if(so_bson_iter_find(&iter, MONGOC_DEFAULT_VALUE_NAME))
				{
					if(det == dave_false)
					{
						binary_len = (uint32_t)(*value_len);
						binary = NULL;
	
						so_bson_iter_binary(&iter, &subtype, &binary_len, &binary);

						if((binary_len > 0) && (binary != NULL))
						{
							if(binary_len <= *value_len)
							{
								if(value != NULL)
								{
									dave_memcpy(value, binary, binary_len);
								}
								*value_len = (ub)binary_len;
								ret = dave_true;
							}
							else
							{
								PARTYABNOR("binary_len:%d is too big<%d>!", binary_len, *value_len);
							}
						}
						else
						{
							PARTYABNOR("invalid binary_len:%d or binary:%x", binary_len, binary);
						}
					}
					else
					{
						ret = dave_true;
					}
				}
				else
				{
					PARTYABNOR("can't find name:%s", MONGOC_DEFAULT_VALUE_NAME);
				}
			}
			else
			{
				PARTYABNOR("iter init failed!");
			}
		}

		so_mongoc_cursor_destroy(cursor);
	}

	return ret;
}

static dave_bool
_mongoc_init_(dave_bool server_flag, ub port, s8 *db_name, s8 *user, s8 *password)
{
	_mongoc_CacheSizeGB = _mongoc_server_system_memory_range();

	if(server_flag == dave_true)
	{
		if(_mongoc_server_start(port, db_name, user, password, _mongoc_CacheSizeGB) == dave_false)
		{
			return dave_false;
		}
	}

	if(_mongoc_client_start() == dave_false)
	{
		return dave_false;
	}

#ifdef MONGOC_DEBUG_ENABLE
	so_mongoc_log_set_handler(_mongoc_log_handler, NULL);
#endif

	PARTYLOG("mongoc startup port:%d user:%s password:%s cache:%fGB", port, user, password, _mongoc_CacheSizeGB);

	return dave_true;
}

static void
_mongoc_exit_(dave_bool server_flag)
{
	_mongoc_client_stop();

	if(server_flag == dave_true)
	{
		_mongoc_server_stop();
	}
}

// =====================================================================

dave_bool
dave_mongoc_init(dave_bool server_flag, ub port, s8 *db_name, s8 *user, s8 *password)
{
	_bson_dll_init();
	_mongoc_dll_init();

	return _mongoc_init_(server_flag, port, db_name, user, password);
}

void
dave_mongoc_exit(dave_bool server_flag)
{
	_mongoc_exit_(server_flag);

	_bson_dll_exit();
	_mongoc_dll_exit();
}

void * 
dave_mongoc_connect(s8 *ip_addr, ub port, s8* db_name, s8 *user, s8 *password, void **ppDatabase)
{
	mongoc_client_t *pClient = NULL;
	mongoc_database_t *pDatabase = NULL;
	*ppDatabase = NULL;
	
	pClient = _mongoc_client_connect(ip_addr, port, db_name, user, password);
	
	if(pClient != NULL)
	{
		pDatabase = _mongoc_database_connect(pClient, db_name);

		*ppDatabase = pDatabase;
	}
	else
	{
		*ppDatabase = NULL;
	}

	return pClient;
}

void
dave_mongoc_disconnect(void *pClient, void *pDatabase)
{
	if(pDatabase != NULL)
	{
		_mongoc_database_disconnect(pDatabase);
	}

	if(pClient != NULL)
	{
		_mongoc_client_disconnect(pClient);
	}
}

void *
dave_mongoc_capture_collection(void *pClient, s8 *db_name, s8 *coll_name)
{
	if(pClient == NULL)
	{
		return NULL;
	}

	return so_mongoc_client_get_collection(pClient, (const char *)db_name, (const char *)coll_name);
}

void
dave_mongoc_release_collection(void *pCollection)
{
	if(pCollection != NULL)
	{
		so_mongoc_collection_destroy(pCollection);
	}
}

dave_bool
__dave_mongoc_add_bin__(void *pCollection, s8 *obj_str, u8 *value, ub value_len, s8 *fun, ub line)
{
	u8 *obj = NULL;
	bson_t *insert;
	bson_error_t error;
	dave_bool ret;

	PARTYDEBUG("%s:%d", fun, line);

	if(pCollection == NULL)
	{
		PARTYABNOR("add pCollection is NULL! <%s:%d>", fun, line);
		return dave_false;
	}

	insert = so_bson_new();

	if(insert == NULL)
	{
		PARTYABNOR("bcon new empty! <%s:%d>", fun, line);
		return dave_false;
	}

	_mongoc_build_oid(obj, obj_str);

	_mongoc_inside_oid(insert, obj, obj_str);

	_mongoc_inside_time(insert);

	so_bson_append_binary(insert, MONGOC_DEFAULT_VALUE_NAME, strlen(MONGOC_DEFAULT_VALUE_NAME), BSON_SUBTYPE_BINARY, value, value_len);

	if(so_mongoc_collection_insert(pCollection, MONGOC_INSERT_NONE, insert, NULL, &error))
	{
		MONGOCSHOWBSON("add key value", insert);

		ret = dave_true;
	}
	else
	{
		PARTYABNOR("error:%s <%s:%d>", error.message, fun, line);

		ret = dave_false;
	}

	so_bson_destroy(insert);

	return ret;
}

dave_bool
__dave_mongoc_upd_bin__(void *pCollection, s8 *obj_str, u8 *value, ub value_len, s8 *fun, ub line)
{
	u8 *obj = NULL;
	bson_t *query, *update;
	bson_error_t error;
	dave_bool ret = dave_false;

	PARTYDEBUG("%s:%d", fun, line);

	if(pCollection == NULL)
	{
		PARTYABNOR("upd pCollection is NULL!");
		return dave_false;
	}

	if((obj_str == NULL) || (obj_str[0] == '\0'))
	{
		PARTYABNOR("invalid obj_str:%x", obj_str);
		return dave_false;
	}

	query = so_bson_new();
	if(query == NULL)
	{
		PARTYABNOR("bcon new empty!");
		return dave_false;
	}

	update = so_bson_new();
	if(update == NULL)
	{
		so_bson_destroy(query);
		PARTYABNOR("bcon new empty!");
		return dave_false;
	}

	if(_mongoc_inside_oid(query, obj, obj_str) == dave_true)
	{
		_mongoc_inside_time(update);

		so_bson_append_binary(update, MONGOC_DEFAULT_VALUE_NAME, strlen(MONGOC_DEFAULT_VALUE_NAME), BSON_SUBTYPE_BINARY, value, value_len);

		if(so_mongoc_collection_update(pCollection, MONGOC_UPDATE_NONE, query, update, NULL, &error))
		{
			MONGOCSHOWBSON("update key value", query);

			ret = dave_true;
		}
		else
		{
			PARTYABNOR("error:%s", error.message);
		}		
	}

	so_bson_destroy(query);
	so_bson_destroy(update);

	return ret;
}

dave_bool
__dave_mongoc_det_bin__(void *pCollection, s8 *obj_str, s8 *fun, ub line)
{
	u8 *obj = NULL; 
	bson_t *query;
	dave_bool ret;

	PARTYDEBUG("%s:%d", fun, line);

	if(pCollection == NULL)
	{
		PARTYABNOR("inq pCollection is NULL!");
		return dave_false;
	}

	if((obj_str == NULL) || (obj_str[0] == '\0'))
	{
		return dave_false;
	}

	query = so_bson_new();

	if(_mongoc_inside_oid(query, obj, obj_str) == dave_true)
	{
		ret = _mongoc_database_query(pCollection, query, NULL, 0, dave_true);
	}
	else
	{
		PARTYABNOR("append key failed!");

		ret = dave_false;
	}

	so_bson_destroy(query);

	return ret;
}

dave_bool
__dave_mongoc_inq_bin__(void *pCollection, s8 *obj_str, u8 *value, ub *value_len, s8 *fun, ub line)
{
	u8 *obj = NULL;
	bson_t *query;
	dave_bool ret;

	PARTYDEBUG("%s:%d", fun, line);

	if(pCollection == NULL)
	{
		PARTYABNOR("inq pCollection is NULL!");
		return dave_false;
	}

	if((obj_str == NULL) || (obj_str[0] == '\0'))
	{
		return dave_false;
	}

	if(value != NULL)
	{
		value[0] = '\0';
	}
	else
	{
		// if value is NULL, get value len.

		*value_len = 1024 * 1024 * 32;
	}

	query = so_bson_new();

	if(_mongoc_inside_oid(query, obj, obj_str) == dave_true)
	{
		ret = _mongoc_database_query(pCollection, query, value, value_len, dave_false);
	}
	else
	{
		PARTYABNOR("append key failed!");

		ret = dave_false;
	}

	so_bson_destroy(query);

	return ret;
}

dave_bool
__dave_mongoc_add_json__(void *pCollection, s8 *pJson_string, s8 *fun, ub line)
{
	bson_t *insert;
	bson_error_t error;
	dave_bool ret;
	
	if(pCollection == NULL)
	{
		PARTYABNOR("add pCollection is NULL!");
		return dave_false;
	}
	if(NULL == pJson_string)
	{
		PARTYABNOR("add object is NULL!");
		return dave_false;
	}
	
	insert = so_bson_new_from_json((const uint8_t *)pJson_string, -1, &error);
	if (NULL == insert)
	{
		PARTYABNOR("bson_new_from_json fail:%s <%s:%d>", error.message, fun, line);
		return dave_false;
	}

	_mongoc_inside_time(insert);

	if(so_mongoc_collection_insert_one(pCollection, insert, NULL, NULL,  &error))
	{
		MONGOCSHOWBSON("add key value", insert);

		ret = dave_true;
	}
	else
	{
		PARTYABNOR("error:%s <%s:%d>", error.message, fun, line);

		ret = dave_false;
	}

	so_bson_destroy(insert);

	return ret;
}

dave_bool
__dave_mongoc_upd_json__(void *pCollection, s8 *key, s8 *src_value, void *object, s8 *fun, ub line)
{
	bson_t *query = NULL;
	bson_t *update = NULL;
	bson_error_t error;
	void *father_object = NULL;
	dave_bool ret = dave_false;
	s8 *string_json;
	ub json_len;

	if(pCollection == NULL)
	{
		PARTYABNOR("update pCollection is NULL!");
		return dave_false;
	}
	
	query = so_bson_new();

	so_bson_append_utf8(query, key, strlen(key), src_value, strlen(src_value));

	if (NULL == query)
	{
		PARTYABNOR("update inside oid is NULL!");
		ret = dave_false;
		goto error_update;
	}
	
	if (NULL == object)
	{
		PARTYABNOR("add object is NULL!");
		ret = dave_false;
		goto error_update;
	}

	father_object = dave_json_malloc();

	dave_json_add_object(father_object, "$set", object);
		
	string_json = dave_json_to_string(father_object, &json_len);
	if (NULL == string_json)
	{
		PARTYABNOR("Invalid object!");
		ret = dave_false;
		goto error_update;
	}

	update = so_bson_new_from_json((const uint8_t *)string_json, -1, &error);
	if (NULL == update)
	{
		PARTYABNOR("bson_new_from_json fail:%s", error.message);
		ret = dave_false;
		goto error_update;
	}

	if(so_mongoc_collection_update_one(pCollection, query, update, NULL, NULL,  &error))
	{
		MONGOCSHOWBSON("update key value", update);

		ret = dave_true;
	}
	else
	{
		PARTYABNOR("error:%s, %s", error.message, string_json);

		ret = dave_false;
	}

error_update:

	if (NULL != query)
	{
		so_bson_destroy(query);
	}
	
	if (NULL != update)
	{
		so_bson_destroy(update);
	}
	if (NULL != father_object)
	{
		dave_json_free(father_object);
	}

	return ret;
}

#endif

