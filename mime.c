// mime.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "log.h"
#include "client.h"
#include "lws.h"
#include "image.h"
#include "script.h"
#include "status.h"
#include "mime.h"

static MimeTypes mime_types[] = {
	{ {4, "html"}, {9, "text/html"                }, script_handler },
	{ {5, "shtml"}, {9, "text/html"               }, img_handler },
	{ {3, "lws"},  { 9, "text/html"               }, lws_handler },
	{ {3, "lws"},  { 9, "text/html"		      }, lws_handler },
	{ {3, "xml"},  { 8, "text/xml"                }, lws_handler },
	{ {3, "txt"},  {10, "text/plain"              }, script_handler },
	{ {3, "svg"},  {13, "image/svg+xml"           }, lws_handler },
	{ {3, "css"},  { 8, "text/css"                }, script_handler },
	{ {2, "js" },  {24, "application/x-javascript"}, script_handler },
	{ {3, "ico"},  {12, "image/x-icon"            }, img_handler },
	{ {3, "jpg"},  {10, "image/jpeg"              }, img_handler },
	{ {3, "png"},  { 9, "image/png"               }, img_handler },
	{ {3, "gif"},  { 9, "image/gif"               }, img_handler },
	{ {3, "mpg"},  {10, "video/mpeg"              }, img_handler },
	{ {3, "mp4"},  { 9, "video/mp4"               }, img_handler },
	{ {3, "mp3"},  { 9, "audio/mp3"               }, img_handler },
	{ {3, "mov"},  {15, "video/quicktime"         }, img_handler },
	{ {3, "pdf"},  {15, "application/pdf"         }, img_handler },
	{ {3, "zip"},  {15, "application/zip"	      }, img_handler },
	{ {2, "ps" },  {22, "application/postscript"  }, img_handler },
	{ {3, "eps"},  {22, "application/postscript"  }, img_handler },
	{ {3, "swf"},  {29, "application/x-shockwave-flash"}, img_handler },
	{ {3, "ttf"},  {24, "application/octet-stream"}, img_handler },
	{ {3, "otf"},  {24, "application/octet-stream"}, img_handler },
	{ {0, NULL },  {24, "application/octet-stream"}, img_handler },
};


MimeTypes*
lookup_mimetype(char* filename)
{
	MimeTypes* mt;
	int i,l = strlen(filename);
	debug("Local: %c",filename);
	for (i = l - 1; i > 0; --i)
		if (filename[i-1] == '.') break;
	l -= i;
	for (mt = mime_types; mt->ending.len; ++mt)
		if (l == mt->ending.len && !strncmp(mt->ending.data,&filename[i],l))
			break;
	debug("Mime: %c (%c)",mt->type.data,&filename[i]);
	return mt;
}

int
mimetype_handler(File fc)
{
	if (!fc)  {
		error("File not found 404");
		return error_handler(404);
	}
	if (!fc->mime) fc->mime = lookup_mimetype(fc->name.data);
	content_type(client.response->headers, fc->mime->type.data);
	return fc->mime->handler(fc);
}

int
parseable_file(File fc)
{
	if (!fc->mime) fc->mime = lookup_mimetype(fc->name.data);
	return fc->mime->handler == lws_handler;
}
