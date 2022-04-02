/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "tools_macro.h"
#ifdef __DAVE_TOOLS__
#include "dave_tools.h"

/* R_STDLIB.C - platform-specific C library routines for RSAREF  
 */   
   
/* Copyright (C) RSA Laboratories, a division of RSA Data Security,  
     Inc., created 1991. All rights reserved.  
 */  
#include "GLOBAL.H"    
#include "RSAREF.H"    

void *R_malloc(unsigned int len)
{
	if(len)
		return dave_malloc((sw_uint16)len);
	else 
		return NULL;
}

void R_free(void *ptr)
{
	if(ptr != NULL)
		dave_free(ptr);
}

void R_memset (
POINTER output,                                             /* output block */   
int value,                                                         /* value */   
unsigned int len                                        /* length of block */)   
{   
  if (len)   
    dave_memset ((sw_uint8 *)output, (sw_uint8)value, (sw_uint16)len);   
}   
   
void R_memcpy (
POINTER output,                                             /* output block */   
POINTER input,                                               /* input block */   
unsigned int len                                       /* length of blocks */)   
{   
  if (len)   
    dave_memcpy ((sw_uint8 *)output, (sw_uint8 *)input, (sw_uint16)len);   
}   
   
int R_memcmp (
POINTER firstBlock,                                          /* first block */   
POINTER secondBlock,                                        /* second block */   
unsigned int len                                       /* length of blocks */)   
{   
  if (len)   
    return (dave_memcmp ((sw_uint8 *)firstBlock, (sw_uint8 *)secondBlock, (sw_uint16)len));   
  else   
    return (0);   
}

#endif
