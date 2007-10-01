// config.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#ifndef PROD
#define SERVER_PORT "8080"
#define TLS_SERVER_PORT "4433"
#define TLS_KEYFILE "/opt/Jawas/Code/Jawas/server.pem"
#else 
#define SERVER_PORT "80"
#define TLS_SERVER_PORT "443"
#define TLS_KEYFILE "/opt/Jawas/Code/Jawas/server.pem"
#endif

#define DB_CONNECT_STRING "dbname=jawas user=jawas"
#define LOG_LEVEL 2
#define TLS_PASSWORD "fullfathomfive"
#define PAGE_GUARD 1
#define CACHE_PAGES 100

#define AMAZON_KEY "/opt/Jawas/Code/Jawas/amazon.key"
#define AMAZON_SECRET "/opt/Jawas/Code/Jawas/amazon.secret"
