// facebook.h
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_FACEBOOK__
#define __HAVE_FACEBOOK__
	
void facebook_auth(str key, str secret);
str facebook_login();
void facebook_method(str method, Headers kv, str callback);

#endif
