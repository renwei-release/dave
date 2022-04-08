/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __PRODUCT_MACRO_H__
#define __PRODUCT_MACRO_H__

/*
 * Product types defined automatically by the
 * compiler macro.
 */
#if !(\
		defined(__DAVE_PRODUCT_BASE__)\
	||	defined(__DAVE_PRODUCT_IO__)\
	)
 #define __DAVE_PRODUCT_NULL__
#endif

#endif

