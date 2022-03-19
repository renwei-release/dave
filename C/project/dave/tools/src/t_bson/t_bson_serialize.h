/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_BSON_SERIALIZE_H__
#define __T_BSON_SERIALIZE_H__

void t_bson_serialize_reset(tBsonObject *pBson);
void t_bson_serialize_clean(tBsonObject *pBson);

size_t t_bson_serialize(tBsonObject *pBson, unsigned char *serialize_ptr, size_t serialize_len);
tBsonObject * t_serialize_bson(unsigned char *serialize_ptr, size_t serialize_len);

#endif

