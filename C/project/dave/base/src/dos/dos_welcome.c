/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_tools.h"
#include "dave_verno.h"
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

	if(show_declaration == dave_false)
	{
		show_declaration = dave_true;
		dos_print(_welcome_word);
	}

	dos_print("Hi, I'm %s\nBUILD INFORMATION: HOST-%s USER-%s MAC-%s!",
		dave_verno(), __BUILD_HOSTNAME__, __BUILD_USERNAME__, __BUILD_MAC_ADDRESS__);
}

#endif

