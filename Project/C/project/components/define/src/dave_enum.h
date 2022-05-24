/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_ENUM_H__
#define __DAVE_ENUM_H__

typedef enum {
	POWERMODE_KEYPAD = 0,
	POWERMODE_ALARM,
	POWERMODE_CHARGER_IN,
	POWERMODE_EXCEPTION,
	POWERMODE_USB,
	POWERMODE_UNINIT,
	POWERMODE_MAX
} PowerMode;

typedef enum {
	PRODUC_DEBUG = 0,
	PRODUC_VOIP,
	PRODUC_OCS,
	PRODUC_JEGOM,
	PRODUC_LOG,
	PRODUC_SYNC,
	PRODUC_BDATA,
	PRODUC_AA,
	PRODUC_IMB,
	PRODUC_SMS,
	PRODUC_AIA,
	PRODUC_AIB,
	PRODUC_AIC,
	PRODUC_IO,
	PRODUC_DBA,
	PRODUC_MNR,
	PRODUC_AIX,
	PRODUC_CSCF,
	PRODUC_BLB,
	PRODUC_AID,
	PRODUC_JEGOR,
	PRODUC_reserve_1,
	PRODUC_BBS,
	PRODUC_reserve_2,
	PRODUC_BASE,
	PRODUC_SMPPDOCK,
	PRODUC_IOPOST,
	PRODUC_SMSHTTP,
	PRODUC_SMPPS,
	PRODUC_AUDIOOCS,
	PRODUC_SMSOCS,
	PRODUC_MAX
} PRODUC;

typedef enum {
	VER_ALPHA = 0,
	VER_BETA,
	VER_RC,
	VER_RELEASE,
	VER_MAX
} VERLEVEL;

typedef enum {
	SyncNotify_read,
	SyncNotify_write_free,
	SyncNotify_dma_read,
	SyncNotify_dma_write_free,
	SyncNotify_device_plugin,
	SyncNotify_device_plugout,
	SyncNotifyEnum_max
} SyncNotifyEnum;

