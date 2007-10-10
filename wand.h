// wand.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#ifndef __HAVE_WAND_H__
#define __HAVE_WAND_H__

#define IMAGE_WIDTH 472
#define IMAGE_HEIGHT 535

#define THUMB_WIDTH 256
#define THUMB_HEIGHT 256

int create_thumb(const char* src, const char* dst);
char** get_image_properties(const char* filename);

int resize_image(const char* src, const char* dst);

int copy_image(const char* src, const char* dst);

#endif
