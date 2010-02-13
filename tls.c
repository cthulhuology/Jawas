// tls.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "memory.h"
#include "str.h"
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
	TLSInfo tls = (TLSInfo)reserve(sizeof(struct tls_struct) + strlen(password) + 1);
	if (!tls) return tls;
	tls->pass_len = strlen(password) + 1;
	memcpy(tls->password,password,tls->pass_len);
	SSL_library_init();
	SSL_load_error_strings();
	tls->err = BIO_new_fp(stderr,BIO_NOCLOSE);	
	tls->method = SSLv23_method();
	tls->ctx = SSL_CTX_new(tls->method);
	if (!SSL_CTX_load_verify_locations(tls->ctx,"certs.pem",NULL)) {
		error("Failed to use certificate chain file certs.pem");
		return NULL;
	}
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

TLSInfo
client_tls(char* certs)
{
	TLSInfo tls = (TLSInfo)reserve(sizeof(struct tls_struct));
	if (!tls) return NULL;
	tls->pass_len = 0;
	tls->err = BIO_new_fp(stderr,BIO_NOCLOSE);
	tls->method = SSLv23_method();
	tls->ctx = SSL_CTX_new(tls->method);
	if (!SSL_CTX_load_verify_locations(tls->ctx,NULL,certs)) {
		error("Failed to load certs directory");
		return NULL;
	}
	return tls;
}

TLSSocket
open_tls(TLSInfo tls, int fd)
{
	TLSSocket retval = (TLSSocket)reserve(sizeof(struct tls_socket_struct));
	if (! tls || !retval) return NULL;
	retval->ssl = SSL_new(tls->ctx);
	retval->bio = BIO_new_socket(fd,BIO_NOCLOSE);
	SSL_set_bio(retval->ssl,retval->bio,retval->bio);
	return retval;
}

int
connect_tls(TLSSocket sc)
{
	int retval = SSL_connect(sc->ssl);
	switch(SSL_get_error(sc->ssl,retval)) {
		case SSL_ERROR_ZERO_RETURN: error("[SSL] connection closed"); break;
		case SSL_ERROR_WANT_CONNECT: error("[SSL] incomplete connection"); break;
		case SSL_ERROR_WANT_X509_LOOKUP: error("[SSL] no client cert callback"); break;
		case SSL_ERROR_SYSCALL: error("[SSL] system error"); break;
		case SSL_ERROR_SSL: error("[SSL] protocol error"); break;
		default: break;
	}
	return retval < 0;
}

int
check_tls(TLSSocket sc)
{
	if (SSL_get_verify_result(sc->ssl) != X509_V_OK) {
		error("SSL certificate not valid");
		return 1;
	}
	str host = blank(256);
	X509* cert = SSL_get_peer_certificate(sc->ssl);
	X509_NAME_get_text_by_NID(X509_get_subject_name(cert),NID_commonName,host->data, 256);
	debug("Common Name: %s",host);
	return 0;
}

int
accept_tls(TLSSocket sc)
{
	int retval;
	retval = SSL_accept(sc->ssl);
	if (retval) switch(SSL_get_error(sc->ssl,retval)) {
		case SSL_ERROR_WANT_READ: return 1;
		case SSL_ERROR_WANT_WRITE: return 1;
		case SSL_ERROR_ZERO_RETURN: error("[SSL] connection closed"); break;
		case SSL_ERROR_WANT_CONNECT: error("[SSL] incomplete connection"); break;
		case SSL_ERROR_WANT_X509_LOOKUP: error("[SSL] no client cert callback"); break;
		case SSL_ERROR_SYSCALL: error("[SSL] system error"); break;
		case SSL_ERROR_SSL: error("[SSL] protocol error"); break;
		default: break;
	}
	return retval ? -1 : 0;
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