#define WAIT_SOCKET_STATE (-2)

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
	CurrencyType_CHF = 10,
	CurrencyType_FRF = 11,
	CurrencyType_CAD = 12,
	CurrencyType_HKD = 13,
	CurrencyType_ATS = 14,
	CurrencyType_FIM = 15,
	CurrencyType_BEF = 16,
	CurrencyType_NZD = 17,
	CurrencyType_SGD = 18,
	CurrencyType_KRW = 19,
	CurrencyType_IEP = 20,
	CurrencyType_ITL = 21,
	CurrencyType_LUF = 22,
	CurrencyType_NLG = 23,
	CurrencyType_PTE = 24,
	CurrencyType_ESP = 25,
	CurrencyType_IDR = 26,
	CurrencyType_MYR = 27,
	CurrencyType_PHP = 28,
	CurrencyType_SUR = 29,
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
	CountryCode_Anguilla = 9, 
	CountryCode_Antigua_and_Barbuda = 10,
	CountryCode_Argentina = 11,
	CountryCode_Armenia = 12,
	CountryCode_Ascension = 13,
	CountryCode_Australia = 14,
	CountryCode_Austria = 15,
	CountryCode_Azerbaijan = 16, 
	CountryCode_Bahamas = 17,
	CountryCode_Bahrain = 18,
	CountryCode_Bangladesh = 19,
	CountryCode_Barbados = 20,
	CountryCode_Belarus = 21,
	CountryCode_Belgium = 22,
	CountryCode_Belize = 23,
	CountryCode_Benin = 24,
	CountryCode_Bermuda = 25,
	CountryCode_Bolivia = 26,
	CountryCode_Botswana = 27,
	CountryCode_Brazil = 28,
	CountryCode_Brunei = 29,
	CountryCode_Bulgaria = 30,
	CountryCode_Burkina_faso = 31,
	CountryCode_Burma = 32,
	CountryCode_Burundi = 33,
	CountryCode_Cameroon = 34,
	CountryCode_Cayman = 35,
	CountryCode_Central_African = 36,
	CountryCode_Chad = 37,
	CountryCode_Chile = 38,
	CountryCode_Colombia = 39,
	CountryCode_Congo = 40,
	CountryCode_Cook = 41,
	CountryCode_Costa_Rica = 42,
	CountryCode_Cuba = 43,
	CountryCode_Cyprus = 44,
	CountryCode_Czech = 45,
	CountryCode_Denmark = 46,
	CountryCode_Djibouti = 47,
	CountryCode_Dominica = 48,
	CountryCode_Ecuador = 49,
	CountryCode_Egypt = 50,
	CountryCode_Salvador = 51,
	CountryCode_Estonia = 52,
	CountryCode_Ethiopia = 53,
	CountryCode_Fiji = 54,
	CountryCode_Finland = 55,
	CountryCode_France = 56,
	CountryCode_French_Guiana = 57,
	CountryCode_Gabon = 58,
	CountryCode_Gambia = 59,
	CountryCode_Georgia = 60,
	CountryCode_Germany = 61,
	CountryCode_Ghana = 62,
	CountryCode_Gibraltar = 63,
	CountryCode_Greece = 64,
	CountryCode_Grenada = 65,
	CountryCode_Guam = 66,
	CountryCode_Guatemala = 67,
	CountryCode_Guinea = 68,
	CountryCode_Guyana = 69,
	CountryCode_Haiti  = 70, 
	CountryCode_Honduras = 71,
	CountryCode_Hungary = 72,
	CountryCode_Iceland = 73,
	CountryCode_India = 74,
	CountryCode_Indonesia = 75,
	CountryCode_Iran = 76,
	CountryCode_Iraq = 77, 
	CountryCode_Ireland = 78,
	CountryCode_Israel = 79,
	CountryCode_Italy = 80, 
	CountryCode_Ivory_Coast = 81, 
	CountryCode_Jamaica = 82, 
	CountryCode_Japan = 83, 
	CountryCode_Jordan = 84, 
	CountryCode_Cambodia = 85,
	CountryCode_Kazakstan = 86,
	CountryCode_Kenya = 87,
	CountryCode_South_Korea = 88,
	CountryCode_Kuwait = 89,
	CountryCode_Kyrgyzstan	= 90, 
	CountryCode_Laos = 91, 
	CountryCode_Latvia = 92,
	CountryCode_Lebanon = 93, 
	CountryCode_Lesotho = 94, 
	CountryCode_Liberia = 95,
	CountryCode_Libya = 96,
	CountryCode_Liechtenstein = 97, 
	CountryCode_Lithuania = 98, 
	CountryCode_Luxembourg = 99, 
	CountryCode_Macao = 100, 
	CountryCode_Madagascar = 101, 
	CountryCode_Malawi = 102, 
	CountryCode_Malaysia = 103, 
	CountryCode_Maldives = 104,
	CountryCode_Mali = 105, 
	CountryCode_Malta = 106,
	CountryCode_Mariana = 107, 
	CountryCode_Martinique = 108, 
	CountryCode_Mauritius = 109, 
	CountryCode_Mexico = 110, 
	CountryCode_Moldova = 111, 
	CountryCode_Monaco = 112, 
	CountryCode_Mongolia  = 113, 
	CountryCode_Montserrat = 114,
	CountryCode_Morocco = 115, 
	CountryCode_Mozambique = 116, 
	CountryCode_Namibia  = 117, 
	CountryCode_Nauru = 118, 
	CountryCode_Nepal = 119, 
	CountryCode_Netheriands_Antilles = 120, 
	CountryCode_Netherlands = 121, 
	CountryCode_New_Zealand = 122, 
	CountryCode_Nicaragua = 123, 
	CountryCode_Niger = 124,
	CountryCode_Nigeria = 125, 
	CountryCode_North_Korea = 126, 
	CountryCode_Norway = 127, 
	CountryCode_Oman = 128, 
	CountryCode_Pakistan = 129,
	CountryCode_Panama = 130, 
	CountryCode_Papua_New_Cuinea = 131,
	CountryCode_Paraguay = 132, 
	CountryCode_Peru = 133,
	CountryCode_Philippines = 134, 
	CountryCode_Poland = 135, 
	CountryCode_French_Polynesia = 136, 
	CountryCode_Portugal = 137, 
	CountryCode_Puerto_Rico = 138,
	CountryCode_Qatar = 139,
	CountryCode_Reunion = 140, 
	CountryCode_Romania = 141, 
	CountryCode_Russia = 142, 
	CountryCode_Saint_Lueia = 143, 
	CountryCode_Saint_Vincent = 144, 
	CountryCode_Samoa_Eastern = 145, 
	CountryCode_Samoa_Western = 146, 
	CountryCode_San_Marino = 147, 
	CountryCode_Sao_Tome_and_Principe = 148, 
	CountryCode_Saudi_Arabia = 149, 
	CountryCode_Senegal = 150, 
	CountryCode_Seychelles = 151, 
	CountryCode_Sierra_Leone = 152, 
	CountryCode_Singapore = 153,
	CountryCode_Slovakia = 154, 
	CountryCode_Slovenia = 155,
	CountryCode_Solomon = 156, 
	CountryCode_Somali = 157, 
	CountryCode_South_Africa = 158, 
	CountryCode_Spain = 159, 
	CountryCode_Sri_Lanka = 160, 
	CountryCode_St_Lucia = 161, 
	CountryCode_St_Vincent = 162, 
	CountryCode_Sudan = 163, 
	CountryCode_Suriname = 164, 
	CountryCode_Swaziland = 165, 
	CountryCode_Sweden = 166, 
	CountryCode_Switzerland = 167, 
	CountryCode_Syria = 168, 
	CountryCode_Taiwan = 169, 
	CountryCode_Tajikstan = 170,
	CountryCode_Tanzania = 171, 
	CountryCode_Thailand = 172, 
	CountryCode_Togo = 173, 
	CountryCode_Tonga = 174, 
	CountryCode_Trinidad_and_Tobago = 175, 
	CountryCode_Tunisia = 176, 
	CountryCode_Turkey = 177,
	CountryCode_Turkmenistan = 178, 
	CountryCode_Uganda = 179, 
	CountryCode_Ukraine = 180, 
	CountryCode_United_Arab_Emirates = 181, 
	CountryCode_United_Kingdom = 182, 
	CountryCode_Uruguay = 183, 
	CountryCode_Uzbekistan = 184, 
	CountryCode_Venezuela = 185, 
	CountryCode_Vietnam = 186, 
	CountryCode_Yemen = 187,
	CountryCode_Yugoslavia = 188,
	CountryCode_Zimbabwe = 189,
	CountryCode_Zaire = 190,
	CountryCode_Zambia = 191,
	CountryCode_Canada = 192,
	CountryCode_USA = 193,
	CountryCode_Kampuchea = 194,
	CountryCode_any,
	CountryCode_unknown,
	CountryCode_error,
	CountryCode_max
} CountryCode;

