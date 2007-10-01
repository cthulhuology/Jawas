// amazon.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __AMAZON_H__
#define __AMAZON_H__

str md5sum(char* data, int len);
str base64(str s);
str hmac1(str secret, str data);

str s3_put_auth_string(str verb, str mime, str date, str bucket, str filename);

str s3_put_jpeg(str bucket, str filename);
str s3_put_thumb(str bucket, str filename);

#endif
