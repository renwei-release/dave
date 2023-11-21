/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_PARAMETERS_H__
#define __BASE_PARAMETERS_H__

#define DAVE_THREAD_NAME_LEN (32)

#define DAVE_GLOBALLY_IDENTIFIER_LEN (33)

#define DAVE_VERNO_STR_LEN (64)

#define DAVE_NORMAL_NAME_LEN (128)

#define DAVE_URL_LEN (128)

#define DAVE_MAC_ADDR_LEN (6)

#define DAVE_IP_V4_ADDR_LEN (4)

#define DAVE_IP_V6_ADDR_LEN (16)

#ifdef __DAVE_LINUX__
 #define DAVE_SERVER_SUPPORT_SOCKET_MAX (40960)
#elif defined(__DAVE_CYGWIN__)
#define DAVE_SERVER_SUPPORT_SOCKET_MAX (1024)
#endif

#define DAVE_BUILDING_BLOCKS_MAX (255)

#define DAVE_HTTP_CONTENT_LEN (16*1024*1024)

#define DAVE_USER_NAME_LEN (128)

#define DAVE_PASSWORD_LEN (64)

#define DAVE_UUID_LEN (64)

#define DAVE_AUTH_KEY_LEN (64)

#define DAVE_AUTH_KEY_STR_LEN (DAVE_AUTH_KEY_LEN * 2 + 1)

#ifdef __DAVE_LINUX__
#define DAVE_THREAD_EMPTY_VALUE -1
#elif defined(__DAVE_CYGWIN__)
#define DAVE_THREAD_EMPTY_VALUE NULL
#endif

#define DAVE_SYS_THREAD_ID_MAX 204800

#define DAVE_UID_LEN (64)

#define DAVE_NORMAL_STR_LEN (2048)

#ifndef DAVE_SEARCH_INDEX_MAX
#define DAVE_SEARCH_INDEX_MAX 4096
#endif

#endif

