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

/* for AIXMSG_AESTHETICS_REQ message */
type AIXMsgAestheticsReq struct {
	Image_path [DAVE_PATH_LEN] byte
	Ptr uint64
}

/* for AIXMSG_AESTHETICS_RSP message */
type AIXMsgAestheticsRsp struct {
	Ret int64
	Score float32
	Ptr uint64
}

/* for AIXMSG_IMAGE_CLASSIFICATION_REQ message */
type AIXMsgImageClassificationReq struct {
	Image_data *MBUF
	Ptr uint64
}

/* for AIXMSG_IMAGE_CLASSIFICATION_RSP message */
type AIXMsgImageClassificationRsp struct {
	Ret int64
	Label uint64
	Score float32
	Ptr uint64
}

/* for APPMSG_FUNCTION_REGISTER_REQ message */
type AppMsgFunctionRegReq struct {
	Thread_name [DAVE_THREAD_NAME_LEN] byte
	Function_id uint64
	Ptr uint64
}

/* for APPMSG_FUNCTION_REGISTER_RSP message */
type AppMsgFunctionRegRsp struct {
	Ret int64
	Thread_name [DAVE_THREAD_NAME_LEN] byte
	Function_id uint64
	Ptr uint64
}

/* for APPMSG_FUNCTION_UNREGISTER_REQ message */
type AppMsgFunctionUnRegReq struct {
	Thread_name [DAVE_THREAD_NAME_LEN] byte
	Function_id uint64
	Ptr uint64
}

/* for APPMSG_FUNCTION_UNREGISTER_RSP message */
type AppMsgFunctionUnRegRsp struct {
	Ret int64
	Thread_name [DAVE_THREAD_NAME_LEN] byte
	Function_id uint64
	Ptr uint64
}

/* for APPMSG_MCARD_REQ message */
type AppMsgMCardReq struct {
	Location GPSLocation
	Radius uint64
	Ptr uint64
}

/* for APPMSG_MCARD_RSP message */
type AppMsgMCardRsp struct {
	Ret int64
	Mcard MCard
	Ptr uint64
}

/* for APPMSG_TALK_MCARD_REQ message */
type AppMsgTalkMCardReq struct {
	Url [DAVE_URL_LEN] byte
	Mcard MCard
	Ptr uint64
}

/* for APPMSG_TALK_MCARD_RSP message */
type AppMsgTalkMCardRsp struct {
	Ret int64
	Mcard MCard
	Ptr uint64
}

/* for MSGID_APPLICATION_BUSY message */
type ApplicationBusy struct {
	Cfg_flag int8
	Ptr uint64
}

/* for MSGID_APPLICATION_IDLE message */
type ApplicationIdle struct {
	Cfg_flag int8
	Ptr uint64
}

/* for BBSMSG_ADD_COMMENT_REQ message */
type BBSMsgAddCommentReq struct {
	Product_name [DAVE_NORMAL_NAME_LEN] byte
	Post_id [DAVE_KEY_LEN_MAX] byte
	Mcard MCard
	Ptr uint64
}

/* for BBSMSG_ADD_COMMENT_RSP message */
type BBSMsgAddCommentRsp struct {
	Ret int64
	Ptr uint64
}

/* for BBSMSG_INQ_COMMENT_REQ message */
type BBSMsgInqCommentReq struct {
	Product_name [DAVE_NORMAL_NAME_LEN] byte
	Post_or_comment_id [DAVE_KEY_LEN_MAX] byte
	Page_id uint64
	Page_number uint64
	Ptr uint64
}

/* for BBSMSG_INQ_COMMENT_RSP message */
type BBSMsgInqCommentRsp struct {
	Ret int64
	Total_page_number uint64
	Page_id uint64
	Page_array_number uint64
	Page_array [DAVE_COMMENT_MCARD_ARRAY_MAX] MCard
	Reply_array_number uint64
	Reply_array [DAVE_COMMENT_MCARD_ARRAY_MAX] MCard
	Ptr uint64
}

/* for BDATA_LOG_REQ message */
type BDataLogReq struct {
	Level int64
	Version [DAVE_VERNO_STR_LEN] byte
	Sub_flag [DAVE_NORMAL_STR_LEN] byte
	Local_date DateStruct
	Fun [DAVE_NORMAL_STR_LEN] byte
	Line uint64
	Host_name [DAVE_NORMAL_NAME_LEN] byte
	Host_mac [DAVE_MAC_ADDR_LEN] byte
	Host_ipv4 [DAVE_IP_V4_ADDR_LEN] byte
	Host_ipv6 [DAVE_IP_V6_ADDR_LEN] byte
	Log_data *MBUF
	Log_file *MBUF
	Ptr uint64
}

