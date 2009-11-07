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
	int total = 0;
	while (total < fc->st.st_size)
		total += send_raw_contents(Req->sc,fc,total,1);
	return 200;
}
