/*
 * ================================================================================
 * (c) Copyright 2016 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
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
	UserType_reserve = 0,
	UserType_share = 1,
	UserType_private = 2,
	UserType_jegoboss = 3,
	UserType_NIM = 4,
	UserType_RNM = 5,
	UserType_AUX = 6, // Auxiliary number user
	UserType_ENT = 7, // Enterprise user
	UserType_max
} UserType;

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

typedef enum {
	PhoneNumberType_unkonw = 0,
	PhoneNumberType_sip = 1,
	PhoneNumberType_tel = 2,
	PhoneNumberType_empty = 3,
	PhoneNumberType_max
} PhoneNumberType;

typedef enum {
	BillingRulesAttrib_reserve = 0,
	BillingRulesAttrib_share = 1,
	BillingRulesAttrib_private = 2,
	BillingRulesAttrib_max
} BillingRulesAttrib;

typedef enum {
	BillingRulesPriority_reserve = 0,
	/* Type has priority, the smaller the value the greater the priority */
	BillingRulesPriority_added_value = 1,
	BillingRulesPriority_free = 2,
	BillingRulesPriority_preferential = 3,
	BillingRulesPriority_toll = 4,
	BillingRulesPriority_max
} BillingRulesPriority;

#endif
