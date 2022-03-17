/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"

// =====================================================================

s8 *
t_a2b_errorstr(ErrCode code)
{
	static s8 static_buffer[16];
	s8 *buffer;

	switch(code)
	{
		case ERRCODE_OK:
				buffer = (s8 *)"'OK'";
			break;
		case ERRCODE_Memory_full:
				buffer = (s8 *)"'Memory full'";
			break;
		case ERRCODE_Limited_resources:
				buffer = (s8 *)"'Limited resources'";
			break;
		case ERRCODE_Arithmetic_error:
				buffer = (s8 *)"'Arithmetic error'";
			break;
        case ERRCODE_Unknown_error:
                buffer = (s8 *)"'Unknown error'";
            break;
        case ERRCODE_Can_not_find_node:
				buffer = (s8 *)"'Can not find node'";
			break;
		case ERRCODE_Send_failed:
				buffer = (s8 *)"'Send failed'";
			break;
		case ERRCODE_Invalid_data:
				buffer = (s8 *)"'Invalid data'";
			break;
		case ERRCODE_Unsupported_type:
				buffer = (s8 *)"'Unsupported type'";
			break;
		case ERRCODE_Invalid_user_name:
				buffer = (s8 *)"'Invalid user name'";
			break;
		case ERRCODE_Invalid_device:
				buffer = (s8 *)"'Invalid device'";
			break;
		case ERRCODE_Invalid_password:
				buffer = (s8 *)"'Invalid password'";
			break;
		case ERRCODE_Invalid_data_crc_check:
				buffer = (s8 *)"'Invalid data crc check'";
			break;
		case ERRCODE_Invalid_parameter:
				buffer = (s8 *)"'Invalid parameter'";
			break;
		case ERRCODE_Send_msg_failed:
				buffer = (s8 *)"'Send msg failed'";
			break;
		case ERRCODE_Resource_conflicts:
				buffer = (s8 *)"'Resource conflicts'";
			break;
		case ERRCODE_Request_failed:
				buffer = (s8 *)"'request failed'";
			break;
		case ERRCODE_can_not_find_ret_code:
				buffer = (s8 *)"'can not find ret code'";
			break;
		case ERRCODE_user_is_registered:
				buffer = (s8 *)"'user is registered'";
			break;
		case ERRCODE_connect_error:
				buffer = (s8 *)"'connect error'";
			break;
		case ERRCODE_function_not_supported:
				buffer = (s8 *)"'function not supported'";
			break;
		case ERRCODE_wait:
				buffer = (s8 *)"'wait'";
			break;
		case ERRCODE_Invalid_state:
				buffer = (s8 *)"'Invalid state'";
			break;
		case ERRCODE_timer_out:
				buffer = (s8 *)"'timer out'";
			break;
		case ERRCODE_invalid_type:
				buffer = (s8 *)"'invalid type'";
			break;
		case ERRCODE_user_does_not_exist:
				buffer = (s8 *)"'user does not exist'";
			break;
		case ERRCODE_ptr_null:
				buffer = (s8 *)"'ptr null'";
			break;
		case ERRCODE_record_not_exist:
				buffer = (s8 *)"'record not exist'";
			break;
		case ERRCODE_db_store_failed:
				buffer = (s8 *)"'DB store Failed'";
			break;
		case ERRCODE_db_not_find:
				buffer = (s8 *)"'DB not find'";
			break;
		case ERRCODE_Invalid_db_store:
				buffer = (s8 *)"'invalid db store'";
			break;
		case ERRCODE_db_sql_failed:
				buffer = (s8 *)"'db sql failed'";
			break;
		case ERRCODE_invalid_option:
				buffer = (s8 *)"'invalid option'";
			break;
		case ERRCODE_Invalid_domain:
				buffer = (s8 *)"'Invalid domain'";
			break;
		case ERRCODE_Invalid_auth_key:
				buffer = (s8 *)"'Invalid auth key'";
			break;
		case ERRCODE_invalid_key:
				buffer = (s8 *)"'invalid key'";
			break;
		case ERRCODE_no_account_found:
				buffer = (s8 *)"'no account found'";
			break;
		case ERRCODE_invalid_phone_number:
				buffer = (s8 *)"'Invalid phone number'";
			break;
		case ERRCODE_not_match_domain:
				buffer = (s8 *)"'not match domain'";
			break;
		case ERRCODE_invalid_account:
				buffer = (s8 *)"'invalid account'";
			break;
		case ERRCODE_db_init_failed:
				buffer = (s8 *)"'db init failed'";
			break;
		case ERRCODE_lost_auth_key:
				buffer = (s8 *)"'lost auth key'";
			break;
		case ERRCODE_lost_serial:
				buffer = (s8 *)"'lost serial'";
			break;
		case ERRCODE_lost_time:
				buffer = (s8 *)"'lost time'";
			break;
		case ERRCODE_lost_uuid:
				buffer = (s8 *)"'lost uuid'";
			break;
		case ERRCODE_lost_user:
				buffer = (s8 *)"'lost user'";
			break;
		case ERRCODE_can_not_find_record:
				buffer = (s8 *)"'can not find record'";
			break;
		case ERRCODE_lost_link:
				buffer = (s8 *)"'lost link'";
			break;
		case ERRCODE_record_not_found:
				buffer = (s8 *)"'record not found'";
			break;
		case ERRCODE_ims_account_incorrect:
				buffer = (s8 *)"'ims account incorrect'";
			break;
		case ERRCODE_invalid_dial_number:
				buffer = (s8 *)"'invalid dial number'";
			break;
		case ERRCODE_Invalid_balance:
				buffer = (s8 *)"'invalid balance'";
			break;
		case ERRCODE_user_exist:
				buffer = (s8 *)"'user exist'";
			break;
		case ERRCODE_store_data_failed:
				buffer = (s8 *)"'store data failed'";
			break;
		case ERRCODE_invalid_user:
				buffer = (s8 *)"'invalid user'";
			break;
		case ERRCODE_unsupport:
				buffer = (s8 *)"'unsupport'";
			break;
		case ERRCODE_Invalid_channel:
				buffer = (s8 *)"'invalid channel'";
			break;
		case ERRCODE_Invalid_package:
				buffer = (s8 *)"'Invalid package'";
			break;
		case ERRCODE_Invalid_rules:
				buffer = (s8 *)"'Invalid rules'";
			break;
		case ERRCODE_busy:
				buffer = (s8 *)"'busy'";
			break;
		case ERRCODE_invalid_account_name:
				buffer = (s8 *)"'invalid account name'";
			break;
		case ERRCODE_Account_in_use:
				buffer = (s8 *)"'Account in use'";
			break;
		case ERRCODE_can_not_find_rules:
				buffer = (s8 *)"'can not find rules'";
			break;
		case ERRCODE_refuse_call_self:
				buffer = (s8 *)"'refuse call self'";
			break;
		case ERRCODE_Invalid_call:
				buffer = (s8 *)"'Invalid call'";
			break;
		case ERRCODE_Invalid_call_type:
				buffer = (s8 *)"'Invalid call type'";
			break;
		case ERRCODE_Invalid_billing:
				buffer = (s8 *)"'Invalid billing'";
			break;
		case ERRCODE_create_thread_failed:
				buffer = (s8 *)"'create thread failed'";
			break;
		case ERRCODE_execute_sql_failed:
				buffer = (s8 *)"'execute sql failed'";
			break;
		case ERRCODE_decode_failed:
				buffer = (s8 *)"'decode failed'";
			break;
		case ERRCODE_encode_failed:
				buffer = (s8 *)"'encode failed'";
			break;
		case ERRCODE_invalid_file:
				buffer = (s8 *)"'invalid file'";
			break;
		case ERRCODE_file_open_failed:
				buffer = (s8 *)"'file open failed'";
			break;
		case ERRCODE_add_user_name_failed:
				buffer = (s8 *)"'add user name failed'";
			break;
		case ERRCODE_call_time_limited:
				buffer = (s8 *)"'call_time_limited'";
			break;
		case ERRCODE_invalid_hour:
				buffer = (s8 *)"'invalid hour'";
			break;
		case ERRCODE_invalid_year:
				buffer = (s8 *)"'invalid year'";
			break;
		case ERRCODE_invalid_rules_number:
				buffer = (s8 *)"'invalid rules number'";
			break;
		case ERRCODE_channel_not_exist:
				buffer = (s8 *)"'channel not exist'";
			break;
		case ERRCODE_invalid_donation_account:
				buffer = (s8 *)"'invalid donation account'";
			break;
		case ERRCODE_channel_exist:
				buffer = (s8 *)"'channel exist'";
			break;
		case ERRCODE_invalid_date:
				buffer = (s8 *)"'invalid date'";
			break;
		case ERRCODE_not_my_data:
				buffer = (s8 *)"'not my data'";
			break;
		case ERRCODE_can_not_find_thread:
				buffer = (s8 *)"'can not find thread'";
			break;
		case ERRCODE_lost_sip_password:
				buffer = (s8 *)"'lost sip password'";
			break;
		case ERRCODE_modify_impu_failed:
				buffer = (s8 *)"'modify impu failed'";
			break;
		case ERRCODE_impu_not_existed:
				buffer = (s8 *)"'number not existed'";
			break;
		case ERRCODE_script_execution_error:
				buffer = (s8 *)"'script execution error'";
			break;
		case ERRCODE_Duplicate_MSISDN:
				buffer = (s8 *)"'Duplicate MSISDN'";
			break;
		case ERRCODE_Duplicate_IMSI:
				buffer = (s8 *)"'Duplicate IMSI'";
			break;
		case ERRCODE_Duplicate_VOIP_MSISDN:
				buffer = (s8 *)"'Duplicate VOIP MSISDN'";
			break;
		case ERRCODE_Not_found:
				buffer = (s8 *)"'Not found'";
			break;
		case ERRCODE_Failure_to_be_actived:
				buffer = (s8 *)"'Boot in the territory'";
			break;
		case ERRCODE_Other_errors:
				buffer = (s8 *)"'Other errors'";
			break;
		case ERRCODE_lost_result_code:
				buffer = (s8 *)"'lost result code'";
			break;
		case ERRCODE_invalid_imsi:
				buffer = (s8 *)"'invalid imsi'";
			break;
		case ERRCODE_invalid_msisdn:
				buffer = (s8 *)"'invalid msisdn'";
			break;
		case ERRCODE_invalid_voip_msisdn:
				buffer = (s8 *)"'invalid voip msisdn'";
			break;
		case ERRCODE_processing_is_complete:
				buffer = (s8 *)"'processing is complete'";
			break;
		case ERRCODE_invalid_content:
				buffer = (s8 *)"'invalid content'";
			break;
		case ERRCODE_secondary_number_ass_repeated:
				buffer = (s8 *)"'secondary_number_ass_repeated'";
			break;
		case ERRCODE_secondary_number_not_available:
				buffer = (s8 *)"'secondary_number_not_available'";
			break;
		case ERRCODE_secondary_number_ret_failed:
				buffer = (s8 *)"'secondary_number_ret_failed'";
			break;
		case ERRCODE_secondary_number_inquiry_failed:
				buffer = (s8 *)"'secondary_number_inquiry_failed'";
			break;
		case ERRCODE_lost_secondary_number:
				buffer = (s8 *)"'lost_secondary_number'";
			break;
		case ERRCODE_invalid_e164_number_type:
				buffer = (s8 *)"'invalid_e164_number_type'";
			break;
		case ERRCODE_lost_number_request_type:
				buffer = (s8 *)"'lost_number_request_type'";
			break;
		case ERRCODE_repeated_request:
				buffer = (s8 *)"'repeated request'";
			break;
		case ERRCODE_rules_exist:
				buffer = (s8 *)"'rules exist'";
			break;
		case ERRCODE_lack_of_donor:
				buffer = (s8 *)"'lack of donor'";
			break;
		case ERRCODE_lost_imsi:
				buffer = (s8 *)"'lost imsi'";
			break;
		case ERRCODE_lost_binding_status:
				buffer = (s8 *)"'lost binding status'";
			break;
		case ERRCODE_package_exist:
				buffer = (s8 *)"'package exist'";
			break;
		case ERRCODE_invalid_billing_user_data:
				buffer = (s8 *)"'invalid billing user data'";
			break;
		case ERRCODE_Failure_to_activate_validity:
				buffer = (s8 *)"'Failure to activate validity'";
			break;
		case ERRCODE_Time_lapse:
				buffer = (s8 *)"'Time lapse'";
			break;
		case ERRCODE_Billing_reached_the_limit:
				buffer = (s8 *)"'Billing reached the limit'";
			break;
		case ERRCODE_invalid_recharge:
				buffer = (s8 *)"'invalid recharge'";
			break;
		case ERRCODE_Date_expired:
				buffer = (s8 *)"'date expired'";
			break;
		case ERRCODE_lost_package_info:
				buffer = (s8 *)"'lost package info'";
			break;
		case ERRCODE_not_access:
				buffer = (s8 *)"'not access'";
			break;
		case ERRCODE_invalid_minute:
				buffer = (s8 *)"'invalid minute'";
			break;
		case ERRCODE_invalid_second:
				buffer = (s8 *)"'invalid second'";
			break;
		case ERRCODE_not_do_anything:
				buffer = (s8 *)"'not do anything'";
			break;
		case ERRCODE_empty_data:
				buffer = (s8 *)"'empty data'";
			break;
		case ERRCODE_on_billing_operation_not_allowed:
				buffer = (s8 *)"'on billing operation not allowed'";
			break;
		case ERRCODE_roaming_not_allowed:
				buffer = (s8 *)"'Roaming is not allowed'";
			break;
		case ERRCODE_invalid_mcard:
				buffer = (s8 *)"'invalid mcard'";
			break;
		case ERRCODE_Too_many_packages:
				buffer = (s8 *)"'Too many packages'";
			break;
		case ERRCODE_Too_many_rules:
				buffer = (s8 *)"'Too many rules'";
			break;
		case ERRCODE_Failure_to_be_deactived:
				buffer = (s8 *)"'Pls deactive before cancel'";
			break;
		case ERRCODE_operating_end:
				buffer = (s8 *)"'operating end'";
			break;
		case ERRCODE_Call_barring:
				buffer = (s8 *)"'Call barring'";
			break;
		case ERRCODE_forbidden_register_user:
				buffer = (s8 *)"'Forbidden register user'";
			break;
		case ERRCODE_Unauthorized:
				buffer = (s8 *)"'Unauthorized'";
			break;
		case ERRCODE_table_exist:
				buffer = (s8 *)"'table exist'";
			break;
		case ERRCODE_record_empty:
				buffer = (s8 *)"'record empty'";
			break;
		case ERRCODE_impu_existed:
				buffer = (s8 *)"'impu existed'";
			break;
		case ERRCODE_invalid_context:
				buffer = (s8 *)"'invalid context'";
			break;
		case ERRCODE_authorized_failed:
				buffer = (s8 *)"'authorized failed'";
			break;
		case ERRCODE_invalid_url:
				buffer = (s8 *)"'invalid url'";
			break;
		case ERRCODE_invalid_country_code_of_e164_number:
				buffer = (s8 *)"'invalid country code'";
			break;
		case ERRCODE_Threshold_limit:
				buffer = (s8 *)"'Threshold limit'";
			break;
		case ERRCODE_Channel_closed:
				buffer = (s8 *)"'Channe closed'";
			break;
		case ERRCODE_invalid_request:
				buffer = (s8 *)"'invalid request'";
			break;
		case ERRCODE_Duplicate_name:
				buffer = (s8 *)"'Duplicate name'";
			break;
		case ERRCODE_lost_parameter:
				buffer = (s8 *)"'lost parameter'";
			break;
		case ERRCODE_lost_protocol:
				buffer = (s8 *)"'lost protocol'";
			break;
		case ERRCODE_system_not_ready:
				buffer = (s8 *)"'system not ready'";
			break;
		case ERRCODE_package_check:
				buffer = (s8 *)"'package check'";
			break;
		case ERRCODE_package_discard:
				buffer = (s8 *)"'package discard'";
			break;
		case ERRCODE_resend:
				buffer = (s8 *)"'resend'";
			break;
		case ERRCODE_Invalid_object:
				buffer = (s8 *)"'Invalidobject'";
			break;
		case ERRCODE_over_max_package_types:
				buffer = (s8 *)"'Over max package types'";
			break;
		case ERRCODE_over_max_package_numbers:
				buffer = (s8 *)"'Over max package numbers'";
			break;
		case ERRCODE_wait_more_data:
				buffer = (s8 *)"'wait more data'";
			break;
		case ERRCODE_unregister:
				buffer = (s8 *)"'unregister'";
			break;
		case ERRCODE_maximum_intents:
				buffer = (s8 *)"'maximum intents'";
			break;
		case ERRCODE_intercept:
				buffer = (s8 *)"'intercept'";
			break;
		case ERRCODE_repetitive_operation:
				buffer = (s8 *)"'repetitive_operation'";
			break;
		case ERRCODE_Failed_to_identify:
				buffer = (s8 *)"'Failed_to_identify'";
			break;
		case ERRCODE_Failed_to_features:
				buffer = (s8 *)"'Failed_to_features'";
			break;
		case ERRCODE_No_clear_intention:
				buffer = (s8 *)"'No_clear_intention'";
			break;
		case ERRCODE_db_no_data:
				buffer = (s8 *)"'database_no_data'";
			break;
		case ERRCODE_insufficient_balance:
				buffer = (s8 *)"'insufficient_balance'";
			break;
		case ERRCODE_lost_lock:
				buffer = (s8 *)"'did_no_grab_the_lock'";
			break;
		case ERRCODE_lost_req_type:
				buffer = (s8 *)"'lost_req_type'";
			break;
		case ERRCODE_can_not_find_client:
				buffer = (s8 *)"'can_not_find_client'";
			break;
		case ERRCODE_valuation:
				buffer = (s8 *)"'valuation'";
			break;
		case ERRCODE_internal_server_error:
				buffer = (s8 *)"'internal server error'";
			break;
		case ERRCODE_update:
				buffer = (s8 *)"'update'";
			break;
		case ERRCODE_lost_atom_ptl:
				buffer = (s8 *)"'lost atom protocol'";
			break;
		case ERRCODE_can_not_find_user:
				buffer = (s8 *)"'can not find user'";
			break;
		case ERRCODE_can_not_find_cost_price:
				buffer = (s8 *)"'can not find cost price'";
			break;
		case ERRCODE_can_not_find_unit_price:
				buffer = (s8 *)"'can not find cost price'";
			break;
		case ERRCODE_name_already_exist:
				buffer = (s8 *)"'This name already exist'";
			break;
		case ERRCODE_cfg_already_exists:
				buffer = (s8 *)"'This configuration already exists'";
			break;
		case ERRCODE_storage:
				buffer = (s8 *)"'storage'";
			break;
		case ERRCODE_Can_not_find_path:
				buffer = (s8 *)"'Can not find path'";
			break;
		case ERRCODE_over_time:
				buffer = (s8 *)"'over time'";
			break;
		case ERRCODE_soft_delete:
				buffer = (s8 *)"soft delete";
			break;
		case ERRCODE_Not_found_ptl_name:
				buffer = (s8 *)"Not found PtlName";
			break;
		case ERRCODE_RELEASE:
				buffer = (s8 *)"Release";
			break;
		case ERRCODE_Not_found_billing:
				buffer = (s8 *)"Not found billing";
			break;
		case ERRCODE_Can_not_find_area:
				buffer = (s8 *)"Can not find area";
			break;
		case ERRCODE_User_Unregister:
				buffer = (s8 *)"User unregister";
			break;
		case ERRCODE_Stateless_Sending:
				buffer = (s8 *)"Stateless sending";
			break;
		case ERRCODE_Blacklist_intercept:
				buffer = (s8 *)"Blacklist intercept";
			break;
		case ERRCODE_data_not_exist:
				buffer = (s8 *)"Data not exist";
			break;
		case ERRCODE_Invalid_length:
				buffer = (s8 *)"Invalid length";
			break;
		default:
				dave_snprintf(static_buffer, sizeof(static_buffer), "[%ld]", (sb)code);
				buffer = static_buffer;
			break;
	}

	return buffer;
}

