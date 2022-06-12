/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __3RDPARTY_MACRO_H__
#define __3RDPARTY_MACRO_H__

#ifdef __DAVE_LINUX__
#define JEMALLOC_3RDPARTY
#endif
#define JSON_3RDPARTY
#ifdef __DAVE_LINUX__
#define COROUTINE_3RDPARTY
#endif
#define NGINX_3RDPARTY
#define MONGO_3RDPARTY
#ifdef __DAVE_PRODUCT_AIB__
#define FASTDFS_3RDPARTY
#endif
#ifdef __DAVE_PRODUCT_AIB__
#define ICONV_3RDPARTY
#endif
#ifdef __DAVE_PRODUCT_AIX__
#define OPENCV_3RDPARTY
#endif
#ifdef __DAVE_PRODUCT_AIX__
#define TENSORFLOW_3RDPARTY
#endif

#endif

