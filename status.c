// Status
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "alloc.h"
#include "uri.h"
#include "status.h"

struct status_struct {
	int code;
	int len;
	char* reason;
	char* filename;
};

static struct status_struct stati[] = {
	{ 100, 0, "100 Continue\r\n", NULL },
	{ 101, 0, "101 Switching Protocols\r\n", NULL },
	{ 200, 0, "200 OK\r\n", NULL },
	{ 201, 0, "201 Created\r\n", NULL },
	{ 202, 0, "202 Accepted\r\n", NULL },
	{ 203, 0, "203 Non-Authoritative Information\r\n", NULL },
	{ 204, 0, "204 No Content\r\n", NULL },
	{ 205, 0, "205 Reset Content\r\n", NULL },
	{ 206, 0, "206 Partial Content\r\n", NULL },
	{ 300, 0, "300 Multiple Choices\r\n", NULL },
	{ 301, 0, "301 Moved Permanently\r\n", NULL },
	{ 302, 0, "302 Found\r\n", NULL },
	{ 303, 0, "303 See Other\r\n", NULL },
	{ 304, 0, "304 Not Modified\r\n", NULL },
	{ 305, 0, "305 Use Proxy\r\n", NULL },
	{ 307, 0, "307 Temporary Redirect\r\n", NULL },
	{ 400, 0, "400 Bad Request\r\n", "/400.html" },
	{ 401, 0, "401 Unauthroized\r\n", NULL },
	{ 402, 0, "402 Payment Required\r\n", NULL },
	{ 403, 0, "403 Forbidden\r\n", NULL },
	{ 404, 0, "404 Not Found\r\n", "/404.html" },
	{ 405, 0, "405 Method Not Allowed\r\n", NULL },
	{ 406, 0, "406 Not Acceptable\r\n", NULL },
	{ 407, 0, "407 Proxy Authentication Required\r\n", NULL },
	{ 408, 0, "408 Request Time-out\r\n", NULL },
	{ 409, 0, "409 Conflict\r\n", NULL },
	{ 410, 0, "410 Gone\r\n", NULL },
	{ 411, 0, "411 Length Required\r\n", NULL },
	{ 412, 0, "412 Precondition Failed\r\n", NULL },
	{ 413, 0, "413 Request Entity Too Large\r\n", NULL },
	{ 414, 0, "414 Request-URI Too Large\r\n", NULL },
	{ 415, 0, "415 Unsupported Media Type\r\n", NULL },
	{ 416, 0, "416 Requested range not satisfiable\r\n", NULL },
	{ 417, 0, "417 Expectation Failed\r\n", NULL },
	{ 500, 0, "500 Internal Server Error\r\n", "/500.html" },
	{ 501, 0, "501 Not Implemented\r\n", NULL },
	{ 502, 0, "502 Bad Gateway\r\n", NULL },
	{ 503, 0, "503 Service Unavailable\r\n", NULL },
	{ 504, 0, "504 Gateway Time-out\r\n", NULL },
	{ 505, 0, "505 HTTP Version not supported\r\n", NULL },
	{ 0, 0, NULL, NULL }
};

static char* http_version = "HTTP/1.0 ";

int
find_status_code(int code)
{
	int i;
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
error_handler(Server srv, int code, Response resp)
{
	int i = find_status_code(code);
	if (! stati[i].filename) return code;
	File fc = load(srv,file_path(char_str("errors",0),char_str(stati[i].filename,0)));
	if (! fc) return code;
	resp->raw_contents = fc;
	return code;				
}
