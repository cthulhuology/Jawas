// defines.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#define SERVER_PORT "8080"

#define PAGE_GUARD 1
#define CACHE_PAGES 125000

#define NUM_BUFFER_SIZE 20

#define NODE_FLAGS NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_RENAME | NOTE_REVOKE

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

#define SERVER_VERSION "Jawas/1.0"

#define RUNTIME_SIZE 8L * 1024L * 1024L
#define CONTEXT_SIZE 8192

#define DB_CONNECT_STRING "dbname=jawas user=jawas"

