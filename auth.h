// auth.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//


#ifndef __HAVE_AUTH_H__
#define __HAVE_AUTH_H__

str md5sum(char* data, int len);
str md5hex(char* data, int len);
str base64(str s);
str hmac1(str secret, str data);

#endif
