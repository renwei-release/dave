/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "tools_macro.h"
#ifdef __DAVE_TOOLS__

/* R_KEYGEN.C - key-pair generation for RSAREF  
 */   
   
/* Copyright (C) RSA Laboratories, a division of RSA Data Security,  
     Inc., created 1991. All rights reserved.  
 */   
   
#include "GLOBAL.H"    
#include "RSAREF.H"    
#include "R_RANDOM.H"    
#include "NN.H"    
#include "PRIME.H"    
   
static int RSAFilter PROTO_LIST   
  ((NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int));   
static int RelativelyPrime PROTO_LIST   
  ((NN_DIGIT *, unsigned int, NN_DIGIT *, unsigned int));   
   
/* Generates an RSA key pair with a given length and public exponent.  
 */   
int R_GeneratePEMKeys (
R_RSA_PUBLIC_KEY *publicKey,                          /* new RSA public key */   
R_RSA_PRIVATE_KEY *privateKey,                       /* new RSA private key */   
R_RSA_PROTO_KEY *protoKey,                             /* RSA prototype key */   
R_RANDOM_STRUCT *randomStruct                          /* random structure */)   
{   
  NN_DIGIT *d,*dP,*dQ,*e,*n,*p,*phiN,*pMinus1,*q,*qInv,*qMinus1,*t,*u,*v;   
  int status;   
  unsigned int nDigits, pBits, pDigits, qBits;   

  d = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  dP = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  dQ = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  e = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  n = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  p = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  phiN = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  pMinus1 = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  q = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  qInv = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  qMinus1 = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  t = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  u = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  v = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if((d == NULL) || (dP == NULL) || (dQ == NULL) || (e == NULL)
  	 || (n == NULL) || (p == NULL) || (phiN == NULL) || (pMinus1 == NULL)
  	 || (q == NULL) || (qInv == NULL) || (qMinus1 == NULL) || (t == NULL)
  	 || (u == NULL) || (v == NULL))
  {
  	if(d != NULL)
		R_free(d);
	if(dP != NULL)
		R_free(dP);
	if(dQ != NULL)
		R_free(dQ);
	if(e != NULL)
		R_free(e);
	if(n != NULL)
		R_free(n);
	if(p != NULL)
		R_free(p);
	if(phiN != NULL)
		R_free(phiN);
	if(pMinus1 != NULL)
		R_free(pMinus1);
	if(q != NULL)
		R_free(q);
	if(qInv != NULL)
		R_free(qInv);
	if(qMinus1 != NULL)
		R_free(qMinus1);	
	if(t != NULL)
		R_free(t);
	if(u != NULL)
		R_free(u);
	if(v != NULL)
		R_free(v);

	return RE_MODULUS_LEN;
	
  }
  if ((protoKey->bits < MIN_RSA_MODULUS_BITS) ||    
      (protoKey->bits > MAX_RSA_MODULUS_BITS))   
    return (RE_MODULUS_LEN);   
  nDigits = (protoKey->bits + NN_DIGIT_BITS - 1) / NN_DIGIT_BITS;   
  pDigits = (nDigits + 1) / 2;   
  pBits = (protoKey->bits + 1) / 2;   
  qBits = protoKey->bits - pBits;   
   
  /* NOTE: for 65537, this assumes NN_DIGIT is at least 17 bits. */   
  NN_ASSIGN_DIGIT   
    (e, protoKey->useFermat4 ? (NN_DIGIT)65537 : (NN_DIGIT)3, nDigits);   
   
  /* Generate prime p between 3*2^(pBits-2) and 2^pBits-1, searching  
       in steps of 2, until one satisfies gcd (p-1, e) = 1.  
   */   
  NN_Assign2Exp (t, pBits-1, pDigits);   
  NN_Assign2Exp (u, pBits-2, pDigits);   
  NN_Add (t, t, u, pDigits);   
  NN_ASSIGN_DIGIT (v, 1, pDigits);   
  NN_Sub (v, t, v, pDigits);   
  NN_Add (u, u, v, pDigits);   
  NN_ASSIGN_DIGIT (v, 2, pDigits);   
  do {   
  	status = GeneratePrime (p, t, u, v, pDigits, randomStruct);
    if (status)   
      return (status);   
  }   
  while (! RSAFilter (p, pDigits, e, 1));   
     
  /* Generate prime q between 3*2^(qBits-2) and 2^qBits-1, searching  
       in steps of 2, until one satisfies gcd (q-1, e) = 1.  
   */   
  NN_Assign2Exp (t, qBits-1, pDigits);   
  NN_Assign2Exp (u, qBits-2, pDigits);   
  NN_Add (t, t, u, pDigits);   
  NN_ASSIGN_DIGIT (v, 1, pDigits);   
  NN_Sub (v, t, v, pDigits);   
  NN_Add (u, u, v, pDigits);   
  NN_ASSIGN_DIGIT (v, 2, pDigits);   
  do {  
  	status = GeneratePrime (q, t, u, v, pDigits, randomStruct);
    if (status)   
      return (status);   
  }   
  while (! RSAFilter (q, pDigits, e, 1));   
     
  /* Sort so that p > q. (p = q case is extremely unlikely.)  
   */   
  if (NN_Cmp (p, q, pDigits) < 0) {   
    NN_Assign (t, p, pDigits);   
    NN_Assign (p, q, pDigits);   
    NN_Assign (q, t, pDigits);   
  }   
   
  /* Compute n = pq, qInv = q^{-1} mod p, d = e^{-1} mod (p-1)(q-1),  
     dP = d mod p-1, dQ = d mod q-1.  
   */   
  NN_Mult (n, p, q, pDigits);   
  NN_ModInv (qInv, q, p, pDigits);   
     
  NN_ASSIGN_DIGIT (t, 1, pDigits);   
  NN_Sub (pMinus1, p, t, pDigits);   
  NN_Sub (qMinus1, q, t, pDigits);   
  NN_Mult (phiN, pMinus1, qMinus1, pDigits);   
   
  NN_ModInv (d, e, phiN, nDigits);   
  NN_Mod (dP, d, nDigits, pMinus1, pDigits);   
  NN_Mod (dQ, d, nDigits, qMinus1, pDigits);   
     
  publicKey->bits = privateKey->bits = protoKey->bits;   
  NN_Encode (publicKey->modulus, MAX_RSA_MODULUS_LEN, n, nDigits);   
  NN_Encode (publicKey->exponent, MAX_RSA_MODULUS_LEN, e, 1);   
  R_memcpy    
    ((POINTER)privateKey->modulus, (POINTER)publicKey->modulus,   
     MAX_RSA_MODULUS_LEN);   
  R_memcpy   
    ((POINTER)privateKey->publicExponent, (POINTER)publicKey->exponent,   
     MAX_RSA_MODULUS_LEN);   
  NN_Encode (privateKey->exponent, MAX_RSA_MODULUS_LEN, d, nDigits);   
  NN_Encode (privateKey->prime[0], MAX_RSA_PRIME_LEN, p, pDigits);   
  NN_Encode (privateKey->prime[1], MAX_RSA_PRIME_LEN, q, pDigits);   
  NN_Encode (privateKey->primeExponent[0], MAX_RSA_PRIME_LEN, dP, pDigits);   
  NN_Encode (privateKey->primeExponent[1], MAX_RSA_PRIME_LEN, dQ, pDigits);   
  NN_Encode (privateKey->coefficient, MAX_RSA_PRIME_LEN, qInv, pDigits);   
      
  /* Zeroize sensitive information.  
   */   
  R_memset ((POINTER)d, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)dP, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)dQ, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));  
  R_memset ((POINTER)e, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)n, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)p, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)phiN, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)pMinus1, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)q, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)qInv, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)qMinus1, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)t, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)u, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)v, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 

  R_free(d);
  R_free(dP);
  R_free(dQ);
  R_free(e);
  R_free(n);
  R_free(p);
  R_free(phiN);
  R_free(pMinus1);
  R_free(q);
  R_free(qInv);
  R_free(qMinus1);	
  R_free(t);
  R_free(u);
  R_free(v);
	
  return (0);   
}   
   
