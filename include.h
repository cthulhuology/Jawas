// jawas include.h
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

#ifdef LINUX
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#ifdef LINUX
#define __USE_GNU
#endif
#include <fcntl.h>
#undef __USE_GNU
#include <netdb.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
