# 
# jawasd makefile
#

PROGRAM = jawasd
LIBRARY =

CFLAGS += -ggdb -DXP_UNIX 
#CFLAGS = -ggdb -DXP_UNIX -DLINUX
INCLUDES = -Ijs -Ijs/Darwin_DBG.OBJ -I/opt/local/include/postgresql82/
#INCLUDES = -Ijs -Ijs/Linux_All_DBG.OBJ -I/usr/include/postgresql/

LIBS = -ljs -lpq -lssl -lcrypto
LDFLAGS = -Ljs/Darwin_DBG.OBJ/ -L/opt/local/lib/postgresql82/
#LDFLAGS = -Ljs/Linux_All_DBG.OBJ/

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
log.c \
methods.c \
mime.c \
pages.c \
requests.c \
responses.c \
server.c \
sockets.c \
status.c  \
str.c \
tls.c \
uri.c

include rules.mk


