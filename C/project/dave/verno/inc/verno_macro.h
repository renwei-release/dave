/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
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

#define __VERNO_PC_LINUX__

#endif

