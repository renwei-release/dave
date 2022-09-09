/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#if defined(__DAVE_CYGWIN__) || defined(__DAVE_LINUX__)
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "os_log.h"

typedef struct {
	DIR *pDir;
	s8 path[2048];
} LinuxDir;

static const char *
_linux_file_home_dir(void)
{
	static s8 file_dir[64];

	dave_snprintf(file_dir, sizeof(file_dir), "/dave/%s/", dave_verno_my_product());

	t_stdio_tolowers(file_dir);

#if defined(__DAVE_IOS__)
    extern const char * getHomePath(void);
    static char home_dir[256];

    dave_snprintf((s8 *)home_dir, sizeof(home_dir), "%s/Library/%s", getHomePath(), file_dir);
    
    return home_dir;
#elif defined(__DAVE_MAC__)
    static char home_dir[256];

    dave_snprintf((s8 *)home_dir, sizeof(home_dir), ".%s", file_dir);
    
    return home_dir;
#elif defined(__DAVE_ANDROID__)
	static char home_dir[256];

	dave_snprintf((s8 *)home_dir, sizeof(home_dir), "/sdcard%s", file_dir);

	return home_dir;
#else
    return (const char *)file_dir;
#endif
}

static void
_linux_file_load_full_name(s8 *full_name, ub full_len, FileOptFlag flag, s8 *file_name)
{
	if((flag & DIRECT_FLAG) == DIRECT_FLAG)
	{
		dave_snprintf(full_name, full_len, "%s", file_name);
	}
	else
	{
		dave_snprintf(full_name, full_len, "%s%s", _linux_file_home_dir(), file_name);
	}
}

static void
_linux_file_creat_dir(const s8 *dir)
{
	s8 edit_buf[2048];
    ub index, file_name_len;

	dave_strcpy(edit_buf, dir, sizeof(edit_buf));

    file_name_len = dave_strlen(dir);

    for(index=1; index<file_name_len; index++)
    {
        if(edit_buf[index] == '/')
        {
            edit_buf[index] = '\0';
            
            if(access((char *)edit_buf, F_OK) == -1)
            {
                mkdir((char *)edit_buf, 0777);
            }
			
            edit_buf[index] = '/';
        }
    }
}

static sb
_linux_file_len_on_file_id(sb file_id)
{
    struct stat file_info;
    
    if(fstat((int)file_id, &file_info) == -1)
    {
        OSABNOR("stat file fail,error(%d):%s", errno, strerror(errno));
		return 0;
    }
    
    return (sb)file_info.st_size;
}

static ub
_linux_dir_file_number(DIR *pDir, s8 *file_path)
{
	char file_full_path[1024];
	struct dirent *rent;
	struct stat f_ftime;
	ub file_number;

	seekdir(pDir, 0);

	file_number = 0;

	for(rent=readdir(pDir); rent!=NULL; rent=readdir(pDir))
	{
		if((dave_strcmp(rent->d_name, ".") == dave_true)
			|| (dave_strcmp(rent->d_name, "..") == dave_true))
		{
			continue;
		}

		dave_snprintf((s8 *)file_full_path, sizeof(file_full_path), "%s/%s", file_path, rent->d_name);

		if(stat(file_full_path, &f_ftime) != 0)
		{
			OSLOG("file:%s %d/%s", file_full_path, errno, strerror(errno));
			break;
		}

		if(S_ISDIR(f_ftime.st_mode))
		{
			continue;
		}

		file_number ++;
	}

	return file_number;
}

// =====================================================================

sb
dave_os_file_len(FileOptFlag flag, s8 *file_name, sb file_id)
{
	sb file_len;

	if(file_name != NULL)
	{
		file_id = dave_os_file_open(flag, file_name);
	}

	if(file_id >= 0)
	{
		file_len = _linux_file_len_on_file_id(file_id);
	}
	else
	{
		file_len = 0;
	}

	if(file_name != NULL)
	{
		dave_os_file_close(file_id);
	}

	return file_len;
}

