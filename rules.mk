#
# rules.mk
#

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

$(PROGRAM) : $(OBJECTS)
	gcc -o $@ $(PROGRAM).c $(OBJECTS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean : 
	rm -rf $(PROGRAM) $(OBJECTS) $(LIBARAY)

%.o : %.c 
	gcc -c $< $(CFLAGS) $(INCLUDES)

all : $(PROGRAM) $(LIBRARY)

