// dates.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"

str
Date()
{
	str retval = char_str(NULL,32);
	time_t now = time(NULL);		
	struct tm* now_tm = gmtime(&now);	
	retval->len = strftime(retval->data,retval->len,"%a, %d %b %Y %H:%M:%S +0000",now_tm);
	return retval;
}
