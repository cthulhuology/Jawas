// config.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#define DB_CONN(X,Y,Z) "dbname=" #X " host=" #Y " port=" #Z " user=jawas"
#define DB_STR(X,Y,Z) DB_CONN(X,Y,Z)

#ifndef PROD
#define halt for(;;) {};
#define LOG_LEVEL 4
#define DB_CONNECT_STRING DB_STR(jawas_dev,HOSTADDR,6060)
#define SERVER_PORT "8080"
#define TLS_SERVER_PORT "4433"
#define TLS_PASSWORD "whatawonderfulworld"
#define TLS_KEYFILE "/Users/dave/Code/Jawas/server.pem"
#define MAILHOST "mail.nexttolast.com"
#define PAGE_GUARD 1
#define CACHE_PAGES 10000
#define MAX_REGIONS 4096
#else 
#define halt exit(1)
#define LOG_LEVEL  4
#define DB_CONNECT_STRING  DB_STR(jawas,HOSTADDR,6060)
#define SERVER_PORT "80"
#define TLS_SERVER_PORT "443"
#define TLS_PASSWORD "thisisthewaytheworldends"
#define TLS_KEYFILE "/usr/local/Jawas/server.pem"
#define MAILHOST "mail.nexttolast.com"
#define PAGE_GUARD 0
#define CACHE_PAGES 10000
#define MAX_REGIONS 4096
#endif


