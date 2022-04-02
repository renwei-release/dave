/* GLOBAL.H - RSAREF types and constants */
 
/* Copyright (C) RSA Laboratories, a division of RSA Data Security, 
     Inc., created 1991. All rights reserved. 
 */ 
 
#ifndef _GLOBAL_H_ 
#define _GLOBAL_H_ 1 
 
/* PROTOTYPES should be set to one if and only if the compiler supports 
     function argument prototyping. 
   The following makes PROTOTYPES default to 1 if it has not already been 
     defined as 0 with C compiler flags. 
 */ 
#ifndef PROTOTYPES 
#define PROTOTYPES 1 
#endif 

/* POINTER defines a generic pointer type */ 
typedef unsigned char *POINTER; 
 
/* UINT2 defines a two byte word */ 
typedef unsigned short int UINT2; 
 
/* UINT4 defines a four byte word */ 
typedef unsigned int UINT4;
 
#ifndef NULL_PTR 
#define NULL_PTR ((POINTER)0) 
#endif 

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef UNUSED_ARG 
#define UNUSED_ARG(x) x = *(&x); 
#endif 
 
/* PROTO_LIST is defined depending on how PROTOTYPES is defined above. 
   If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it 
     returns an empty list.   
 */ 
#if PROTOTYPES 
#define PROTO_LIST(list) list 
#else 
#define PROTO_LIST(list) () 
#endif 

#define MIN_RSA_MODULUS_BITS (512) 
#define MAX_RSA_MODULUS_BITS (1024)
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8) 
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2) 
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)
#define DAVE_RSA_KEY_LEN (1024)

#if (DAVE_RSA_KEY_LEN<MIN_RSA_MODULUS_BITS) || (DAVE_RSA_KEY_LEN>MAX_RSA_MODULUS_BITS)
#error DAVE_RSA_KEY_LEN define error!!!
#endif

/* 
 * RSA public and private key. 
 */ 
typedef struct { 
  unsigned int bits;
  unsigned char modulus[MAX_RSA_MODULUS_LEN];
  unsigned char exponent[MAX_RSA_MODULUS_LEN];
} R_RSA_PUBLIC_KEY; 
 
typedef struct { 
  unsigned int bits;
  unsigned char modulus[MAX_RSA_MODULUS_LEN];
  unsigned char publicExponent[MAX_RSA_MODULUS_LEN];
  unsigned char exponent[MAX_RSA_MODULUS_LEN];
  unsigned char prime[2][MAX_RSA_PRIME_LEN];
  unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];
  unsigned char coefficient[MAX_RSA_PRIME_LEN];
} R_RSA_PRIVATE_KEY;

/* Random structure. 
 */ 
typedef struct { 
  unsigned int bytesNeeded; 
  unsigned char state[16]; 
  unsigned int outputAvailable; 
  unsigned char output[16]; 
} R_RANDOM_STRUCT; 

#endif /* end _GLOBAL_H_ */ 

