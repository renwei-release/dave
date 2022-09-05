/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_JEMALLOC_H__
#define __DAVE_JEMALLOC_H__

#ifdef FORM_PRODUCT_BIN
#define ENABLE_JE_MALLOC
#endif

extern void * je_malloc_for_dave(size_t size);
extern void je_free_for_dave(void *ptr);
extern size_t je_malloc_size_for_dave(void *ptr);

extern void *malloc(size_t size);
extern void free(void *);
extern size_t malloc_usable_size(void *);

#ifdef ENABLE_JE_MALLOC
#define dave_jemalloc(size) je_malloc_for_dave((size_t)size)
#define dave_jefree(ptr) je_free_for_dave(ptr)
#define dave_jelen(ptr) je_malloc_size_for_dave(ptr)
#else
#define dave_jemalloc(size) malloc((size_t)size)
#define dave_jefree(ptr) free(ptr)
#define dave_jelen(ptr) malloc_usable_size(ptr)
#endif

#endif

