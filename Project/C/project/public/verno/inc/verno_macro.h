/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __VERNO_MACRO_H__
#define __VERNO_MACRO_H__

/*
 * Please define your version of the class here.
 */
#ifdef LEVEL_PRODUCT_alpha
#define __VERNO_ALPHA_VERSION__
#elif defined(LEVEL_PRODUCT_beta)
#define __VERNO_BETA_VERSION__
#else
#define __VERNO_ALPHA_VERSION__
#endif

#endif

