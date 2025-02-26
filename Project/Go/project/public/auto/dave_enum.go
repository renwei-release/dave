package auto
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 *
 * ############################# IMPORTANT INFORMATION ############################
 * The code of this file is automatically generated by tools(Tools/auto_code),
 * please do not modify it manually!
 * ############################# IMPORTANT INFORMATION ############################
 * ================================================================================
 */

const (
	AIPlaceType_accounting int64 = iota
	AIPlaceType_airport
	AIPlaceType_amusement_park
	AIPlaceType_aquarium
	AIPlaceType_art_gallery
	AIPlaceType_atm
	AIPlaceType_bakery
	AIPlaceType_bank
	AIPlaceType_bar
	AIPlaceType_beauty_salon
	AIPlaceType_bicycle_store
	AIPlaceType_book_store
	AIPlaceType_bowling_alley
	AIPlaceType_bus_station
	AIPlaceType_cafe
	AIPlaceType_campground
	AIPlaceType_car_dealer
	AIPlaceType_car_rental
	AIPlaceType_car_repair
	AIPlaceType_car_wash
	AIPlaceType_casino
	AIPlaceType_cemetery
	AIPlaceType_church
	AIPlaceType_city_hall
	AIPlaceType_clothing_store
	AIPlaceType_convenience_store
	AIPlaceType_courthouse
	AIPlaceType_dentist
	AIPlaceType_department_store
	AIPlaceType_doctor
	AIPlaceType_electrician
	AIPlaceType_electronics_store
	AIPlaceType_embassy
	AIPlaceType_fire_station
	AIPlaceType_florist
	AIPlaceType_funeral_home
	AIPlaceType_furniture_store
	AIPlaceType_gas_station
	AIPlaceType_gym
	AIPlaceType_hair_care
	AIPlaceType_hardware_store
	AIPlaceType_hindu_temple
	AIPlaceType_home_goods_store
	AIPlaceType_hospital
	AIPlaceType_insurance_agency
	AIPlaceType_jewelry_store
	AIPlaceType_laundry
	AIPlaceType_lawyer
	AIPlaceType_library
	AIPlaceType_liquor_store
	AIPlaceType_local_government_office
	AIPlaceType_locksmith
	AIPlaceType_lodging
	AIPlaceType_meal_delivery
	AIPlaceType_meal_takeaway
	AIPlaceType_mosque
	AIPlaceType_movie_rental
	AIPlaceType_movie_theater
	AIPlaceType_moving_company
	AIPlaceType_museum
	AIPlaceType_night_club
	AIPlaceType_painter
	AIPlaceType_park
	AIPlaceType_parking
	AIPlaceType_pet_store
	AIPlaceType_pharmacy
	AIPlaceType_physiotherapist
	AIPlaceType_plumber
	AIPlaceType_police
	AIPlaceType_post_office
	AIPlaceType_real_estate_agency
	AIPlaceType_restaurant
	AIPlaceType_roofing_contractor
	AIPlaceType_rv_park
	AIPlaceType_school
	AIPlaceType_shoe_store
	AIPlaceType_shopping_mall
	AIPlaceType_spa
	AIPlaceType_stadium
	AIPlaceType_storage
	AIPlaceType_store
	AIPlaceType_subway_station
	AIPlaceType_synagogue
	AIPlaceType_taxi_stand
	AIPlaceType_train_station
	AIPlaceType_transit_station
	AIPlaceType_travel_agency
	AIPlaceType_university
	AIPlaceType_veterinary_care
	AIPlaceType_zoo
	AIPlaceType_max = 0x1fffffffffffffff
)

const (
	BDataLogLevel_normal = 0 + iota
	BDataLogLevel_report
	BDataLogLevel_max = 0x1fffffffffffffff
)

const (
	CVSearchOpt_0 int64 = iota
	CVSearchOpt_1
	CVSearchOpt_2
	CVSearchOpt_max
)

const (
	EchoType_start int64 = iota
	EchoType_stop
	EchoType_single
	EchoType_random
	EchoType_max = 0x1fffffffffffffff
)

const (
	KeepAlive_enable = 0x01234567 + iota
	KeepAlive_disable = 0x09abcdef
)

const (
	NetCardBind_enable = 0x01234567 + iota
	NetCardBind_disable = 0x09abcdef
)

