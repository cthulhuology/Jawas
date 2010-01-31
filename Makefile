# 
# jawasd makefile
#

PROGRAM = jawasd
LIBRARY =
ARCH := $(shell uname)

LIBS = -lpq -lssl -lcrypto -llua

CFLAGS += --std=c99 -Wall -I/usr/local/include/ -m64
LDFLAGS =

ifeq ($(ARCH),Darwin)
	CFLAGS += -ggdb -DXP_UNIX -fnested-functions
	INCLUDES = -I/opt/local/include/postgresql84/
	LDFLAGS += -L/opt/local/lib/postgresql84/  -L/usr/local/lib
endif
ifeq ($(ARCH),FreeBSD)
	CFLAGS += -ggdb -DXP_UNIX  -DFREEBSD
	INCLUDES = -Ijs -Ijs/FreeBSD_DBG.OBJ -I/usr/local/include/
	LDFLAGS += -Ljs/FreeBSD_DBG.OBJ/
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
linux.c \
log.c \
lua_db.c \
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


