// Status
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"

struct status_struct {
	int code;
	int len;
	char* reason;
};

static struct status_struct stati[] = {
	{ 100, 0, "100 Continue\r\n" },
	{ 101, 0, "101 Switching Protocols\r\n" },
	{ 200, 0, "200 OK\r\n" },
	{ 201, 0, "201 Created\r\n" },
	{ 202, 0, "202 Accepted\r\n" },
	{ 203, 0, "203 Non-Authoritative Information\r\n" },
	{ 204, 0, "204 No Content\r\n" },
	{ 205, 0, "205 Reset Content\r\n" },
	{ 206, 0, "206 Partial Content\r\n" },
	{ 300, 0, "300 Multiple Choices\r\n" },
	{ 301, 0, "301 Moved Permanently\r\n" },
	{ 302, 0, "302 Found\r\n" },
	{ 303, 0, "303 See Other\r\n" },
	{ 304, 0, "304 Not Modified\r\n" },
	{ 305, 0, "305 Use Proxy\r\n" },
	{ 307, 0, "307 Temporary Redirect\r\n" },
	{ 400, 0, "400 Bad Request\r\n" },
	{ 401, 0, "401 Unauthroized\r\n" },
	{ 402, 0, "402 Payment Required\r\n" },
	{ 403, 0, "403 Forbidden\r\n" },
	{ 404, 0, "404 Not Found\r\n" },
	{ 405, 0, "405 Method Not Allowed\r\n" },
	{ 406, 0, "406 Not Acceptable\r\n" },
	{ 407, 0, "407 Proxy Authentication Required\r\n" },
	{ 408, 0, "408 Request Time-out\r\n" },
	{ 409, 0, "409 Conflict\r\n" },
	{ 410, 0, "410 Gone\r\n" },
	{ 411, 0, "411 Length Required\r\n" },
	{ 412, 0, "412 Precondition Failed\r\n" },
	{ 413, 0, "413 Request Entity Too Large\r\n" },
	{ 414, 0, "414 Request-URI Too Large\r\n" },
	{ 415, 0, "415 Unsupported Media Type\r\n" },
	{ 416, 0, "416 Requested range not satisfiable\r\n" },
	{ 417, 0, "417 Expectation Failed\r\n" },
	{ 500, 0, "500 Internal Server Error\r\n" },
	{ 501, 0, "501 Not Implemented\r\n" },
	{ 502, 0, "502 Bad Gateway\r\n" },
	{ 503, 0, "503 Service Unavailable\r\n" },
	{ 504, 0, "504 Gateway Time-out\r\n" },
	{ 505, 0, "505 HTTP Version not supported\r\n" },
	{ 0, 0, NULL }
};

static char* http_version = "HTTP/1.1 ";

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
	char* line = (char*)malloc(10 + stati[i].len);
	memset(line,0,10+stati[i].len);
	strncpy(line,http_version,9);
	strncpy(line+9,stati[i].reason,stati[i].len);
	return line;
}
