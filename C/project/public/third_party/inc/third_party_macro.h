/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __THIRD_PARTY_MACRO_H__
#define __THIRD_PARTY_MACRO_H__

/*
 * Please define your version of the class here.
 */
#ifdef LEVEL_PRODUCT_alpha
#define __THIRD_PARTY_ALPHA_VERSION__
#elif defined(LEVEL_PRODUCT_beta)
#define __THIRD_PARTY_BETA_VERSION__
#else
#define __THIRD_PARTY_ALPHA_VERSION__
#endif

#define JEMALLOC_3RDPARTY
#define JSON_3RDPARTY
#define NGINX_3RDPARTY

#endif

