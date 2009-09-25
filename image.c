// image.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "server.h"
#include "image.h"

int
img_handler(File fc)
{
	append_header(Resp->headers,Str("Cache-Control"),Str("max-age=3600, public"));
	Resp->raw_contents = fc;		
	return 200;
}
