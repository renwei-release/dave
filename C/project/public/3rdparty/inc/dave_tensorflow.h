/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_TENSORFLOW_H__
#define __DAVE_TENSORFLOW_H__

void dave_tensorflow_init(void);

void dave_tensorflow_exit(void);

Matrix __dave_tensorflow_vgg_feature__(u8 *pic_data, ub pic_length, ub *time, s8 *fun, ub line);
#define dave_tensorflow_vgg_feature(pic_data, pic_length, time) __dave_tensorflow_vgg_feature__(pic_data, pic_length, time, (s8 *)__func__, (ub)__LINE__)

Matrix __dave_tensorflow_centernet_predict__(u8 *pic_data, ub pic_length, ub *time, s8 *fun, ub line);
#define dave_tensorflow_centernet_predict(pic_data, pic_length, time) __dave_tensorflow_centernet_predict__(pic_data, pic_length, time, (s8 *)__func__, (ub)__LINE__)

Matrix __dave_tensorflow_unet_predict__(u8 *pic_data, ub pic_length, ub *time, s8 *fun, ub line);
#define dave_tensorflow_unet_predict(pic_data, pic_length, time) __dave_tensorflow_unet_predict__(pic_data, pic_length, time, (s8 *)__func__, (ub)__LINE__)

dave_bool __dave_tensorflow_traffic_predict__(float *score, ub *label, u8 *pic_data, ub pic_length, ub *time, s8 *fun, ub line);
#define dave_tensorflow_traffic_predict(score, label, pic_data, pic_length, time) __dave_tensorflow_traffic_predict__(score, label, pic_data, pic_length, time, (s8 *)__func__, (ub)__LINE__)

#endif

