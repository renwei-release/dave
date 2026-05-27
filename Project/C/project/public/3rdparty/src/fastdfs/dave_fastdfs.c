/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(FASTDFS_3RDPARTY)
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "fdfs_client.h"
#include "dave_os.h"
#include "dave_tools.h"
#include "party_log.h"

#define SNPRINTF(fmt, value) config_index += dave_snprintf(&config_buffer[config_index], config_length-config_index, fmt"\n", value)

static void
_fastdfs_load_config(s8 *config_buffer, ub config_length, s8 *host_domain, s8 *host_port)
{
	s8 tracker_server[1024];
	ub config_index;

	config_index = 0;
	config_length -= 1;

	// connect timeout in seconds
	// default value is 30s
	SNPRINTF("connect_timeout=%d", 30);

	// network timeout in seconds
	// default value is 60s
	SNPRINTF("network_timeout=%d", 60);

	// the base path to store log files
	SNPRINTF("base_path=%s", dave_os_file_home_dir());

	// tracker_server can ocur more than once, and tracker_server format is
	//  "host:port", host can be hostname or ip address
	dave_snprintf(tracker_server, sizeof(tracker_server), "%s:%s", host_domain, host_port);
	SNPRINTF("tracker_server=%s", tracker_server);

	//standard log level as syslog, case insensitive, value list:
	//## emerg for emergency
	//## alert
	//## crit for critical
	//## error
	//## warn for warning
	//## notice
	//## info
	SNPRINTF("debuglog_level=%s", "emerg");

	// if use connection pool
	// default value is false
	// since V4.05
	SNPRINTF("use_connection_pool=%s", "false");

	// connections whose the idle time exceeds this time will be closed
	// unit: second
	// default value is 3600
	// since V4.05
	SNPRINTF("connection_pool_max_idle_time=%d", 3600);

	// if load FastDFS parameters from tracker server
	// since V4.05
	// default value is false
	SNPRINTF("load_fdfs_parameters_from_tracker=%s", "false");

	// if use storage ID instead of IP address
	// same as tracker.conf
	// valid only when load_fdfs_parameters_from_tracker is false
	// default value is false
	// since V4.05
	SNPRINTF("use_storage_id = %s", "false");

	// specify storage ids filename, can use relative or absolute path
	// same as tracker.conf
	// valid only when load_fdfs_parameters_from_tracker is false
	// since V4.05
	SNPRINTF("storage_ids_filename = %s", "storage_ids.conf");

	//HTTP settings
	SNPRINTF("http.tracker_server_port=%d", 80);

	if(config_index >= config_length)
	{
		PARTYABNOR("The prepared cache is too small!");
	}
	else
	{
		config_buffer[config_index] = '\0';
	}
}

static void
_fastdfs_init(s8 *host_domain, s8 *host_port)
{
	s8 config_buffer[4096];

	_fastdfs_load_config(config_buffer, sizeof(config_buffer), host_domain, host_port);

	log_init();
	g_log_context.log_level = LOG_ERR;

	if(fdfs_client_init_from_buffer((const char *)config_buffer) != 0)
	{
		return;
	}
}

static void
_fastdfs_exit(void)
{
	fdfs_client_destroy();
}

static MBUF *
_fastdfs_download_to_mbuf(char *file_buff, int64_t file_size, dave_bool download_to_base64)
{
	MBUF *download;

	if(download_to_base64 == dave_true)
	{
		download = dave_mmalloc(file_size * 4);
		download->tot_len = download->len = t_crypto_base64_encode((const u8 *)file_buff, (ub)file_size, (s8 *)(download->payload), download->len);
	}
	else
	{
		download = dave_mmalloc(file_size + 1);
		dave_memcpy(download->payload, file_buff, file_size);
		((s8 *)(download->payload))[file_size] = '\0';
	}

	return download;
}

// =====================================================================

void
dave_fastdfs_init(s8 *host_domain, s8 *host_port)
{
	PARTYLOG("%s:%s", host_domain, host_port);

	_fastdfs_init(host_domain, host_port);
}

void
dave_fastdfs_exit(void)
{
	_fastdfs_exit();
}

dave_bool
dave_fastdfs_upload(s8 *group, s8 *image_path, s8 *image_bin, ub image_length, s8 *file_ext_name, s8 *file_id)
{
	ConnectionInfo trackerServer;
	ConnectionInfo storageServer;
	ConnectionInfo *pTrackerServer = &trackerServer;
	int store_path_index;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	int result;
	dave_bool ret;

	file_id[0] = '\0';

	pTrackerServer = tracker_get_connection_r(pTrackerServer, &result);
	if(pTrackerServer == NULL)
	{
		PARTYABNOR("invalid server:%s group:%s!", STRERROR(result), group);
		return dave_false;
	}

	dave_snprintf((s8 *)group_name, sizeof(group_name), "%s", group);
	result = tracker_query_storage_store(pTrackerServer, &storageServer, group_name, &store_path_index);
	if(result != 0)
	{
		PARTYABNOR("tracker_query_storage fail, error no:%d, error info:%s", result, STRERROR(result));
		ret = dave_false;
	}
	else
	{
		if(image_path != NULL)
		{
			result = storage_upload_by_filename1(pTrackerServer, &storageServer, store_path_index, (const char *)image_path, (const char *)file_ext_name, NULL, 0, group_name, (char *)file_id);
		}
		else if((image_bin != NULL) && (image_length != 0))
		{
			result = storage_upload_by_filebuff1(pTrackerServer, &storageServer, store_path_index, (const char *)image_bin, (const int64_t)image_length, (const char *)file_ext_name, NULL, 0, group_name, (char *)file_id);
		}
		else
		{
			result = 0;
		}
		if(result != 0)
		{
			PARTYABNOR("tracker_query_storage fail, error no:%d, error info:%s", result, STRERROR(result));
			ret = dave_false;
		}
		else
		{
			ret = dave_true;
		}
	}

	tracker_disconnect_server_ex(pTrackerServer, true);

	return ret;
}

MBUF *
dave_fastdfs_download(s8 *group, s8 *file_id, dave_bool download_to_base64)
{
	ConnectionInfo trackerServer;
	ConnectionInfo storageServer;
	ConnectionInfo *pTrackerServer = &trackerServer;
	int store_path_index;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	int result;
	char *file_buff;
	int64_t file_size;
	MBUF *download = NULL;

	pTrackerServer = tracker_get_connection_r(pTrackerServer, &result);
	if(pTrackerServer == NULL)
	{
		PARTYABNOR("invalid server:%s!", STRERROR(result));
		return NULL;
	}

	dave_snprintf((s8 *)group_name, sizeof(group_name), "%s", group);
	result = tracker_query_storage_store(pTrackerServer, &storageServer, group_name, &store_path_index);
	if(result != 0)
	{
		PARTYABNOR("tracker_query_storage fail, error no:%d, error info:%s", result, STRERROR(result));
	}
	else
	{
		file_buff = NULL;

		if(storage_download_file_to_buff1(pTrackerServer, &storageServer, (const char *)file_id, &file_buff, &file_size) == 0)
		{
			download = _fastdfs_download_to_mbuf(file_buff, file_size, download_to_base64);
		}
		else
		{
			PARTYABNOR("error info:%s", STRERROR(22));
		}

		if(file_buff != NULL)
		{
			free(file_buff);
		}
	}

	tracker_disconnect_server_ex(pTrackerServer, true);

	return download;
}

#endif

