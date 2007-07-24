#
# rules.mk
#

OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

$(PROGRAM) : $(OBJECTS) $(PROGRAM).c
	gcc -o $@ $(PROGRAM).c $(OBJECTS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean : 
	rm -rf $(PROGRAM) $(OBJECTS) $(LIBARAY)
	rm -rf localhost/logs
	mkdir -p localhost/logs

%.o : %.c 
	gcc -c $< $(CFLAGS) $(INCLUDES)

all : $(PROGRAM) $(LIBRARY)

