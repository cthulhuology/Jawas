// Events
//
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#include "include.h"
#include "alloc.h"
#include "log.h"
#include "defines.h"
#include "server.h"
#include "events.h"

Scratch escratch = NULL;

Event
queue_event(Event ec, int fd, enum event_types type, enum event_flags flag, void* udata)
{
//	debug("Queued fd %i ev %i ptr %p",fd, type, udata);
	if (escratch == NULL) escratch = new_scratch(NULL);
	Event retval = (Event)alloc_scratch(escratch,sizeof(struct event_cache_struct));
	retval->fd = fd;
	retval->type = type;	
	retval->flag = flag;
	retval->data = udata;
	retval->next = ec;
	retval->pos = (ec ? ec->pos + 1 : 1);
	return retval;
}

void
free_events()
{
	if (escratch) free_scratch(escratch);
	escratch = NULL;
}

