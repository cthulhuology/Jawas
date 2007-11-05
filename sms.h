// sms.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_SMS_H__
#define __HAVE_SMS_H__

#define MAX_SMS_MESSAGE 512
#define SMS_STACK_SIZE 64

int sms_open_modem();
int sms_init_modem();
int sms_set_text_mode();
int sms_set_delivery_mode();
int sms_process_incoming();
int sms_query_unread();
int sms_query_all();
int sms_delete_msg(str index);
int sms_read_msg(str index);
int sms_write_msg(str number,str text);
int sms_send_msg(str index);
int sms_process_stack();
int sms_process_line();
int sms_wait();
int sms_again();
int sms_next();

extern int modem;

#endif
