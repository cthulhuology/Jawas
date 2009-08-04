// uri.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __URI_H__
#define __URI_H__

#include "str.h"
#include "requests.h"

str uri_encode(str s);
str uri_decode(str s);

Headers parse_uri_encoded(Headers head, str buf, int pos);

#endif
