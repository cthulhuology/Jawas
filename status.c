// Status
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "uri.h"
#include "status.h"

int
find_status_code(int code)
{
	int i;
//	debug("finding status of code %i", code);
	for (i = 0; stati[i].code; ++i) {
		if (stati[i].code == code) return i;
	}
	return find_status_code(500);
}

char*
status_line(int code)
{
	int i = find_status_code(code);
	if (stati[i].len == 0) {
		stati[i].len = strlen(stati[i].reason);
	}
	char* line = (char*)salloc(10 + stati[i].len);
	memset(line,0,10+stati[i].len);
	strncpy(line,http_version,9);
	strncpy(line+9,stati[i].reason,stati[i].len);
	return line;
}

int
error_handler(int code)
{
	int i = find_status_code(code);
	if (! stati[i].filename) return code;
	File fc = load(file_path(char_str("errors",0),char_str(stati[i].filename,0)));
	if (! fc) return code;
	Resp->raw_contents = fc;
	return code;				
}
