#include "dave_osip.h"

static osip_t *pOsip = NULL;

static void
_osip_trace_fun(const char *fi, int li, osip_trace_level_t level, const char *chfr, va_list ap)
{

}

static void
_osip_register_trace(void)
{
	osip_trace_level_t level;

	for(level=TRACE_LEVEL0; level<END_TRACE_LEVEL; level++)
	{
		osip_trace_initialize_func(level, _osip_trace_fun);
		osip_trace_enable_level(level);
	}
}

static void
_osip_init(void)
{
	if(osip_init(&pOsip) >= 0)
	{
		_osip_register_trace();
	}
}

static void
_osip_exit(void)
{
	if(pOsip != NULL)
	{
		osip_release(pOsip);
	}
}

// =====================================================================

void
dave_osip_init(void)
{
	_osip_init();
}

void
dave_osip_exit(void)
{
	_osip_exit();
}

osip_t *
dave_osip_body(void)
{
	return pOsip;
}

