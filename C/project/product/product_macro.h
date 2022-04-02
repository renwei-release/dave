/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __PRODUCT_MACRO_H__
#define __PRODUCT_MACRO_H__

/*
 * Please define your version of the class here.
 */
#ifdef LEVEL_PRODUCT_alpha
#define __PRODUCT_ALPHA_VERSION__
#elif defined(LEVEL_PRODUCT_beta)
#define __PRODUCT_BETA_VERSION__
#else
#define __PRODUCT_ALPHA_VERSION__
#endif

/*
 * Product types defined automatically by the
 * compiler macro.
 */
#if defined(DAVE_PRODUCT_BASE)
 #define __DAVE_PRODUCT_BASE__
#elif defined(DAVE_PRODUCT_IO)
 #define __DAVE_PRODUCT_IO__
#else
 #define __DAVE_PRODUCT_NULL__
#endif


// $$$$$$$$$$$$$$$$$$interface compatible$$$$$$$$$$$$$$$$$$$$
#ifdef LEVEL_PRODUCT_alpha
#define __DAVE_ALPHA_VERSION__
#elif defined(LEVEL_PRODUCT_beta)
#define __DAVE_BETA_VERSION__
#else
#define __DAVE_ALPHA_VERSION__
#endif


#endif

