// wand.c
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved

#include "include.h"
#include "wand.h"
#include "alloc.h"
#include "str.h"
#include "files.h"
#include "server.h"
#include <wand/MagickWand.h>

int
create_thumb(const char* src, const char* dst)
{
	MagickWand* mw;
	int h,w,x,y;
	
#ifndef LINUX
	MagickWandGenesis();
#endif
	mw = NewMagickWand();
	if (MagickFalse == MagickReadImage(mw,src)) {
		fprintf(stderr,"Failed to read %s\n",src);
		return 1;
	}
	MagickResetIterator(mw);
	while (MagickNextImage(mw) != MagickFalse) {
		h = MagickGetImageHeight(mw);
		w = MagickGetImageWidth(mw);
		if (w > h) {
			w = w * IMAGE_HEIGHT / h;
			h = IMAGE_HEIGHT;
		} else {
			h = h * IMAGE_WIDTH / w;
			w = IMAGE_WIDTH;
		}
		fprintf(stderr,"Scaling to %i x %i\n",w,h);
		MagickScaleImage(mw,w,h);
		x = (w - IMAGE_WIDTH) / 2;
		y = (h - IMAGE_HEIGHT) / 2;
		fprintf(stderr,"Cropping to %i x %i at %i,%i\n",IMAGE_WIDTH,IMAGE_HEIGHT,x,y);
		MagickCropImage(mw,IMAGE_WIDTH,IMAGE_HEIGHT,x,y);
		if (MagickFalse == MagickWriteImage(mw,dst)) {
			fprintf(stderr, "Failed to write %s\n",dst);
			return 1;
		}
	}
	DestroyMagickWand(mw);
#ifndef LINUX
	MagickWandTerminus();
#endif
	return 0;
}

char**
get_image_properties(const char* filename)
{
	MagickWand* mw;
	
	char** array = (char**)salloc(sizeof(char*)*16);
	char* fields[] = {
		"EXIF:Make",
		"EXIF:Model",
		"EXIF:Orientation",
		"EXIF:DateTime",
		NULL
	};
	int i;
	str width;
	str height;
	str size;
	File fc = load(Str("%c",filename));

#ifndef LINUX
	MagickWandGenesis();
#endif
	mw = NewMagickWand();
	if (MagickFalse == MagickReadImage(mw,filename)) {
		fprintf(stderr,"Failed to read %s\n",filename);
		return NULL;
	}
	MagickResetIterator(mw);
	while (MagickNextImage(mw) != MagickFalse) {
		for (i = 0; fields[i]; ++i) {
			array[i*2] = fields[i];
			array[i*2+1] = MagickGetImageProperty(mw,fields[i]);
		}
		height = Str("%i",MagickGetImageHeight(mw));
		array[i*2] = "xxxxxHeight";
		array[i*2+1] =  height->data;
		++i;
		width = Str("%i",MagickGetImageWidth(mw));
		array[i*2] = "xxxxxWidth";
		array[i*2+1] = width->data;
		++i;
		size = Str("%i",fc->st.st_size);
		array[i*2] = "xxxxxSize";
		array[i*2+1] = size->data;
		++i;
		array[i*2] = NULL;
		array[i*2+1] = NULL;
	}
	DestroyMagickWand(mw);
#ifndef LINUX
	MagickWandTerminus();
#endif
	return array;
}
