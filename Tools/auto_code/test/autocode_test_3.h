/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_ENUM_H__
#define __DAVE_ENUM_H__
#include "dave_macro.h"

typedef enum {
	GPSBaseLocation_East,
	GPSBaseLocation_West,
	GPSBaseLocation_North,
	GPSBaseLocation_South
} GPSBaseLocation;

typedef enum {
	CurrencyType_reserve = 0,
	CurrencyType_audio = 1,
	CurrencyType_video = 2,
	CurrencyType_RMB = 3,
	CurrencyType_USD = 4,
	CurrencyType_GBP = 5,
	CurrencyType_JPY = 6,
	CurrencyType_EUR = 7,
	CurrencyType_AUD = 8,
	CurrencyType_DEM = 9,
	CurrencyType_THB = 30,
	CurrencyType_reserve1 = 31,

	CurrencyType_max,
} CurrencyType;

/* http://www.supfree.net/search.asp?id=6073 */

typedef enum {
	CountryCode_reserve = 0,
	CountryCode_Angola = 1,
	CountryCode_Afghanistan = 2,
	CountryCode_Canada_or_USA = 3,
	CountryCode_China = 4,
	CountryCode_Hongkong = 5,
	CountryCode_Albania = 6,
	CountryCode_Algeria = 7,
	CountryCode_Andorra = 8, 
	CountryCode_any,
	CountryCode_unknown,
	CountryCode_error,
	CountryCode_max
} CountryCode;

/* https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes */

typedef enum {
	LanguageCode_Abkhazian,
	LanguageCode_Afar,
	LanguageCode_Traditional_Chinese,
	LanguageCode_Dutch,
	LanguageCode_max
} LanguageCode;

#endif
