/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"

void dos_exit_reset(void);
void dos_ls_reset(void);
void dos_hi_reset(void);
void dos_debug_reset(void);

// =====================================================================

void
dos_app_reset(void)
{
	dos_ls_reset();
	dos_exit_reset();
	dos_hi_reset();
	dos_debug_reset();
}

#endif

