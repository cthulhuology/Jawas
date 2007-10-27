// sms.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "events.h"
#include "server.h"
#include "database.h"
#include "sms.h"

int modem = -1;

typedef int(*sms_t)();

int sms_stack_index = 0;
struct sms_stack_struct {
	char* name;
	sms_t func;
} sms_stack[] = {
	{ "INIT_MODEM", sms_init_modem },
	{ "TEXT_MODE", sms_set_text_mode },
	{ "DELIVERY_MODE", sms_set_delivery_mode },
	{ "PROCESS_INCOMING", sms_process_incoming },
	{ NULL, NULL}
};
	

int
out_modem(str msg)
{
	int total = 0;
	while (total < msg->len)
		total += write(modem,msg->data + total,msg->len-total);
	debug("[SMS] sent %s",msg);
	return total != msg->len;
}

str
in_modem()
{
	int i,delta;	
	str retval = char_str(NULL,MAX_SMS_MESSAGE);
	retval->len = 0;
	for (i = 0; i < MAX_SMS_MESSAGE; ++i) {
		char tmp;
		delta = read(modem,&tmp,1);
		if (delta == 0 && i == 0) return NULL;
		retval->data[i] = '\0';
		if (tmp == '\n') break;	
		if (tmp != '\n' && tmp != '\r') {
			retval->data[i] = tmp;
			retval->len += delta;
		}
	}
	retval->data[i] = '\0';
	if (retval->len > 0) 
		debug("[SMS] read %s", retval);
	return retval->len > 0 ? retval : NULL;	
}

str
read_ok()
{
	str tmp, retval = NULL;
	for (tmp = in_modem(); !tmp || !cmp_str(tmp,Str("OK")); tmp = in_modem())
		if (tmp) retval = retval ? Str("%s%s",retval,tmp) : tmp;
	return retval;
}

int
sms_open_modem()
{
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
sms_process_incoming()
{
	return 0;
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
	int i;
	str retval = NULL;
	if (line->data[0] != '+') return NULL;
	for (i = 1; i < line->len && line->data[i] != ':'; ++i); 
	if (i >= line->len) return NULL;
	retval =  char_str(line->data + 1, i - 1);
	debug("[SMS] Found CMD %s",retval);
	return retval;
}

str 
skip_fields(str line, int n)
{
	int i;
	if (n == 0) {
		for(i = 0; i < line->len && line->data[i] != ':'; ++i);
		if (i < line->len) return char_str(line->data + i + 2, line->len - i - 2);
		return line;
	}
	for (i = 0; i < line->len && n; ++i) 
		if (line->data[i] == ',') 
			--n;
	return char_str(line->data + i, line->len - i);
}

str
dequote(str line)
{
	int i, j;
	for (i = 0; i < line->len && line->data[i] != '"'; ++i);
	for (j = i+1; j < line->len && line->data[j] != '"'; ++j);
	return char_str(line->data + i + 1, j - i - 1);
} 

void
sms_read_ack()
{
	str line,cmd;
	while (line = in_modem()) {
		debug("Line is [%s] len %i",line,line->len);
		if (cmp_str(line,Str("OK"))) {
			debug("[MODEM] OK");
 			++sms_stack_index;
			sms_process();
		} else if (ncmp_str(line,Str("ERROR"),5)) {
			debug("[MODEM] ERROR: %s",line);
			sms_process();
			return;
		} else if (cmd = parse_sms_cmd(line)) {
			if (cmp_str(cmd,Str("CMTI"))) {
				str type = dequote(line);
				str index = skip_fields(line,1);
				debug("Incoming Type: %s Index: %s",type,index);
				sms_read_msg(index);
			}
			if (cmp_str(cmd,Str("CMGR"))) {
				str number = dequote(skip_fields(line,1));
				str date = dequote(skip_fields(line,2));
				str body = read_ok();
				debug("Received Number: %s Date: %s Body: [%s]",number,date,body);
				Database db = new_database();
				if (0 > query(Str("INSERT INTO sms_inbound (id, number, date, body) VALUES (nextval('guid'), '%s', 'now', '%s')"))) {
					error("Failed to insert SMS message from %s", number);
				}
				close_database(db);
			}
			if (cmp_str(cmd,Str("CMGW"))) {
				str index  = skip_fields(line,0);
				debug("Wrote index: %s",index);
				sms_send_msg(index);
			}
			if (cmp_str(cmd,Str("CMSS"))) {
				str index  = skip_fields(line,0);
				debug("Sent Index: %s",index);
			}
		} else {
			debug("[MODEM] [%s]",line);
		}
	}
}

void
sms_process()
{
	debug("SMS process");
	sms_t func = sms_stack[sms_stack_index].func;
	char* name = sms_stack[sms_stack_index].name;
	if (!func) {
		sms_stack_index = 0;
		return sms_process();	
	}
	int retval = func();
	debug("[SMS] %c returned %i",name,retval);
}
