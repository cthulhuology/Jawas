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
	struct { int len; char* data; } ending;
	struct { int len; char* data; } type;
	int (*handler)(File);
};

int mimetype_handler(File fc);

#endif
