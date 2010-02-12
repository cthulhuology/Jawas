# 
# jawasd makefile
#

PROGRAM = jawasd
LIBRARY =
ARCH := $(shell uname)

LIBS = -lpq -lssl -lcrypto -llua -lz

CFLAGS += --std=c99 -Wall -I/usr/local/include/luajit-2.0  -DHOSTADDR=`ifconfig | grep inet | cut -f2 -d" " | head -n1` 
LDFLAGS =

ifeq ($(ARCH),Darwin)
	CFLAGS += -ggdb -DXP_UNIX -fnested-functions
	INCLUDES = -I/opt/local/include/postgresql84/
	LDFLAGS += -L/opt/local/lib/postgresql84/  -L/usr/local/lib
endif
ifeq ($(ARCH),FreeBSD)
	CFLAGS += -ggdb -DXP_UNIX  -DFREEBSD
	INCLUDES = -I/usr/local/include/
endif
ifeq ($(ARCH),Linux)
	CFLAGS += -ggdb -DXP_UNIX -DLINUX
	INCLUDES = -I/usr/include/postgresql/
endif


SOURCES = \
amazon.c \
auth.c \
bsd.c \
database.c \
dates.c \
events.c \
facebook.c \
files.c \
forms.c \
gzip.c \
headers.c \
hostnames.c \
image.c \
index.c \
log.c \
lua_db.c \
lua_json.c \
lws.c \
memory.c \
methods.c \
mime.c \
requests.c \
responses.c \
script.c \
server.c \
signals.c \
sockets.c \
status.c  \
str.c \
strings.c \
timers.c \
tls.c \
transfer.c \
uri.c \
usage.c

include rules.mk
include dist.mk


