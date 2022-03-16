/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.09.28.
 * ================================================================================
 */

#ifndef __NETINFO_COPY_H__
#define __NETINFO_COPY_H__
#include "base_define.h"

void __T_CopyNetInfo__(SocNetInfo *dst, SocNetInfo *src, s8 *file, ub line);
#define T_CopyNetInfo(dst, src) __T_CopyNetInfo__(dst, src, (s8 *)__func__, (ub)__LINE__)

void T_NetToIPInfo(IPBaseInfo *pIPInfo, SocNetInfo *pSocInfo);

#endif

