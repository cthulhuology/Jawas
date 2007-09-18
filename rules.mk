#
# rules.mk
#

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

$(PROGRAM) : $(OBJECTS) $(PROGRAM).c
	gcc -o $@ $(PROGRAM).c $(INCLUDES) $(CFLAGS) $(OBJECTS) $(OTHER_OBJS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean : 
	rm -rf $(PROGRAM) $(OBJECTS) $(LIBARAY)

%.o : %.c 
	gcc -c $< $(CFLAGS) $(INCLUDES)

all : $(PROGRAM) $(LIBRARY)

.PHONY: prod
prod :
	$(MAKE) clean
	CFLAGS=-DPROD $(MAKE)	
