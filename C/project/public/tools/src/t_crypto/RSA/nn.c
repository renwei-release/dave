/* NN.C - natural numbers routines  
 */   
   
/* Copyright (C) RSA Laboratories, a division of RSA Data Security,  
     Inc., created 1991. All rights reserved.  
 */   
   
/* CHANGES MADE TO THIS FILE UNDER RSAREF 2.0 license clause 1(c):  
  
   For the MIT PGP 2.5 distribution, this file was modified to permit  
   replacement of the NN_ModExp routine by an equivalent routine contained   
   in the PGP 2.5 sources.  To enable this change, an #ifdef was added to this   
   file (search for #ifndef USEMPILIB below).  RSAREF *must* be compiled with   
   USEMPILIB defined for this change to occur.  
  
   Change made May 8, 1994.  */   
   
#include "GLOBAL.H"    
#include "RSAREF.H"    
#include "NN.H" 
#include "DIGIT.H"    
   
static NN_DIGIT NN_AddDigitMult PROTO_LIST    
  ((NN_DIGIT *, NN_DIGIT *, NN_DIGIT, NN_DIGIT *, unsigned int));   
static NN_DIGIT NN_SubDigitMult PROTO_LIST    
  ((NN_DIGIT *, NN_DIGIT *, NN_DIGIT, NN_DIGIT *, unsigned int));   
   
static unsigned int NN_DigitBits PROTO_LIST ((NN_DIGIT));   
   
/* Decodes character string b into a, where character string is ordered  
   from most to least significant.  
  
   Lengths: a[digits], b[len].  
   Assumes b[i] = 0 for i < len - digits * NN_DIGIT_LEN. (Otherwise most  
   significant bytes are truncated.)  
 */   
void NN_Decode (NN_DIGIT *a, unsigned int digits, unsigned char *b, unsigned int len)   
{   
  NN_DIGIT t;   
  int j;   
  unsigned int i, u;   
     
  for (i = 0, j = len - 1; i < digits && j >= 0; i++) {   
    t = 0;   
    for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)   
      t |= ((NN_DIGIT)b[j]) << u;   
    a[i] = t;   
  }   
     
  for (; i < digits; i++)   
    a[i] = 0;   
}   
   
/* Encodes b into character string a, where character string is ordered  
   from most to least significant.  
  
   Lengths: a[len], b[digits].  
   Assumes NN_Bits (b, digits) <= 8 * len. (Otherwise most significant  
   digits are truncated.)  
 */   
void NN_Encode (unsigned char *a, unsigned int len, NN_DIGIT *b, unsigned int digits)  
{
  NN_DIGIT t;   
  int j;   
  unsigned int i, u;   
   
  for (i = 0, j = len - 1; i < digits && j >= 0; i++) {   
    t = b[i];   
    for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)   
      a[j] = (unsigned char)(t >> u);   
  }   
   
  for (; j >= 0; j--)   
    a[j] = 0;   
}   
   
/* Assigns a = b.  
  
   Lengths: a[digits], b[digits].  
 */   
void NN_Assign (
NN_DIGIT *a, NN_DIGIT *b,   
unsigned int digits)   
{   
  unsigned int i;   
   
  for (i = 0; i < digits; i++)   
    a[i] = b[i];   
}   
   
/* Assigns a = 0.  
  
   Lengths: a[digits].  
 */   
void NN_AssignZero (
NN_DIGIT *a,
unsigned int digits)   
{   
  unsigned int i;   
   
  for (i = 0; i < digits; i++)   
    a[i] = 0;   
}   
   
/* Assigns a = 2^b.  
  
   Lengths: a[digits].  
   Requires b < digits * NN_DIGIT_BITS.  
 */   
void NN_Assign2Exp (
NN_DIGIT *a,
unsigned int b, unsigned int digits)  
{   
  NN_AssignZero (a, digits);   
   
  if (b >= digits * NN_DIGIT_BITS)   
    return;   
   
  a[b / NN_DIGIT_BITS] = (NN_DIGIT)1 << (b % NN_DIGIT_BITS);   
}   
   
/* Computes a = b + c. Returns carry.  
  
   Lengths: a[digits], b[digits], c[digits].  
 */   
NN_DIGIT NN_Add (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, 
unsigned int digits)   
{   
  NN_DIGIT ai, carry;   
  unsigned int i;   
   
  carry = 0;   
   
  for (i = 0; i < digits; i++) {   
    if ((ai = b[i] + carry) < carry)   
      ai = c[i];   
    else if ((ai += c[i]) < c[i])   
      carry = 1;   
    else   
      carry = 0;   
    a[i] = ai;   
  }   
   
  return (carry);   
}   
   
