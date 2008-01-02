// transfer.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_TRANSFER_H__
#define __HAVE_TRANSFER_H__

#include "str.h"
#include "headers.h"
#include "files.h"

int is_chunked(Headers headers);
int chunked_length(str src);
int inbound_content_length(str src, Headers headers);
int outbound_content_length(str src, File fc);

#endif
