// dates.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "str.h"

str
Date(time_t now)
{
	str retval = blank(32);
	struct tm* now_tm = gmtime(&now);	
	retval->length = strftime(retval->data,retval->length,"%a, %d %b %Y %H:%M:%S +0000",now_tm);
	return retval;
}

str
Expires()
{
	time_t now = 86400 + time(NULL);
	return Date(now);
}

str
MicroTime()
{
	struct timeval tv;
	if (gettimeofday(&tv,NULL))
		return Expires();
	return _("%i", (tv.tv_sec << 20) + tv.tv_usec);
}