/* Computes a = b - c. Returns borrow.  
  
   Lengths: a[digits], b[digits], c[digits].  
 */   
NN_DIGIT NN_Sub (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, 
unsigned int digits)  
{   
  NN_DIGIT ai, borrow;   
  unsigned int i;   
   
  borrow = 0;   
   
  for (i = 0; i < digits; i++) {   
    if ((ai = b[i] - borrow) > (MAX_NN_DIGIT - borrow))   
      ai = MAX_NN_DIGIT - c[i];   
    else if ((ai -= c[i]) > (MAX_NN_DIGIT - c[i]))   
      borrow = 1;   
    else   
      borrow = 0;   
    a[i] = ai;   
  }   
   
  return (borrow);   
}   
   
/* Computes a = b * c.  
  
   Lengths: a[2*digits], b[digits], c[digits].  
   Assumes digits < MAX_NN_DIGITS.  
 */   
void NN_Mult (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c,
unsigned int digits)   
{   
  NN_DIGIT *t;   
  unsigned int bDigits, cDigits, i;   

  t = R_malloc(2*MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if(t == NULL)
  	return ;
  
  NN_AssignZero (t, 2 * digits);   
     
  bDigits = NN_Digits (b, digits);   
  cDigits = NN_Digits (c, digits);   
   
  for (i = 0; i < bDigits; i++)   
    t[i+cDigits] += NN_AddDigitMult (&t[i], &t[i], b[i], c, cDigits);   
     
  NN_Assign (a, t, 2 * digits);   
     
  /* Zeroize potentially sensitive information.  
   */   
  R_memset ((POINTER)t, 0, 2*MAX_NN_DIGITS*sizeof(NN_DIGIT));   

  R_free(t);
}   
   
/* Computes a = b * 2^c (i.e., shifts left c bits), returning carry.  
  
   Lengths: a[digits], b[digits].  
   Requires c < NN_DIGIT_BITS.  
 */   
NN_DIGIT NN_LShift (
NN_DIGIT *a, NN_DIGIT *b,
unsigned int c, unsigned int digits)   
{   
  NN_DIGIT bi, carry;   
  unsigned int i, t;   
     
  if (c >= NN_DIGIT_BITS)   
    return (0);   
     
  t = NN_DIGIT_BITS - c;   
   
  carry = 0;   
   
  for (i = 0; i < digits; i++) {   
    bi = b[i];   
    a[i] = (bi << c) | carry;   
    carry = c ? (bi >> t) : 0;   
  }   
     
  return (carry);   
}   
   
/* Computes a = c div 2^c (i.e., shifts right c bits), returning carry.  
  
   Lengths: a[digits], b[digits].  
   Requires: c < NN_DIGIT_BITS.  
 */   
NN_DIGIT NN_RShift (
NN_DIGIT *a, NN_DIGIT *b,   
unsigned int c, unsigned int digits)   
{   
  NN_DIGIT bi, carry;   
  int i;   
  unsigned int t;   
     
  if (c >= NN_DIGIT_BITS)   
    return (0);   
     
  t = NN_DIGIT_BITS - c;   
   
  carry = 0;   
   
  for (i = digits - 1; i >= 0; i--) {   
    bi = b[i];   
    a[i] = (bi >> c) | carry;   
    carry = c ? (bi << t) : 0;   
  }   
     
  return (carry);   
}   
   
/* Computes a = c div d and b = c mod d.  
  
   Lengths: a[cDigits], b[dDigits], c[cDigits], d[dDigits].  
   Assumes d > 0, cDigits < 2 * MAX_NN_DIGITS,  
           dDigits < MAX_NN_DIGITS.  
 */   
void NN_Div (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c,  
unsigned int cDigits,  NN_DIGIT *d,  unsigned int dDigits)
{   
  NN_DIGIT ai, *cc, *dd, t;   
  int i;   
  unsigned int ddDigits, shift;   

  cc = R_malloc((2*MAX_NN_DIGITS+1)*sizeof(NN_DIGIT));
  dd = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if((cc == NULL) || (dd == NULL))
  {
  	if(cc != NULL)
		R_free(cc);
	if(dd != NULL)
		R_free(dd);

	return;
  }
  
  ddDigits = NN_Digits (d, dDigits);   
  if (ddDigits == 0)   
    return;   
     
  /* Normalize operands.  
   */   
  shift = NN_DIGIT_BITS - NN_DigitBits (d[ddDigits-1]);   
  NN_AssignZero (cc, ddDigits);   
  cc[cDigits] = NN_LShift (cc, c, shift, cDigits);   
  NN_LShift (dd, d, shift, ddDigits);   
  t = dd[ddDigits-1];   
     
  NN_AssignZero (a, cDigits);   
   
  for (i = cDigits-ddDigits; i >= 0; i--) {   
    /* Underestimate quotient digit and subtract.  
     */   
    if (t == MAX_NN_DIGIT)   
      ai = cc[i+ddDigits];   
    else   
      NN_DigitDiv (&ai, &cc[i+ddDigits-1], t + 1);   
    cc[i+ddDigits] -= NN_SubDigitMult (&cc[i], &cc[i], ai, dd, ddDigits);   
   
    /* Correct estimate.  
     */   
    while (cc[i+ddDigits] || (NN_Cmp (&cc[i], dd, ddDigits) >= 0)) {   
      ai++;   
      cc[i+ddDigits] -= NN_Sub (&cc[i], &cc[i], dd, ddDigits);   
    }   
       
    a[i] = ai;   
  }   
     
  /* Restore result.  
   */   
  NN_AssignZero (b, dDigits);   
  NN_RShift (b, cc, shift, ddDigits);   
   
  /* Zeroize potentially sensitive information.  
   */   
  R_memset ((POINTER)cc, 0, (2*MAX_NN_DIGITS+1)*sizeof(NN_DIGIT));   
  R_memset ((POINTER)dd, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));  

  R_free(cc);
  R_free(dd);
}   
   
