/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "tools_macro.h"
#ifdef __DAVE_TOOLS__
#include "dave_os.h"
#include "dave_base.h"
#include "dave_tools.h"
#include "tools_log.h"

static const char BASE_CODE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char
_GetCharIndex(char c)
{
	if((c >= 'A') && (c <= 'Z')) {   
		return c - 'A';  
    } else if((c >= 'a') && (c <= 'z')) {
    	return c - 'a' + 26;
    } else if((c >= '0') && (c <= '9')) {
    	return c - '0' + 52;
    } else if(c == '+') {
    	return 62;  
    } else if(c == '/') {
    	return 63;
    } else if(c == '=') {
    	return 0;  
    }
    return 0;  
}

// =====================================================================

ub
t_base64_encode(const u8 *in, ub inlen, s8 *out, ub outlen)
{
	register ub vLen = 0;

	while((inlen > 0) && ((vLen + 4) < outlen))
	{
		*out++ = BASE_CODE[(in[0] >> 2 ) & 0x3F];

		if(inlen >= 3)
		{
			*out++ = BASE_CODE[((in[0] & 3) << 4) | (in[1] >> 4)];  
			*out++ = BASE_CODE[((in[1] & 0xF) << 2) | (in[2] >> 6)];	
			*out++ = BASE_CODE[in[2] & 0x3F];

			in += 3; inlen -= 3;
		}
		else
		{
			switch(inlen)
			{
				case 1:  
						*out ++ = BASE_CODE[(in[0] & 3) << 4 ];  
						*out ++ = '=';  
						*out ++ = '=';

						in += 1; inlen -= 1;
					break;
				case 2:  
						*out ++ = BASE_CODE[((in[0] & 3) << 4) | (in[1] >> 4)];  
						*out ++ = BASE_CODE[((in[1] & 0x0F) << 2)];  
						*out ++ = '=';

						in += 2; inlen -= 2;
					break;
				default:
						inlen = 0;
					break;
			}
		}

		vLen +=4;	
	}

	*out = '\0';

	return vLen;
}

dave_bool
t_base64_decode(const s8 *in, ub inlen, u8 *out, ub *outlen)
{
	char lpCode[4];
	register ub vLen = 0;

	if(inlen % 4)
	{
		out[0] = '\0';
		*outlen = 0;
		TOOLSLOG("invalid inlen:%d", inlen);
		return dave_false;
	}

	while((inlen >= 4) && ((*outlen) >= (vLen + 3)))
	{
		lpCode[0] = _GetCharIndex(in[0]);
		lpCode[1] = _GetCharIndex(in[1]);
		lpCode[2] = _GetCharIndex(in[2]);
		lpCode[3] = _GetCharIndex(in[3]);

		*out++ = (lpCode[0] << 2) | (lpCode[1] >> 4);	
		*out++ = (lpCode[1] << 4) | (lpCode[2] >> 2);	
		*out++ = (lpCode[2] << 6) | (lpCode[3]);

		in += 4;  
		inlen -= 4;	
		vLen += 3;	
	}

	if((*outlen) > vLen)
	{
		*out = '\0';
	}

	*outlen = vLen;

	return dave_true;
}

#endif