/* for BDATA_LOG_RSP message */
type BDataLogRsp struct {
	Ret int64
	Ptr uint64
}

/* for MSGID_CFG_REMOTE_SYNC_UPDATE message */
type CFGRemoteSyncUpdate struct {
	Put_flag int8
	Cfg_mbuf_name *MBUF
	Cfg_mbuf_value *MBUF
	Ttl int64
}

/* for MSGID_CFG_REMOTE_UPDATE message */
type CFGRemoteUpdate struct {
	Put_flag int8
	Cfg_name [1024] byte
	Cfg_value [262144] byte
	Ttl int64
}

/* for MSGID_CFG_UPDATE message */
type CFGUpdate struct {
	Cfg_name [DAVE_NORMAL_NAME_LEN] byte
	Cfg_length uint64
	Cfg_value [8196] byte
}

/* for CVMSG_FEATURES_DETECTED_REQ message */
type CVMsgFeaturesDetectedReq struct {
	Image_path [DAVE_PATH_LEN] byte
	Nfeatures uint64
	Ptr uint64
}

/* for CVMSG_FEATURES_DETECTED_RSP message */
type CVMsgFeaturesDetectedRsp struct {
	Ret int64
	Image_path [DAVE_PATH_LEN] byte
	Ptr uint64
}

/* for CVMSG_IMAGE_SEARCH_REQ message */
type CVMsgImageSearchReq struct {
	Content_type int64
	Language_code int32
	Image_data *MBUF
	Ptr uint64
}

/* for CVMSG_IMAGE_SEARCH_RSP message */
type CVMsgImageSearchRsp struct {
	Ret int64
	Cv_result CVResult
	Ptr uint64
}

/* for CVMSG_PAINTING_AESTHETICS_REQ message */
type CVMsgPaintingAestheticsReq struct {
	Content_type int64
	Language_code int32
	Image_data *MBUF
	Ptr uint64
}

/* for CVMSG_PAINTING_AESTHETICS_RSP message */
type CVMsgPaintingAestheticsRsp struct {
	Ret int64
	Cv_result CVResult
	Ptr uint64
}

/* for CVMSG_SCULPTURES_SEARCH_REQ message */
type CVMsgSculpturesSearchReq struct {
	Content_type int64
	Language_code int32
	Image_data *MBUF
	Ptr uint64
}

/* for CVMSG_SCULPTURES_SEARCH_RSP message */
type CVMsgSculpturesSearchRsp struct {
	Ret int64
	Cv_result CVResult
	Ptr uint64
}

/* for MSGID_COROUTINE_WAKEUP message */
type CoroutineWakeup struct {
	Wakeup_id uint64
	Thread_index uint64
	Wakeup_index uint64
	Ptr uint64
}

/* for DBMSG_HYBRID_ADD_LIST_REQ message */
type DBHybridAddListReq struct {
	Table [DAVE_NORMAL_NAME_LEN] byte
	Key [DAVE_KEY_LEN_MAX] byte
	Value *MBUF
	Ptr uint64
}

/* for DBMSG_HYBRID_ADD_LIST_RSP message */
type DBHybridAddListRsp struct {
	Ret int64
	Table [DAVE_NORMAL_NAME_LEN] byte
	Key [DAVE_KEY_LEN_MAX] byte
	Ptr uint64
}

/* for DBMSG_HYBRID_INQ_LIST_REQ message */
type DBHybridInqListReq struct {
	Direct int8
	Table [DAVE_NORMAL_NAME_LEN] byte
	Key [DAVE_KEY_LEN_MAX] byte
	Page_id uint64
	Page_number uint64
	Ptr uint64
}

/* for DBMSG_HYBRID_INQ_LIST_RSP message */
type DBHybridInqListRsp struct {
	Ret int64
	Direct int8
	Table [DAVE_NORMAL_NAME_LEN] byte
	Key [DAVE_KEY_LEN_MAX] byte
	Page_id uint64
	Total_number uint64
	Page_number uint64
	Value *MBUF
	Ptr uint64
}

/* for DBMSG_NOSQL_ADD_TALK_REQ message */
type DBNosqlAddTalkReq struct {
	Portal_data *MBUF
	From_client MCard
	From_server MCard
	Label UniversalLabel
	Model_raw_data *MBUF
	Ptr uint64
}

/* for DBMSG_NOSQL_ADD_TALK_RSP message */
type DBNosqlAddTalkRsp struct {
	Ret int64
	Ptr uint64
}

/* for DBMSG_SYS_ADD_IMAGE_FEATURE_REQ message */
type DBSysAddImageFeatureReq struct {
	Table_name [DAVE_NORMAL_NAME_LEN] byte
	Image_id [DAVE_SHA1_IMAGE_ID] byte
	Point CVKeyPoint
	Mat OpenCVMat
	Vgg_feature [DAVE_VGG_FEATURE_LEN] float32
	Ptr uint64
}

