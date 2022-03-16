/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.01.28.
 * ================================================================================
 */

#ifndef __T_BSON_JSON_H__
#define __T_BSON_JSON_H__
#include "dave_third_party.h"
#include "t_bson_define.h"

json_object * t_bson_json(tBsonObject *pBson);
tBsonObject * t_json_bson(json_object *pJson);

#endif

