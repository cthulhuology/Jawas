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
	str ending;
	str type;
	int (*handler)(Server,File,Response);
};

int mimetype_handler(Server srv, File fc, Response resp);

#endif