/* for DBMSG_SYS_ADD_IMAGE_FEATURE_RSP message */
type DBSysAddImageFeatureRsp struct {
	Ret int64
	Table_name [DAVE_NORMAL_NAME_LEN] byte
	Image_id [DAVE_SHA1_IMAGE_ID] byte
	Ptr uint64
}

/* for DBMSG_SYS_ADD_WEICHAT_REQ message */
type DBSysAddWeiChatReq struct {
	Info WeiChatUserInfo
	Ptr uint64
}

/* for DBMSG_SYS_ADD_WEICHAT_RSP message */
type DBSysAddWeiChatRsp struct {
	Ret int64
	Ptr uint64
}

/* for DBMSG_SYS_INQ_IMAGE_FEATURE_REQ message */
type DBSysInqImageFeatureReq struct {
	Table_name [DAVE_NORMAL_NAME_LEN] byte
	Table_id uint64
	Image_id [DAVE_KEY_OPT_MAX*DAVE_SHA1_IMAGE_ID] byte
	Ptr uint64
}

/* for DBMSG_SYS_INQ_IMAGE_FEATURE_RSP message */
type DBSysInqImageFeatureRsp struct {
	Ret int64
	Process_number uint64
	Table_name [DAVE_NORMAL_NAME_LEN] byte
	Table_id uint64
	Image_id [DAVE_KEY_OPT_MAX*DAVE_SHA1_IMAGE_ID] byte
	Point [DAVE_KEY_OPT_MAX] CVKeyPoint
	Mat [DAVE_KEY_OPT_MAX] OpenCVMat
	Vgg_feature [DAVE_VGG_FEATURE_LEN] float32
	Process_time uint64
	Ptr uint64
}

/* for DBMSG_SYS_INQ_IMAGE_REQ message */
type DBSysInqImageReq struct {
	Image_id [DAVE_SHA1_IMAGE_ID] byte
	Language_code int32
	Details int8
	Ptr uint64
}

/* for DBMSG_SYS_INQ_IMAGE_RSP message */
type DBSysInqImageRsp struct {
	Ret int64
	Image_id [DAVE_SHA1_IMAGE_ID] byte
	Image ImageIntroduction
	Ptr uint64
}

/* for DBMSG_SYS_INQ_MUSEUM_PAGE_REQ message */
type DBSysInqMuseumPageReq struct {
	Table_id uint64
	Page_id uint64
	Page_number uint64
	Ptr uint64
}

/* for DBMSG_SYS_INQ_MUSEUM_PAGE_RSP message */
type DBSysInqMuseumPageRsp struct {
	Ret int64
	Museum_id [DAVE_SHA1_IMAGE_ID] byte
	Table_id uint64
	Page_id uint64
	Total_number uint64
	Page_number uint64
	Image_id_page [DAVE_DBA_PAGE_MAX*DAVE_SHA1_IMAGE_ID] byte
	Ptr uint64
}

/* for DBMSG_SYS_INQ_MUSEUM_REQ message */
type DBSysInqMuseumReq struct {
	Museum_id [DAVE_SHA1_IMAGE_ID] byte
	Language_code int32
	Details int8
	Ptr uint64
}

/* for DBMSG_SYS_INQ_MUSEUM_RSP message */
type DBSysInqMuseumRsp struct {
	Ret int64
	Museum_id [DAVE_SHA1_IMAGE_ID] byte
	Museum MuseumIntroduction
	Ptr uint64
}

/* for DBMSG_SYS_INQ_PAINTER_PAGE_REQ message */
type DBSysInqPainterPageReq struct {
	Table_id uint64
	Page_id uint64
	Page_number uint64
	Ptr uint64
}

/* for DBMSG_SYS_INQ_PAINTER_PAGE_RSP message */
type DBSysInqPainterPageRsp struct {
	Ret int64
	Painter_id [DAVE_SHA1_IMAGE_ID] byte
	Table_id uint64
	Page_id uint64
	Total_number uint64
	Page_number uint64
	Image_id_page [DAVE_DBA_PAGE_MAX*DAVE_SHA1_IMAGE_ID] byte
	Ptr uint64
}

/* for DBMSG_SYS_INQ_WEICHAT_REQ message */
type DBSysInqWeiChatReq struct {
	Uuid [64] byte
	Openid [64] byte
	Ptr uint64
}

/* for DBMSG_SYS_INQ_WEICHAT_RSP message */
type DBSysInqWeiChatRsp struct {
	Ret int64
	Table_id uint64
	Info WeiChatUserInfo
	Ptr uint64
}

/* for MSGID_DEBUG_REQ message */
type DebugReq struct {
	Msg [4096] byte
	Ptr uint64
}

