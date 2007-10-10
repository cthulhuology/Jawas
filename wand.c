// wand.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#include "include.h"
#include "defines.h"
#include "wand.h"
#include "alloc.h"
#include "str.h"
#include "files.h"
#include "server.h"
#include "log.h"
#include <wand/MagickWand.h>

typedef char*(*image_method_t)(MagickWand*, char*, File);

int
copy_image(const char* src, const char* dst)
{
	return link(src,dst);
}

int
resize_image(const char* src, const char* dst)
{
	MagickWand* mw;
	int h,w,x,y;
	MagickWandGenesis();
	mw = NewMagickWand();
	if (MagickFalse == MagickReadImage(mw,src)) {
		error("Failed to read %c\n",src);
		return 1;
	}
	MagickResetIterator(mw);
	while (MagickNextImage(mw) != MagickFalse) {
		h = MagickGetImageHeight(mw);
		w = MagickGetImageWidth(mw);
		if (w < h) {
			w = w * IMAGE_HEIGHT / h;
			h = IMAGE_HEIGHT;
		} else {
			h = h * IMAGE_WIDTH / w;
			w = IMAGE_WIDTH;
		}
		MagickScaleImage(mw,w,h);
		if (MagickFalse == MagickWriteImage(mw,dst)) {
			error("Failed to write %c\n",dst);
			return 1;
		}
	}
	DestroyMagickWand(mw);
	MagickWandTerminus();
	return 0;
}

int
create_thumb(const char* src, const char* dst)
{
	MagickWand* mw;
	int h,w,x,y;
	
	MagickWandGenesis();
	mw = NewMagickWand();
	if (MagickFalse == MagickReadImage(mw,src)) {
		error("Failed to read %c\n",src);
		return 1;
	}
	MagickResetIterator(mw);
	while (MagickNextImage(mw) != MagickFalse) {
		h = MagickGetImageHeight(mw);
		w = MagickGetImageWidth(mw);
		if (w > h) {
			w = w * THUMB_HEIGHT / h;
			h = THUMB_HEIGHT;
		} else {
			h = h * THUMB_WIDTH / w;
			w = THUMB_WIDTH;
		}
		MagickScaleImage(mw,w,h);
		x = (w - THUMB_WIDTH) / 2;
		y = (h - THUMB_HEIGHT) / 2;
		MagickCropImage(mw,THUMB_WIDTH,THUMB_HEIGHT,x,y);
		if (MagickFalse == MagickWriteImage(mw,dst)) {
			error("Failed to write %c\n",dst);
			return 1;
		}
	}
	DestroyMagickWand(mw);
	MagickWandTerminus();
	return 0;
}

char*
get_image_size(MagickWand* mw, char* field, File fc)
{
	str size = Str("%i",fc->st.st_size);
	return size->data;
}

char*
get_image_width(MagickWand* mw, char* field, File fc)
{
	str width = Str("%i",MagickGetImageWidth(mw));
	return width->data;
}

char*
get_image_height(MagickWand* mw, char* field, File fc)
{
	str height = Str("%i",MagickGetImageHeight(mw));
	return height->data;
}

char**
get_image_properties(const char* filename)
{
	MagickWand* mw;
	int i;
	struct image_fields {
		char* name;
		image_method_t method;
	} fields[] = {
		{ "EXIF:Make", (image_method_t)MagickGetImageProperty },
		{ "EXIF:Model", (image_method_t)MagickGetImageProperty },
		{ "EXIF:Orientation", (image_method_t)MagickGetImageProperty },
		{ "EXIF:DateTime", (image_method_t)MagickGetImageProperty },
		{ "xxxxxHeight", get_image_height },
		{ "xxxxxWidth", get_image_width },
		{ "xxxxxSize", get_image_size },
		{ NULL, (image_method_t)NULL }
	};
	char** array = (char**)salloc(sizeof(char*)*16);
	File fc = load(Str("%c",filename));

	MagickWandGenesis();
	mw = NewMagickWand();
	if (MagickFalse == MagickReadImage(mw,filename)) {
		error("Failed to read %c\n",filename);
		return NULL;
	}
	MagickResetIterator(mw);
	while (MagickNextImage(mw) != MagickFalse) {
		for (i = 0; fields[i].name; ++i) {
			array[i*2] = fields[i].name;
			array[i*2+1] = fields[i].method(mw,fields[i].name,fc);
		}
		array[i*2] = NULL;
		array[i*2+1] = NULL;
	}
	DestroyMagickWand(mw);
	MagickWandTerminus();
	return array;
}
