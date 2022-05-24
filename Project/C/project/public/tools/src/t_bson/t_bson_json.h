/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_BSON_JSON_H__
#define __T_BSON_JSON_H__
#include "dave_3rdparty.h"
#include "t_bson_define.h"

json_object * t_bson_json(tBsonObject *pBson);
tBsonObject * t_json_bson(json_object *pJson);

#endif

