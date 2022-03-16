/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#ifndef __BASE_MACRO_H__
#define __BASE_MACRO_H__

/*
 * Please define your version of the class here.
 */
#ifdef LEVEL_PRODUCT_alpha
#define __BASE_ALPHA_VERSION__
#elif defined(LEVEL_PRODUCT_beta)
#define __BASE_BETA_VERSION__
#else
#define __BASE_ALPHA_VERSION__
#endif

/*
 * BASE platform macro defined by the compiler macro.
 */
#if defined(ANDROID_NDK_VOIP)
 #define __BASE_ANDROID__
#else
 #define __BASE_PC_LINUX__
#endif

/*
 * Product types defined automatically by the
 * compiler macro.
 */
#if defined(PC_LINUX_LOG) || defined(APPLE_MAC_LOG)
 #define __BASE_PRODUCT_LOG__
#elif defined(PC_LINUX_SYNC) || defined(APPLE_MAC_SYNC)
 #define __BASE_PRODUCT_SYNC__
#elif defined(PC_LINUX_BASE)
 #define __BASE_PRODUCT_BASE__
#elif defined(PC_LINUX_DEBUG)
 #define __BASE_PRODUCT_DEBUG__
#else
 #define __BASE_PRODUCT_BASE__
#endif

#define __DAVE_BASE__

#endif