const (
	ERRCODE_OK = 0 + iota
	ERRCODE_Memory_full = -1
	ERRCODE_Limited_resources = -2
	ERRCODE_Arithmetic_error = -3
	ERRCODE_Unknown_error = -4
	ERRCODE_Can_not_find_node = -5
	ERRCODE_Send_failed = -6
	ERRCODE_Recv_failed = -7
	ERRCODE_Invalid_data = -8
	ERRCODE_Unsupported_type = -9
	ERRCODE_Invalid_user_name = -10
	ERRCODE_Parameter_conflicts = -11
	ERRCODE_Invalid_device = -12
	ERRCODE_Invalid_Event = -13
	ERRCODE_Heartbeat_timeout = -14
	ERRCODE_Invalid_password = -15
	ERRCODE_save_failed = -16
	ERRCODE_Invalid_data_too_short = -17
	ERRCODE_Invalid_data_too_long = -18
	ERRCODE_Invalid_data_crc_check = -19
	ERRCODE_Invalid_parameter = -20
	ERRCODE_Send_msg_failed = -21
	ERRCODE_Resource_conflicts = -22
	ERRCODE_CREDIT_LIMIT_REACHED = -23
	ERRCODE_Exceeded_number = -24
	ERRCODE_do_not_free = -25
	ERRCODE_Request_failed = -26
	ERRCODE_can_not_find_ret_code = -27
	ERRCODE_user_is_registered = -28
	ERRCODE_connect_error = -29
	ERRCODE_function_not_supported = -30
	ERRCODE_msg_queue_is_full = -31
	ERRCODE_msg_can_not_resend_wakeup = -32
	ERRCODE_msg_memory_if_full = -33
	ERRCODE_msg_competition_for_resources = -34
	ERRCODE_wait = -35
	ERRCODE_Invalid_state = -36
	ERRCODE_usr_default = -37
	ERRCODE_timer_out = -38
	ERRCODE_invalid_type = -39
	ERRCODE_user_does_not_exist = -40
	ERRCODE_version_new = -41
	ERRCODE_version_old = -42
	ERRCODE_ptr_null = -43
	ERRCODE_record_not_exist = -44
	ERRCODE_db_store_failed = -45
	ERRCODE_db_not_find = -46
	ERRCODE_Invalid_channel_name = -47
	ERRCODE_Invalid_db_store = -48
	ERRCODE_version_mismatch = -49
	ERRCODE_invalid_version_file = -50
	ERRCODE_version_identical = -51
	ERRCODE_db_sql_failed = -52
	ERRCODE_invalid_option = -53
	ERRCODE_Invalid_domain = -54
	ERRCODE_Invalid_auth_key = -55
	ERRCODE_invalid_key = -56
	ERRCODE_no_account_found = -57
	ERRCODE_invalid_phone_number = -58
	ERRCODE_not_match_domain = -59
	ERRCODE_invalid_account = -60
	ERRCODE_please_help_free = -61
	ERRCODE_db_init_failed = -62
	ERRCODE_update_fail = -63
	ERRCODE_maybe_have_data = -64
	ERRCODE_do_not_restart = -65
	ERRCODE_lost_auth_key = -66
	ERRCODE_lost_serial = -67
	ERRCODE_lost_time = -68
	ERRCODE_lost_uuid = -69
	ERRCODE_lost_user = -70
	ERRCODE_can_not_find_record = -71
	ERRCODE_lost_dial_number = -72
	ERRCODE_call_prohibition = -73
	ERRCODE_lost_link = -74
	ERRCODE_record_not_found = -75
	ERRCODE_ims_account_incorrect = -76
	ERRCODE_invalid_dial_number = -77
	ERRCODE_Invalid_balance = -78
	ERRCODE_user_exist = -79
	ERRCODE_store_data_failed = -80
	ERRCODE_Invalid_user_type = -81
	ERRCODE_invalid_user = -82
	ERRCODE_unsupport = -83
	ERRCODE_unpackage_failed = -84
	ERRCODE_creat_db_failed = -85
	ERRCODE_Invalid_channel = -86
	ERRCODE_heartbeat_timer_out = -87
	ERRCODE_Invalid_package = -88
	ERRCODE_Invalid_rules = -89
	ERRCODE_busy = -90
	ERRCODE_data_overflow = -91
	ERRCODE_invalid_account_name = -92
	ERRCODE_Account_in_use = -93
	ERRCODE_invalid_address = -94
	ERRCODE_can_not_find_rules = -95
	ERRCODE_refuse_call_self = -96
	ERRCODE_Invalid_call = -97
	ERRCODE_mismatch = -98
	ERRCODE_Invalid_call_type = -99
	ERRCODE_Invalid_billing = -100
	ERRCODE_invalid_magic = -101
	ERRCODE_create_thread_failed = -102
	ERRCODE_execute_sql_failed = -103
	ERRCODE_decode_failed = -104
	ERRCODE_encode_failed = -105
	ERRCODE_invalid_file = -106
	ERRCODE_file_open_failed = -107
	ERRCODE_process_now = -108
	ERRCODE_queue_too_large = -109
	ERRCODE_add_user_name_failed = -110
	ERRCODE_call_time_limited = -111
	ERRCODE_invalid_hour = -112
	ERRCODE_invalid_day = -113
	ERRCODE_invalid_week = -114
	ERRCODE_invalid_month = -115
	ERRCODE_invalid_year = -116
	ERRCODE_invalid_unit = -117
	ERRCODE_invalid_rules_number = -118
	ERRCODE_invalid_package_private = -119
	ERRCODE_rules_id_mismatch = -120
	ERRCODE_channel_not_exist = -121
	ERRCODE_invalid_donation_account = -122
	ERRCODE_channel_exist = -123
	ERRCODE_invalid_date = -124
	ERRCODE_not_my_data = -125
	ERRCODE_only_share_user_pwd_can_modify = -126
	ERRCODE_can_not_find_thread = -127
	ERRCODE_lost_register_e164_number = -128
	ERRCODE_lost_modify_impu_flag = -129
	ERRCODE_lost_country_code_of_e164_number = -130
	ERRCODE_lost_sip_password = -131
	ERRCODE_invalid_modify_impu_flag = -132
	ERRCODE_invalid_country_code_of_e164_number = -133
	ERRCODE_invalid_register_e164_number = -134
	ERRCODE_modify_impu_failed = -135
	ERRCODE_impu_not_existed = -136
	ERRCODE_script_execution_error = -137
	ERRCODE_invalid_share_ram = -138
	ERRCODE_Duplicate_MSISDN = -139
	ERRCODE_Duplicate_IMSI = -140
	ERRCODE_Duplicate_VOIP_MSISDN = -141
	ERRCODE_Not_found = -142
	ERRCODE_Boot_in_the_territory = -143
	ERRCODE_Other_errors = -144
	ERRCODE_lost_result_code = -145
	ERRCODE_invalid_imsi = -146
	ERRCODE_invalid_msisdn = -147
	ERRCODE_invalid_voip_msisdn = -148
	ERRCODE_processing_is_complete = -149
	ERRCODE_secondary_number_ass_repeated = -150
	ERRCODE_secondary_number_not_available = -151
	ERRCODE_secondary_number_ret_failed = -152
	ERRCODE_secondary_number_inquiry_failed = -153
	ERRCODE_lost_secondary_number = -154
	ERRCODE_invalid_e164_number_type = -155
	ERRCODE_lost_number_request_type = -156
	ERRCODE_secondary_number_not_existed = -157
	ERRCODE_invalid_content = -158
	ERRCODE_invalid_email = -159
	ERRCODE_repeated_request = -160
	ERRCODE_rules_exist = -161
	ERRCODE_lack_of_donor = -162
	ERRCODE_lost_im_user_name = -163
	ERRCODE_lost_im_password = -164
	ERRCODE_invalid_im_nickname = -165
	ERRCODE_im_user_dereg_failed = -166
	ERRCODE_im_user_not_existed = -167
	ERRCODE_im_user_reg_failed = -168
	ERRCODE_invalid_im_user_name = -169
	ERRCODE_invalid_im_password = -170
	ERRCODE_lost_imsi = -171
	ERRCODE_lost_binding_status = -172
	ERRCODE_invalid_binding_status = -173
	ERRCODE_package_exist = -174
	ERRCODE_invalid_billing_user_data = -175
	ERRCODE_Failure_to_activate_validity = -176
	ERRCODE_Time_lapse = -177
	ERRCODE_Billing_reached_the_limit = -178
	ERRCODE_invalid_recharge = -179
	ERRCODE_Date_expired = -180
	ERRCODE_lost_package_info = -181
	ERRCODE_not_access = -182
	ERRCODE_invalid_minute = -183
	ERRCODE_invalid_second = -184
	ERRCODE_not_do_anything = -185
	ERRCODE_empty_data = -186
	ERRCODE_not_group_rules = -187
	ERRCODE_on_billing_operation_not_allowed = -188
	ERRCODE_roaming_not_allowed = -189
	ERRCODE_invalid_mcard = -190
	ERRCODE_Too_many_packages = -191
	ERRCODE_Too_many_rules = -192
	ERRCODE_can_not_find_mcard = -193
	ERRCODE_Failure_to_be_deactived = -194
	ERRCODE_operating_end = -195
	ERRCODE_invalid_voip_setting = -196
	ERRCODE_Call_barring = -197
	ERRCODE_add_forbidden_user_failed = -198
	ERRCODE_forbidden_register_user = -199
	ERRCODE_Unauthorized = -200
	ERRCODE_is_rnm_user = -201
	ERRCODE_table_exist = -202
	ERRCODE_record_empty = -203
	ERRCODE_impu_existed = -204
	ERRCODE_invalid_context = -205
	ERRCODE_authorized_failed = -206
	ERRCODE_logout = -207
	ERRCODE_mute_set_failed = -208
	ERRCODE_invalid_url = -209
	ERRCODE_Threshold_limit = -210
	ERRCODE_Channel_closed = -211
	ERRCODE_invalid_request = -212
	ERRCODE_Duplicate_name = -213
	ERRCODE_lost_parameter = -214
	ERRCODE_lost_protocol = -215
	ERRCODE_system_not_ready = -216
	ERRCODE_package_check = -217
	ERRCODE_package_discard = -218
	ERRCODE_resend = -219
	ERRCODE_Invalid_object = -220
	ERRCODE_over_max_package_types = -221
	ERRCODE_over_max_package_numbers = -222
	ERRCODE_wait_more_data = -223
	ERRCODE_unregister = -224
	ERRCODE_maximum_intents = -225
	ERRCODE_intercept = -226
	ERRCODE_repetitive_operation = -227
	ERRCODE_Failed_to_identify = -228
	ERRCODE_Failed_to_features = -229
	ERRCODE_No_clear_intention = -230
	ERRCODE_db_no_data = -231
	ERRCODE_insufficient_balance = -232
	ERRCODE_lost_lock = -233
	ERRCODE_lost_req_type = -234
	ERRCODE_can_not_find_client = -235
	ERRCODE_out_of_bounds = -236
	ERRCODE_valuation = -237
	ERRCODE_Invalid_version = -238
	ERRCODE_auth_key_exist = -239
	ERRCODE_predict_failed = -240
	ERRCODE_internal_server_error = -241
	ERRCODE_update = -242
	ERRCODE_lost_atom_ptl = -243
	ERRCODE_bs_repeat = -244
	ERRCODE_can_not_find_user = -245
	ERRCODE_name_already_exist = -246
	ERRCODE_can_not_find_cost_price = -247
	ERRCODE_can_not_find_unit_price = -248
	ERRCODE_cfg_already_exists = -249
	ERRCODE_storage = -250
	ERRCODE_Can_not_find_path = -251
	ERRCODE_over_time = -252
	ERRCODE_soft_delete = -253
	ERRCODE_Not_found_ptl_name = -254
	ERRCODE_RELEASE = -255
	ERRCODE_Not_found_billing = -256
	ERRCODE_Can_not_find_area = -257
	ERRCODE_User_Unregister = -258
	ERRCODE_Stateless_Sending = -259
	ERRCODE_Can_not_find_business_rules = -260
	ERRCODE_reconnect = -261
	ERRCODE_Blacklist_intercept = -262
	ERRCODE_data_not_exist = -263
	ERRCODE_Invalid_length = -264
	ERRCODE_Route_Direct = -265
	ERRCODE_Can_not_find_isp = -266
	ERRCODE_DB_Already_Exist = -267
	ErrCode_max = 0x1fffffffffffffff
)

