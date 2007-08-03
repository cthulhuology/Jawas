// mime.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "server.h"
#include "jws.h"
#include "image.h"
#include "mime.h"

MimeTypes*
lookup_mimetype(char* filename)
{
	MimeTypes* mt;
	int i,l = strlen(filename);
	for (i = l - 1; i > 0; --i)
		if (filename[i-1] == '.') break;
	l -= i;
	for (mt = mime_types; mt->ending.len; ++mt)
		if (l == mt->ending.len && !strncmp(mt->ending.data,&filename[i],l))
			return mt;
	return mt;
}

int
mimetype_handler(File fc)
{
	MimeTypes* mt;
	if (!fc) 
		return error_handler(404);
	mt = lookup_mimetype(fc->name);
	content_type(Resp->headers, mt->type.data);
	return mt->handler(fc);
}
