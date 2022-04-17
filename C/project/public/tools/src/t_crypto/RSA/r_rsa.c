/* RSA.C - RSA routines for RSAREF  
 */   
   
/* Copyright (C) RSA Laboratories, a division of RSA Data Security,  
     Inc., created 1991. All rights reserved.  
 */   
   
#include "GLOBAL.H"    
#include "RSAREF.H"    
#include "R_RANDOM.H"    
#include "RSA.H"    
#include "NN.H"    
   
static int RSAPublicBlock PROTO_LIST    
  ((unsigned char *, unsigned int *, unsigned char *, unsigned int,   
    R_RSA_PUBLIC_KEY *));   
static int RSAPrivateBlock PROTO_LIST    
  ((unsigned char *, unsigned int *, unsigned char *, unsigned int,   
    R_RSA_PRIVATE_KEY *));   
   
/* RSA public-key encryption, according to PKCS #1.  
 */   
int RSAPublicEncrypt(
unsigned char *output,                                      /* output block */   
unsigned int *outputLen,                          /* length of output block */   
unsigned char *input,                                        /* input block */   
unsigned int inputLen,                             /* length of input block */   
R_RSA_PUBLIC_KEY *publicKey,                              /* RSA public key */   
R_RANDOM_STRUCT *randomStruct                          /* random structure */)   
{   
  int status;   

  unsigned char byte;
  unsigned char *pkcsBlock;
  unsigned int i, modulusLen;   

  pkcsBlock = R_malloc(MAX_RSA_MODULUS_LEN);
  if(pkcsBlock == NULL)
  	return RE_LEN;
  R_memset ((POINTER)pkcsBlock, 0, MAX_RSA_MODULUS_LEN);   
  	
  modulusLen = (publicKey->bits + 7) / 8;   
  if (inputLen + 11 > modulusLen)   
    return (RE_LEN);   
     
  pkcsBlock[0] = 0;   
  /* block type 2 */   
  pkcsBlock[1] = 2;   
   
  for (i = 2; i < modulusLen - inputLen - 1; i++) {   
    /* Find nonzero random byte.  
     */   
    do {   
      R_GenerateBytes (&byte, 1, randomStruct);   
    } while (byte == 0);   
    pkcsBlock[i] = byte;   
  }   
  /* separator */   
  pkcsBlock[i++] = 0;   
     
  R_memcpy ((POINTER)&pkcsBlock[i], (POINTER)input, inputLen);   
     
  status = RSAPublicBlock   
    (output, outputLen, pkcsBlock, modulusLen, publicKey);   

  R_free(pkcsBlock);

     
  return (status);   
}   
 
/* RSA public-key decryption, according to PKCS #1.  
 */   
int RSAPublicDecrypt (
unsigned char *output,                                      /* output block */   
unsigned int *outputLen,                          /* length of output block */   
unsigned char *input,                                        /* input block */   
unsigned int inputLen,                             /* length of input block */   
R_RSA_PUBLIC_KEY *publicKey                              /* RSA public key */)   
{   
  int status;   
  unsigned char *pkcsBlock;   
  unsigned int i, modulusLen, pkcsBlockLen;   

  pkcsBlock = R_malloc(MAX_RSA_MODULUS_LEN);
  if(pkcsBlock == NULL)
  	return RE_LEN;
  R_memset ((POINTER)pkcsBlock, 0, MAX_RSA_MODULUS_LEN); 
  
  modulusLen = (publicKey->bits + 7) / 8;   
  if (inputLen > modulusLen)   
    return (RE_LEN);   

  status = RSAPublicBlock(pkcsBlock, &pkcsBlockLen, input, inputLen, publicKey);
  if (status)
    return (status);   
     
  if (pkcsBlockLen != modulusLen)   
    return (RE_LEN);   
     
  /* Require block type 1.  
   */   
  if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 1))   
   return (RE_DATA);   
   
  for (i = 2; i < modulusLen-1; i++)   
    if (pkcsBlock[i] != 0xff)   
      break;   
       
  /* separator */   
  if (pkcsBlock[i++] != 0)   
    return (RE_DATA);   
     
  *outputLen = modulusLen - i;   
     
  if (*outputLen + 11 > modulusLen)   
    return (RE_DATA);   
   
  R_memcpy ((POINTER)output, (POINTER)&pkcsBlock[i], *outputLen);   
     
  R_free(pkcsBlock); 
     
  return (0);   
}   
   
/* RSA private-key encryption, according to PKCS #1.  
 */   
