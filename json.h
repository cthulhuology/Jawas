// json.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_JSON_H__
#define __HAVE_JSON_H__

#include "str.h"
#include "jsapi.h"
#include "headers.h"

str json(JSContext* cx, JSObject* obj);
Headers parse_json(Headers hd, str buf, int pos);

#endif
