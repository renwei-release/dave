/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#ifndef __BASE_TYPE_H__
#define __BASE_TYPE_H__
#include "base_macro.h"
#include <limits.h>
#include <stdbool.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#if defined(__DAVE_PC_WINDOWS__)
	#ifdef __FUNCTION__
	#define __func__ __FUNCTION__
	#else
	#define __func__  __FILE__
	#endif
#endif

#ifndef BITS_PER_LONG
	#ifndef LONG_MAX
	#error Please define LONG_MAX!!!!
	#endif
	
	#if LONG_MAX > 2147483647L
	#define PLATFORM_64_BIT
	#else
	#define PLATFORM_32_BIT
	#endif
#else
	#if (BITS_PER_LONG == 64)
	#define PLATFORM_64_BIT
	#else
	#define PLATFORM_32_BIT
	#endif
#endif

#define dave_bool _Bool
#define dave_false false
#define dave_true true

typedef char sw_int8;
typedef signed short sw_int16;
typedef signed int sw_int32;
#ifdef PLATFORM_64_BIT
typedef signed long sw_int64;
#else
	#ifdef WIN32
	typedef __int64 sw_int64;
	#else
	typedef signed long long sw_int64;
	#endif
#endif
typedef unsigned char sw_uint8;
typedef unsigned short sw_uint16;
typedef unsigned int sw_uint32;
#ifdef PLATFORM_64_BIT
typedef unsigned long sw_uint64;
#else
	#ifdef WIN32
	typedef unsigned __int64 sw_uint64;
	#else
	typedef unsigned long long sw_uint64;
	#endif
#endif
typedef unsigned int DAVEPID;
#ifdef PLATFORM_64_BIT
typedef sw_uint64 sw_ubase;
typedef sw_int64 sw_base;
#else
typedef sw_uint32 sw_ubase;
typedef sw_int32 sw_base;
#endif

#ifdef PLATFORM_64_BIT
#define MAX_VALUE_DEFINE 0x1234567812345678
#else
#define MAX_VALUE_DEFINE 0x12345678
#endif

typedef sw_uint64 u64;
typedef sw_int64 s64;
typedef sw_uint32 u32;
typedef sw_int32 s32;
typedef sw_uint16 u16;
typedef sw_int16 s16;
typedef sw_uint8 u8;
typedef sw_int8 s8;

#ifndef uint8_t
#define uint8_t u8
#endif
#ifndef int8_t
#define int8_t s8
#endif
#ifndef uint16_t
#define uint16_t u16
#endif
#ifndef int16_t
#define int16_t s16
#endif
#ifndef uint32_t
#define uint32_t u32
#endif
#ifndef int32_t
#define int32_t s32
#endif
#ifndef uint64_t
#define uint64_t u64
#endif
#ifndef int64_t
#define int64_t s64
#endif

typedef sw_ubase ub;
typedef sw_base sb;

#endif