sb
dave_os_file_open(FileOptFlag flag, s8 *file_name)
{
    sw_int32 oflag;
    s8 file_full_name[512];
	sb file_id;

	_linux_file_load_full_name(file_full_name, sizeof(file_full_name), flag, file_name);

    oflag = O_RDWR;
    if((flag & CREAT_FLAG) == CREAT_FLAG)
    {
		_linux_file_creat_dir(file_full_name);
        oflag |= O_CREAT;
    }

    file_id = (sb)open((char *)file_full_name, oflag, 0777);
	if(file_id < 0)
	{
		OSDEBUG("flag:%x %s->%s open failed!", flag, file_name, file_full_name);		
	}

	return file_id;
}

dave_bool
dave_os_file_delete(FileOptFlag flag, s8 *file_name)
{
    sw_int32 file_id;
    sw_int32 oflag;
    sw_int8 file_full_name[200];
    
	_linux_file_load_full_name(file_full_name, sizeof(file_full_name), flag, file_name);

    _linux_file_creat_dir(file_full_name);

    oflag = O_RDWR;
    file_id = open((char *)file_full_name, oflag, 0777);
    if(file_id < 0)
    {
        return dave_true;
    }

	close(file_id);

    if(remove((char *)file_full_name) == -1)
    {
		OSABNOR("delete file fail,error(%d):%s", errno, strerror(errno));
		return dave_false;
    }
	else
	{
		return dave_true;
	}
}

sb
dave_os_file_load(sb file_id, ub pos, ub data_len, u8 *data)
{
    off_t offset;
    ssize_t read_len;
    
    offset = lseek((int)file_id, pos, SEEK_SET);
    if(offset < 0)
    {
		OSABNOR("lseek file fail,error(%d):%s", errno, strerror(errno));
		return 0;
    }

    read_len = read((int)file_id, data, data_len);
    if(read_len < 0)
    {
		OSABNOR("read file fail,error(%d):%s", errno, strerror(errno));
	}
    
    return (sb)read_len;
}

sb
dave_os_file_save(sb file_id, ub pos, ub data_len, u8 *data)
{
    off_t offset;
    ssize_t write_len;
    
    offset = lseek((int)file_id, pos, SEEK_SET);
    if(offset == -1)
    {
        OSABNOR("lseek file fail,error(%d):%s", errno, strerror(errno));
        return (sb)offset;
    }
    
    write_len = write((int)file_id, data, data_len);
    
    return (sb)write_len;
}

dave_bool
dave_os_file_close(sb file_id)
{
	if(file_id >= 0)
    	close((int)file_id);
    return dave_true;
}

dave_bool
dave_os_file_valid(s8 *file_name)
{
	FileOptFlag flag = READ_FLAG | DIRECT_FLAG;
	sb file_id;

	file_id = dave_os_file_open(flag, file_name);
	if(file_id < 0)
		return dave_false;

	dave_os_file_close(file_id);

	return dave_true;
}

ub
dave_os_file_read(FileOptFlag flag, s8 *file_name, ub file_index, ub data_len, u8 *data)
{
	sb file_id, read_len;

	file_id = dave_os_file_open(flag, file_name);
	if(file_id < 0)
	{
		return 0;
	}

	read_len = dave_os_file_load(file_id, file_index, data_len, data);

	dave_os_file_close(file_id);

	return read_len;
}

dave_bool
dave_os_file_write(FileOptFlag flag, s8 *file_name, ub file_index, ub data_len, u8 *data)
{
	sb file_id, write_len;

	file_id = dave_os_file_open(flag, file_name);
	if(file_id < 0)
	{
		OSABNOR("flag:%x file_name:%s can't open!", flag, file_name);
		return dave_false;
	}

	write_len = dave_os_file_save(file_id, file_index, data_len, data);

	dave_os_file_close(file_id);

	if((ub)write_len != data_len)
	{
		OSABNOR("flag:%x file_name:%s file_index:%d data_len:%d save failed!",
			flag, file_name, file_index, data_len);
		return dave_false;
	}

	return dave_true;
}

void *
dave_os_dir_open(s8 *dir_path, ub *file_number)
{
	LinuxDir *pLinuxDir = NULL;
	DIR *pDir;

    pDir = opendir((const char *)dir_path);
	if(pDir != NULL)
	{
		if(file_number != NULL)
		{
			*file_number = _linux_dir_file_number(pDir, dir_path);
		}

		seekdir(pDir, 0);

		pLinuxDir = dave_malloc(sizeof(LinuxDir));

		pLinuxDir->pDir = pDir;
		dave_strcpy(pLinuxDir->path, dir_path, sizeof(pLinuxDir->path));
	}

	return pLinuxDir;
}

