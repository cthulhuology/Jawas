# 
# jawasd makefile
#

PROGRAM = jawasd
LIBRARY =
ARCH := $(shell uname)

LIBS = -ljs -lpq -lssl -lcrypto -lluajit

CFLAGS += --std=c99 -Wall -I/usr/local/include/luajit-2.0  -DHOSTADDR=`ifconfig | grep inet | cut -f2 -d" " | head -n1`
LDFLAGS =

ifeq ($(ARCH),Darwin)
	CFLAGS += -ggdb -DXP_UNIX -fnested-functions
	INCLUDES = -Ijs -Ijs/Darwin_DBG.OBJ 
	LDFLAGS += -Ljs/Darwin_DBG.OBJ/
endif
ifeq ($(ARCH),FreeBSD)
	CFLAGS += -ggdb -DXP_UNIX  -DFREEBSD
	INCLUDES = -Ijs -Ijs/FreeBSD_DBG.OBJ -I/usr/local/include/
	LDFLAGS += -Ljs/FreeBSD_DBG.OBJ/ -L/usr/local/lib
endif
ifeq ($(ARCH),Linux)
	CFLAGS += -ggdb -DXP_UNIX -DLINUX
	INCLUDES = -Ijs -Ijs/Linux_All_DBG.OBJ -I/usr/include/postgresql/
	LDFLAGS += -Ljs/Linux_All_DBG.OBJ/
endif


SOURCES = \
alloc.c \
amazon.c \
auth.c \
bsd.c \
database.c \
dates.c \
events.c \
facebook.c \
files.c \
forms.c \
headers.c \
hostnames.c \
image.c \
index.c \
json.c \
jws.c \
linux.c \
log.c \
lua_json.c \
lws.c \
mail.c \
methods.c \
mime.c \
pages.c \
requests.c \
responses.c \
script.c \
server.c \
signals.c \
sms.c \
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


