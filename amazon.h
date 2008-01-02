// amazon.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __AMAZON_H__
#define __AMAZON_H__

void s3_auth(str key, str secret);
str s3_put(str file, str bucket, str filename, str mime);
str s3_put_jpeg(str file, str bucket, str filename);

#endif
