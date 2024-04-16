/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __EMAIL_SEND_H__
#define __EMAIL_SEND_H__
#include "http_param.h"
#include "http_msg.h"

RetCode email_send(s8 *subject, s8 *context, s8 *attachment);

#endif