/* Returns nonzero iff GCD (a-1, b) = 1.  
  
   Lengths: a[aDigits], b[bDigits].  
   Assumes aDigits < MAX_NN_DIGITS, bDigits < MAX_NN_DIGITS.  
 */   
static int RSAFilter (NN_DIGIT *a, unsigned int aDigits,  NN_DIGIT *b, unsigned int bDigits)   
{   
  int status;   
  NN_DIGIT *aMinus1, *t;   

  aMinus1 = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  t = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if((aMinus1 == NULL) || (t == NULL))
  {
  	if(aMinus1 != NULL)
		R_free(aMinus1);
  	if(t != NULL)
		R_free(t);

	return -1;
  }
  
  NN_ASSIGN_DIGIT (t, 1, aDigits);   
  NN_Sub (aMinus1, a, t, aDigits);   
     
  status = RelativelyPrime (aMinus1, aDigits, b, bDigits);   
   
  /* Zeroize sensitive information.  
   */   
  R_memset ((POINTER)aMinus1, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)t, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  
  R_free(aMinus1);
  R_free(t);
  
  return (status);   
}   
   
/* Returns nonzero iff a and b are relatively prime.  
  
   Lengths: a[aDigits], b[bDigits].  
   Assumes aDigits >= bDigits, aDigits < MAX_NN_DIGITS.  
 */   
static int RelativelyPrime (NN_DIGIT *a, unsigned int aDigits, NN_DIGIT *b, unsigned int bDigits)
{   
  int status;   
  NN_DIGIT *t, *u;   

  t = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  u = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if((t == NULL) || (u == NULL))
  {
  	if(t != NULL)
		R_free(t);
	if(u != NULL)
		R_free(u);

	return -1;
  }
  
  NN_AssignZero (t, aDigits);   
  NN_Assign (t, b, bDigits);   
  NN_Gcd (t, a, t, aDigits);   
  NN_ASSIGN_DIGIT (u, 1, aDigits);   
   
  status = NN_EQUAL (t, u, aDigits);   
     
  /* Zeroize sensitive information.  
   */   
  R_memset ((POINTER)t, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)u, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  
  R_free(t);
  R_free(u);
  
  return (status);   
}

#endif
