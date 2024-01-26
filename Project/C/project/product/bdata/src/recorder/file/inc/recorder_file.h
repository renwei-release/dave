/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __RECORDER_FILE_H__
#define __RECORDER_FILE_H__

void recorder_file_init(void);

void recorder_file_exit(void);

void * recorder_file_open(s8 *file_dir);

void recorder_file_close(void *ptr);

void * recorder_file_json(void *ptr);

dave_bool recorder_file_str(void *ptr, char *key_name, s8 *str_data, ub str_len);

dave_bool recorder_file_bin(void *ptr, char *key_name, u8 *bin_data, ub bin_len);

dave_bool recorder_file_obj(void *ptr, char *key_name, s8 *str_data, ub str_len);

ub recorder_file_info(s8 *info_ptr, ub info_len);

#endif