const (
	FixedPort = 0x01234567 + iota
	NotFixedPort = 0x09abcdef
)

const (
	ListenHttp int64 = iota
	ListenHttps
	ListenWeb
	ListenMax
)

const (
	LocationMatch_Accurate = 0 + iota
	LocationMatch_Prefix
	LocationMatch_CaseRegular
	LocationMatch_Regular
	LocationMatch_CaseRegularExcl
	LocationMatch_RegularExcl
	LocationMatch_Wildcard
	LocationMatch_Max
)

const (
	HttpContentType_json int64 = iota
	HttpContentType_text
	HttpContentType_xml
	HttpContentType_xwww
	HttpContentType_max
)

const (
	HttpMethod_post int64 = iota
	HttpMethod_get
	HttpMethod_put
	HttpMethod_options
	HttpMethod_delete
	HttpMethod_max
)

const (
	IPProtocol_ICMP = 1 + iota
	IPProtocol_TCP = 6
	IPProtocol_UDP = 17
	IPProtocol_GRE = 47
	IPProtocol_MAX = 0x1fffffff
)

const (
	IPVER_IPV4 = 4 + iota
	IPVER_IPV6 = 6
	IPVER_MAX = 0x1fffffff
)

const (
	LanguageCode_Abkhazian int64 = iota
	LanguageCode_Afar
	LanguageCode_Afrikaans
	LanguageCode_Akan
	LanguageCode_Albanian
	LanguageCode_Amharic
	LanguageCode_Arabic
	LanguageCode_Aragonese
	LanguageCode_Armenian
	LanguageCode_Assamese
	LanguageCode_Avaric
	LanguageCode_Avestan
	LanguageCode_Aymara
	LanguageCode_Chinese
	LanguageCode_Chuvash
	LanguageCode_Cornish
	LanguageCode_Corsican
	LanguageCode_English
	LanguageCode_French
	LanguageCode_German
	LanguageCode_Italian
	LanguageCode_Japanese
	LanguageCode_Korean
	LanguageCode_Russian
	LanguageCode_Spanish_Castilian
	LanguageCode_Sundanese
	LanguageCode_Swahili
	LanguageCode_Swati
	LanguageCode_Swedish
	LanguageCode_Tamil
	LanguageCode_Telugu
	LanguageCode_Tajik
	LanguageCode_Thai
	LanguageCode_Traditional_Chinese
	LanguageCode_Dutch
	LanguageCode_max
)

