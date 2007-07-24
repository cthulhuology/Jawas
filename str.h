// str.h
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifndef __HAVE_STR_H__
#define __HAVE_STR_H__

typedef struct str_struct str;
struct str_struct {
	int len;
	char* data;
};

#endif