/* for MSGID_DEBUG_RSP message */
type DebugRsp struct {
	Msg [1048576] byte
	Ptr uint64
}

/* for MSGID_DOS_FORWARD message */
type DosForward struct {
	Cmd *MBUF
	Param *MBUF
	Ptr uint64
}

/* for EMAIL_SEND_REQ message */
type EmailSendReq struct {
	Subject [1024] byte
	Content *MBUF
	Attachment *MBUF
	Ptr uint64
}

/* for EMAIL_SEND_RSP message */
type EmailSendRsp struct {
	Ret int64
	Ptr uint64
}

/* for FREE_MESSAGE_AREA_1 message */
type FreeMessageArea1 struct {
	General_type [256] byte
	General_data *MBUF
	Send_req_us_time uint64
	Recv_req_us_time uint64
	Send_rsp_us_time uint64
	Ptr uint64
}

/* for FREE_MESSAGE_AREA_2 message */
type FreeMessageArea2 struct {
	General_type [256] byte
	General_data *MBUF
	Send_req_us_time uint64
	Recv_req_us_time uint64
	Send_rsp_us_time uint64
	Ptr uint64
}

/* for FREE_MESSAGE_AREA_3 message */
type FreeMessageArea3 struct {
	General_type [256] byte
	General_data *MBUF
	Send_req_us_time uint64
	Recv_req_us_time uint64
	Send_rsp_us_time uint64
	Ptr uint64
}

/* for FREE_MESSAGE_AREA_4 message */
type FreeMessageArea4 struct {
	General_type [256] byte
	General_data *MBUF
	Send_req_us_time uint64
	Recv_req_us_time uint64
	Send_rsp_us_time uint64
	Ptr uint64
}

/* for FREE_MESSAGE_AREA_5 message */
type FreeMessageArea5 struct {
	General_type [256] byte
	General_data *MBUF
	Send_req_us_time uint64
	Recv_req_us_time uint64
	Send_rsp_us_time uint64
	Ptr uint64
}

/* for MSGID_GENERAL_REQ message */
type GeneralReq struct {
	General_type [256] byte
	General_data *MBUF
	General_bin *MBUF
	Send_req_us_time uint64
	Ptr uint64
}

/* for MSGID_GENERAL_RSP message */
type GeneralRsp struct {
	General_type [256] byte
	General_data *MBUF
	General_bin *MBUF
	Send_req_us_time uint64
	Recv_req_us_time uint64
	Send_rsp_us_time uint64
	Ptr uint64
}

/* for HTTPMSG_CLOSE_REQ message */
type HTTPCloseReq struct {
	Listen_port uint64
	Path [DAVE_PATH_LEN] byte
	Ptr uint64
}

/* for HTTPMSG_CLOSE_RSP message */
type HTTPCloseRsp struct {
	Ret int64
	Listen_port uint64
	Path [DAVE_PATH_LEN] byte
	Ptr uint64
}

/* for HTTPMSG_LISTEN_AUTO_CLOSE_REQ message */
type HTTPListenAutoCloseReq struct {
	Path [DAVE_PATH_LEN] byte
	Listening_seconds_time uint64
	Ptr uint64
}

/* for HTTPMSG_LISTEN_AUTO_CLOSE_RSP message */
type HTTPListenAutoCloseRsp struct {
	Ret int64
	Path [DAVE_PATH_LEN] byte
	Ptr uint64
}

/* for HTTPMSG_LISTEN_REQ message */
type HTTPListenReq struct {
	Listen_port uint64
	Rule int32
	Type int32
	Path [DAVE_PATH_LEN] byte
	Ptr uint64
}

/* for HTTPMSG_LISTEN_RSP message */
type HTTPListenRsp struct {
	Ret int64
	Listen_port uint64
	Path [DAVE_PATH_LEN] byte
	Ptr uint64
}

/* for HTTPMSG_POST_REQ message */
type HTTPPostReq struct {
	Url [DAVE_URL_LEN] byte
	Head [DAVE_HTTP_HEAD_MAX] HttpKeyValue
	Content_type int32
	Content *MBUF
	Ptr uint64
}

/* for HTTPMSG_POST_RSP message */
type HTTPPostRsp struct {
	Ret int64
	Head [DAVE_HTTP_HEAD_MAX] HttpKeyValue
	Content *MBUF
	Ptr uint64
}

/* for HTTPMSG_RECV_REQ message */
type HTTPRecvReq struct {
	Listen_port uint64
	Remote_address [DAVE_URL_LEN] byte
	Remote_port uint64
	Method int32
	Head [DAVE_HTTP_HEAD_MAX] HttpKeyValue
	Content_type int32
	Content *MBUF
	Local_creat_time uint64
	Ptr uint64
}

