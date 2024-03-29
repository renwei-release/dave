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

import "unsafe"

type MCardVerText struct {
	Version int64
	Type int64
	User [DAVE_USER_NAME_LEN] byte
	Location MCardLocation
	Time MCardTime
	Utf8_txt *MBUF
}

type MCardVerMedia struct {
	Version int64
	Type int64
	Source int64
	Poi MCardPOI
	Author_name [DAVE_USER_NAME_LEN] byte
	Author_url [DAVE_URL_LEN] byte
	Language int32
	Author_photo_url [DAVE_URL_LEN] byte
	Rating float64
	Utf8_txt *MBUF
	Second uint64
}

type MCardVerTalk struct {
	Version int64
	Type int64
	Channel [DAVE_NORMAL_NAME_LEN] byte
	Uuid [DAVE_UUID_LEN] byte
	App_id uint64
	Src_user [DAVE_USER_NAME_LEN] byte
	Dst_user [DAVE_USER_NAME_LEN] byte
	Location MCardLocation
	Time MCardTime
	Main_serial uint64
	Sub_serial uint64
	Total_sub_serial uint64
	Content MCardContent
}

type MCardVerComment struct {
	Version int64
	Head MCardCommentHead
	Content MCardContent
}

type CVModelResult struct {
	Search_opt int32
	Content_type int64
	Language_code int32
	Image_local_path [DAVE_PATH_LEN] byte
	Image_url_path [DAVE_PATH_LEN] byte
	Opt_number uint64
	Face_number uint64
	Vgg_id [DAVE_KEY_OPT_MAX*DAVE_SHA1_IMAGE_ID] byte
	Vgg_score [DAVE_KEY_OPT_MAX] float32
	Rectangle [DAVE_KEY_OPT_MAX] CVRectangle
	Image_title [DAVE_KEY_OPT_MAX*DAVE_IMAGE_TITLE_LEN] byte
	Image_painter [DAVE_KEY_OPT_MAX*DAVE_USER_NAME_LEN] byte
	Inliners_num [DAVE_KEY_OPT_MAX] uint64
	Inliners_score [DAVE_KEY_OPT_MAX] float32
	Keypoints_num [DAVE_KEY_OPT_MAX] uint64
	Keypoints_score [DAVE_KEY_OPT_MAX] float32
	Confidence int8
	Label [DAVE_LABEL_STR_MAX] byte
	Score float32
	Cnn_model_work_time uint64
	Features_db_req_time uint64
	Features_db_rsp_time uint64
	Features_db_process_time uint64
	Introduce_db_req_time uint64
	Introduce_db_rsp_time uint64
	Model_raw_data *MBUF
}

type TerminalInformation struct {
	Language [64] byte
	Country [64] byte
	Province [64] byte
	City [64] byte
	Nickname [64] byte
	Gender [64] byte
	Brand [64] byte
	Model [64] byte
	Version [64] byte
	System [64] byte
	Platform [64] byte
	Sdkversion [64] byte
}

type PainterIntroduction struct {
	Table_id uint64
	Painter_name [128] byte
	Painter_birth [128] byte
	Painter_death [128] byte
	Painter_introduction [16384] byte
	Painter_avatar_id [DAVE_SHA1_IMAGE_ID] byte
	Painter_avatar_url [512] byte
	Painter_related_painting [DAVE_KEY_OPT_MAX*DAVE_SHA1_IMAGE_ID] byte
	Total_painting_number uint64
}

type SocNetInfoAddr struct {
	Ip SocNetInfoIp
	Url [DAVE_URL_LEN] byte
}

type SocNetInfoIp struct {
	Ver int32
	Ip_addr [16] byte
}

type MCardLocation struct {
	Latitude float64
	Longitude float64
	Altitude float64
	Course float64
	Slope float64
}

type MCardTime struct {
	Write_time DateStruct
	Failure_type int64
	Failure_time DateStruct
}

type MCardPOI struct {
	Location MCardLocation
	Type int64
	Name [DAVE_POI_NAME_MAX] byte
	Phone_number [DAVE_MSISDN_LEN] byte
	Address [DAVE_POI_ADDRESS_MAX] byte
	Rating float64
}

type MCardContent struct {
	Id int64
	Content_type int64
	Content_language int32
	Pcontent *MBUF
}

type MCardCommentHead struct {
	Nosql_head NoSQLHead
	Comment_head MCardCommentHeadData
	Reserve_data [DAVE_MACRD_HEAD_MAX-2976] byte
}

type CVRectangle struct {
	X1 float32
	Y1 float32
	X2 float32
	Y2 float32
	W float32
	H float32
}

type NoSQLHead struct {
	Key_str [DAVE_NOSQL_KEY_STR_MAX] byte
	Reserve_data [512-DAVE_NOSQL_KEY_STR_MAX] byte
}

