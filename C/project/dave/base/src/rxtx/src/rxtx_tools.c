/*
 * ================================================================================
 * (c) Copyright 2020 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.11.23.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_tools.h"
#include "rxtx_secure.h"
#include "rxtx_log.h"

static inline ub
_rxtx_build_crc(u8 *package, ub package_len)
{
	u32 crc_checksum;

	crc_checksum = t_crypto_crc32(package, package_len);

	package[package_len ++] = (u8)(crc_checksum >> 24);
	package[package_len ++] = (u8)(crc_checksum >> 16);
	package[package_len ++] = (u8)(crc_checksum >> 8);
	package[package_len ++] = (u8)(crc_checksum);

	return package_len;
}

static inline ub
_rxtx_check_crc(u8 *package, ub package_len)
{
	u32 crc_checksum;

	if(package_len < 4)
	{
		RTABNOR("invalid package len:%d", package_len);
		return 0;
	}

	crc_checksum = ((u32)(package[-- package_len]));
	crc_checksum += (((u32)(package[-- package_len])) << 8);
	crc_checksum += (((u32)(package[-- package_len])) << 16);
	crc_checksum += (((u32)(package[-- package_len])) << 24);

	if(t_crypto_crc32(package, package_len) != crc_checksum)
	{
		RTDEBUG("package crc check failed!");
		return 0;
	}
	else
	{
		return package_len;
	}
}

// =====================================================================

ub
rxtx_build_crc(u8 *package, ub package_len)
{
	return _rxtx_build_crc(package, package_len);
}

ub
rxtx_build_crc_v2(MBUF *data, u8 *crc_ptr, ub crc_len)
{
	u32 crc_checksum;

	crc_checksum = t_crypto_mcrc32(data);

	crc_ptr[0] = (u8)(crc_checksum >> 24);
	crc_ptr[1] = (u8)(crc_checksum >> 16);
	crc_ptr[2] = (u8)(crc_checksum >> 8);
	crc_ptr[3] = (u8)(crc_checksum);

	return 4;
}

ub
rxtx_check_crc(u8 *package, ub package_len)
{
	return _rxtx_check_crc(package, package_len);
}

#endif

