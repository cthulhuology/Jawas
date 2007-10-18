// smpp.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_SMPP_H__
#define __HAVE_SMPP_H__

typedef struct smpp_header_struct* SMPP_HEADER;
typedef struct smpp_header_struct* SMPP_GENERIC_NAK;
typedef struct smpp_bind_struct* SMPP_BIND_RX;
typedef struct smpp_bind_struct* SMPP_BIND_TX;
typedef struct smpp_bind_resp_struct* SMPP_BIND_RX_RESP;
typedef struct smpp_bind_resp_struct* SMPP_BIND_TXR_RESP;
typedef struct smpp_header_struct* SMPP_UNBIND:
typedef struct smpp_header_struct* SMPP_UNBIND_RESP:
typedef struct smpp_


struct smpp_header_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char message[0];
};

struct smpp_bind_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char system_id[16];
	char password[9];
	char system_type[13];
	char interface_version;
	char addr_ton;
	char addr_npi;
	char address_range[41];
};

struct smpp_bind_resp_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char sytem_id[16];
};

struct smpp_submit_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char service_type[6];
	char source_addr_ton;
	char source_addr_npi;
	char source_addr[21];
	char dest_addr_ton;
	char dest_addr_npi;
	char destination_addr[21];
	char esm_class;
	char protocol_id;
	char priority_flag;
	char schedule_delivery_time[17];
	char validity_period[17];
	char registered_delivery_flag;
	char replace_if_present_flag;
	char data_coding;
	char sms_default_msg_id;
	char sm_length;
	char short_message[161];
};

struct smpp_submit_resp_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char message_id[9];
};

struct smpp_query_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char orig_message_id[9];
	char orig_ton;
	char orig_npi;
	char orig_addr[21];
};

struct smpp_query_resp_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char orig_message_id[9];
	char final_date[17];
	char message_status;
	char error_code;	
};

struct smpp_cancel_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char service_type[6];
	char orig_message_id[9];
	char source_addr_ton;
	char source_addr_npi;
	char source_addr[21];
	char dest_addr_ton;
	char dest_addr_npi;
	char dest_addr[21];
};

struct smpp_replace_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char orig_message_id[9];
	char source_addr_ton;
	char source_addr_npi;
	char source_addr[21];
	char schedule_delivery_time[17];
	char validity_period[17];
	char registered_delivery_flag;
	char default_msg_id;
	char sm_length;
	char short_message[161];
};

struct smpp_param_retrieve_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char param_name[32];
};

struct smpp_param_retrieve_resp_struct {
	int cmd_len;
	int cmd_id;
	int cmd_status;
	int seq_no;
	char param_value[101];
};


#endif