/* https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes */

typedef enum {
	LanguageCode_Abkhazian,
	LanguageCode_Afar,
	LanguageCode_Afrikaans,
	LanguageCode_Akan,
	LanguageCode_Albanian,
	LanguageCode_Amharic,
	LanguageCode_Arabic,
	LanguageCode_Aragonese,
	LanguageCode_Armenian,
	LanguageCode_Assamese,
	LanguageCode_Avaric,
	LanguageCode_Avestan,
	LanguageCode_Aymara,
	LanguageCode_Chinese,
	LanguageCode_Chuvash,
	LanguageCode_Cornish,
	LanguageCode_Corsican,
	LanguageCode_English,
	LanguageCode_French,
	LanguageCode_German,
	LanguageCode_Italian,
	LanguageCode_Japanese,
	LanguageCode_Korean,
	LanguageCode_Russian,
	LanguageCode_Spanish_Castilian,
	LanguageCode_Sundanese,
	LanguageCode_Swahili,
	LanguageCode_Swati,
	LanguageCode_Swedish,
	LanguageCode_Tamil,
	LanguageCode_Telugu,
	LanguageCode_Tajik,
	LanguageCode_Thai,
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

typedef enum {
	BillingRulesType_reserve = 0,
	BillingRulesType_atomic = 1,
	BillingRulesType_reserved1 = 2,
	BillingRulesType_period_day = 3,
	BillingRulesType_reserved2 = 4,
	BillingRulesType_period_month = 5,
	BillingRulesType_reserved3 = 6,
	BillingRulesType_oveylay = 7,
	BillingRulesType_oveylay_user_customized_time = 8,
	BillingRulesType_group_user_customized_value = 9,
	BillingRulesType_oveylay_user_customized_time_and_value = 10,
	BillingRulesType_oveylay_user_customized_time_and_oveylay_value = 11,
	BillingRulesType_conference_oveylay_user_customized_time = 12,
	BillingRulesType_conference_oveylay_user_customized_time_and_oveylay_value = 13,
	BillingRulesType_group_period_month_user_customized_time = 14,
	BillingRulesType_oveylay_user_customized_and_free_time = 15,
	BillingRulesType_oveylay_self_customized_time = 16,
	BillingRulesType_oveylay_self_customized_time_and_free_time = 17,
	BillingRulesType_group_period_month_self_customized_time = 18,
	BillingRulesType_max
} BillingRulesType;

typedef enum {
	BillingAddressType_reserve = 0,
	BillingAddressType_sip = 1,
	BillingAddressType_tel = 2,
	BillingAddressType_ip = 3,
	BillingAddressType_url = 4,
	BillingAddressType_all = 5,
	BillingAddressType_sip_or_tel = 6,
	BillingAddressType_max
} BillingAddressType;

typedef enum {
	BillingMeasurementUnit_reserve = 0,
	BillingMeasurementUnit_seconds = 1,
	BillingMeasurementUnit_byte = 2,
	BillingMeasurementUnit_server = 3,
	BillingMeasurementUnit_max
} BillingMeasurementUnit;

typedef enum {
	BillingEvent_Idle = 0,
	BillingEvent_Initial,
	BillingEvent_Update,
	BillingEvent_Termination,
	BillingEvent_Total,
	BillingEvent_max
} BillingEvent;

typedef enum {
	BillingType_reserve = 0,
	BillingType_Caller = 1,
	BillingType_Called = 2,
	BillingType_Multi_Party = 3,
	BillingType_Strange = 4,
	BillingType_Failure_Account = 5,
	BillingType_VtoV = 6,
	BillingType_CtoC = 7,
	BillingType_anonymous = 8,
	BillingType_call_forwarding = 9,
	BillingType_max
} BillingType;

typedef enum {
	RNMBusinessType_add = 0,
	RNMBusinessType_del,
	RNMBusinessType_active,
	RNMBusinessType_deactive,
	RNMBusinessType_inquire,	
	RNMBusinessType_notify,
} RNMBusinessType;

typedef enum {
	RNMQueryType_all = 0,
	RNMQueryType_register,
	RNMQueryType_unregister,
	RNMQueryType_max
} RNMQueryType;

typedef enum {
	GuardianMailLevel_Developer = 0,
	GuardianMailLevel_Statistician,
	GuardianMailLevel_max
} GuardianMailLevel;

/* Smpp_v3_4.pdf 5.2.19 data_coding */

typedef enum {
	TextCoding_SMSC_Default_Alphabet = 0x00,	/* 7 bit format */
	TextCoding_Ascii 	= 0x01,
	TextCoding_Latin1 	= 0x03,
	TextCoding_Jis_x_0208_1990 = 0x05,
	TextCoding_Cyrillic = 0x06,
	TextCoding_iso_8859_8 = 0x07,
	TextCoding_UCS2 	= 0x08,
	TextCoding_iso_2022_jp = 0x0A,
	TextCoding_jis_x0212_1990 = 0x0D,
	TextCoding_jis_ksc_5601 = 0x0E,
	TextCoding_UTF8 = 0x0f,	/*0x0f:Custom, please replace if it conflicts with the standard*/
	TextCoding_max = 0xff,
} TextCoding;

typedef enum {
	SMSCoding_idle = 0,
	SMSCoding_utf8_to_asscii,
	SMSCoding_utf8_to_ucs2,
	SMSCoding_ucse_to_utf8,
	SMSCoding_unchanged,
	SMSCoding_latin1_to_utf8,
	SMSCoding_utf8_to_latin1,
	SMSCoding_gsm_to_utf8,
	SMSCoding_utf8_to_gsm,
	SMSCoding_Jis_x_0208_1990_to_utf8,
	SMSCoding_Cyrillic_to_utf8,
	SMSCoding_iso_8859_8_to_utf8,
	SMSCoding_iso_2022_jp_to_utf8,
	SMSCoding_jis_x0212_1990_to_utf8,
	SMSCoding_jis_ksc_5601_to_utf8,
	SMSCoding_Max
} SMSCodingType;

typedef enum {
	singature_type_idle  = 0,
	head_boldface_square = 1,
	tail_boldface_square = 2,
	head_square_brackets = 3,
	tail_square_brackets = 4,
	stay_the_same = 5,
	singature_type_max
} SMSSignatureType;

typedef enum {
	MOD_IMPU_FLAG_OPEN = 1,
	MOD_IMPU_FLAG_CLOSE = 2,
	MOD_IMPU_FLAG_max
} ModImpuFlag;

typedef enum {
	NumberReqType_PURCHASE_E164_NUMBER = 1,
	NumberReqType_CANCEL_PURCHASE_E164_NUMBER = 2,
	NumberReqType_SET_REG_NUMBER_AS_CALLING_NUMBER = 3,
	NumberReqType_SET_E164_NUMBER_AS_CALLING_NUMBER = 4,
	NumberReqType_ASS_E164_NUMBER = 5,
	NumberReqType_INQ_RECORD = 6,
	NumberReqType_RET_NUMBER = 7,
	NumberReqType_FIRST_PURCHASE_E164_NUMBER = 8,
	NumberReqType_RE_PURCHASE_E164_NUMBER = 9,
	NumberReqType_PRE_PURCHASE_AUX_NUMBER = 10,
	NumberReqType_max
} NumberReqType;

typedef enum {
	NumberType_HOST_NUMBER = 1,
	NumberType_AUXILIARY_NUMBER = 2,	
	NumberType_VIRTUAL_CALLING_NUMBER = 3,
	NumberType_VOIP_NUMBER = 4,
	NumberType_max
} NumberType;

typedef enum {
	CallingNumberFlag_OPEN = 1,
	CallingNumberFlag_CLOSE = 2,
	CallingNumberFlag_max
} CallingNumberFlag;

typedef enum {
	AuthType_sha1,
	AuthType_max
} AuthType;

typedef enum {
	BillingRechargeType_add,
	BillingRechargeType_del,
	BillingRechargeType_inq,
	BillingRechargeType_add_value,
	BillingRechargeType_del_value,
	BillingRechargeType_del_all,		/* Delete account data for a single user */
	BillingRechargeType_inq_all,		/* Query account data of all users */
	BillingRechargeType_max
} BillingRechargeType;

typedef enum {
	MainReqSrcType_FromUI = 1,
	MainReqSrcType_FromUIP = 2,	
	MainReqSrcType_max
} MainReqSrcType;

typedef enum {
	ReportMsgType_USER_CREAT = 1,
	ReportMsgType_AUX_NUMBER_ASS = 2,
	ReportMsgType_NUMBER_INQ = 3,
	ReportMsgType_max
} ReportMsgType;

typedef enum {
	DBInqType_BillingUserInfo = 1,
	DBInqType_NumberInfo = 2,
	DBInqType_RnmInfo = 3,
	DBInqType_BasicUserInfo = 4,
	DBInqType_All = 5,
	DBInqType_max
} DBInqType;

typedef enum {
	VoIPForwardSetting_Call_Only = 1,
	VoIPForwardSetting_SMS_Only = 2,
	VoIPForwardSetting_Call_And_SMS = 3,
	VoIPForwardSetting_CLOSE = 4,
	VoIPForwardSetting_max
} VoIPForwardSettingType;

typedef enum {
	AIVisionType_TYPE_UNSPECIFIED = 0x0000000000000001,
	AIVisionType_FACE_DETECTION = 0x0000000000000002,
	AIVisionType_LANDMARK_DETECTION = 0x0000000000000004,
	AIVisionType_LOGO_DETECTION = 0x0000000000000008,
	AIVisionType_LABEL_DETECTION = 0x0000000000000010,
	AIVisionType_TEXT_DETECTION = 0x0000000000000020,
	AIVisionType_DOCUMENT_TEXT_DETECTION = 0x0000000000000040,
	AIVisionType_SAFE_SEARCH_DETECTION = 0x0000000000000080,
	AIVisionType_IMAGE_PROPERTIES = 0x0000000000000100,
	AIVisionType_CROP_HINTS = 0x0000000000000200,
	AIVisionType_WEB_DETECTION = 0x0000000000000400,
} AIVisionType;

/* https://developers.google.com/places/web-service/supported_types */

typedef enum {
	AIPlaceType_accounting,
	AIPlaceType_airport,
	AIPlaceType_amusement_park,
	AIPlaceType_aquarium,
	AIPlaceType_art_gallery,
	AIPlaceType_atm,
	AIPlaceType_bakery,
	AIPlaceType_bank,
	AIPlaceType_bar,
	AIPlaceType_beauty_salon,
	AIPlaceType_bicycle_store,
	AIPlaceType_book_store,
	AIPlaceType_bowling_alley,
	AIPlaceType_bus_station,
	AIPlaceType_cafe,
	AIPlaceType_campground,
	AIPlaceType_car_dealer,
	AIPlaceType_car_rental,
	AIPlaceType_car_repair,
	AIPlaceType_car_wash,
	AIPlaceType_casino,
	AIPlaceType_cemetery,
	AIPlaceType_church,
	AIPlaceType_city_hall,
	AIPlaceType_clothing_store,
	AIPlaceType_convenience_store,
	AIPlaceType_courthouse,
	AIPlaceType_dentist,
	AIPlaceType_department_store,
	AIPlaceType_doctor,
	AIPlaceType_electrician,
	AIPlaceType_electronics_store,
	AIPlaceType_embassy,
	AIPlaceType_fire_station,
	AIPlaceType_florist,
	AIPlaceType_funeral_home,
	AIPlaceType_furniture_store,
	AIPlaceType_gas_station,
	AIPlaceType_gym,
	AIPlaceType_hair_care,
	AIPlaceType_hardware_store,
	AIPlaceType_hindu_temple,
	AIPlaceType_home_goods_store,
	AIPlaceType_hospital,
	AIPlaceType_insurance_agency,
	AIPlaceType_jewelry_store,
	AIPlaceType_laundry,
	AIPlaceType_lawyer,
	AIPlaceType_library,
	AIPlaceType_liquor_store,
	AIPlaceType_local_government_office,
	AIPlaceType_locksmith,
	AIPlaceType_lodging,
	AIPlaceType_meal_delivery,
	AIPlaceType_meal_takeaway,
	AIPlaceType_mosque,
	AIPlaceType_movie_rental,
	AIPlaceType_movie_theater,
	AIPlaceType_moving_company,
	AIPlaceType_museum,
	AIPlaceType_night_club,
	AIPlaceType_painter,
	AIPlaceType_park,
	AIPlaceType_parking,
	AIPlaceType_pet_store,
	AIPlaceType_pharmacy,
	AIPlaceType_physiotherapist,
	AIPlaceType_plumber,
	AIPlaceType_police,
	AIPlaceType_post_office,
	AIPlaceType_real_estate_agency,
	AIPlaceType_restaurant,
	AIPlaceType_roofing_contractor,
	AIPlaceType_rv_park,
	AIPlaceType_school,
	AIPlaceType_shoe_store,
	AIPlaceType_shopping_mall,
	AIPlaceType_spa,
	AIPlaceType_stadium,
	AIPlaceType_storage,
	AIPlaceType_store,
	AIPlaceType_subway_station,
	AIPlaceType_synagogue,
	AIPlaceType_taxi_stand,
	AIPlaceType_train_station,
	AIPlaceType_transit_station,
	AIPlaceType_travel_agency,
	AIPlaceType_university,
	AIPlaceType_veterinary_care,
	AIPlaceType_zoo,

	AIPlaceType_max = 0xffffffffffffffff
} AIPlaceType;

typedef enum {
	CallFunctionType_1,
} CallFunctionType;

typedef enum {
	SIPCallType_caller,
	SIPCallType_called,
	SIPCallType_conf,
    SIPCallType_message,
	SIPCallType_max
} SIPCallType;

typedef enum {
	SIPConfType_direct_conf_room = 0,
	SIPConfType_hold_call_conf_room_CSBA,
	SIPConfType_hold_call_conf_room_CSBT,
} SIPConfType;

typedef enum {
	SIPTransportType_udp,
	SIPTransportType_tcp,
	SIPTransportType_tls,
	SIPTransportType_max = 0xffffffffffffffff
} SIPTransportType;

typedef enum {
	UIPSocketType_binding_client,
	UIPSocketType_sms_client,
	UIPSocketType_max
} UIPSocketType;

typedef enum {
	SMSLowPriority=0,
	SMSHigherPriority,
	SMSFirstPriority,
	SMSPriority_MAX
} SMSPriority;

typedef enum {
	SMSType_MKT=0,
	SMSType_NOTIFY,
	SMSType_OTP,
	SMSTypeMax,
	SMSTypeInvalid = 0x1fffffff
} SMSType;

typedef enum {
	RESTPushEventType_power_on = 1,
	RESTPushEventType_kick_off = 2,
	RESTPushEventType_others = 3,
	RESTPushEventType_max
} RESTPushEventType;

typedef enum {
	AES_KEY_128 = 128,
	AES_KEY_192 = 192,
	AES_KEY_256 = 256
} AESKeyBin;

typedef enum {
	MsisdnUnknown,
	ChinaMobile,
	ChinaUnicom,
	ChinaTelecom
}MsisdnOperator;

typedef enum {
	AudioCodec_G711 = 0,
	AudioCodec_G722,
	AudioCodec_G7221,
	AudioCodec_G729,
	AudioCodec_iLBC,
	AudioCodec_GSM,
	AudioCodec_H263,
	AudioCodec_H264,
	AudioCodec_max
} AudioCodec;

typedef enum {
	EnvironmentFlag_test,
	EnvironmentFlag_STG,
	EnvironmentFlag_Product,
	EnvironmentFlag_max = 0x100000000
} EnvironmentFlag;

typedef enum {
	SMSMsgState_Init = 0,
	SMSMsgState_ENROUTE = 1,
	SMSMsgState_DELIVERED = 2,
	SMSMsgState_EXPIRED = 3,
	SMSMsgState_DELETED = 4,
	SMSMsgState_UNDELIVERABLE = 5,
	SMSMsgState_ACCEPTED = 6,
	SMSMsgState_UNKNOWN = 7,
	SMSMsgState_REJECTED = 8,
	SMSMsgState_LOCAL_REJECTED = 60,
	SMSMsgState_TIMEOUT = 99
} SMSMsgState;

typedef enum {
	SMSOTCODE_IDLE,
	/* This matches mccmnc */
	SMSOTCODE_CN_mobile     = 46000,
	SMSOTCODE_CN_unicom     = 46001,
	SMSOTCODE_CN_telecom    = 46003,
	/* This is a custom value */
	SMSOTCODE_CN_unmobile   = 46080,
	SMSOTCODE_CN_overseas   = 46081,
	SMSOTCODE_CN_unknow     = 46090,
	SMSOTCODE_MAX           = 0x1fffffff
} SMSOperatorCode;

typedef enum {
	JUPHOON_NOTICE_TYPE_REG_USER = 1,
	JUPHOON_NOTICE_TYPE_DEREG_USER = 2,
	JUPHOON_NOTICE_TYPE_BIND_STATUS = 3,
	JUPHOON_NOTICE_TYPE_MAX
} JuphoonNoticeType;

typedef enum {
	JUPHOON_BINDING_STATUS_OPEN = 2,
	JUPHOON_BINDING_STATUS_CLOSE = 3,
	JUPHOON_BINDING_STATUS_TEMP_CLOSE = 4, //User logged out
	JUPHOON_BINDING_STATUS_MAX
} JuphoonBindingStatus;

typedef enum {
	PACKAGE_STATUS_ACTIVE = 1,
	PACKAGE_STATUS_FINISHED = 2,
	PACKAGE_STATUS_EXPIRED = 3,
	PACKAGE_STATUS_MAX
} PackageStatus;

typedef enum {
	TIME_TYPE_INIT = 0,
	TIME_TYPE_MINUTE = 1,
	TIME_TYPE_HOUR = 2,
	TIME_TYPE_DAY= 3,
	TIME_TYPE_WEEK= 4,
	TIME_TYPE_MONTH= 5,
	TIME_TYPE_MAX
} TimeType;

typedef enum {
	PythonFun_painting_aesthetics = 0,
	PythonFun_image_search_engine,
	PythonFun_painting_recommend,
	PythonFun_style_transfer,
	PythonFun_sculptures_search_engine,
	PythonFun_menu_recognition,
	PythonFun_travel_aesthetics,
	PythonFun_weichat_openid,
	PythonFun_poster,
	PythonFun_bagword,
	PythonFun_museum_recommend,
	PythonFun_painting_recommend_page,
	PythonFun_museum_recommend_page,
	PythonFun_refresh_recommend_cache,
	PythonFun_max = 0xffffffffffffffff
} PythonFun;

typedef enum {
	IOStackType_uip,
	IOStackType_uip2,
	IOStackType_json,
	IOStackType_h5_form,
	IOStackType_weichat_form,
} IOStackType;

typedef enum {
	DaveDataType_char,
	DaveDataType_int,
	DaveDataType_float,
	DaveDataType_double,
} DaveDataType;
	
typedef enum {
  JsonType_Null,
  JsonType_Boolean,
  JsonType_Double,
  JsonType_Int,
  JsonType_Object,
  JsonType_Array,
  JsonType_String,
  JsonType_Invalid
} DaveJsonType;

#endif

