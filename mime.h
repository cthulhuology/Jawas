// mime.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_MIME_H__
#define __HAVE_MIME_H__

#include "str.h"

typedef struct mime_struct MimeTypes;
struct mime_struct {
	struct cstr_struct ending;
	struct cstr_struct type;
	int (*handler)(File);
};

static MimeTypes mime_types[] = {
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
	{ 3, "mp3" ,   9, "audio/mp3"               , img_handler },
	{ 3, "mov" ,  15, "video/quicktime"         , img_handler },
	{ 3, "pdf" ,  15, "application/pdf"         , img_handler },
	{ 2, "ps"  ,  22, "application/postscript"  , img_handler },
	{ 3, "eps" ,  22, "application/postscript"  , img_handler },
	{ 3, "swf" ,  29, "application/x-shockwave-flash", img_handler },
	{ 0, NULL  ,  24, "application/octet-stream", img_handler },
};

int mimetype_handler(File fc);

#endif
