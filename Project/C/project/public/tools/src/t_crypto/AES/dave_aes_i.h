/*
 * AES (Rijndael) cipher
 * Copyright (c) 2003-2012, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef __DAVE_AES_I_H__
#define __DAVE_AES_I_H__
#include "dave_aes.h"

/* #define FULL_UNROLL */
#define AES_SMALL_TABLES

extern const u32 dave_aes_Te0[256];
extern const u32 dave_aes_Te1[256];
extern const u32 dave_aes_Te2[256];
extern const u32 dave_aes_Te3[256];
extern const u32 dave_aes_Te4[256];
extern const u32 dave_aes_Td0[256];
extern const u32 dave_aes_Td1[256];
extern const u32 dave_aes_Td2[256];
extern const u32 dave_aes_Td3[256];
extern const u32 dave_aes_Td4[256];
extern const u32 dave_aes_rcon[10];
extern const u8 dave_aes_Td4s[256];
extern const u8 dave_aes_rcons[10];

#ifndef AES_SMALL_TABLES

#define RCON(i) dave_aes_rcon[(i)]

#define TE0(i) dave_aes_Te0[((i) >> 24) & 0xff]
#define TE1(i) dave_aes_Te1[((i) >> 16) & 0xff]
#define TE2(i) dave_aes_Te2[((i) >> 8) & 0xff]
#define TE3(i) dave_aes_Te3[(i) & 0xff]
#define TE41(i) (dave_aes_Te4[((i) >> 24) & 0xff] & 0xff000000)
#define TE42(i) (dave_aes_Te4[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE43(i) (dave_aes_Te4[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE44(i) (dave_aes_Te4[(i) & 0xff] & 0x000000ff)
#define TE421(i) (dave_aes_Te4[((i) >> 16) & 0xff] & 0xff000000)
#define TE432(i) (dave_aes_Te4[((i) >> 8) & 0xff] & 0x00ff0000)
#define TE443(i) (dave_aes_Te4[(i) & 0xff] & 0x0000ff00)
#define TE414(i) (dave_aes_Te4[((i) >> 24) & 0xff] & 0x000000ff)
#define TE411(i) (dave_aes_Te4[((i) >> 24) & 0xff] & 0xff000000)
#define TE422(i) (dave_aes_Te4[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE433(i) (dave_aes_Te4[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE444(i) (dave_aes_Te4[(i) & 0xff] & 0x000000ff)
#define TE4(i) (dave_aes_Te4[(i)] & 0x000000ff)

#define TD0(i) dave_aes_Td0[((i) >> 24) & 0xff]
#define TD1(i) dave_aes_Td1[((i) >> 16) & 0xff]
#define TD2(i) dave_aes_Td2[((i) >> 8) & 0xff]
#define TD3(i) dave_aes_Td3[(i) & 0xff]
#define TD41(i) (dave_aes_Td4[((i) >> 24) & 0xff] & 0xff000000)
#define TD42(i) (dave_aes_Td4[((i) >> 16) & 0xff] & 0x00ff0000)
#define TD43(i) (dave_aes_Td4[((i) >> 8) & 0xff] & 0x0000ff00)
#define TD44(i) (dave_aes_Td4[(i) & 0xff] & 0x000000ff)
#define TD0_(i) dave_aes_Td0[(i) & 0xff]
#define TD1_(i) dave_aes_Td1[(i) & 0xff]
#define TD2_(i) dave_aes_Td2[(i) & 0xff]
#define TD3_(i) dave_aes_Td3[(i) & 0xff]

#else /* AES_SMALL_TABLES */

#define RCON(i) (dave_aes_rcons[(i)] << 24)

u32 dave_rotr(u32 val, int bits);

#define TE0(i) dave_aes_Te0[((i) >> 24) & 0xff]
#define TE1(i) dave_rotr(dave_aes_Te0[((i) >> 16) & 0xff], 8)
#define TE2(i) dave_rotr(dave_aes_Te0[((i) >> 8) & 0xff], 16)
#define TE3(i) dave_rotr(dave_aes_Te0[(i) & 0xff], 24)
#define TE41(i) ((dave_aes_Te0[((i) >> 24) & 0xff] << 8) & 0xff000000)
#define TE42(i) (dave_aes_Te0[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE43(i) (dave_aes_Te0[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE44(i) ((dave_aes_Te0[(i) & 0xff] >> 8) & 0x000000ff)
#define TE421(i) ((dave_aes_Te0[((i) >> 16) & 0xff] << 8) & 0xff000000)
#define TE432(i) (dave_aes_Te0[((i) >> 8) & 0xff] & 0x00ff0000)
#define TE443(i) (dave_aes_Te0[(i) & 0xff] & 0x0000ff00)
#define TE414(i) ((dave_aes_Te0[((i) >> 24) & 0xff] >> 8) & 0x000000ff)
#define TE411(i) ((dave_aes_Te0[((i) >> 24) & 0xff] << 8) & 0xff000000)
#define TE422(i) (dave_aes_Te0[((i) >> 16) & 0xff] & 0x00ff0000)
#define TE433(i) (dave_aes_Te0[((i) >> 8) & 0xff] & 0x0000ff00)
#define TE444(i) ((dave_aes_Te0[(i) & 0xff] >> 8) & 0x000000ff)
#define TE4(i) ((dave_aes_Te0[(i)] >> 8) & 0x000000ff)

#define TD0(i) dave_aes_Td0[((i) >> 24) & 0xff]
#define TD1(i) dave_rotr(dave_aes_Td0[((i) >> 16) & 0xff], 8)
#define TD2(i) dave_rotr(dave_aes_Td0[((i) >> 8) & 0xff], 16)
#define TD3(i) dave_rotr(dave_aes_Td0[(i) & 0xff], 24)
#define TD41(i) (dave_aes_Td4s[((i) >> 24) & 0xff] << 24)
#define TD42(i) (dave_aes_Td4s[((i) >> 16) & 0xff] << 16)
#define TD43(i) (dave_aes_Td4s[((i) >> 8) & 0xff] << 8)
#define TD44(i) (dave_aes_Td4s[(i) & 0xff])
#define TD0_(i) dave_aes_Td0[(i) & 0xff]
#define TD1_(i) dave_rotr(dave_aes_Td0[(i) & 0xff], 8)
#define TD2_(i) dave_rotr(dave_aes_Td0[(i) & 0xff], 16)
#define TD3_(i) dave_rotr(dave_aes_Td0[(i) & 0xff], 24)

#endif /* AES_SMALL_TABLES */

#ifdef _MSC_VER
#define SWAP(x) (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00)
#define GETU32(p) SWAP(*((u32 *)(p)))
#define PUTU32(ct, st) { *((u32 *)(ct)) = SWAP((st)); }
#else
#define GETU32(pt) (((u32)(pt)[0] << 24) ^ ((u32)(pt)[1] << 16) ^ \
((u32)(pt)[2] <<  8) ^ ((u32)(pt)[3]))
#define PUTU32(ct, st) { \
(ct)[0] = (u8)((st) >> 24); (ct)[1] = (u8)((st) >> 16); \
(ct)[2] = (u8)((st) >>  8); (ct)[3] = (u8)(st); }
#endif

#define AES_PRIV_SIZE (4 * 4 * 15 + 4)
#define AES_PRIV_NR_POS (4 * 15)

int dave_rijndaelKeySetupEnc(u32 rk[], const u8 cipherKey[], int keyBits);

#endif /* AES_I_H */

