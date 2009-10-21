// mail.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "log.h"
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

	Socket sc = connect_socket(Str(MAILHOST),25,0);
	if (! sc) {
		error("Failed to connnect to STMP server");
		return 1;
	}
	debug("[SMTP] %s", read_socket(sc));
	write_socket(sc,hl);
	debug("[SMTP] %s", read_socket(sc));
	write_socket(sc,mf);
	debug("[SMTP] %s", read_socket(sc));
	write_socket(sc,rt);
	debug("[SMTP] %s", read_socket(sc));
	write_socket(sc,dt);
	debug("[SMTP] %s", read_socket(sc));
	write_socket(sc,th);
	write_socket(sc,sb);
	write_socket(sc,ct);
	write_socket(sc,eoh);
	write_socket(sc,body);
	write_socket(sc,dn);
	debug("[SMTP] %s", read_socket(sc));
	write_socket(sc,qt);
	debug("[SMTP] %s", read_socket(sc));
	return 0;
}
