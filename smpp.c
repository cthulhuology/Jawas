// smpp.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#define SMPP_HEADER "iiii"

str
SMPP(const char* fmt, ...)
{
	int i, j, o, l, len = strlen(fmt);
	str retval = char_str(NULL,MAX_SMPP_MSG_SIZE);
	va_list args;
	va_strat(args,fmt);
	int o = 0;
	for (i = 0; i < len; ++i) {
		switch(fmt[i]) {
		case 'i':
			int x = va_arg(args,int);
			*((int*)&retval->data[o]) = x; 
			o += 4;
			break;
		case 'c':
			fixed = 1;
		case 'v':
			char* v = va_arg(args,char*);
			l = 0;
			for (j = 1; '0' <= fmt[i+j] && fmt[i+j] <= '9'; ++j)
				l += (fmt[i+j] - '0') + 10*l;
			char* v = va_arg(args,char*);
			for (j = 0; j < l && (v[j] || fixed); ++j)
				retval->data[o++] = v[j];
			break;
		case 'b':
			char b = va_arg(args,char);
			retval->data[o++] = b;
			break;
		default:
			error("Unknown format character in %c",fmt);
			break;
		}
	}
	retval->data[o] = '\0';
	retval->len = o;
	*((int*)&retval->data[0]) = o; // set command length
	va_end(args);
	return retval;
}

Socket
smpp_transmitter()
{
	str msg = SMPP(SMPP_HEADER "v16v9v13bbbv41", 
		0, 		// command length
		ESME_BNDTRN,	// command id
		0,		// command status
		smpp_tx_seq,	// sequence no.
		SMPP_SYS_ID,	// system id
		SMPP_PASSWD,	// password
		SMPP_SYS_TYPE,	// system_type
		SMPP_VERSION,	// interface version
		0,		// type of number
		0,		// numbering plan identity 
		0);		// address range
	Socket retval = connect_socket(SMPP_HOST,SMPP_PORT); 
	write_socket(retval,msg->data,msg->len);
	return retval;
}

Socket
smpp_reciever()
{
	Socket retval = connect_socket(SMPP_HOST,SMPP_PORT); 
	if (! retval) return NULL;
	str msg = SMPP(SMPP_HEADER "v16v9v13bbbv41", 
		0, 		// command length
		ESME_BNDRCV,	// command id
		0,		// command status
		retval->seq++,	// sequence no.
		SMPP_SYS_ID,	// system id
		SMPP_PASSWD,	// password
		SMPP_SYS_TYPE,	// system_type
		SMPP_VERSION,	// interface version
		SMPP_TON,	// type of number
		SMPP_NPI,	// numbering plan identity 
		SMPP_ADDRS);	// address range
	write_socket(retval,msg->data,msg->len);
	return retval;
}

int
smpp_close(Socket s)
{
	str msg = SMPP(SMPP_HEADER, 0, ESME_UBD, 0, s->seq++);
	write_socket(s,msg->data,msg->len);
	close_socket(s);	
}

int
smpp_send(Socket s, str number, str text)
{
	str msg = SMPP(SMPP_HEADER "v6bbv21bbv21bbbc17c17bbbbbv161", 
		0, 		// command length
		ESME_SUB_SM, 	// command id
		0, 		// status
		s->seq++,	// sequence no
		SMPP_SERV_TYPE,	// service type
		SMPP_SRC_TON,	// source type of number
		SMPP_SRC_NPI,	// source numer plan identity
		SMPP_SRC_ADDR,	// source address of message
		SMPP_DST_TON,	// destination type of number
		SMPP_DST_NPI,	// destination number plan identity
		number->data,	// destination address of message
		0,		// esm_class = NULL on submit_sm
		SMPP_GSM_PROT,	// gsm protocol id (cf gsm 03.40.[2] 9.2.3.9)
		0,		// priority flag 0 = non-priority, 1 = priority
		delivery_time(),// scheduled delivery time 
		expires_time(),	// expries time
		1,		// registered delivery, expect receipt
		0,		// replace message, nope
		SMPP_CODING,	// data coding gsm 03.40 [2] 9.2.3.10
		0,		// default message, never going to use
		text->len,	// length of message
		text->data);	// message body
	return write_socket(s,msg->data,msg->len);
}

int
smpp_keepalive(Socket s)
{
	str msg = SMPP(SMPP_HEADER "", 0, ESME_QRYLINK, 0, s->seq++);
	return write_socket(s,msg->data,msg->len);
}

int
smpp_nak(Socket s)
{
	str msg = SMPP(SMPP_HEADER "", 0, ESME_NACK, 0, s->seq++);
	return write_socket(s,msg->data,msg->len);
}

Headers
smpp_read(Socket s)
{

}


