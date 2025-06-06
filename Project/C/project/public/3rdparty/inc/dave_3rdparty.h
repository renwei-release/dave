/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_3RDPARTY_H__
#define __DAVE_3RDPARTY_H__
#include "dave_base.h"
#include "3rdparty_macro.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifdef JEMALLOC_3RDPARTY
#include "dave_jemalloc.h"
#endif
#ifdef JSON_3RDPARTY
#include "dave_json.h"
#endif
#ifdef GTEST_3RDPARTY
#include "dave_gtest.h"
#endif
#ifdef NGINX_3RDPARTY
#include "dave_nginx.h"
#endif
#ifdef MONGO_3RDPARTY
#include "dave_mongoc.h"
#endif
#ifdef MYSQL_3RDPARTY
#include "dave_mysql.h"
#endif
#ifdef FASTDFS_3RDPARTY
#include "dave_fastdfs.h"
#endif
#ifdef ICONV_3RDPARTY
#include "dave_libiconv.h"
#endif
#ifdef OPENCV_3RDPARTY
#include "dave_opencv.h"
#endif
#ifdef TENSORFLOW_3RDPARTY
#include "dave_tensorflow.h"
#endif
#ifdef ETCD_3RDPARTY
#include "dave_etcd.h"
#endif
#ifdef PJSIP_3RDPARTY
#include "dave_pjsip.h"
#endif
#ifdef REDIS_3RDPARTY
#include "dave_redis.h"
#endif
#ifdef CURL_3RDPARTY
#include "dave_curl.h"
#endif
#ifdef QUICKMAIL_3RDPARTY
#include "dave_quickmail.h"
#endif
#ifdef WEBSOCKET_3RDPARTY
#include "dave_websocket.h"
#endif
#ifdef WEBRTC_3RDPARTY
#include "dave_webrtc.h"
#endif

#ifdef __cplusplus
}
#endif

#endif

