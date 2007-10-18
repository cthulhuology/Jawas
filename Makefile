# 
# jawasd makefile
#

PROGRAM = jawasd
LIBRARY =
ARCH := $(shell uname)

LIBS = -ljs -lpq -lssl -lcrypto

CFLAGS += -O2 -pg `Wand-config --cppflags`
#CFLAGS += -foptimize-sibling-calls -fomit-frame-pointer

LDFLAGS  += `Wand-config --ldflags --libs`

ifeq ($(ARCH),Darwin)
	CFLAGS += -ggdb -DXP_UNIX 
	INCLUDES = -Ijs -Ijs/Darwin_DBG.OBJ -I/opt/local/include/postgresql82/
	LDFLAGS += -Ljs/Darwin_DBG.OBJ/ -L/opt/local/lib/postgresql82/
else	
	CFLAGS += -ggdb -DXP_UNIX -DLINUX
	INCLUDES = -Ijs -Ijs/Linux_All_DBG.OBJ -I/usr/include/postgresql/
	LDFLAGS += -Ljs/Linux_All_DBG.OBJ/
endif


SOURCES = \
alloc.c \
amazon.c \
auth.c \
buffers.c \
bsd.c \
dates.c \
events.c \
facebook.c \
files.c \
forms.c \
headers.c \
image.c \
index.c \
jws.c \
linux.c \
log.c \
mail.c \
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
timers.c \
tls.c \
uri.c \
usage.c \
wand.c

include rules.mk
include dist.mk


