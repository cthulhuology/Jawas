// tls.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "alloc.h"
#include "log.h"
#include "tls.h"

static int 
password_callback(char *str, int num, int flags, void* udata)
{
	TLSInfo tls = (TLSInfo)udata;
	if (num<tls->pass_len) return 0;
	strcpy(str,tls->password);
	return tls->pass_len - 1;
}

TLSInfo
init_tls(char* keyfile, char* password)
{
	TLSInfo tls = (TLSInfo)salloc(sizeof(struct tls_struct) + strlen(password) + 1);
	if (!tls) return tls;
	tls->pass_len = strlen(password) + 1;
	memcpy(tls->password,password,tls->pass_len);
	SSL_library_init();
	SSL_load_error_strings();
	tls->err = BIO_new_fp(stderr,BIO_NOCLOSE);	
	tls->method = SSLv23_method();
	tls->ctx = SSL_CTX_new(tls->method);
	if (!SSL_CTX_use_certificate_file(tls->ctx,keyfile,SSL_FILETYPE_PEM)) {
		error("Failed to use certificate chain file %c\n",keyfile);
		return NULL;
	}
	SSL_CTX_set_default_passwd_cb(tls->ctx,password_callback);
	SSL_CTX_set_default_passwd_cb_userdata(tls->ctx,tls);
	if(!SSL_CTX_use_PrivateKey_file(tls->ctx,keyfile,SSL_FILETYPE_PEM)) {
		error("Failed to use private key file %c\n",keyfile);
		return NULL;
	}
	return tls;
}

TLSSocket
open_tls(TLSInfo tls, int fd)
{
	TLSSocket retval = (TLSSocket)salloc(sizeof(struct tls_socket_struct));;
	if (! tls) return NULL;
	if (! retval) return retval;
	retval->ssl = SSL_new(tls->ctx);
	retval->bio = BIO_new_socket(fd,BIO_NOCLOSE);
	SSL_set_bio(retval->ssl,retval->bio,retval->bio);
	SSL_accept(retval->ssl);
	return retval;
}

int
read_tls(TLSSocket ts, char* data, int len)
{
	return SSL_read(ts->ssl,data,len);	
}

int
write_tls(TLSSocket ts, char* data, int len)
{
	return SSL_write(ts->ssl,data,len);
}

void
close_tls(TLSSocket ts)
{
	SSL_shutdown(ts->ssl);
}
