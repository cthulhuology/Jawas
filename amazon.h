// amazon.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __AMAZON_H__
#define __AMAZON_H__

void s3_auth(str key, str secret);

str s3_put_jpeg(str bucket, str filename);
str s3_put_thumb(str bucket, str filename);
str s3_put_orig(str bucket, str filename);
str s3_put_resized(str bucket, str filename);

#endif