int RSAPrivateEncrypt (
unsigned char *output,                                      /* output block */   
unsigned int *outputLen,                          /* length of output block */   
unsigned char *input,                                        /* input block */   
unsigned int inputLen,                             /* length of input block */   
R_RSA_PRIVATE_KEY *privateKey                           /* RSA private key */)   
{   
  int status;   
  unsigned char *pkcsBlock;   
  unsigned int i, modulusLen;   

  pkcsBlock = R_malloc(MAX_RSA_MODULUS_LEN);
  if(pkcsBlock == NULL)
  	return RE_LEN;
  R_memset ((POINTER)pkcsBlock, 0, MAX_RSA_MODULUS_LEN); 
  
  modulusLen = (privateKey->bits + 7) / 8;   
  if (inputLen + 11 > modulusLen)   
    return (RE_LEN);   
     
  pkcsBlock[0] = 0;   
  /* block type 1 */   
  pkcsBlock[1] = 1;   
   
  for (i = 2; i < modulusLen - inputLen - 1; i++)   
    pkcsBlock[i] = 0xff;   
   
  /* separator */   
  pkcsBlock[i++] = 0;   
     
  R_memcpy ((POINTER)&pkcsBlock[i], (POINTER)input, inputLen);   
     
  status = RSAPrivateBlock   
    (output, outputLen, pkcsBlock, modulusLen, privateKey);   
    
  R_free(pkcsBlock); 
   
  return (status);   
}   
   
/* RSA private-key decryption, according to PKCS #1.  
 */   
int RSAPrivateDecrypt (
unsigned char *output,                                      /* output block */   
unsigned int *outputLen,                          /* length of output block */   
unsigned char *input,                                        /* input block */   
unsigned int inputLen,                             /* length of input block */   
R_RSA_PRIVATE_KEY *privateKey                           /* RSA private key */)   
{   
  int status;   
  unsigned char *pkcsBlock;   
  unsigned int i, modulusLen, pkcsBlockLen;   

  pkcsBlock = R_malloc(MAX_RSA_MODULUS_LEN);
  if(pkcsBlock == NULL)
  	return RE_LEN;
  R_memset ((POINTER)pkcsBlock, 0, MAX_RSA_MODULUS_LEN); 
  

  modulusLen = (privateKey->bits + 7) / 8;   
  if (inputLen > modulusLen)   
    return (RE_LEN);   

  status = RSAPrivateBlock(pkcsBlock, &pkcsBlockLen, input, inputLen, privateKey);
  if (status)
    return (status);   
     
  if (pkcsBlockLen != modulusLen)   
    return (RE_LEN);   
     
  /* Require block type 2.  
   */   
  if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 2))   
   return (RE_DATA);   
   
  for (i = 2; i < modulusLen-1; i++)   
    /* separator */   
    if (pkcsBlock[i] == 0)   
      break;   
       
  i++;   
  if (i >= modulusLen)   
    return (RE_DATA);   
       
  *outputLen = modulusLen - i;   
     
  if (*outputLen + 11 > modulusLen)   
    return (RE_DATA);   
   
  R_memcpy ((POINTER)output, (POINTER)&pkcsBlock[i], *outputLen);   
     
  R_free(pkcsBlock);  
     
  return (0);   
}   
   
/* Raw RSA public-key operation. Output has same length as modulus.  
  
   Assumes inputLen < length of modulus.  
   Requires input < modulus.  
 */   
static int RSAPublicBlock (
unsigned char *output,                                      /* output block */   
unsigned int *outputLen,                          /* length of output block */   
unsigned char *input,                                        /* input block */   
unsigned int inputLen,                             /* length of input block */   
R_RSA_PUBLIC_KEY *publicKey                              /* RSA public key */)   
{   
  NN_DIGIT *c, *e, *m, *n;   
  unsigned int eDigits, nDigits;   

  c = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  e = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  m = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  n = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));

  if((c == NULL) || (e == NULL) || (m == NULL) || (n == NULL))
  {
  	  if(c != NULL)
		  R_free(c);
	  if(e != NULL)
		  R_free(e);
   	  if(m != NULL)
		  R_free(m);
   	  if(n != NULL)
		  R_free(n);

	  return RE_DATA;
  }

  R_memset ((POINTER)c, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)e, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)m, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)n, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
   
  NN_Decode (m, MAX_NN_DIGITS, input, inputLen);   
  NN_Decode (n, MAX_NN_DIGITS, publicKey->modulus, MAX_RSA_MODULUS_LEN);   
  NN_Decode (e, MAX_NN_DIGITS, publicKey->exponent, MAX_RSA_MODULUS_LEN);   
  nDigits = NN_Digits (n, MAX_NN_DIGITS);   
  eDigits = NN_Digits (e, MAX_NN_DIGITS);   
     
  if (NN_Cmp (m, n, nDigits) >= 0)   
    return (RE_DATA);   
     
  /* Compute c = m^e mod n.  
   */   
  NN_ModExp (c, m, e, eDigits, n, nDigits);   
   
  *outputLen = (publicKey->bits + 7) / 8;   
  NN_Encode (output, *outputLen, c, nDigits);   
     
  R_free(c);
  R_free(e);
  R_free(m);
  R_free(n);
   
  return (0);   
}   
   
