/*
 * ================================================================================
 * (c) Copyright 2020 Renwei(CMI) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2020.10.13.
 * ================================================================================
 */

#ifndef __DAVE_JEMALLOC_H__
#define __DAVE_JEMALLOC_H__

void dave_jemalloc_init(void);

void dave_jemalloc_exit(void);

void * dave_jemalloc(ub length);

void dave_jefree(void *ptr);

ub dave_jelen(void *ptr);

#endif

