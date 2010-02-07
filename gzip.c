// gzip.c
//

#include <zlib.h>
#include "gzip.h"

str
shrink(File fc)
{
	size_t len = compressBound(fc->st.st_size);
	char* buf = new_region(len);
	if (Z_OK != compress(buf,&len,fc->data,fc->st.st_size)) {
		error("Failed to compress");
		return NULL;
	}
	return ref(buf,len);
}

