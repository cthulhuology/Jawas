// mime.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_MIME_H__
#define __HAVE_MIME_H__

#include "str.h"
#include "files.h"

int mimetype_handler(File fc);
int parseable_file(File fc);

#endif
