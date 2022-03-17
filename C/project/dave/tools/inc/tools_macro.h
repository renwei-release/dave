/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TOOLS_MACRO_H__
#define __TOOLS_MACRO_H__

/*
 * Please define your version of the class here.
 */
#ifdef LEVEL_PRODUCT_alpha
#define __TOOLS_ALPHA_VERSION__
#elif defined(LEVEL_PRODUCT_beta)
#define __TOOLS_BETA_VERSION__
#else
#define __TOOLS_ALPHA_VERSION__
#endif

#define __DAVE_TOOLS__

#endif

