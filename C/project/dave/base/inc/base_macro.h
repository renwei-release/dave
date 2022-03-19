/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
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
 * Product types defined automatically by the
 * compiler macro.
 */
#if defined(DAVE_PRODUCT_BASE)
 #define __BASE_PRODUCT_BASE__
#elif defined(DAVE_PRODUCT_DEBUG)
 #define __BASE_PRODUCT_DEBUG__
#elif defined(DAVE_PRODUCT_LOG)
 #define __BASE_PRODUCT_LOG__
#elif defined(DAVE_PRODUCT_SYNC)
 #define __BASE_PRODUCT_SYNC__
#else
 #define __BASE_PRODUCT_BASE__
#endif

#define __DAVE_BASE__

#endif

