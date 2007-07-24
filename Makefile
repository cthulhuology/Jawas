# 
# jawasd makefile
#

PROGRAM = jawasd
LIBRARY =

CFLAGS = -ggdb -DXP_UNIX
INCLUDES = -Ijs -Ijs/Darwin_DBG.OBJ -I/opt/local/include/postgresql82/

LIBS = -ljs -lpq -lssl -lcrypto
LDFLAGS = -Ljs/Darwin_DBG.OBJ/ -L/opt/local/lib/postgresql82/

SOURCES = \
buffers.c \
events.c \
files.c \
headers.c \
index.c \
js.c \
log.c \
pages.c \
requests.c \
responses.c \
server.c \
sockets.c \
status.c  \
tls.c \
uri.c

include rules.mk


