// defines.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#define SERVER_VERSION "Jawas/1.0"

#define DB_CONNECT_STRING "dbname=jawas user=jawas"

#define LOG_LEVEL 2

#define SERVER_PORT "80"
#define TLS_SERVER_PORT "443"

#define TLS_KEYFILE "/Users/dave/Desktop/Jawas/server.pem"
#define TLS_PASSWORD "fullfathomfive"

#define PAGE_GUARD 1
#define CACHE_PAGES 125000

#define NUM_BUFFER_SIZE 20
#define MAX_INDEX_LEN 12

#define NODE_FLAGS NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE

#define min(x,y) (x < y ? x : y)

#define HEADER_FUNC(f,k) \
Headers \
f (Headers headers, char* value) {\
	int i = free_header_slot(headers);\
	 headers[i].key = new_buffer(NULL,0);\
	 headers[i].value = new_buffer(NULL,0);\
	 write_buffer(headers[i].key,k,strlen(k)); \
	 write_buffer(headers[i].value,value,strlen(value)); \
	 return headers;\
}

#define MAX_HEADERS (getpagesize() / sizeof(struct headers_struct))

#define MAX_WRITE_SIZE (getpagesize() * 10)

#define RUNTIME_SIZE 8L * 1024L * 1024L
#define CONTEXT_SIZE 8192

#define error(msg ...)	log_msg(0,msg)
#define notice(msg ...)	log_msg(1,msg)
#define debug(msg ...)	log_msg(2,msg)

