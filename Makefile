# 
# jawasd makefile
#

PROGRAM = jawasd
LIBRARY =
ARCH := $(shell uname)

HOSTADDR="0.0.0.0"

LIBS = -lpq -lssl -lcrypto -lz

CFLAGS += --std=c99 -Wall -DHOSTADDR="$(HOSTADDR)"  -O2

LDFLAGS =

ifeq ($(ARCH),Darwin)
	CFLAGS += -ggdb -DXP_UNIX -fnested-functions -m64 -DBITS64
	INCLUDES = -I/opt/local/include/postgresql84/
	LDFLAGS += -L/opt/local/lib/postgresql84/  -L/usr/local/lib
	LIBS += -llua
	SOURCES = bsd.c
endif
ifeq ($(ARCH),FreeBSD)
	CFLAGS += -ggdb -DXP_UNIX  -DFREEBSD  -fpic -DBITS64
	INCLUDES = -I/usr/local/include/ -I/usr/local/include/luajit-2.0/
	LDFLAGS += -L/usr/local/lib
	LIBS += -lluajit
	SOURCES = bsd.c
endif
ifeq ($(ARCH),Linux)
	CFLAGS += -ggdb -DXP_UNIX -DLINUX -DBITS64
	INCLUDES = -I/usr/include/postgresql/ -I/usr/local/include/luajit-2.0/
	LDFLAGS += -L/usr/local/lib
	LIBS += -lluajit
	SOURCES = linux.c
endif


SOURCES += \
auth.c \
client.c \
daemon.c \
database.c \
dates.c \
events.c \
files.c \
forms.c \
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

graph:
	perl dep_graph.pl > graph.dot && dot -Gsize=1000,800  -Tsvg graph.dot > graph.svg
