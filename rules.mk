#
# rules.mk
#

ARCH = `uname`

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
#	$(MAKE) javascript
	CFLAGS=-DPROD $(MAKE)	

.PHONY: javascript
javascript:
	cd js && $(MAKE) clean && $(MAKE)

commit :
	git commit -a
