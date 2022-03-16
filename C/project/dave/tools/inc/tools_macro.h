/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
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

