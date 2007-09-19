# 
# jawasd makefile
#

PROGRAM = jawasd
LIBRARY =
ARCH := $(shell uname)

LIBS = -ljs -lpq -lssl -lcrypto

ifeq ($(ARCH),Darwin)
	CFLAGS += -ggdb -DXP_UNIX 
	INCLUDES = -Ijs -Ijs/Darwin_DBG.OBJ -I/opt/local/include/postgresql82/
	LDFLAGS = -Ljs/Darwin_DBG.OBJ/ -L/opt/local/lib/postgresql82/
else	
	CFLAGS += -ggdb -DXP_UNIX -DLINUX
	INCLUDES = -Ijs -Ijs/Linux_All_DBG.OBJ -I/usr/include/postgresql/
	LDFLAGS = -Ljs/Linux_All_DBG.OBJ/
endif


SOURCES = \
alloc.c \
buffers.c \
bsd.c \
events.c \
files.c \
headers.c \
image.c \
index.c \
jws.c \
linux.c \
log.c \
methods.c \
mime.c \
pages.c \
requests.c \
responses.c \
server.c \
signals.c \
sockets.c \
status.c  \
str.c \
tls.c \
uri.c

include rules.mk
include dist.mk


