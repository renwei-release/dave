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
#define THREAD_MAX (256)
#define THREAD_THREAD_MAX (1024)
#define THREAD_MSG_QUEUE_NUM (128)
#define THREAD_SEQ_QUEUE_NUM (512)
#define THREAD_THREAD_QUEUE_NUM (1)

#define THREAD_WAKEUP_INTERVAL 1

/*
 * 这个宏开关用于提醒本地线程一个远端线程上下线的状态。
 */
#define ENABLE_THREAD_REMOTE_NOTIFY

#ifdef LEVEL_PRODUCT_alpha
#define ENABLE_THREAD_STATISTICS
#endif

#endif
