/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(TENSORFLOW_3RDPARTY)
#include "dave_define.h"
#include "c_api.h"
#include "c_api_experimental.h"
#include "tensorflow_param.h"
#include "tensorflow_vgg.hpp"
#include "tensorflow_centernet.hpp"
#include "tensorflow_unet.hpp"
#include "tensorflow_traffic.hpp"
#include "tensorflow_test.hpp"
#include "dave_os.h"
#include "party_log.h"

#define TENSORFLOW_TIME_START if(time != NULL){\
		*time = dave_os_time_us();\
	}

#define TENSORFLOW_TIME_END if(time != NULL){\
		*time = dave_os_time_us() - (*time);\
	}

// =====================================================================

void
dave_tensorflow_init(void)
{
	PARTYLOG("version:%s", TF_Version());

#ifdef __TENSORFLOW_VGG__
	tensorflow_vgg_init();
#endif

#ifdef __TENSORFLOW_CENTERNET__
	tensorflow_centernet_init();
#endif

#ifdef __TENSORFLOW_UNET__
	tensorflow_unet_init();
#endif

#ifdef __TENSORFLOW_TRAFFIC__
	tensorflow_traffic_init();
#endif
}

void
dave_tensorflow_exit(void)
{
#ifdef __TENSORFLOW_VGG__
	tensorflow_vgg_exit();
#endif

#ifdef __TENSORFLOW_CENTERNET__
	tensorflow_centernet_exit();
#endif

#ifdef __TENSORFLOW_UNET__
	tensorflow_unet_exit();
#endif

#ifdef __TENSORFLOW_TRAFFIC__
	tensorflow_traffic_exit();
#endif
}

Matrix
__dave_tensorflow_vgg_feature__(u8 *pic_data, ub pic_length, ub *time, s8 *fun, ub line)
{
	Matrix matrix;

#ifdef __TENSORFLOW_VGG__

	TENSORFLOW_TIME_START;

	matrix = __tensorflow_vgg_feature__(pic_data, pic_length, fun, line);

	TENSORFLOW_TIME_END;

#else

	Matrix_reset(&matrix);

#endif

	return matrix;
}

Matrix
__dave_tensorflow_centernet_predict__(u8 *pic_data, ub pic_length, ub *time, s8 *fun, ub line)
{
	Matrix matrix;

#ifdef __TENSORFLOW_CENTERNET__

	TENSORFLOW_TIME_START;

	matrix = __tensorflow_centernet_predict__(pic_data, pic_length, fun, line);

	TENSORFLOW_TIME_END;

#else

	Matrix_reset(&matrix);

#endif

	return matrix;
}

Matrix
__dave_tensorflow_unet_predict__(u8 *pic_data, ub pic_length, ub *time, s8 *fun, ub line)
{
	Matrix matrix;

#ifdef __TENSORFLOW_UNET__

	TENSORFLOW_TIME_START;

	matrix = __tensorflow_unet_predict__(pic_data, pic_length, fun, line);

	TENSORFLOW_TIME_END;

#else

	Matrix_reset(&matrix);

#endif

	return matrix;
}

dave_bool
__dave_tensorflow_traffic_predict__(float *score, ub *label, u8 *pic_data, ub pic_length, ub *time, s8 *fun, ub line)
{
	dave_bool ret = dave_false;

#ifdef __TENSORFLOW_TRAFFIC__

	TENSORFLOW_TIME_START;

	ret = __tensorflow_traffic_predict__(score, label, pic_data, pic_length, fun, line);

	TENSORFLOW_TIME_END;

#endif

	return ret;
}

#endif

