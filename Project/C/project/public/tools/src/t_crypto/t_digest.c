#include "dave_base.h"
#include "dave_tools.h"
#include "GLOBAL.H"
#include "crypto_MD5.H"

#define MD5Init crypto_MD5Init
#define MD5Update crypto_MD5Update
#define MD5Final crypto_MD5Final

#define HASHLEN 16
typedef char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];

static void CvtHex(
    HASH Bin,
    HASHHEX Hex
    )
{
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++) {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
            Hex[i*2] = (j + '0');
         else
            Hex[i*2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
            Hex[i*2+1] = (j + '0');
         else
            Hex[i*2+1] = (j + 'a' - 10);
    };
    Hex[HASHHEXLEN] = '\0';
};

/* calculate H(A1) as per spec */
static void DigestCalcHA1(
    unsigned char * pszAlg,
    unsigned char * pszUserName,
    unsigned char * pszRealm,
    unsigned char * pszPassword,
    unsigned char * pszNonce,
    unsigned char * pszCNonce,
    HASHHEX SessionKey
    )
{
      MD5_CTX Md5Ctx;
      HASH HA1;

      MD5Init(&Md5Ctx);
      MD5Update(&Md5Ctx, pszUserName, dave_strlen(pszUserName));
      MD5Update(&Md5Ctx, (unsigned char *)":", 1);
      MD5Update(&Md5Ctx, pszRealm, dave_strlen(pszRealm));
      MD5Update(&Md5Ctx, (unsigned char *)":", 1);
      MD5Update(&Md5Ctx, pszPassword, dave_strlen(pszPassword));
      MD5Final((unsigned char *)HA1, &Md5Ctx);
      if (dave_strcmp(pszAlg, "md5-sess") == dave_true) {
            MD5Init(&Md5Ctx);
            MD5Update(&Md5Ctx, (unsigned char *)HA1, HASHLEN);
            MD5Update(&Md5Ctx, (unsigned char *)":", 1);
            MD5Update(&Md5Ctx, pszNonce, dave_strlen(pszNonce));
            MD5Update(&Md5Ctx, (unsigned char *)":", 1);
            MD5Update(&Md5Ctx, pszCNonce, dave_strlen(pszCNonce));
            MD5Final((unsigned char *)HA1, &Md5Ctx);
      };
      CvtHex(HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
static void DigestCalcResponse(
    HASHHEX HA1,           /* H(A1) */
    unsigned char * pszNonce,       /* nonce from server */
    unsigned char * pszNonceCount,  /* 8 hex digits */
    unsigned char * pszCNonce,      /* client nonce */
    unsigned char * pszQop,         /* qop-value: "", "auth", "auth-int" */
    unsigned char * pszMethod,      /* method from the request */
    unsigned char * pszDigestUri,   /* requested URL */
    HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
    HASHHEX Response      /* request-digest or response-digest */
    )
{
      MD5_CTX Md5Ctx;
      HASH HA2;
      HASH RespHash;
      HASHHEX HA2Hex;

      // calculate H(A2)
      MD5Init(&Md5Ctx);
      MD5Update(&Md5Ctx, pszMethod, dave_strlen(pszMethod));
      MD5Update(&Md5Ctx, (unsigned char *)":", 1);
      MD5Update(&Md5Ctx, pszDigestUri, dave_strlen(pszDigestUri));
      if (dave_strcmp(pszQop, "auth-int") == dave_true) {
            MD5Update(&Md5Ctx, (unsigned char *)":", 1);
            MD5Update(&Md5Ctx, (unsigned char *)HEntity, HASHHEXLEN);
      };
      MD5Final((unsigned char *)HA2, &Md5Ctx);
      CvtHex(HA2, HA2Hex);

      // calculate response
      MD5Init(&Md5Ctx);
      MD5Update(&Md5Ctx, (unsigned char *)HA1, HASHHEXLEN);
      MD5Update(&Md5Ctx, (unsigned char *)":", 1);
      MD5Update(&Md5Ctx, pszNonce, dave_strlen(pszNonce));
      MD5Update(&Md5Ctx, (unsigned char *)":", 1);
      if (*pszQop) {
          MD5Update(&Md5Ctx, pszNonceCount, dave_strlen(pszNonceCount));
          MD5Update(&Md5Ctx, (unsigned char *)":", 1);
          MD5Update(&Md5Ctx, pszCNonce, dave_strlen(pszCNonce));
          MD5Update(&Md5Ctx, (unsigned char *)":", 1);
          MD5Update(&Md5Ctx, pszQop, dave_strlen(pszQop));
          MD5Update(&Md5Ctx, (unsigned char *)":", 1);
      };
      MD5Update(&Md5Ctx, (unsigned char *)HA2Hex, HASHHEXLEN);
      MD5Final((unsigned char *)RespHash, &Md5Ctx);
      CvtHex(RespHash, Response);
}

// =====================================================================

s8 *
t_crypto_digest(s8 *nonce, s8 *cnonce, s8 *user, s8 *realm, s8 *pwd, s8 *nc, s8 *method, s8 *qop, s8 *uri)
{
	// https://www.ietf.org/rfc/rfc2617.txt
	static HASHHEX Response;
	unsigned char *pszAlg = (unsigned char *)"md5";
	HASHHEX HA1;
	HASHHEX HA2 = "";

	DigestCalcHA1(pszAlg, (unsigned char *)user, (unsigned char *)realm, (unsigned char *)pwd, (unsigned char *)nonce, (unsigned char *)cnonce, HA1);

	DigestCalcResponse(HA1, (unsigned char *)nonce, (unsigned char *)nc, (unsigned char *)cnonce, (unsigned char *)qop, (unsigned char *)method, (unsigned char *)uri, HA2, Response);

	return (s8 *)Response;
}
