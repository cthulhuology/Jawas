// config.h
//
// Copyright (C) 2013 David J. Goehrig
// All Rights Reserved

#define DB_CONN(X) "dbname=" #X " user=jawas"
#define DB_STR(X) DB_CONN(X)

#define halt for(;;) {};
#define LOG_LEVEL 4
#define DB_CONNECT_STRING DB_STR(jawas)
#define SERVER_PORT "8080"
#define TLS_SERVER_PORT "4433"
#define TLS_PASSWORD "jawas"
#define TLS_KEYFILE "/etc/jawas/server.pem"
#define MAILHOST "localhost"
#define PAGE_GUARD 1
#define CACHE_PAGES 10000
#define MAX_REGIONS 2
#define MAX_EVENTS 1024
#define SYSTEM_REGION_SIZE 10000000
#define MAX_FILES 1024
#define IDLE_TIMEOUT 5
#define use_tls 0
