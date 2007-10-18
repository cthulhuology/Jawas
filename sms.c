// sms.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "events.h"
#include "server.h"
#include "sms.h"

int modem = -1;

int
out_modem(str msg)
{
	int total = 0;
	while (total < msg->len)
		total += write(modem,msg->data + total,msg->len-total);
	return total != msg->len;
}

str
in_modem()
{

}

int
sms_open_modem()
{
	modem = open("/dv/tty.Bluetooth-Modem",O_RDWR,0600);
	if (modem < 0) {
		error("Failed to open modem!");
		return 1;	
	}
	monitor_socket(modem);
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
        return out_modem(Str("AT+CMGR=%sr\n",index));
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
	for (i = 1; i < line->len && line->data[i] != ":", ++i); 
	if (i >= line->len) return NULL;
	retval =  char_str(line->data + 1, i - 1);
	debug("[SMS] Found CMD %s",retval);
	return retval;
}

void
sms_read_ack()
{
	str line,cmd;
	while (line = in_modem()) {
		if (cmp_str(line,Str("OK"))) {
			debug("[MODEM] OK");
		} else if (ncmp_str(line,Str("ERROR",5))) {
			debug("[MODEM] ERROR: %s",line);
		} else if (cmd = parse_sms_cmd(line)) {
			File fc = load(file_path(char_str("sms",0),cmd));
			run_script(fc,NULL);
		}
	}
}