type MCardCommentHeadData struct {
	Type int64
	Post_id [DAVE_KEY_LEN_MAX] byte
	Reply_comment_id [DAVE_KEY_LEN_MAX] byte
	Src_uuid [DAVE_UUID_LEN] byte
	Dst_uuid [DAVE_UUID_LEN] byte
	Src_user [DAVE_USER_NAME_LEN] byte
	Dst_user [DAVE_USER_NAME_LEN] byte
	Location MCardLocation
	Time MCardTime
	Ti TerminalInformation
	User UserInformation
	Comment_attributes uint64
}

type UserInformation struct {
	Language [64] byte
	Country [64] byte
	Province [64] byte
	City [64] byte
	Nickname [64] byte
	Gender [16] byte
	Phone_number_1 [64] byte
	Phone_number_2 [64] byte
	Email_1 [64] byte
	Email_2 [64] byte
	Home_address [128] byte
	Avatar_url [256] byte
}

type MBUF struct {
	Next unsafe.Pointer
	Payload unsafe.Pointer
	Tot_len int64
	Len int64
	Ref int64
	Alloc_len int64
}

type GPSLocation struct {
	Latitude float64
	Longitude float64
	Altitude float64
	Course float64
}

type MCard struct {
	Version int64
	Text MCardVerText
	Media MCardVerMedia
	Talk MCardVerTalk
	Comment MCardVerComment
}

type DateStruct struct {
	Year uint16
	Month byte
	Day byte
	Hour byte
	Minute byte
	Second byte
	Week byte
	Zone byte
}

type CVResult struct {
	Model_result CVModelResult
	Image_introduction ImageIntroduction
}

type UniversalLabel struct {
	Label_extra_information [DAVE_LABEL_EXTRA_INFO_MAX] byte
	Label_id uint64
	Label_str [DAVE_LABEL_STR_MAX] byte
	Label_score float32
}

type CVKeyPoint struct {
	Width int32
	Height int32
	Size int32
	_keypoint_ *MBUF
}

type OpenCVMat struct {
	Type int32
	Flags int32
	Dims int32
	Rows int32
	Cols int32
	Mat *MBUF
}

type WeiChatUserInfo struct {
	Uuid [64] byte
	Openid [64] byte
	Code [64] byte
	User_id [64] byte
	Session_key [64] byte
	Share_uuid [64] byte
	Ti TerminalInformation
}

type ImageIntroduction struct {
	Image_id [DAVE_SHA1_IMAGE_ID] byte
	Painter PainterIntroduction
	Museum MuseumIntroduction
	Creat_time [128] byte
	Dimensions [128] byte
	Title [DAVE_IMAGE_TITLE_LEN] byte
	Medium [128] byte
	Image_introduction [16384] byte
	Image_url [DAVE_URL_LEN] byte
	Page_url [DAVE_URL_LEN] byte
	Collection_location [512] byte
	Audio_url [DAVE_URL_LEN] byte
}

type MuseumIntroduction struct {
	Table_id uint64
	Museum_id [DAVE_SHA1_IMAGE_ID] byte
	Museum_name [128] byte
	Address [1024] byte
	Ticket [256] byte
	Phone [128] byte
	Web_url [256] byte
	Opening_hours [256] byte
	Museum_introduction [16384] byte
}

type HttpKeyValue struct {
	Key [DAVE_HTTP_KEY_LEN] byte
	Value [DAVE_HTTP_VALUE_LEN] byte
}

type MsgIdEcho struct {
	Type int64
	Gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Thread [DAVE_THREAD_NAME_LEN] byte
	Echo_total_counter uint64
	Echo_total_time uint64
	Echo_cycle_counter uint64
	Echo_cycle_time uint64
	Echo_req_time uint64
	Echo_rsp_time uint64
	Concurrent_flag int8
	Concurrent_tps_time uint64
	Concurrent_tps_counter uint64
	Concurrent_cycle_counter uint64
	Concurrent_total_counter uint64
	S8_echo byte
	U8_echo byte
	S16_echo int16
	U16_echo uint16
	S32_echo int32
	U32_echo uint32
	S64_echo int64
	U64_echo uint64
	Float_echo float32
	Double_echo float64
	Void_echo unsafe.Pointer
	String_echo [256] byte
	Mbuf_echo *MBUF
}

type SocNetInfo struct {
	Domain int32
	Type int32
	Addr_type int32
	Addr SocNetInfoAddr
	Port uint16
	Fixed_src_flag int32
	Src_ip SocNetInfoIp
	Src_port uint16
	Enable_keepalive_flag int32
	Keepalive_second int64
	Netcard_bind_flag int32
	Netcard_name [DAVE_NORMAL_NAME_LEN] byte
}

type IPBaseInfo struct {
	Protocol int32
	Ver int32
	Src_ip [16] byte
	Src_port uint16
	Dst_ip [16] byte
	Dst_port uint16
	Keepalive_second int64
	Netcard_name [DAVE_NORMAL_NAME_LEN] byte
	Fixed_port_flag int32
}