const (
	MCardContentType_begin int64 = iota
	MCardContentType_utf8
	MCardContentType_json
	MCardContentType_xml
	MCardContentType_login
	MCardContentType_jpg_cropped_bin
	MCardContentType_jpg_uncropped_bin
	MCardContentType_image_sha1_id
	MCardContentType_face_matching
	MCardContentType_poster_generate
	MCardContentType_start_up
	MCardContentType_museum_page
	MCardContentType_painter_page
	MCardContentType_bbs_comment
	MCardContentType_bbs_load
	MCardContentType_painting_recommend
	MCardContentType_portrait_transfer_list
	MCardContentType_portrait_transfer_action
	MCardContentType_landscape_transfer_list
	MCardContentType_landscape_transfer_action
	MCardContentType_image_search_cropped_bin
	MCardContentType_image_search_uncropped_bin
	MCardContentType_sculptures_search_cropped_bin
	MCardContentType_sculptures_search_uncropped_bin
	MCardContentType_menu_recognition
	MCardContentType_tourism_image_search
	MCardContentType_bagword
	MCardContentType_museum_recommend
	MCardContentType_painting_recommend_page
	MCardContentType_museum_recommend_page
	MCardContentType_museum_sha1_id
	MCardContentType_refresh_recommend_cache
	MCardContentType_detection
	MCardContentType_generator
	MCardContentType_end
	MCardContentType_max = 0x1fffffffffffffff
)

