/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(WEBRTC_3RDPARTY)
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "api/peer_connection_interface.h"
#include "api/create_peerconnection_factory.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"

#include "dave_os.h"
#include "party_log.h"

static void *_webrtc_thread_body = NULL;

static void *
_webrtc_thread(void *arg)
{
    rtc::InitializeSSL();

    rtc::Thread* network_thread = rtc::Thread::Create().release();
    rtc::Thread* worker_thread = rtc::Thread::Create().release();
    rtc::Thread* signaling_thread = rtc::Thread::Create().release();

    network_thread->Start();
    worker_thread->Start();
    signaling_thread->Start();

    while (dave_os_thread_canceled(_webrtc_thread_body) == dave_false)
	{
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    rtc::CleanupSSL();

	return NULL;
}

// =====================================================================

void
dave_webrtc_init(void)
{
	_webrtc_thread_body = dave_os_create_thread((char *)"webrtc", _webrtc_thread, NULL);
	if(_webrtc_thread_body == NULL)
	{
		PARTYLOG("i can not start webrtc thread!");
	}
}

void
dave_webrtc_exit(void)
{
	if(_webrtc_thread_body != NULL)
	{
		dave_os_release_thread(_webrtc_thread_body);
	}
}

#endif

