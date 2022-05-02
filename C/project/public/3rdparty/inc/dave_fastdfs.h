/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_FASTDFS_H__
#define __DAVE_FASTDFS_H__

void dave_fastdfs_init(s8 *host_domain, s8 *host_port);

void dave_fastdfs_exit(void);

dave_bool dave_fastdfs_upload(s8 *group, s8 *image_path, s8 *image_bin, ub image_length, s8 *file_ext_name, s8 *file_id);

MBUF * dave_fastdfs_download(s8 *group, s8 *file_id, dave_bool download_to_base64);

#endif