const (
	MCardIdentityType_none int64 = iota
	MCardIdentityType_initialization
	MCardIdentityType_user
	MCardIdentityType_AI
	MCardIdentityType_max
	MCardIdentityType_MAX = 0x1fffffffffffffff
)

const (
	MCardSource_aia int64 = iota
	MCardSource_google
	MCardSource_aws
	MCardSource_aic
	MCardSource_aib
	MCardSource_aib_io
	MCardSource_bbs
	MCardSource_max = 0x1fffffffffffffff
)

const (
	MCardTimeType_Permanent int64 = iota
	MCardTimeType_Normal
	MCardTimeType_max = 0x1fffffffffffffff
)

const (
	MCardType_Private_Message_Board int64 = iota
	MCardType_Public_Message_Board
	MCardType_POI
	MCardType_TALK
	MCardType_weichat_talk
	MCardType_h5_talk
	MCardType_max = 0x1fffffffffffffff
)

const (
	MCardVer_reserve int64 = iota
	MCardVer_text
	MCardVer_media
	MCardVer_talk
	MCardVer_comment
	MCardVer_max = 0x1fffffffffffffff
)

const (
	NetAddrIPType = 0 + iota
	NetAddrIPBroadcastType
	NetAddrURLType = 0x12345678
)

const (
	PythonFun_painting_aesthetics = 0 + iota
	PythonFun_image_search_engine
	PythonFun_painting_recommend
	PythonFun_style_transfer
	PythonFun_sculptures_search_engine
	PythonFun_menu_recognition
	PythonFun_travel_aesthetics
	PythonFun_weichat_openid
	PythonFun_poster
	PythonFun_bagword
	PythonFun_museum_recommend
	PythonFun_painting_recommend_page
	PythonFun_museum_recommend_page
	PythonFun_refresh_recommend_cache
	PythonFun_detection
	PythonFun_generator
	PythonFun_max = 0x1fffffffffffffff
)

