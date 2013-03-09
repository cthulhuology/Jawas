// config.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#define DB_CONN(X) "dbname=" #X " user=jawas"
#define DB_STR(X) DB_CONN(X)

#ifndef PROD
#define halt for(;;) {};
#define LOG_LEVEL 4
#define DB_CONNECT_STRING DB_STR(jawas)
#define SERVER_PORT "8080"
#define TLS_SERVER_PORT "4433"
#define TLS_PASSWORD "jawas"
#define TLS_KEYFILE "/home/dave/Jawas2/server.pem"
#define MAILHOST "localhost"
#define PAGE_GUARD 1
#define CACHE_PAGES 10000
#define MAX_REGIONS 2
#define MAX_EVENTS 1024
#define SYSTEM_REGION_SIZE 10000000
#define MAX_FILES 1024
#define IDLE_TIMEOUT 5
#else 
#define halt exit(1)
#define LOG_LEVEL  4
#define DB_CONNECT_STRING  DB_STR(jawas,HOSTADDR,6060)
#define SERVER_PORT "8080"
#define TLS_SERVER_PORT "5443"
#define TLS_PASSWORD "jawas"
#define TLS_KEYFILE "/home/dave/Jawas2/server.pem"
#define MAILHOST "mail.nexttolast.com"
#define PAGE_GUARD 0
#define CACHE_PAGES 10000
#define MAX_REGIONS 2 
#define CACHE_TTL 1000000
#define MAX_EVENTS 1024
#define SYSTEM_REGION_SIZE 10000000
#define MAX_FILES 1024
#define IDLE_TIMEOUT 5
#endif