/* Computes a = b mod c.  
  
   Lengths: a[cDigits], b[bDigits], c[cDigits].  
   Assumes c > 0, bDigits < 2 * MAX_NN_DIGITS, cDigits < MAX_NN_DIGITS.  
 */   
void NN_Mod (
NN_DIGIT *a, NN_DIGIT *b, unsigned int bDigits, NN_DIGIT *c,   
 unsigned int cDigits)  
{   
  NN_DIGIT *t;   

  t = R_malloc(2*MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if(t == NULL)
  	return ;
	 
  NN_Div (t, a, b, bDigits, c, cDigits);   
     
  /* Zeroize potentially sensitive information.  
   */   
  R_memset ((POINTER)t, 0, 2*MAX_NN_DIGITS*sizeof(NN_DIGIT));  

  R_free(t);
}   
   
/* Computes a = b * c mod d.  
  
   Lengths: a[digits], b[digits], c[digits], d[digits].  
   Assumes d > 0, digits < MAX_NN_DIGITS.  
 */   
void NN_ModMult (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_DIGIT *d,   
unsigned int digits)
{   
  NN_DIGIT *t;

  t = R_malloc(2*MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if(t == NULL)
  	return;
   
  NN_Mult (t, b, c, digits);   
  NN_Mod (a, t, 2 * digits, d, digits);   
     
  /* Zeroize potentially sensitive information.  
   */   
  R_memset ((POINTER)t, 0, 2*MAX_NN_DIGITS*sizeof(NN_DIGIT));   

  R_free(t);
}   
   
/* Computes a = b^c mod d.  
  
   Lengths: a[dDigits], b[dDigits], c[cDigits], d[dDigits].  
   Assumes d > 0, cDigits > 0, dDigits < MAX_NN_DIGITS.  
 */   
   
/* PGP 2.5's mpilib contains a faster modular exponentiation routine, mp_modexp.  
   If USEMPILIB is defined, NN_ModExp is replaced in the PGP 2.5 sources with a   
   stub call to mp_modexp.  If USEMPILIB is not defined, we'll get a pure (albeit  
   slower) RSAREF implementation.  
  
   The RSAREF 2.0 license, clause 1(c), permits "...modify[ing] the Program in any  
   manner for porting or performance improvement purposes..." */   
   
#ifndef USEMPILIB    
void NN_ModExp (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c,  
unsigned int cDigits, NN_DIGIT *d, unsigned int dDigits)
{   
  static NN_DIGIT bPower[3][MAX_NN_DIGITS];
  NN_DIGIT ci, *t;   
  int i;   
  unsigned int ciBits, j, s;   

  t = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if(t == NULL)
  	return;
  /* Store b, b^2 mod d, and b^3 mod d.  
   */   
  NN_Assign (bPower[0], b, dDigits);   
  NN_ModMult (bPower[1], bPower[0], b, d, dDigits);   
  NN_ModMult (bPower[2], bPower[1], b, d, dDigits);   
     
  NN_ASSIGN_DIGIT (t, 1, dDigits);   
   
  cDigits = NN_Digits (c, cDigits);   
  for (i = cDigits - 1; i >= 0; i--) {   
    ci = c[i];   
    ciBits = NN_DIGIT_BITS;   
       
    /* Scan past leading zero bits of most significant digit.  
     */   
    if (i == (int)(cDigits - 1)) {   
      while (! DIGIT_2MSB (ci)) {   
        ci <<= 2;   
        ciBits -= 2;   
      }   
    }   
   
    for (j = 0; j < ciBits; j += 2, ci <<= 2) {   
      /* Compute t = t^4 * b^s mod d, where s = two MSB's of ci.  
       */   
      NN_ModMult (t, t, t, d, dDigits);   
      NN_ModMult (t, t, t, d, dDigits);   
      if ((s = DIGIT_2MSB (ci)) != 0)   
        NN_ModMult (t, t, bPower[s-1], d, dDigits);   
    }   
  }   
     
  NN_Assign (a, t, dDigits);   
     
  /* Zeroize potentially sensitive information.  
   */   
  R_memset ((POINTER)bPower, 0, sizeof (bPower));   
  R_memset ((POINTER)t, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT)); 

  R_free(t);
}   
#endif    
   
/* Compute a = 1/b mod c, assuming inverse exists.  
     
   Lengths: a[digits], b[digits], c[digits].  
   Assumes gcd (b, c) = 1, digits < MAX_NN_DIGITS.  
 */   
void NN_ModInv (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c,
unsigned int digits)   
{   
  NN_DIGIT *q, *t1, *t3, *u1, *u3, *v1, *v3, *w;
  int u1Sign;   

  q = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  t1 = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  t3 = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  u1 = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  u3 = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  v1 = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  v3 = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  w = R_malloc(2*MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if((q == NULL) || (t1 == NULL) || (t3 == NULL) || (u1 == NULL) || (u3 == NULL)
  	 || (v1 == NULL) || (v3 == NULL) || (w == NULL))
  {
  	if(q != NULL)
		R_free(q);
	if(t1 != NULL)
		R_free(t1);
	if(t3 != NULL)
		R_free(t3);
	if(u1 != NULL)
		R_free(u1);
	if(u3 != NULL)
		R_free(u3);
	if(v1 != NULL)
		R_free(v1);
	if(v3 != NULL)
		R_free(v3);
	if(w != NULL)
		R_free(w);	

	return;
  }
  
  /* Apply extended Euclidean algorithm, modified to avoid negative  
     numbers.  
   */   
  NN_ASSIGN_DIGIT (u1, 1, digits);   
  NN_AssignZero (v1, digits);   
  NN_Assign (u3, b, digits);   
  NN_Assign (v3, c, digits);   
  u1Sign = 1;   
   
  while (! NN_Zero (v3, digits)) {   
    NN_Div (q, t3, u3, digits, v3, digits);   
    NN_Mult (w, q, v1, digits);   
    NN_Add (t1, u1, w, digits);   
    NN_Assign (u1, v1, digits);   
    NN_Assign (v1, t1, digits);   
    NN_Assign (u3, v3, digits);   
    NN_Assign (v3, t3, digits);   
    u1Sign = -u1Sign;   
  }   
     
  /* Negate result if sign is negative.  
    */   
  if (u1Sign < 0)   
    NN_Sub (a, c, u1, digits);   
  else   
    NN_Assign (a, u1, digits);   
   
  /* Zeroize potentially sensitive information.  
   */   
  R_memset ((POINTER)q, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)t1, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)t3, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)u1, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)u3, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)v1, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)v3, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)w, 0, 2*MAX_NN_DIGITS*sizeof(NN_DIGIT));  

  R_free(q);
  R_free(t1);
  R_free(t3);
  R_free(u1);
  R_free(u3);
  R_free(v1);
  R_free(v3);
  R_free(w);
}   
   
