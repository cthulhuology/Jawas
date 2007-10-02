// forms.c 
// 
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "defines.h"
#include "headers.h"
#include "buffers.h"
#include "str.h"
#include "server.h"
#include "forms.h"

str 
find_boundary(str enc)
{
	int i;
	cstr tmp;
	debug("Enc data: %p len: %i",enc->data, enc->len);
	for (i = 19; i < enc->len; ++i) {
		tmp = Cstr(enc->data + i, enc->len - i);
		debug("TMP %x (%p, %i)",tmp, tmp->data, tmp->len);
		if (ncmp_str(Str("boundary="),tmp,9)) {
			tmp = Cstr(enc->data + i + 9, enc->len - (i+9));
			debug("tmp is %x (%p,%i)", tmp, tmp->data,tmp->len);
			return Str("--%x",tmp);
		}
	}
	return NULL;
}

str
parse_name(Buffer buf, int pos)
{
	str retval = NULL;
	int len = length_buffer(buf);
	int off;
	off = 6 + search_buffer(buf,pos,Str("name=\""),0);
	debug("name offset %i, %i",off,len);
	if (off >= len) return NULL;
	debug("Working offset %i",off);
	int end = find_buffer(buf,off+1,"\"");
	debug("End working offset %i",end);
	if (end - off > 4000) {
		error("Filename exceeds legit size");
		return NULL;	
	}
	debug("off %i len %i",off,end-off);
	retval =  read_str(buf,off,end-off);
	debug("Found file name: %s",retval);
	return retval;
}

int
is_file(Buffer buf, int pos, int end)
{
	int len = length_buffer(buf);
	int off = 6 + search_buffer(buf,pos,Str("filename=\""),0);
	return off < len && off < end;
}

int
skip_content_headers(Buffer buf, int pos)
{
	return 4 + search_buffer(buf,pos,Str("\r\n\r\n"),0);
}

str
save_contents(str fname, Buffer buf, int pos, int end)
{
	int delta;
	Buffer tmp = NULL;
	str filename = Str("/tmp/%i%i-%s",random(),time(NULL),fname);
	debug("Save contents filename %s",filename);
	int fd = open(filename->data,O_WRONLY|O_CREAT,0644);
	if (fd < 0 ) {
		error("Failed to open file for writing %s",filename);
		perror("open");
		return filename;	
	}
	debug("Writing [%i,%i)",pos,end);
	for (tmp = seek_buffer(buf,pos); tmp && tmp->pos < end; tmp = seek_buffer(buf,pos)) {
		delta = pos - tmp->pos;
		if (end <= tmp->length + tmp->pos)  {
			debug("A. Writing chunk [%i,%i]",delta + tmp->pos, end);
			write(fd,&tmp->data[delta],end - tmp->pos - delta);
			break; // done
		} else {
			debug("B. Writing chunk [%i,%i]",delta + tmp->pos, tmp->pos + tmp->length);
			write(fd,&tmp->data[delta],tmp->length - delta);
			pos = tmp->pos + tmp->length;
		}
	}
	close(fd);
	return filename;		
}

str
get_contents(Buffer buf, int pos, int end)
{
	return read_str(buf,pos,end-pos);
}

Headers
parse_multipart_body(Headers headers, str enctype)
{
	str dstname;
	int i, e, n, len = length_buffer(Req->contents);
	debug("Body is ");
	dump_buffer(Req->contents,Req->body);
	str boundary = find_boundary(enctype);
	debug("Boundary: %s", boundary);
	if (!boundary) {
		error("multipart/form-data without boundary");
		return headers;
	}
	for (i = search_buffer(Req->contents,Req->body,boundary,0); 
		i < len;
		i = n + boundary->len) {
		debug(">> %i",i);
		n = search_buffer(Req->contents,i+1,boundary,0);
		debug("<< %i",n);
		if (n-i < 4000) debug("Contents [%s]",read_str(Req->contents,i,n-i));
		else debug("Contents exceed string size");
		str srcname = parse_name(Req->contents,i);
		if (! srcname) {
			i = skip_content_headers(Req->contents,i);
			debug(">> %i",i);
			continue;
		}
		debug("srcname = %s",srcname);
		e = skip_content_headers(Req->contents,i);
		debug(">>== %i",e);
		if (is_file(Req->contents,i,e))
			dstname = save_contents(srcname,Req->contents,e,n-2);
		else
			dstname = get_contents(Req->contents,e,n-2);
		debug("dstname = %s",dstname);
		headers = append_header(headers,srcname,dstname);
	}
	return headers;
}
