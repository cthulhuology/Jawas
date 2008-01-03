// sms.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "events.h"
#include "server.h"
#include "database.h"
#include "strings.h"
#include "sms.h"

int modem = -1;

typedef int(*sms_t)();

sms_t orig_sms_stack[] = {
	sms_wait,
	sms_set_delivery_mode,
	sms_set_text_mode,
	sms_init_modem,
	NULL
};
	
sms_t* sms_stack;
int sms_sp = 0;
str sms_line;

int
out_modem(str msg)
{
	int total = 0;
	if (modem < 0) return 1;
	str t;
	for(t = msg; t; t = t->next)
		total += write(modem,t->data,t->length);
	debug("[SMS] sent %s",msg);
	return total != len(msg);
}

str
in_modem()
{
	int i,delta;	
	str retval = blank(MAX_SMS_MESSAGE);
	retval->length = 0;
	for (i = 0; i < MAX_SMS_MESSAGE; ++i) {
		char tmp;
		delta = read(modem,&tmp,1);
		if (delta == 0 && i == 0) return NULL;
		if (tmp == '\n') break;	
		if (tmp != '\n' && tmp != '\r') {
			set(retval,i,tmp);
			retval->length += delta;
		}
	}
	if (retval->length > 0) 
		debug("[SMS] read %s", retval);
	return retval->length > 0 ? retval : NULL;	
}

str
read_ok()
{
	str tmp, retval = NULL;
	for (tmp = in_modem(); !tmp || !cmp(tmp,OK); tmp = in_modem())
		if (tmp) retval = append(retval,tmp);
	return retval;
}

int
sms_push(sms_t func)
{
	if (sms_sp < SMS_STACK_SIZE - 1) {
		sms_stack[++sms_sp] = func;
		return 0;
	}
	error("SMS STACK OVERFLOW!");
	return 1;
}

int
sms_init_stack()
{
	int i;
	sms_sp = 0;
	sms_stack = (sms_t*)salloc(sizeof(sms_t)*SMS_STACK_SIZE);
	for (i = 0; orig_sms_stack[i]; ++i) {
		sms_stack[i] = orig_sms_stack[i];
		sms_sp = i;
	}
	for (; i < SMS_STACK_SIZE; ++i)
		sms_stack[i] = NULL;
	return 0;
}

int
sms_open_modem()
{
	if (modem > 0) return 0;
	sms_init_stack();
	modem = open("/dev/tty.Bluetooth-Modem",O_RDWR,0600);
	if (modem < 0) {
		error("Failed to open modem!");
		return 1;	
	}
	nonblock(modem);
	return 0;
}

int
sms_init_modem()
{
	return out_modem(Str("AT\r\n"));	
}

int
sms_set_text_mode()
{
	return out_modem(Str("AT+CMGF=1\r\n"));
}

int
sms_set_delivery_mode() 
{
	return out_modem(Str("AT+CNMI=3,1,0,0,0\r\n"));
}

int
sms_query_unread()
{
        return out_modem(Str("AT+CMGL=\"REC UNREAD\"\r\n"));
}

int
sms_query_all()
{
        return out_modem(Str("AT+CMGL=\"ALL\"\r\n"));
}

int
sms_delete_msg(str index) 
{
        return out_modem(Str("AT+CMGD=%s\r\n",index));
}

int
sms_read_msg(str index)
{
        return out_modem(Str("AT+CMGR=%s\r\n",index));
}

int
sms_write_msg(str number,str text)
{
        return out_modem(Str("AT+CMGW=\"%s\"\r\n%s\r\n\032",number,text));
}

int
sms_send_msg(str index) 
{
        return out_modem(Str("AT+CMSS=%s\r\n",index));
}

str
parse_sms_cmd(str line)
{
	int i,l = len(line);
	str retval = NULL;
	if (cmp(line,OK)) return OK;
	if (ncmp(line,ERROR,5)) return ERROR;
	if (at(line,0) != '+') return NULL;
	for (i = 1; i < l && at(line,i) != ':'; ++i); 
	if (i >= l) return NULL;
	retval = from(line, 1, i - 1);
	debug("[SMS] Found CMD %s",retval);
	return retval;
}

int
sms_process_stack()
{
	if (sms_open_modem()) return 1;
	debug("SMS PROCESS STACK");
	sms_t func = sms_stack[sms_sp];
	return func ? func() : sms_init_stack();
}

int
sms_wait()
{
	debug("SMS WAIT");
	return 0;
}

int
sms_read_wait()
{
	str type = dequote(sms_line);
	str index = skip_fields(sms_line,1);
	debug("Incoming Type: %s Index: %s",type,index);
	return sms_read_msg(index);
}

int
sms_write_wait()
{
	str index  = skip_fields(sms_line,0);
	debug("Wrote index: %s",index);
	return sms_send_msg(index);
}

int
sms_send_wait()
{
	str index  = skip_fields(sms_line,0);
	debug("Sent Index: %s",index);
	return sms_delete_msg(index);
}

int
sms_next ()
{
	debug("SMS NEXT");
	if (sms_sp > 0) --sms_sp;
	return sms_process_stack();
}

int
sms_again ()
{
	debug("SMS AGAIN");
	return sms_process_stack();
}

int
sms_callback ()
{
	str number = dequote(skip_fields(sms_line,1));
	str date = dequote(skip_fields(sms_line,2));
	str body = read_ok();
	debug("Received Number: %s Date: %s Body: [%s]",number,date,body);
	Timers t = add_timer(Str("sms/incoming.js"),0);
	set_timer_value(t,Str("number"),number);
	set_timer_value(t,Str("date"),date);
	set_timer_value(t,Str("body"),body);
	return 0;
}

struct sms_cmd_struct {
	char* cmd;
	sms_t func;
} sms_cmds[] = {
	{ "OK", sms_next },
	{ "ERROR", sms_again },
	{ "CMTI", sms_read_wait },
	{ "CMGR", sms_callback },
	{ "CMGW", sms_write_wait },
	{ "CMSS", sms_send_wait },
	{ "CMGD", sms_next },
	{ NULL, NULL }
};

int
dispatch_sms_cmd(str cmd) 
{
	int i;
	int cl = len(cmd);
	for (i = 0; cmd && sms_cmds[i].cmd; ++i) 
		if (cl == strlen(sms_cmds[i].cmd) &&!strncmp(sms_cmds[i].cmd,cmd->data,cl))
			return sms_cmds[i].func();
	return 0;
}

int
sms_process_line()
{
	if (sms_open_modem()) return 1;
	while ((sms_line = in_modem())) 
		dispatch_sms_cmd(parse_sms_cmd(sms_line));	
	return 0;
}
