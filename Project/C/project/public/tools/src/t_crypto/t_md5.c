#include "dave_base.h"
#include "dave_tools.h"
#include "tools_log.h"

/* MD5C.C - RSA Data Security, Inc., MD5 message-digest algorithm  
 */   
   
/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All  
   rights reserved.  
  
   License to copy and use this software is granted provided that it  
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest  
   Algorithm" in all material mentioning or referencing this software  
   or this function.  
  
   License is also granted to make and use derivative works provided  
   that such works are identified as "derived from the RSA Data  
   Security, Inc. MD5 Message-Digest Algorithm" in all material  
   mentioning or referencing the derived work.    
                                                                      
   RSA Data Security, Inc. makes no representations concerning either  
   the merchantability of this software or the suitability of this  
   software for any particular purpose. It is provided "as is"  
   without express or implied warranty of any kind.    
                                                                      
   These notices must be retained in any copies of any part of this  
   documentation and/or software.    
 */

#ifdef __STDC__
#define UL(x)     x##U
#else 
#define UL(x)     x
#endif 

/* F, G, H and I are basic MD5 functions */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 
// Rotation is separate from addition to prevent recomputation 
#define FF(a, b, c, d, x, s, ac){(a) += F ((b), (c), (d)) + (x) + (unsigned int)(ac);(a) = ROTATE_LEFT ((a), (s));(a) += (b);}
#define GG(a, b, c, d, x, s, ac){(a) += G ((b), (c), (d)) + (x) + (unsigned int)(ac);(a) = ROTATE_LEFT ((a), (s));(a) += (b);}
#define HH(a, b, c, d, x, s, ac){(a) += H ((b), (c), (d)) + (x) + (unsigned int)(ac);(a) = ROTATE_LEFT ((a), (s));(a) += (b);}
#define II(a, b, c, d, x, s, ac){(a) += I ((b), (c), (d)) + (x) + (unsigned int)(ac);(a) = ROTATE_LEFT ((a), (s));(a) += (b);}

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

typedef struct
{
    unsigned int i[2];			/* number of _bits_ handled mod 2^64 */
    unsigned int buf[4];		/* scratch buffer */
    unsigned char in[64];		/* input buffer */
    unsigned char digest[16];	/* actual digest after MD5Final call */
} dave_md5_ctx;