/* Computes a = gcd(b, c).  
  
   Lengths: a[digits], b[digits], c[digits].  
   Assumes b > c, digits < MAX_NN_DIGITS.  
 */   
void NN_Gcd (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c,   
unsigned int digits)   
{   
  NN_DIGIT *t;
  NN_DIGIT *u;
  NN_DIGIT *v;

  t= R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  u = R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  v= R_malloc(MAX_NN_DIGITS*sizeof(NN_DIGIT));
  if((t == NULL) || (u == NULL) || (v == NULL))
  {
  	if(t != NULL)
		R_free(t);
	if(u != NULL)
		R_free(u);
	if(v != NULL)
		R_free(v);

	return;
  }
  	
  NN_Assign (u, b, digits);   
  NN_Assign (v, c, digits);   
   
  while (! NN_Zero (v, digits)) {   
    NN_Mod (t, u, digits, v, digits);   
    NN_Assign (u, v, digits);   
    NN_Assign (v, t, digits);   
  }   
   
  NN_Assign (a, u, digits);   
   
  /* Zeroize potentially sensitive information.  
   */   
  R_memset ((POINTER)t, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)u, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));   
  R_memset ((POINTER)v, 0, MAX_NN_DIGITS*sizeof(NN_DIGIT));  

  R_free(t);
  R_free(u);
  R_free(v);
}   
   
