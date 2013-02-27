// jawas include.h
// Copyright (C) 2007 David J. Goehrig
// All Rights Reserved
//

typedef unsigned short u_short;

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
#include <netinet/tcp.h>
#include <ctype.h>
#include <fts.h>
#include <dirent.h>
#include <errno.h>
#ifdef LINUX
#include <sys/epoll.h>
#include <sys/inotify.h>
#else
#include <sys/event.h>
#endif
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
#include <time.h>
#include <unistd.h>
