// mime.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "server.h"
#include "jws.h"
#include "image.h"
#include "mime.h"

MimeTypes mime_types[] = {
	{ 4, "html",   9, "text/html"               , jws_handler },
	{ 3, "css" ,   8, "text/css"                , jws_handler },
	{ 2, "js"  ,  24, "application/x-javascript", jws_handler },
	{ 3, "jws" ,   9, "text/html"               , jws_handler },
	{ 3, "xml" ,   8, "text/xml"                , jws_handler },
	{ 3, "txt" ,  10, "text/plain"              , jws_handler },
	{ 3, "ico" ,  12, "image/x-icon"            , img_handler },
	{ 3, "jpg" ,  10, "image/jpeg"              , img_handler },
	{ 3, "png" ,   9, "image/png"               , img_handler },
	{ 3, "gif" ,   9, "image/gif"               , img_handler },
	{ 3, "svg" ,  13, "image/svg+xml"           , jws_handler },
	{ 3, "mpg" ,  10, "video/mpeg"              , img_handler },
	{ 3, "mp4" ,   9, "video/mp4"               , img_handler },
	{ 3, "mov" ,  15, "video/quicktime"         , img_handler },
	{ 3, "pdf" ,  15, "application/pdf"         , img_handler },
	{ 2, "ps"  ,  22, "application/postscript"  , img_handler },
	{ 3, "eps" ,  22, "application/postscript"  , img_handler },
	{ 0, NULL  ,  24, "application/octet-stream", img_handler },
};

MimeTypes*
lookup_mimetype(char* filename)
{
	MimeTypes* mt;
	int i,l = strlen(filename) - 1;
	for (i = l; i > 0; --i)
		if (filename[i-1] == '.') break;
	l -= i;
	for (mt = mime_types; mt->ending.len; ++mt) {
		if (l == mt->ending.len
		&& !strncmp(mt->ending.data,&filename[i],l))
			return mt;
	}
	return mt;
}

int
mimetype_handler(Server srv, File fc, Response resp)
{
	MimeTypes* mt;
	if (!srv || !fc || !resp) return 404;
	mt = lookup_mimetype(fc->name);
	content_type(resp->headers, mt->type.data);
	return mt->handler(srv,fc,resp);
}

