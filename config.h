// config.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#ifndef PROD
#define halt for(;;) {};
#define LOG_LEVEL 3
#define DB_CONNECT_STRING "dbname=jawas_dev user=jawas"
#define SERVER_PORT "8080"
#define TLS_SERVER_PORT "4433"
#define TLS_PASSWORD "whatawonderfulworld"
#define TLS_KEYFILE "/Users/dave/Code/Jawas/server.pem"
#define MAILHOST "mail.nexttolast.com"
#define PAGE_GUARD 1
#define CACHE_PAGES 10000
#else 
#define halt
#define LOG_LEVEL 2
#define DB_CONNECT_STRING "dbname=jawas user=jawas"
#define SERVER_PORT "80"
#define TLS_SERVER_PORT "443"
#define TLS_PASSWORD "whatawonderfulworld"
#define TLS_KEYFILE "/usr/local/Jawas/server.pem"
#define MAILHOST "mail.nexttolast.com"
#define PAGE_GUARD 0
#define CACHE_PAGES 10000
#endif


