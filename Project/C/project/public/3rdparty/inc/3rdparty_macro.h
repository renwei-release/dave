/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __3RDPARTY_MACRO_H__
#define __3RDPARTY_MACRO_H__

#if defined(__DAVE_LINUX__)
#define JEMALLOC_3RDPARTY
#endif

#define JSON_3RDPARTY

#if defined(__DAVE_LINUX__) && defined(FORM_PRODUCT_BIN)
#define GTEST_3RDPARTY
#endif

#define NGINX_3RDPARTY

#if defined(__DAVE_PRODUCT_STORE__) || defined(__DAVE_PRODUCT_DBA__)
#define MONGO_3RDPARTY
#endif

#if defined(__DAVE_PRODUCT_STORE__) || defined(__DAVE_PRODUCT_DBA__)
#define MYSQL_3RDPARTY
#endif

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

#ifdef __DAVE_PRODUCT_SYNC__
#define ETCD_3RDPARTY
#endif

#ifdef __DAVE_PRODUCT_VOIP__
// #define PJSIP_3RDPARTY
#endif

#ifdef __DAVE_PRODUCT_IO__
#define CURL_3RDPARTY
#endif

#ifdef __DAVE_PRODUCT_IO__
#define QUICKMAIL_3RDPARTY
#endif

#define REDIS_3RDPARTY

#endif

