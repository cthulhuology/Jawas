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

str
resize_image(str file, str width, str height)
{
	MagickWand* mw;
	str retval = temp_file();
	int max_height,max_width,h,w;
	max_height = str_int(height);
	max_width = str_int(width);
	MagickWandGenesis();
	mw = NewMagickWand();
	char* fname = dump(file);
	if (MagickFalse == MagickReadImage(mw,fname)) {
		error("Failed to read %s\n",file);
		free(fname);
		return NULL;
	}
	free(fname);
	MagickResetIterator(mw);
	while (MagickNextImage(mw) != MagickFalse) {
		h = MagickGetImageHeight(mw);
		w = MagickGetImageWidth(mw);
		if (w < h) {
			w = w * max_height / h;
			h = max_height;
		} else {
			h = h * max_width / w;
			w = max_width;
		}
		MagickScaleImage(mw,w,h);
		if (MagickFalse == MagickWriteImage(mw,retval->data)) {
			error("Failed to write %s\n",retval);
			return NULL;
		}
	}
	DestroyMagickWand(mw);
	MagickWandTerminus();
	return retval;
}

str
rotate_image(str file, str degrees, str color)
{
	MagickWand* mw;
	PixelWand* pw;
	str retval = temp_file();
	double deg = (double)str_int(degrees);
	MagickWandGenesis();
	mw = NewMagickWand();
	pw = NewPixelWand();
	char* clr = dump(color);
	color ? PixelSetColor(pw,clr) : PixelSetColor(pw,"black");
	free(clr);
	char* fname = dump(file);
	if (MagickFalse == MagickReadImage(mw,fname)) {
		free(fname);
		error("Failed to read %s\n",file);
		return NULL;
	}
	free(fname);
	MagickResetIterator(mw);
	while (MagickNextImage(mw) != MagickFalse) {
		MagickRotateImage(mw,pw,deg);
		if (MagickFalse == MagickWriteImage(mw,retval->data)) {
			error("Failed to write %s\n",retval);
			return NULL;
		}
	}
	DestroyPixelWand(pw);
	DestroyMagickWand(mw);
	MagickWandTerminus();
	return retval;
}

str
crop_image(str file, str width, str height)
{
	str retval = temp_file();
	MagickWand* mw;
	int max_height,max_width,h,w,x,y;
	max_height = str_int(height);
	max_width = str_int(width);
	MagickWandGenesis();
	mw = NewMagickWand();
	char* fname = dump(file);
	if (MagickFalse == MagickReadImage(mw,fname)) {
		free(fname);
		error("Failed to read %s\n",file);
		return NULL;
	}
	free(fname);
	MagickResetIterator(mw);
	while (MagickNextImage(mw) != MagickFalse) {
		h = MagickGetImageHeight(mw);
		w = MagickGetImageWidth(mw);
		if (w > h) {
			w = w * max_height / h;
			h = max_height;
		} else {
			h = h * max_width / w;
			w = max_width;
		}
		MagickScaleImage(mw,w,h);
		x = (w - max_width) / 2;
		y = (h - max_height) / 2;
		MagickCropImage(mw,max_width,max_height,x,y);
		fname = dump(retval);
		if (MagickFalse == MagickWriteImage(mw,fname)) {
			free(fname);
			error("Failed to write %s\n",retval);
			return NULL;
		}
		free(fname);
	}
	DestroyMagickWand(mw);
	MagickWandTerminus();
	return retval;
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
#ifndef FREEBSD
		{ "EXIF:Make", (image_method_t)MagickGetImageProperty },
		{ "EXIF:Model", (image_method_t)MagickGetImageProperty },
		{ "EXIF:Orientation", (image_method_t)MagickGetImageProperty },
		{ "EXIF:DateTime", (image_method_t)MagickGetImageProperty },
#endif
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