static const unsigned char _PADDING[64] = 
{
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void
_transform(unsigned int *buf, unsigned int *in)
{
    unsigned int a = buf[0], b = buf[1], c = buf[2], d = buf[3];

    /* Round 1 */
    FF(a, b, c, d, in[0], S11, UL(3614090360));
    FF(d, a, b, c, in[1], S12, UL(3905402710));
    FF(c, d, a, b, in[2], S13, UL(606105819));
    FF(b, c, d, a, in[3], S14, UL(3250441966));
    FF(a, b, c, d, in[4], S11, UL(4118548399));
    FF(d, a, b, c, in[5], S12, UL(1200080426));
    FF(c, d, a, b, in[6], S13, UL(2821735955));
    FF(b, c, d, a, in[7], S14, UL(4249261313));
    FF(a, b, c, d, in[8], S11, UL(1770035416));
    FF(d, a, b, c, in[9], S12, UL(2336552879));
    FF(c, d, a, b, in[10], S13, UL(4294925233));
    FF(b, c, d, a, in[11], S14, UL(2304563134));
    FF(a, b, c, d, in[12], S11, UL(1804603682));
    FF(d, a, b, c, in[13], S12, UL(4254626195));
    FF(c, d, a, b, in[14], S13, UL(2792965006));
    FF(b, c, d, a, in[15], S14, UL(1236535329));

    /* Round 2 */
    GG(a, b, c, d, in[1], S21, UL(4129170786));
    GG(d, a, b, c, in[6], S22, UL(3225465664));
    GG(c, d, a, b, in[11], S23, UL(643717713));
    GG(b, c, d, a, in[0], S24, UL(3921069994));
    GG(a, b, c, d, in[5], S21, UL(3593408605));
    GG(d, a, b, c, in[10], S22, UL(38016083));
    GG(c, d, a, b, in[15], S23, UL(3634488961));
    GG(b, c, d, a, in[4], S24, UL(3889429448));
    GG(a, b, c, d, in[9], S21, UL(568446438));
    GG(d, a, b, c, in[14], S22, UL(3275163606));
    GG(c, d, a, b, in[3], S23, UL(4107603335));
    GG(b, c, d, a, in[8], S24, UL(1163531501));
    GG(a, b, c, d, in[13], S21, UL(2850285829));
    GG(d, a, b, c, in[2], S22, UL(4243563512));
    GG(c, d, a, b, in[7], S23, UL(1735328473));
    GG(b, c, d, a, in[12], S24, UL(2368359562));

    /* Round 3 */
    HH(a, b, c, d, in[5], S31, UL(4294588738));
    HH(d, a, b, c, in[8], S32, UL(2272392833));
    HH(c, d, a, b, in[11], S33, UL(1839030562));
    HH(b, c, d, a, in[14], S34, UL(4259657740));
    HH(a, b, c, d, in[1], S31, UL(2763975236));
    HH(d, a, b, c, in[4], S32, UL(1272893353));
    HH(c, d, a, b, in[7], S33, UL(4139469664));
    HH(b, c, d, a, in[10], S34, UL(3200236656));
    HH(a, b, c, d, in[13], S31, UL(681279174));
    HH(d, a, b, c, in[0], S32, UL(3936430074));
    HH(c, d, a, b, in[3], S33, UL(3572445317));
    HH(b, c, d, a, in[6], S34, UL(76029189));
    HH(a, b, c, d, in[9], S31, UL(3654602809));
    HH(d, a, b, c, in[12], S32, UL(3873151461));
    HH(c, d, a, b, in[15], S33, UL(530742520));
    HH(b, c, d, a, in[2], S34, UL(3299628645));

    /* Round 4 */
    II(a, b, c, d, in[0], S41, UL(4096336452));
    II(d, a, b, c, in[7], S42, UL(1126891415));
    II(c, d, a, b, in[14], S43, UL(2878612391));
    II(b, c, d, a, in[5], S44, UL(4237533241));
    II(a, b, c, d, in[12], S41, UL(1700485571));
    II(d, a, b, c, in[3], S42, UL(2399980690));
    II(c, d, a, b, in[10], S43, UL(4293915773));
    II(b, c, d, a, in[1], S44, UL(2240044497));
    II(a, b, c, d, in[8], S41, UL(1873313359));

	II(d, a, b, c, in[15], S42, UL(4264355552));
    II(c, d, a, b, in[6], S43, UL(2734768916));
    II(b, c, d, a, in[13], S44, UL(1309151649));
    II(a, b, c, d, in[4], S41, UL(4149444226));
    II(d, a, b, c, in[11], S42, UL(3174756917));
    II(c, d, a, b, in[2], S43, UL(718787259));
    II(b, c, d, a, in[9], S44, UL(3951481745));

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

static void
_md5_init(dave_md5_ctx *mdContext)
{
    mdContext->i[0] = mdContext->i[1] = (unsigned int) 0;

    /* Load magic initialization constants. */
    mdContext->buf[0] = (unsigned int) 0x67452301;
    mdContext->buf[1] = (unsigned int) 0xefcdab89;
    mdContext->buf[2] = (unsigned int) 0x98badcfe;
    mdContext->buf[3] = (unsigned int) 0x10325476;
}

static void
_md5_update(dave_md5_ctx *mdContext, const unsigned char *inBuf, unsigned int inLen)
{
    unsigned int in[16];
    int mdi;
    unsigned int i, ii;

    /* compute number of bytes mod 64 */
    mdi = (int) ((mdContext->i[0] >> 3) & 0x3F);

    /* update number of bits */
    if ((mdContext->i[0] + ((unsigned int) inLen << 3)) < mdContext->i[0])
    {
        mdContext->i[1]++;
    }
    mdContext->i[0] += ((unsigned int) inLen << 3);
    mdContext->i[1] += ((unsigned int) inLen >> 29);

    while (inLen--)
    {
        /* add new character to buffer, increment mdi */
        mdContext->in[mdi++] = *inBuf++;

        /* transform if necessary */
        if (mdi == 0x40)
        {
            for (i = 0, ii = 0; i < 16; i++, ii += 4)
            {
                in[i] = (((unsigned int) mdContext->in[ii + 3]) << 24) |
                    (((unsigned int) mdContext->in[ii + 2]) << 16) |
                    (((unsigned int) mdContext->in[ii + 1]) << 8) | ((unsigned int) mdContext->in[ii]);
            }
            _transform(mdContext->buf, in);
            mdi = 0;
        }
    }
}

static void
_md5_final(unsigned char hash[], dave_md5_ctx *mdContext)
{
	unsigned int in[16];
	int mdi;
	unsigned int i, ii;
	unsigned int padLen;

	if(mdContext == NULL)
	{
		TOOLSLOG("invalid hash:%lx mdContext:%lx", hash, mdContext);
		return;
	}

	/* save number of bits */
	in[14] = mdContext->i[0];
	in[15] = mdContext->i[1];
	
	/* compute number of bytes mod 64 */
	mdi = (int) ((mdContext->i[0] >> 3) & 0x3F);
	
	/* pad out to 56 mod 64 */
	padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
	_md5_update(mdContext, _PADDING, padLen);
	
	/* append length in bits and transform */
	for (i = 0, ii = 0; i < 14; i++, ii += 4)
	{
		in[i] = (((unsigned int) mdContext->in[ii + 3]) << 24) |
			(((unsigned int) mdContext->in[ii + 2]) << 16) |
			(((unsigned int) mdContext->in[ii + 1]) << 8) | ((unsigned int) mdContext->in[ii]);
	}
	_transform(mdContext->buf, in);
	
	/* store buffer in digest */
	for (i = 0, ii = 0; i < 4; i++, ii += 4)
	{
		mdContext->digest[ii] = (unsigned char) (mdContext->buf[i] & 0xFF);
		mdContext->digest[ii + 1] = (unsigned char) ((mdContext->buf[i] >> 8) & 0xFF);
		mdContext->digest[ii + 2] = (unsigned char) ((mdContext->buf[i] >> 16) & 0xFF);
		mdContext->digest[ii + 3] = (unsigned char) ((mdContext->buf[i] >> 24) & 0xFF);
	}
	dave_memcpy(hash, mdContext->digest, 16);
}

static dave_bool
_t_crypto_md5(u8 md5_hash[DAVE_MD5_HASH_LEN], u8 *encode_ptr, ub encode_len)
{
	dave_md5_ctx *md5_ctx;

	md5_ctx = (dave_md5_ctx *)dave_ralloc(sizeof(dave_md5_ctx));

	_md5_init(md5_ctx);
	_md5_update(md5_ctx, (const unsigned char *)encode_ptr, (unsigned int)encode_len);
	_md5_final(md5_hash, md5_ctx);

	dave_free(md5_ctx);

	return dave_true;
}

// =====================================================================

dave_bool
t_crypto_md5(u8 md5_hash[DAVE_MD5_HASH_LEN], u8 *encode_ptr, ub encode_len)
{
	return _t_crypto_md5(md5_hash, encode_ptr, encode_len);
}

dave_bool
t_crypto_md5_str(s8 md5_str[DAVE_MD5_HASH_STR_LEN], u8 *encode_ptr, ub encode_len)
{
	u8 md5_hash[DAVE_MD5_HASH_LEN];

	if(_t_crypto_md5(md5_hash, encode_ptr, encode_len) == dave_false)
	{
		dave_memset(md5_str, 0x00, DAVE_MD5_HASH_STR_LEN);
		return dave_false;
	}

	t_a2b_bin_to_hex_string(md5_str, DAVE_MD5_HASH_STR_LEN, md5_hash, DAVE_MD5_HASH_LEN);

	return dave_true;
}

