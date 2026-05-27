/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TENSORFLOW_TRAFFIC_HPP__
#define __TENSORFLOW_TRAFFIC_HPP__

void tensorflow_traffic_init(void);

void tensorflow_traffic_exit(void);

dave_bool __tensorflow_traffic_predict__(float *score, ub *label, u8 *pic_data, ub pic_length, s8 *fun, ub line);

#endif