/* for HTTPMSG_RECV_RSP message */
type HTTPRecvRsp struct {
	Ret int64
	Content_type int32
	Content *MBUF
	Local_creat_time uint64
	Ptr uint64
}

/* for MSGID_INTERNAL_EVENTS message */
type InternalEvents struct {
	Event_id uint64
	Ptr uint64
}

/* for MSGID_INTERNAL_LOOP message */
type InternalLoop struct {
	Event_id uint64
	Ptr uint64
}

/* for MAINMSG_PYTHON_REQ message */
type MainMsgPythonReq struct {
	Fun int64
	Opt_param uint64
	File_path [DAVE_PATH_LEN] byte
	Req_data *MBUF
	Ptr uint64
}

/* for MAINMSG_PYTHON_RSP message */
type MainMsgPythonRsp struct {
	Ret int64
	Time uint64
	Rsp_data *MBUF
	Ptr uint64
}

/* for MSGID_ECHO_REQ message */
type MsgIdEchoReq struct {
	Echo MsgIdEcho
	Ptr uint64
}

/* for MSGID_ECHO_RSP message */
type MsgIdEchoRsp struct {
	Echo MsgIdEcho
	Ptr uint64
}

/* for MSGID_INNER_LOOP message */
type MsgInnerLoop struct {
	Param *MBUF
	Ptr uint64
}

/* for MSGID_OS_NOTIFY message */
type MsgOSNotify struct {
	Notify_info uint64
}

/* for MSGID_POWER_OFF message */
type POWEROFFMSG struct {
	Reason [128] byte
}

/* for MSGID_PROCESS_MSG_TIMER_OUT message */
type ProcessMsgTimerOutMsg struct {
	Msg_id uint64
	Msg_len uint64
	Msg_body unsafe.Pointer
}

/* for MSGID_PROTECTOR_REG message */
type ProtectorReg struct {
	Ptr uint64
}

/* for MSGID_PROTECTOR_UNREG message */
type ProtectorUnreg struct {
	Ptr uint64
}

/* for MSGID_PROTECTOR_WAKEUP message */
type ProtectorWakeup struct {
	Ptr uint64
}

/* for MSGID_QUEUE_DOWNLOAD_MESSAGE_REQ message */
type QueueDownloadMsgReq struct {
	Name [DAVE_THREAD_NAME_LEN] byte
	Gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Ptr uint64
}

/* for MSGID_QUEUE_DOWNLOAD_MESSAGE_RSP message */
type QueueDownloadMsgRsp struct {
	Ret int64
	Src_name [DAVE_THREAD_NAME_LEN] byte
	Dst_name [DAVE_THREAD_NAME_LEN] byte
	Src_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Dst_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Msg *MBUF
	Ptr uint64
}

/* for MSGID_QUEUE_RUN_MESSAGE_REQ message */
type QueueRunMsgReq struct {
	Src_name [DAVE_THREAD_NAME_LEN] byte
	Dst_name [DAVE_THREAD_NAME_LEN] byte
	Src_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Dst_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Msg *MBUF
	Ptr uint64
}

/* for MSGID_QUEUE_RUN_MESSAGE_RSP message */
type QueueRunMsgRsp struct {
	Ret int64
	Name [DAVE_THREAD_NAME_LEN] byte
	Msg_number uint64
	Thread_number uint64
	Ptr uint64
}

/* for MSGID_QUEUE_UPDATE_STATE_REQ message */
type QueueUpdateStateReq struct {
	Src_name [DAVE_THREAD_NAME_LEN] byte
	Dst_name [DAVE_THREAD_NAME_LEN] byte
	Src_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Dst_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Msg_number uint64
	Msg *MBUF
	Queue_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Ptr uint64
}

/* for MSGID_QUEUE_UPDATE_STATE_RSP message */
type QueueUpdateStateRsp struct {
	Ret int64
	Src_name [DAVE_THREAD_NAME_LEN] byte
	Dst_name [DAVE_THREAD_NAME_LEN] byte
	Src_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Dst_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Msg *MBUF
	Ptr uint64
}

/* for MSGID_QUEUE_UPLOAD_MESSAGE_REQ message */
type QueueUploadMsgReq struct {
	Src_name [DAVE_THREAD_NAME_LEN] byte
	Dst_name [DAVE_THREAD_NAME_LEN] byte
	Src_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Dst_gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Msg_id uint64
	Msg *MBUF
	Ptr uint64
}

/* for MSGID_QUEUE_UPLOAD_MESSAGE_RSP message */
type QueueUploadMsgRsp struct {
	Ret int64
	Ptr uint64
}

/* for MSGID_RESTART_REQ message */
type RESTARTREQMSG struct {
	Reason [128] byte
	Times uint64
}

/* for MSGID_RESTART_RSP message */
type RESTARTRSPMSG struct {
	Wait_flag int8
}

