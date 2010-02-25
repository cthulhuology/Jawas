// gzip.c
//

#include <zlib.h>
#include "str.h"
#include "gzip.h"
#include "log.h"

str
shrink(File fc)
{
	size_t len = compressBound(fc->st.st_size);
	str buf = blank(len);
	if (Z_OK != compress((Bytef*)buf->data,&len,(const Bytef*)fc->data,fc->st.st_size)) {
		error("Failed to compress");
		return NULL;
	}
	return buf;
}

