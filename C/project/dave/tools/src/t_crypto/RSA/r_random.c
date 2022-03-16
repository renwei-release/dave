/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "dave_os.h"
#include "dave_base.h"
/* R_RANDOM.C - random objects for RSAREF  
 */   
   
/* Copyright (C) RSA Laboratories, a division of RSA Data Security,  
     Inc., created 1991. All rights reserved.  
 */   
   
#include "GLOBAL.H"    
#include "RSAREF.H"    
#include "R_RANDOM.H"    
#include "crypto_MD5.H"    

#define RANDOM_BYTES_NEEDED 256    
   
int R_RandomInit (
R_RANDOM_STRUCT *randomStruct                      /* new random structure */)   
{   
  randomStruct->bytesNeeded = RANDOM_BYTES_NEEDED;   
  R_memset ((POINTER)randomStruct->state, 0, sizeof (randomStruct->state));   
  randomStruct->outputAvailable = 0;   
     
  return (0);   
}   
   
int R_RandomUpdate (
R_RANDOM_STRUCT *randomStruct,                          /* random structure */   
unsigned char *block,                          /* block of values to mix in */   
unsigned int blockLen                                   /* length of block */)   
{   
  MD5_CTX context;   
  unsigned char digest[16];   
  unsigned int i, x;   
     
  crypto_MD5Init (&context);   
  crypto_MD5Update (&context, block, blockLen);   
  crypto_MD5Final (digest, &context);   

  /* add digest to state */   
  x = 0;   
  for (i = 0; i < 16; i++) {   
    x += randomStruct->state[15-i] + digest[15-i];   
    randomStruct->state[15-i] = (unsigned char)x;   
    x >>= 8;   
  }   
     
  if (randomStruct->bytesNeeded < blockLen)   
    randomStruct->bytesNeeded = 0;   
  else   
    randomStruct->bytesNeeded -= blockLen;   
     
  /* Zeroize sensitive information.  
   */   
  R_memset ((POINTER)digest, 0, sizeof (digest));
  return (0);   
}   
   
int R_GetRandomBytesNeeded (
unsigned int *bytesNeeded,                 /* number of mix-in bytes needed */   
R_RANDOM_STRUCT *randomStruct                          /* random structure */)   
{   
  *bytesNeeded = randomStruct->bytesNeeded;   
     
  return (0);   
}   
   
int R_GenerateBytes (
unsigned char *block,                                              /* block */   
unsigned int blockLen,                                   /* length of block */   
R_RANDOM_STRUCT *randomStruct                          /* random structure */)   
{   
  MD5_CTX context;   
  unsigned int available, i;   
     
  if (randomStruct->bytesNeeded)   
    return (RE_NEED_RANDOM);   
     
  available = randomStruct->outputAvailable;   
     
  while (blockLen > available) {   
    R_memcpy   
      ((POINTER)block, (POINTER)&randomStruct->output[16-available],   
       available);   
    block += available;   
    blockLen -= available;   
   
    /* generate new output */   
    crypto_MD5Init (&context);   
    crypto_MD5Update (&context, randomStruct->state, 16);   
    crypto_MD5Final (randomStruct->output, &context);   
    available = 16;   
   
    /* increment state */   
    for (i = 0; i < 16; i++)   
      if (randomStruct->state[15-i]++)   
        break;   
  }   
   
  R_memcpy    
    ((POINTER)block, (POINTER)&randomStruct->output[16-available], blockLen);   
  randomStruct->outputAvailable = available - blockLen;   
   
  return (0);   
}   
   
void R_RandomFinal (
R_RANDOM_STRUCT *randomStruct)                          /* random structure */   
{   
  R_memset ((POINTER)randomStruct, 0, sizeof (*randomStruct));   
}

