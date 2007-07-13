# Makefile
#
#

CFLAGS = -ggdb -DXP_UNIX
INCLUDES = -Ijs -Ijs/Darwin_DBG.OBJ -I/opt/local/include/postgresql82/
LIBS = -ljs -lpq
LDFLAGS = -Ljs/Darwin_DBG.OBJ/ -L/opt/local/lib/postgresql82/

SOURCES = \
buffers.c \
events.c \
files.c \
headers.c \
js.c \
pages.c \
requests.c \
responses.c \
server.c \
sockets.c \
status.c  \
uri.c

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
PROGRAM = jawasd

all : jawasd

%.o : %.c 
	gcc -c $< $(CFLAGS) $(INCLUDES)

$(PROGRAM) : $(OBJECTS)
	gcc -o $@ $(OBJECTS) $(LDFLAGS) $(LIBS)

test : test.c buffers.o files.o
	gcc -o test test.c buffers.o files.o

.PHONY: clean
clean : 
	rm -rf $(PROGRAM) $(OBJECTS)
