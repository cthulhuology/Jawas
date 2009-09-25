// script.c
//
// Copyright (C) 2009 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "server.h"
#include "image.h"

int
script_handler(File fc)
{
	Resp->raw_contents = fc;
	return 200;
}
