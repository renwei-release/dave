/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef __DAVE_OSIP_H__
#define __DAVE_OSIP_H__
#include "osip.h"
#include "osip_dialog.h"
#include <osipparser2/sdp_message.h>
#include <osip2/fsm.h>
#include "osip_sdp.h"
#include "osip_tag.h"
#include "osip_invite.h"
#include "osip_register.h"
#include "osip_ack.h"
#include "osip_status.h"
#include "osip_bye.h"
#include "osip_tools.h"

void dave_osip_init(void);

void dave_osip_exit(void);

osip_t * dave_osip_body(void);

#endif