/* Raw RSA private-key operation. Output has same length as modulus.  
  
   Assumes inputLen < length of modulus.  
   Requires input < modulus.  
 */   
static int RSAPrivateBlock (
unsigned char *output,                                      /* output block */   
unsigned int *outputLen,                          /* length of output block */   
unsigned char *input,                                        /* input block */   
unsigned int inputLen,                             /* length of input block */   
R_RSA_PRIVATE_KEY *privateKey                           /* RSA private key */)   
{   
  NN_DIGIT *c,*cP,*cQ,*dP,*dQ,*mP,*mQ,*n,*p,*q,*qInv,*t;   
  unsigned int cDigits, nDigits, pDigits;   

  c = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  cP = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  cQ = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  dP = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  dQ = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  mP = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  mQ = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  n = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  p = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  q = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  qInv = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  t = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));

  if((c == NULL) || (cP == NULL) || (cQ == NULL) || (dP == NULL) || (dQ == NULL) || (mP == NULL)
  	 || (mQ == NULL) || (n == NULL) || (p == NULL) || (q == NULL) || (qInv == NULL) || (t == NULL))
  {
  	if(c != NULL)
		R_free(c);
	if(cP != NULL)
		R_free(cP);
	if(cQ != NULL)
		R_free(cQ);
	if(dP != NULL)
		R_free(dP);
    if(dQ != NULL)
		R_free(dQ);
	if(mP != NULL)
		R_free(mP);
	if(mQ != NULL)
		R_free(mQ);
	if(n != NULL)
		R_free(n);
	if(p != NULL)
		R_free(p);
	if(q != NULL)
		R_free(q);
	if(qInv != NULL)
		R_free(qInv);
	if(t != NULL)
		R_free(t);

	return RE_DATA;
  }

  R_memset ((POINTER)c, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)cP, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)cQ, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)dP, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)dQ, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)mP, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)mQ, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)n, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)p, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)q, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  R_memset ((POINTER)qInv, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));
  R_memset ((POINTER)t, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 
  
  NN_Decode (c, MAX_NN_DIGITS, input, inputLen);   
  NN_Decode (n, MAX_NN_DIGITS, privateKey->modulus, MAX_RSA_MODULUS_LEN);   
  NN_Decode (p, MAX_NN_DIGITS, privateKey->prime[0], MAX_RSA_PRIME_LEN);   
  NN_Decode (q, MAX_NN_DIGITS, privateKey->prime[1], MAX_RSA_PRIME_LEN);   
  NN_Decode    
    (dP, MAX_NN_DIGITS, privateKey->primeExponent[0], MAX_RSA_PRIME_LEN);   
  NN_Decode    
    (dQ, MAX_NN_DIGITS, privateKey->primeExponent[1], MAX_RSA_PRIME_LEN);   
  NN_Decode (qInv, MAX_NN_DIGITS, privateKey->coefficient, MAX_RSA_PRIME_LEN);   
  cDigits = NN_Digits (c, MAX_NN_DIGITS);   
  nDigits = NN_Digits (n, MAX_NN_DIGITS);   
  pDigits = NN_Digits (p, MAX_NN_DIGITS);   
   
  if (NN_Cmp (c, n, nDigits) >= 0)   
    return (RE_DATA);   
     
  /* Compute mP = cP^dP mod p  and  mQ = cQ^dQ mod q. (Assumes q has  
     length at most pDigits, i.e., p > q.)  
   */   
  NN_Mod (cP, c, cDigits, p, pDigits);   
  NN_Mod (cQ, c, cDigits, q, pDigits);   
  NN_ModExp (mP, cP, dP, pDigits, p, pDigits);   
  NN_AssignZero (mQ, nDigits);   
  NN_ModExp (mQ, cQ, dQ, pDigits, q, pDigits);   
     
  /* Chinese Remainder Theorem:  
       m = ((((mP - mQ) mod p) * qInv) mod p) * q + mQ.  
   */   
  if (NN_Cmp (mP, mQ, pDigits) >= 0)   
    NN_Sub (t, mP, mQ, pDigits);   
  else {   
    NN_Sub (t, mQ, mP, pDigits);   
    NN_Sub (t, p, t, pDigits);   
  }   
  NN_ModMult (t, t, qInv, p, pDigits);   
  NN_Mult (t, t, q, pDigits);   
  NN_Add (t, t, mQ, nDigits);   
   
  *outputLen = (privateKey->bits + 7) / 8;   
  NN_Encode (output, *outputLen, t, nDigits);   
   
  R_free(c);
  R_free(cP);
  R_free(cQ);
  R_free(dP);
  R_free(dQ);
  R_free(mP);
  R_free(mQ);
  R_free(n);
  R_free(p);
  R_free(q);
  R_free(qInv);
  R_free(t);
   
  return (0);   
}