/* Returns sign of a - b.  
  
   Lengths: a[digits], b[digits].  
 */   
int NN_Cmp (
NN_DIGIT *a, NN_DIGIT *b,
unsigned int digits)  
{   
  int i;   
     
  for (i = digits - 1; i >= 0; i--) {   
    if (a[i] > b[i])   
      return (1);   
    if (a[i] < b[i])   
      return (-1);   
  }   
   
  return (0);   
}   
   
/* Returns nonzero iff a is zero.  
  
   Lengths: a[digits].  
 */   
int NN_Zero (
NN_DIGIT *a,  
unsigned int digits)   
{   
  unsigned int i;   
     
  for (i = 0; i < digits; i++)   
    if (a[i])   
      return (0);   
       
  return (1);   
}   
   
/* Returns the significant length of a in bits.  
  
   Lengths: a[digits].  
 */   
unsigned int NN_Bits (
NN_DIGIT *a,
unsigned int digits) 
{   
  if ((digits = NN_Digits (a, digits)) == 0)   
    return (0);   
   
  return ((digits - 1) * NN_DIGIT_BITS + NN_DigitBits (a[digits-1]));   
}   
   
/* Returns the significant length of a in digits.  
  
   Lengths: a[digits].  
 */   
unsigned int NN_Digits (
NN_DIGIT *a,
unsigned int digits)   
{   
  int i;   
     
  for (i = digits - 1; i >= 0; i--)   
    if (a[i])   
      break;   
   
  return (i + 1);   
}   
   
/* Computes a = b + c*d, where c is a digit. Returns carry.  
  
   Lengths: a[digits], b[digits], d[digits].  
 */   
static NN_DIGIT NN_AddDigitMult (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d,
unsigned int digits)
{   
  NN_DIGIT carry, t[2];   
  unsigned int i;   
   
  if (c == 0)   
    return (0);   
   
  carry = 0;   
  for (i = 0; i < digits; i++) {   
    NN_DigitMult (t, c, d[i]);   
    if ((a[i] = b[i] + carry) < carry)   
      carry = 1;   
    else   
      carry = 0;   
    if ((a[i] += t[0]) < t[0])   
      carry++;   
    carry += t[1];   
  }   
     
  return (carry);   
}   
   
/* Computes a = b - c*d, where c is a digit. Returns borrow.  
  
   Lengths: a[digits], b[digits], d[digits].  
 */   
static NN_DIGIT NN_SubDigitMult (
NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d,  
unsigned int digits)
{   
  NN_DIGIT borrow, t[2];   
  unsigned int i;   
   
  if (c == 0)   
    return (0);   
   
  borrow = 0;   
  for (i = 0; i < digits; i++) {   
    NN_DigitMult (t, c, d[i]);   
    if ((a[i] = b[i] - borrow) > (MAX_NN_DIGIT - borrow))   
      borrow = 1;   
    else   
      borrow = 0;   
    if ((a[i] -= t[0]) > (MAX_NN_DIGIT - t[0]))   
      borrow++;   
    borrow += t[1];   
  }   
     
  return (borrow);   
}   
   
/* Returns the significant length of a in bits, where a is a digit.  
 */   
static unsigned int NN_DigitBits (
NN_DIGIT a)
{   
  unsigned int i;   
     
  for (i = 0; i < NN_DIGIT_BITS; i++, a >>= 1)   
    if (a == 0)   
      break;   
       
  return (i);   
}

