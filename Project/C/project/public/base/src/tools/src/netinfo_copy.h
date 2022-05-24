/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __NETINFO_COPY_H__
#define __NETINFO_COPY_H__
#include "base_define.h"

void __T_CopyNetInfo__(SocNetInfo *dst, SocNetInfo *src, s8 *file, ub line);
#define T_CopyNetInfo(dst, src) __T_CopyNetInfo__(dst, src, (s8 *)__func__, (ub)__LINE__)

void T_NetToIPInfo(IPBaseInfo *pIPInfo, SocNetInfo *pSocInfo);

#endif

