# 
# jawasd makefile
#

PROGRAM = jawasd
LIBRARY =
ARCH := $(shell uname)

HOSTADDR=$(shell ifconfig | grep inet | cut -f2 -d" " | head -n1)

LIBS = -lpq -lssl -lcrypto -lz

CFLAGS += --std=c99 -Wall -DHOSTADDR="$(HOSTADDR)" -save-temps

LDFLAGS =

ifeq ($(ARCH),Darwin)
	CFLAGS += -ggdb -DXP_UNIX -fnested-functions -m64
	INCLUDES = -I/opt/local/include/postgresql84/
	LDFLAGS += -L/opt/local/lib/postgresql84/  -L/usr/local/lib
	LIBS += -llua
endif
ifeq ($(ARCH),FreeBSD)
	CFLAGS += -ggdb -DXP_UNIX  -DFREEBSD  -fpic
	INCLUDES = -I/usr/local/include/ -I/usr/local/include/luajit-2.0
	LDFLAGS += -L/usr/local/lib
	LIBS += -lluajit
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


