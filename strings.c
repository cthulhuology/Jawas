// strings.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"
#include "strings.h"

char* Cache_Control_MSG = "Cache-Control";
char* Connection_MSG = "Connection";
char* Date_MSG = "Date";
char* Transfer_Encoding_MSG = "Transfer-Encoding";
char* Content_Length_MSG = "Content-Length";
char* Content_Type_MSG = "Content-Type";
char* Expires_MSG = "Expires";
char* Location_MSG = "Location";
char* Server_MSG = "Server";

const_str(OK);
const_str(ERROR);
const_str(CMSS);
const_str(CMGW);
const_str(CMGR);
const_str(CMGD);
const_str(CMTI);

int
init_strings()
{
	init_str(OK);
	init_str(ERROR);
	init_str(CMSS);
	init_str(CMGW);
	init_str(CMGR);
	init_str(CMGD);
	init_str(CMTI);
	return 0;
}