/* for MSGID_RPC_DEBUG_REQ message */
type RPCDebugReq struct {
	Ret_debug int64
	Req_thread [64] byte
	Str_debug [16] byte
	S8_debug byte
	U8_debug byte
	S16_debug int16
	U16_debug uint16
	S32_debug int32
	U32_debug uint32
	S64_debug int64
	U64_debug uint64
	Float_debug float32
	Double_debug float64
	Void_debug unsafe.Pointer
	Date_debug DateStruct
	Mbuf_debug *MBUF
	Req_time uint64
	Rsp_time uint64
	Ptr uint64
}

/* for MSGID_RPC_DEBUG_RSP message */
type RPCDebugRsp struct {
	Ret_debug int64
	Rsp_thread [64] byte
	Str_debug [16] byte
	S8_debug byte
	U8_debug byte
	S16_debug int16
	U16_debug uint16
	S32_debug int32
	U32_debug uint32
	S64_debug int64
	U64_debug uint64
	Float_debug float32
	Double_debug float64
	Void_debug unsafe.Pointer
	Date_debug DateStruct
	Mbuf_debug *MBUF
	Req_time uint64
	Rsp_time uint64
	Ptr uint64
}

/* for RTC_DATA_REQ message */
type RTCDataReq struct {
	Token [512] byte
	Src [128] byte
	Dst [128] byte
	Sequence_number uint16
	Data_format [64] byte
	Data *MBUF
	Ptr uint64
}

/* for RTC_DATA_RSP message */
type RTCDataRsp struct {
	Token [512] byte
	Src [128] byte
	Dst [128] byte
	Sequence_number uint16
	Data_format [64] byte
	Data *MBUF
	Ptr uint64
}

/* for RTC_REG_REQ message */
type RTCRegReq struct {
	Terminal_type [128] byte
	Terminal_id [128] byte
	Ptr uint64
}

/* for RTC_REG_RSP message */
type RTCRegRsp struct {
	Terminal_type [128] byte
	Terminal_id [128] byte
	Token [512] byte
	Ptr uint64
}

/* for RTC_TRANSLATION_DATA_REQ message */
type RTCTranslationDataReq struct {
	Translation_id [128] byte
	Sequence_number uint64
	Payload_data *MBUF
	Ptr uint64
}

/* for RTC_TRANSLATION_DATA_RSP message */
type RTCTranslationDataRsp struct {
	Translation_id [128] byte
	Sequence_number uint64
	Payload_data *MBUF
	Ptr uint64
}

/* for RTC_TRANSLATION_START_REQ message */
type RTCTranslationStartReq struct {
	Translation_id [128] byte
	Src_lang [128] byte
	Dst_lang [128] byte
	Ptr uint64
}

/* for RTC_TRANSLATION_START_RSP message */
type RTCTranslationStartRsp struct {
	Ret int64
	Gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Translation_id [128] byte
	Src_lang [128] byte
	Dst_lang [128] byte
	Ptr uint64
}

/* for RTC_TRANSLATION_STOP_REQ message */
type RTCTranslationStopReq struct {
	Gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Translation_id [128] byte
	Ptr uint64
}

/* for RTC_TRANSLATION_STOP_RSP message */
type RTCTranslationStopRsp struct {
	Ret int64
	Gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Translation_id [128] byte
	Ptr uint64
}

/* for RTC_UNREG_REQ message */
type RTCUnregReq struct {
	Terminal_type [128] byte
	Terminal_id [128] byte
	Token [512] byte
	Ptr uint64
}

/* for RTC_UNREG_RSP message */
type RTCUnregRsp struct {
	Terminal_type [128] byte
	Terminal_id [128] byte
	Token [512] byte
	Ptr uint64
}

/* for RTP_DATA_REQ message */
type RTPDataReq struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Payload_type byte
	Sequence_number uint16
	Timestamp uint32
	Ssrc uint32
	Payload_data *MBUF
	Ptr uint64
}

/* for RTP_DATA_RSP message */
type RTPDataRsp struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Payload_type byte
	Sequence_number uint16
	Timestamp uint32
	Ssrc uint32
	Payload_data *MBUF
	Ptr uint64
}

/* for RTP_START_REQ message */
type RTPStartReq struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Ptr uint64
}

/* for RTP_START_RSP message */
type RTPStartRsp struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Ptr uint64
}

/* for RTP_STOP_REQ message */
type RTPStopReq struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Ptr uint64
}

/* for RTP_STOP_RSP message */
type RTPStopRsp struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Ptr uint64
}

/* for MSGID_RUN_FUNCTION message */
type RUNFUNCTIONMSG struct {
	Thread_fun unsafe.Pointer
	Last_fun unsafe.Pointer
	Param unsafe.Pointer
	Run_thread_id uint64
	Initialization_flag int8
}

