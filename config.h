// config.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#ifndef PROD
#define LOG_LEVEL 2
#define DB_CONNECT_STRING "dbname=jawas_dev user=jawas"
#define SERVER_PORT "8080"
#define TLS_SERVER_PORT "4433"
#define TLS_PASSWORD "fullfathomfive"
#define TLS_KEYFILE "/opt/Jawas/Code/Jawas/server.pem"
#define AMAZON_KEY "/opt/Jawas/Code/Jawas/amazon.key"
#define AMAZON_SECRET "/opt/Jawas/Code/Jawas/amazon.secret"
#define FACEBOOK_KEY "/opt/Jawas/Code/Jawas/facebook.key"
#define FACEBOOK_SECRET "/opt/Jawas/Code/Jawas/facebook.secret"
#define MAILHOST "mail.nexttolast.com"
#define PAGE_GUARD 1
#define CACHE_PAGES 100
#else 
#define LOG_LEVEL 1
#define DB_CONNECT_STRING "dbname=jawas user=jawas"
#define SERVER_PORT "80"
#define TLS_SERVER_PORT "443"
#define TLS_PASSWORD "fullfathomfive"
#define TLS_KEYFILE "/opt/Jawas/Code/Jawas/server.pem"
#define AMAZON_KEY "/opt/Jawas/Code/Jawas/amazon.key"
#define AMAZON_SECRET "/opt/Jawas/Code/Jawas/amazon.secret"
#define FACEBOOK_KEY "/opt/Jawas/Code/Jawas/facebook.key"
#define FACEBOOK_SECRET "/opt/Jawas/Code/Jawas/facebook.secret"
#define MAILHOST "mail.nexttolast.com"
#define PAGE_GUARD 0
#define CACHE_PAGES 100
#endif


