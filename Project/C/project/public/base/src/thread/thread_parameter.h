/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THREAD_PARAMETER_H__
#define __THREAD_PARAMETER_H__

#define THREAD_NAME_MAX (DAVE_THREAD_NAME_LEN)
// the max is 1024, please see LOCAL_MAX_VALUE define.
#define THREAD_MAX (64)
#define THREAD_THREAD_MAX (1024)
#define THREAD_MSG_QUEUE_NUM (64)
#define THREAD_SEQ_QUEUE_NUM (32)
#define THREAD_THREAD_QUEUE_NUM (1)
#define THREAD_PRE_QUEUE_NUM (1)
// 对于某个服务，这个参数决定了系统最大支持的服务的数量
#define THREAD_CLIENT_MAX (128)

#define THREAD_WAKEUP_INTERVAL 1

/*
 * 这个宏开关用于提醒本地线程一个远端线程上下线的状态。
 */
#define ENABLE_THREAD_REMOTE_NOTIFY

#ifdef LEVEL_PRODUCT_alpha
#define ENABLE_THREAD_STATISTICS
#endif
#if defined(__DAVE_LINUX__) || defined(__DAVE_CYGWIN__)
#define ENABLE_THREAD_COROUTINE
#endif

/*
 * 这个宏开关目前主要是为了能在函数：base_thread_creat
 * 初始化线程的过程中，能在base_thread_creat的初始化函数
 * 里面可直接使用协程能力。
 * 但，目前经过调试，内部状态还没处理好，
 * 通过valgrind/memcheck.sh base测试发现有太多未初始化空间被使用了。
 */
// #define ENABLE_MSG_INIT

#endif