/* for SIP_BYE_REQ message */
type SIPByeReq struct {
	Call_id [128] byte
	Phone_number [512] byte
	Ptr uint64
}

/* for SIP_BYE_RSP message */
type SIPByeRsp struct {
	Ret int64
	Call_id [128] byte
	Phone_number [512] byte
	Ptr uint64
}

/* for SIP_CALL_REQ message */
type SIPCallReq struct {
	Phone_number [512] byte
	Ptr uint64
}

/* for SIP_CALL_RSP message */
type SIPCallRsp struct {
	Ret int64
	Call_id [128] byte
	Phone_number [512] byte
	Ptr uint64
}

/* for SIP_START_REQ message */
type SIPStartReq struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Ptr uint64
}

/* for SIP_START_RSP message */
type SIPStartRsp struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Ptr uint64
}

/* for SIP_STOP_REQ message */
type SIPStopReq struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Ptr uint64
}

/* for SIP_STOP_RSP message */
type SIPStopRsp struct {
	Call_id [128] byte
	Call_from [128] byte
	Call_to [128] byte
	Ptr uint64
}

/* for SOCKET_BIND_REQ message */
type SocketBindReq struct {
	Netinfo SocNetInfo
	Ptr uint64
}

/* for SOCKET_BIND_RSP message */
type SocketBindRsp struct {
	Socket int32
	Netinfo SocNetInfo
	Bindinfo int32
	Thread_id uint64
	Ptr uint64
}

/* for SOCKET_CONNECT_REQ message */
type SocketConnectReq struct {
	Netinfo SocNetInfo
	Ptr uint64
}

/* for SOCKET_CONNECT_RSP message */
type SocketConnectRsp struct {
	Socket int32
	Netinfo SocNetInfo
	Connectinfo int32
	Thread_id uint64
	Ptr uint64
}

/* for SOCKET_DISCONNECT_REQ message */
type SocketDisconnectReq struct {
	Socket int32
	Ptr uint64
}

/* for SOCKET_DISCONNECT_RSP message */
type SocketDisconnectRsp struct {
	Socket int32
	Result int32
	Ptr uint64
}

/* for SOCKET_NOTIFY message */
type SocketNotify struct {
	Socket int32
	Notify int32
	Data uint64
	Ptr uint64
}

/* for SOCKET_PLUGIN message */
type SocketPlugIn struct {
	Father_socket int32
	Child_socket int32
	Netinfo SocNetInfo
	Thread_id uint64
	Ptr uint64
}

/* for SOCKET_PLUGOUT message */
type SocketPlugOut struct {
	Socket int32
	Reason int32
	Netinfo SocNetInfo
	Thread_id uint64
	Ptr uint64
}

/* for SOCKET_RAW_EVENT message */
type SocketRawEvent struct {
	Socket int32
	Os_socket int32
	Event int32
	Netinfo SocNetInfo
	Data *MBUF
	Ptr uint64
}

/* for SOCKET_READ message */
type SocketRead struct {
	Socket int32
	Ipinfo IPBaseInfo
	Data_len uint64
	Data *MBUF
	Ptr uint64
}

/* for SOCKET_WRITE message */
type SocketWrite struct {
	Socket int32
	Ipinfo IPBaseInfo
	Data_len uint64
	Data *MBUF
	Close_flag int32
}

/* for STORE_MYSQL_REQ message */
type StoreMysqlReq struct {
	Sql *MBUF
	Ptr uint64
}

/* for STORE_MYSQL_RSP message */
type StoreMysqlRsp struct {
	Ret int64
	Msg [4096] byte
	Data *MBUF
	Ptr uint64
}

/* for STORE_REDIS_REQ message */
type StoreRedisReq struct {
	Command *MBUF
	Ptr uint64
}

/* for STORE_REDIS_RSP message */
type StoreRedisRsp struct {
	Ret int64
	Msg [4096] byte
	Reply *MBUF
	Ptr uint64
}

/* for MSGID_SYSTEM_BUSY message */
type SystemBusy struct {
	Gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Verno [DAVE_VERNO_STR_LEN] byte
	Ptr uint64
}

/* for MSGID_SYSTEM_DECOUPLING message */
type SystemDecoupling struct {
	Socket int32
	Verno [DAVE_VERNO_STR_LEN] byte
	Netinfo SocNetInfo
}