dave_bool
dave_os_dir_valid(s8 *dir_path)
{
	DIR *pDir;
	dave_bool result;

	if(dir_path == NULL)
		return dave_false;

    pDir = opendir((const char *)dir_path);
	if(pDir != NULL)
	{
		result = dave_true;
		closedir(pDir);
	}
	else
	{
		result = dave_false;
	}

	return result;
}

s8 *
dave_os_dir_read(void *pDir, s8 *name_file, ub name_length)
{
	LinuxDir *pLinuxDir = (LinuxDir *)pDir;
	struct dirent *rent;
	struct stat f_ftime;
	ub safe_counter;

	safe_counter = 0;

	while((++ safe_counter) < 102400)
	{
		rent = readdir(pLinuxDir->pDir);
		if(rent != NULL)
		{
			if((dave_strcmp(rent->d_name, ".") == dave_true)
				|| (dave_strcmp(rent->d_name, "..") == dave_true))
			{
				continue;
			}

			dave_snprintf(name_file, name_length, "%s/%s", pLinuxDir->path, rent->d_name);

			if(stat((const char *)name_file, &f_ftime) != 0)
			{
				break;
			}

			if(S_ISDIR(f_ftime.st_mode))
			{
				continue;
			}

			return name_file;
		}
	}

	return NULL;
}

void
dave_os_dir_close(void *pDir)
{
	LinuxDir *pLinuxDir = (LinuxDir *)pDir;

	if(pLinuxDir != NULL)
	{
		if(pLinuxDir->pDir != NULL)
		{
			closedir((DIR *)(pLinuxDir->pDir));
		}

		dave_free(pLinuxDir);
	}
}

MBUF *
dave_os_dir_subdir_list(s8 *dir_path)
{
	LinuxDir *pLinuxDir;
	ub safe_counter = 0;
	struct dirent *rent;
	struct stat f_ftime;
	s8 file_name[1024];
	MBUF *pList = NULL, *pSubDirName;
	ub d_name_len;

	if(dave_os_dir_valid(dir_path) == dave_false)
		return pList;

	pLinuxDir = dave_os_dir_open(dir_path, NULL);

	while((++ safe_counter) < 102400)
	{
		rent = readdir(pLinuxDir->pDir);
		if(rent != NULL)
		{
			if((dave_strcmp(rent->d_name, ".") == dave_true)
				|| (dave_strcmp(rent->d_name, "..") == dave_true))
			{
				continue;
			}

			dave_snprintf(file_name, sizeof(file_name), "%s/%s", pLinuxDir->path, rent->d_name);

			if(stat((const char *)file_name, &f_ftime) != 0)
			{
				break;
			}

			if(S_ISDIR(f_ftime.st_mode))
			{
				d_name_len = dave_strlen(rent->d_name) + 1;
				pSubDirName = dave_mmalloc(d_name_len);
				dave_strcpy(dave_mptr(pSubDirName), rent->d_name, d_name_len);
				pList = dave_mchain(pList, pSubDirName);
			}
		}
	}

	dave_os_dir_close(pLinuxDir);

	return pList;
}

s8 *
dave_os_file_home_dir(void)
{
	return (s8 *)_linux_file_home_dir();
}

void
dave_os_file_creat_dir(s8 *dir)
{
	_linux_file_creat_dir(dir);
}

MBUF *
dave_os_file_read_mbuf(s8 *file_path)
{
	sb file_len;
	MBUF *file_data;

	file_len = dave_os_file_len(READ_FLAG|DIRECT_FLAG, file_path, 0);
	if(file_len <= 0)
	{
		return NULL;
	}

	file_data = dave_mmalloc(file_len);

	file_data->len = file_data->tot_len = dave_os_file_read((FileOptFlag)(READ_FLAG|DIRECT_FLAG), file_path, 0, (ub)file_len, (u8 *)(file_data->payload));

	if(file_data->len == 0)
	{
		dave_mfree(file_data);
		file_data = NULL;
	}

	return file_data;
}

#endif

