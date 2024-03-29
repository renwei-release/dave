/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_tools.h"
#include "dave_verno.h"
#include "dave_os.h"
#include "dos_show.h"
#include "dos_welcome.h"

static const char *_welcome_word = "\
\n\n\n\
Programs are meant to be read by humans and only incidentally for computers to execute.\n\
                                                                     ------Donald Knuth\n\
\n\n\n";

// =====================================================================

void
dos_welcome_screen(void)
{
	static dave_bool show_declaration = dave_false;
	s8 hostname[128];
	u8 mac[DAVE_MAC_ADDR_LEN];

	if(show_declaration == dave_false)
	{
		show_declaration = dave_true;
		dos_print(_welcome_word);
	}

	dave_os_load_host_name(hostname, sizeof(hostname));
	dave_os_load_mac(mac);

	dos_print("Hi, I'm %s\nBUILD INFORMATION: BUILDER=%s HOST=%s MAC=%s!\nRUNNER INFORMATION: HOST=%s MAC=%s",
		dave_verno(),
		__BUILD_USERNAME__, __BUILD_HOSTNAME__, __BUILD_MAC_ADDRESS__,
		hostname, macstr(mac));
}

#endif

