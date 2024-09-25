/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OS_FILE_H__
#define __DAVE_OS_FILE_H__

sb dave_os_file_len(FileOptFlag flag, s8 *file_name, sb file_id);

sb dave_os_file_open(FileOptFlag flag, s8 *file_name);

ub dave_os_file_read(FileOptFlag flag, s8 *file_name, ub file_index, ub data_len, u8 *data_ptr);

dave_bool dave_os_file_write(FileOptFlag flag, s8 *file_name, ub file_index, ub data_len, u8 *data_ptr);

sb dave_os_file_load(sb file_id, ub pos, ub data_len, u8 *data_ptr);

sb dave_os_file_save(sb file_id, ub pos, ub data_len, u8 *data_ptr);

dave_bool dave_os_file_close(sb file_id);

dave_bool dave_os_file_valid(s8 *file_name);

dave_bool dave_os_file_delete(FileOptFlag flag, s8 * file_name);

void * dave_os_dir_open(s8 *dir_path, ub *file_number);

dave_bool dave_os_dir_valid(s8 *dir_path);

s8 * dave_os_dir_read(void *pDir, s8 *name_file, ub name_length);

void dave_os_dir_close(void *pDir);

MBUF * dave_os_dir_subdir_list(s8 *dir_path);

MBUF * dave_os_dir_subfile_list(s8 *dir_path);

s8 * dave_os_file_home_dir(void);

void dave_os_file_creat_dir(s8 *dir);

dave_bool dave_os_file_remove_dir(s8 *dir);

MBUF * dave_os_file_read_mbuf(s8 *file_path);

#endif


