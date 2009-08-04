// wand.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#ifndef __HAVE_WAND_H__
#define __HAVE_WAND_H__

#include "str.h"

char** get_image_properties(const char* filename);

str resize_image(str file, str width, str height);
str rotate_image(str file, str degrees, str color);
str crop_image(str file, str width, str height);

#endif
