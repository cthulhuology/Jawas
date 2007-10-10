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

str
Expires()
{
	time_t now = 120 + time(NULL);
	struct tm* now_tm = gmtime(&now);	
	return Str("%i",timegm(now_tm));	
}

str
MicroTime()
{
	struct timeval tv;
	if (gettimeofday(&tv,NULL))
		return Expires();
	return Str("%i", (tv.tv_sec << 20) + tv.tv_usec);
}

