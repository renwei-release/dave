/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "uac_class.h"
#include "uac_client.h"
#include "uac_reg.h"
#include "uac_cfg.h"
#include "uac_log.h"

UACClass *_uac_class = NULL;

static void
_uac_main_booting(TIMERID timer_id, ub thread_index)
{
	s8 *server_ip, *server_port, *username, *password, *local_ip, *local_port, *rtp_ip, *rtp_port;

	server_ip = uac_cfg_sbc_server();
	server_port = uac_cfg_sbc_port();
	username = uac_cfg_username();
	password = uac_cfg_password();
	local_ip = uac_cfg_local_ip();
	local_port = uac_cfg_local_port();
	rtp_ip = uac_cfg_rtp_ip();
	rtp_port = uac_cfg_rtp_port();

	_uac_class = uac_class_creat(server_ip, server_port, username, password, local_ip, local_port, rtp_ip, rtp_port);

	if(_uac_class == NULL)
	{
		UACLOG("%s:%s->%s:%s class creat failed!",
			local_ip, local_port, server_ip, server_port);
		return;
	}

	UACLOG("%s:%s->%s:%s socket:%d", local_ip, local_port, server_ip, server_port, _uac_class->signal.signal_socket);

	uac_reg(_uac_class);

	base_timer_die(timer_id);
}

static void
_uac_main_init(void)
{
	_uac_class = NULL;
}

static void
_uac_main_exit(void)
{
	uac_class_release(_uac_class);

	_uac_class = NULL;
}

// =====================================================================

void
uac_main_init(void)
{
	_uac_main_init();

	base_timer_creat("umainbooting", _uac_main_booting, 5000);
}

void
uac_main_exit(void)
{
	_uac_main_exit();
}

UACClass *
uac_main_class(void)
{
	return _uac_class;
}

