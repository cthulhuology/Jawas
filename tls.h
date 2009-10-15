// tls.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __TLS_H__
#define __TLS_H__

#include <openssl/ssl.h>

typedef struct tls_struct* TLSInfo;
struct tls_struct {
	SSL_METHOD* method;
	SSL_CTX* ctx;
	RSA* rsa;
	BIO* err;	
	size_t pass_len;
	char password[0];
};

typedef  struct tls_socket_struct* TLSSocket;
struct tls_socket_struct {
	SSL* ssl;
	BIO* bio;
};

TLSInfo init_tls(char* keyfile, char* password);
TLSInfo client_tls(char* certs);
TLSSocket open_tls(TLSInfo tls, int fd);
int read_tls(TLSSocket ts, char* data, int len);
int write_tls(TLSSocket ts, char* data, int len);
void close_tls(TLSSocket ts);
int accept_tls(TLSSocket sc);
int check_tls(TLSSocket sc);
int connect_tls(TLSSocket sc);


#endif