/* for MSGID_SYSTEM_IDLE message */
type SystemIdle struct {
	Gid [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
	Verno [DAVE_VERNO_STR_LEN] byte
	Ptr uint64
}

/* for MSGID_SYSTEM_MOUNT message */
type SystemMount struct {
	Socket int32
	Verno [DAVE_VERNO_STR_LEN] byte
	Netinfo SocNetInfo
}

/* for MSGID_TEST message */
type TESTMSG struct {
	Test_msg [4096] byte
}

/* for MSGID_TIMER message */
type TIMERMSG struct {
	Timer_id int64
}

/* for MSGID_TEMPORARILY_DEFINE_MESSAGE message */
type TemporarilyDefineMessageMsg struct {
	Parameter unsafe.Pointer
}

/* for MSGID_THREAD_BUSY message */
type ThreadBusy struct {
	Thread_id uint64
	Thread_name [DAVE_THREAD_NAME_LEN] byte
	Msg_number uint64
}

/* for MSGID_THREAD_IDLE message */
type ThreadIdle struct {
	Thread_id uint64
	Thread_name [DAVE_THREAD_NAME_LEN] byte
}

/* for MSGID_LOCAL_THREAD_READY message */
type ThreadLocalReadyMsg struct {
	Local_thread_id uint64
	Local_thread_name [128] byte
	Thread_flag uint64
}

/* for MSGID_LOCAL_THREAD_REMOVE message */
type ThreadLocalRemoveMsg struct {
	Local_thread_id uint64
	Local_thread_name [128] byte
	Thread_flag uint64
}

/* for MSGID_REMOTE_THREAD_ID_READY message */
type ThreadRemoteIDReadyMsg struct {
	Remote_thread_id uint64
	Remote_thread_name [128] byte
	Globally_identifier [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
}

/* for MSGID_REMOTE_THREAD_ID_REMOVE message */
type ThreadRemoteIDRemoveMsg struct {
	Remote_thread_id uint64
	Remote_thread_name [128] byte
	Globally_identifier [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
}

/* for MSGID_REMOTE_THREAD_READY message */
type ThreadRemoteReadyMsg struct {
	Remote_thread_id uint64
	Remote_thread_name [128] byte
	Globally_identifier [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
}

/* for MSGID_REMOTE_THREAD_REMOVE message */
type ThreadRemoteRemoveMsg struct {
	Remote_thread_id uint64
	Remote_thread_name [128] byte
	Globally_identifier [DAVE_GLOBALLY_IDENTIFIER_LEN] byte
}

/* for MSGID_TRACE_SWITCH message */
type TraceSwitchMsg struct {
	Thread_id uint64
	Trace_on int8
}

/* for UIP_DATA_RECV_REQ message */
type UIPDataRecvReq struct {
	Remote_address [DAVE_URL_LEN] byte
	Remote_port uint64
	Uip_type int32
	Channel [DAVE_NORMAL_NAME_LEN] byte
	Method [DAVE_UIP_METHOD_MAX_LEN] byte
	Customer_body *MBUF
	Data *MBUF
	Ptr uint64
}

/* for UIP_DATA_RECV_RSP message */
type UIPDataRecvRsp struct {
	Ret int64
	Method [DAVE_UIP_METHOD_MAX_LEN] byte
	Data *MBUF
	Ptr uint64
}

/* for UIP_DATA_SEND_REQ message */
type UIPDataSendReq struct {
	Remote_url [DAVE_URL_LEN] byte
	Channel [DAVE_NORMAL_NAME_LEN] byte
	Method [DAVE_UIP_METHOD_MAX_LEN] byte
	Customer_head *MBUF
	Customer_body *MBUF
	Data *MBUF
	Ptr uint64
}

/* for UIP_DATA_SEND_RSP message */
type UIPDataSendRsp struct {
	Ret int64
	Method [DAVE_UIP_METHOD_MAX_LEN] byte
	Data *MBUF
	Ptr uint64
}

/* for UIP_REGISTER_REQ message */
type UIPRegisterReq struct {
	Method [DAVE_UIP_METHOD_MAX_NUM*DAVE_UIP_METHOD_MAX_LEN] byte
	Ptr uint64
}

/* for UIP_REGISTER_RSP message */
type UIPRegisterRsp struct {
	Ret int64
	Method [DAVE_UIP_METHOD_MAX_NUM*DAVE_UIP_METHOD_MAX_LEN] byte
	Ptr uint64
}

/* for UIP_UNREGISTER_REQ message */
type UIPUnregisterReq struct {
	Method [DAVE_UIP_METHOD_MAX_NUM*DAVE_UIP_METHOD_MAX_LEN] byte
	Ptr uint64
}

/* for UIP_UNREGISTER_RSP message */
type UIPUnregisterRsp struct {
	Ret int64
	Method [DAVE_UIP_METHOD_MAX_NUM*DAVE_UIP_METHOD_MAX_LEN] byte
	Ptr uint64
}

/* for MSGID_WAKEUP message */
type WAKEUPMSG struct {
	Null_msg unsafe.Pointer
	Some_msg uint32
}

