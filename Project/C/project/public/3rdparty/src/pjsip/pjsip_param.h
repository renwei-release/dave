/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef __PJSIP_PARAM_H__
#define __PJSIP_PARAM_H__

typedef struct {
	dave_bool success;

	pjsip_dialog *dlg;
	pjsip_evsub *sub;

	sb acc_id;
	s8 request_uri[DAVE_URL_LEN];
} SubscribeModData;

#endif