const (
	RetCode_begin_value = 1 + iota
	RetCode_OK = 0
	RetCode_Memory_full = -1
	RetCode_Limited_resources = -2
	RetCode_Arithmetic_error = -3
	RetCode_Unknown_error = -4
	RetCode_Can_not_find_node = -5
	RetCode_Send_failed = -6
	RetCode_Recv_failed = -7
	RetCode_Invalid_data = -8
	RetCode_Unsupported_type = -9
	RetCode_Invalid_user_name = -10
	RetCode_Parameter_conflicts = -11
	RetCode_Invalid_device = -12
	RetCode_Invalid_Event = -13
	RetCode_Heartbeat_timeout = -14
	RetCode_invalid_content = -15
	RetCode_save_failed = -16
	RetCode_Invalid_data_too_short = -17
	RetCode_Invalid_data_too_long = -18
	RetCode_Invalid_data_crc_check = -19
	RetCode_Invalid_parameter = -20
	RetCode_Send_msg_failed = -21
	RetCode_Resource_conflicts = -22
	RetCode_CREDIT_LIMIT_REACHED = -23
	RetCode_Exceeded_number = -24
	RetCode_do_not_free = -25
	RetCode_Request_failed = -26
	RetCode_can_not_find_ret_code = -27
	RetCode_user_is_registered = -28
	RetCode_connect_error = -29
	RetCode_function_not_supported = -30
	RetCode_msg_queue_is_full = -31
	RetCode_msg_can_not_resend_wakeup = -32
	RetCode_msg_memory_if_full = -33
	RetCode_msg_competition_for_resources = -34
	RetCode_wait = -35
	RetCode_Invalid_state = -36
	RetCode_usr_default = -37
	RetCode_timer_out = -38
	RetCode_invalid_type = -39
	RetCode_user_does_not_exist = -40
	RetCode_version_new = -41
	RetCode_version_old = -42
	RetCode_ptr_null = -43
	RetCode_record_not_exist = -44
	RetCode_db_store_failed = -45
	RetCode_db_not_find = -46
	RetCode_Invalid_channel_name = -47
	RetCode_Invalid_db_store = -48
	RetCode_version_mismatch = -49
	RetCode_invalid_version_file = -50
	RetCode_version_identical = -51
	RetCode_channel_not_exist = -52
	RetCode_invalid_option = -53
	RetCode_Invalid_domain = -54
	RetCode_Invalid_auth_key = -55
	RetCode_invalid_key = -56
	RetCode_no_account_found = -57
	RetCode_invalid_phone_number = -58
	RetCode_not_match_domain = -59
	RetCode_invalid_account = -60
	RetCode_please_help_free = -61
	RetCode_db_init_failed = -62
	RetCode_update_fail = -63
	RetCode_maybe_have_data = -64
	RetCode_do_not_restart = -65
	RetCode_lost_auth_key = -66
	RetCode_lost_serial = -67
	RetCode_lost_time = -68
	RetCode_lost_uuid = -69
	RetCode_lost_user = -70
	RetCode_can_not_find_record = -71
	RetCode_empty_data = -72
	RetCode_Channel_closed = -73
	RetCode_lost_link = -74
	RetCode_record_not_found = -75
	RetCode_not_access = -76
	RetCode_can_not_find_thread = -77
	RetCode_can_not_find_client = -78
	RetCode_user_exist = -79
	RetCode_store_data_failed = -80
	RetCode_Invalid_user_type = -81
	RetCode_invalid_user = -82
	RetCode_unsupport = -83
	RetCode_unpackage_failed = -84
	RetCode_creat_db_failed = -85
	RetCode_Invalid_channel = -86
	RetCode_heartbeat_timer_out = -87
	RetCode_not_my_data = -88
	RetCode_Invalid_rules = -89
	RetCode_busy = -90
	RetCode_data_overflow = -91
	RetCode_invalid_account_name = -92
	RetCode_Account_in_use = -93
	RetCode_invalid_address = -94
	RetCode_can_not_find_rules = -95
	RetCode_refuse_call_self = -96
	RetCode_Invalid_call = -97
	RetCode_mismatch = -98
	RetCode_Invalid_call_type = -99
	RetCode_Invalid_billing = -100
	RetCode_invalid_magic = -101
	RetCode_create_thread_failed = -102
	RetCode_script_execution_error = -103
	RetCode_decode_failed = -104
	RetCode_encode_failed = -105
	RetCode_invalid_file = -106
	RetCode_file_open_failed = -107
	RetCode_process_now = -108
	RetCode_queue_too_large = -109
	RetCode_repeated_request = -110
	RetCode_Can_not_find_path = -111
	RetCode_invalid_hour = -112
	RetCode_invalid_day = -113
	RetCode_invalid_week = -114
	RetCode_invalid_month = -115
	RetCode_invalid_year = -116
	RetCode_invalid_date = -117
	RetCode_table_exist = -118
	RetCode_db_sql_failed = -119
	RetCode_Unauthorized_access = -120
	RetCode_not_ready = -121
	RetCode_Unauthorized = -122
	RetCode_can_not_find_method = -123
	RetCode_invalid_call = -124
	RetCode_max = 0x1fffffffffffffff
)

