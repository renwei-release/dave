/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 *
 * ############################# IMPORTANT INFORMATION ############################
 * The code of this file is automatically generated by tools(Tools/refresh_version),
 * please do not modify it manually!
 * ############################# IMPORTANT INFORMATION ############################
 * ================================================================================
 */

#ifndef __DAVE_VERNO_H__
#define __DAVE_VERNO_H__
#include "verno_macro.h"
#include "dave_base.h"

#define VERSION_PRODUCT "BASE"

#define VERSION_MISC "linux"

#define VERSION_MAIN "4"
#if defined(__VERNO_ALPHA_VERSION__)
 #define VERSION_SUB "5"
#else
 #define VERSION_SUB "6"
#endif
#define VERSION_REV "1"

#define VERSION_DATE_TIME "20220408095919"

#ifdef __VERNO_ALPHA_VERSION__
 #define VERSION_LEVEL "Alpha"
#elif defined(__VERNO_BETA_VERSION__)
 #define VERSION_LEVEL "Beta"
#endif

#define __BUILD_MAC_ADDRESS__ "000C2930311F"
#define __BUILD_HOSTNAME__ "dave"
#define __BUILD_USERNAME__ "root"

s8 * dave_verno(void);
s8 * dave_verno_reset(s8 *verno);
s8 * dave_verno_product(s8 *verno, s8 *buf_ptr, ub buf_len);
s8 * dave_verno_my_product(void);

#endif
