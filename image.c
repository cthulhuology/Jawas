// image.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "server.h"
#include "image.h"

int
img_handler(Server srv, File fc, Response resp)
{
	resp->raw_contents = fc;		
	return 200;
}