const (
	DM_SOC_PF_INET = 0 + iota
	DM_SOC_PF_INET6
	DM_SOC_PF_UART
	DM_SOC_PF_LOCAL_INET
	DM_SOC_PF_LOCAL_INET6
	DM_SOC_PF_RAW
	DM_SOC_PF_RAW_INET
	SOCDOMAIN_MAX = 0x1fffffff
)

const (
	SOC_EVENT_START = 0 + iota
	SOC_EVENT_WAIT_CREAT
	SOC_EVENT_CONNECT
	SOC_EVENT_CONNECT_FAIL
	SOC_EVENT_WAIT_CONNECT
	SOC_EVENT_ACCEPT
	SOC_EVENT_REV
	SOC_EVENT_SND
	SOC_EVENT_CLOSE
	SOC_EVENT_SILENCE
	SOC_EVENT_MAX
)

const (
	SOCKETINFO_BIND = 0 + iota
	SOCKETINFO_BIND_OK
	SOCKETINFO_BIND_FAIL
	SOCKETINFO_CONNECT
	SOCKETINFO_CONNECT_OK
	SOCKETINFO_CONNECT_FAIL
	SOCKETINFO_CONNECT_WAIT
	SOCKETINFO_DISCONNECT
	SOCKETINFO_DISCONNECT_OK
	SOCKETINFO_DISCONNECT_FAIL
	SOCKETINFO_DISCONNECT_WAIT
	SOCKETINFO_CREAT
	SOCKETINFO_WAIT_CREAT
	SOCKETINFO_ACCEPT
	SOCKETINFO_REV
	SOCKETINFO_REV_MBUF
	SOCKETINFO_SND
	SOCKETINFO_CLOSE
	SOCKETINFO_SILENCE
	SOCKETINFO_DEVICE_CONNECT
	SOCKETINFO_DEVICE_DISCONNECT
	SOCKETINFO_LINK_LOST
	SOCKETINFO_SEND_TIMER_OUT
	SOCKETINFO_PORT_EXIST
	SOCKETINFO_WRITE_THEN_CLOSE
	SOCKETINFO_SND_URG
	SOCKETINFO_RAW_EVENT_RECV_LENGTH
	SOCKETINFO_MAX = 0x1fffffff
)

const (
	TYPE_SOCK_STREAM = 0 + iota
	TYPE_SOCK_DGRAM
	TYPE_SOCK_RAW
	TYPE_SOCK_SCTP
	TYPE_SOCK_max
	TYPE_SOCK_MAX = 0x1fffffff
)

const (
	UIPType_uip int64 = iota
	UIPType_json
	UIPType_h5_form
	UIPType_wechat_form
)

