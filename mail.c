// mail.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "sockets.h"
#include "mail.h"

int
mail(str to, str from, str subject, str body)
{
	// Commands
	str hl = Str("HELO %c\r\n", MAILHOST);
	str mf = Str("MAIL FROM: <%s>\r\n",from);
	str rt = Str("RCPT TO: <%s>\r\n",to);
	str dt = Str("DATA\r\n");
	str dn = Str("\r\n.\r\n");
	str qt = Str("QUIT\r\n");
	str eoh = Str("\r\n");

	// Headers
	str th = Str("To: %s\r\n", to);
	str sb = Str("Subject: %s\r\n", subject);
	str ct = Str("Content-Type: text/plain\r\n");

	Socket sc = connect_socket(MAILHOST,25);
	if (! sc) {
		error("Failed to connnect to STMP server");
		return 1;
	}
	debug("[SMTP] %s", readstr_socket(sc));
	write_socket(sc,hl->data,hl->len);
	debug("[SMTP] %s", readstr_socket(sc));
	write_socket(sc,mf->data,mf->len);
	debug("[SMTP] %s", readstr_socket(sc));
	write_socket(sc,rt->data,rt->len);
	debug("[SMTP] %s", readstr_socket(sc));
	write_socket(sc,dt->data,dt->len);	
	debug("[SMTP] %s", readstr_socket(sc));
	write_socket(sc,th->data,th->len);
	write_socket(sc,sb->data,sb->len);
	write_socket(sc,ct->data,ct->len);
	write_socket(sc,eoh->data,eoh->len);
	write_socket(sc,body->data,body->len);
	write_socket(sc,dn->data,dn->len);
	debug("[SMTP] %s", readstr_socket(sc));
	write_socket(sc,qt->data,qt->len);	
	debug("[SMTP] %s", readstr_socket(sc));
	return 0;
}
