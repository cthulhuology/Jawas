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

.PHONY: jsclean
jsclean: 
	cd js && $(MAKE) clean

install :
	for I in dev prod; do \
		sudo mkdir -p /usr/local/Jawas/$$I/Sites; \
		sudo rm -f /usr/local/Jawas/$$I/Sites/jawasd ;\
		sudo rm -f /usr/local/Jawas/$$I/Sites/libjs.so; \
		cd /usr/local/Jawas/$$I/Sites && sudo ln -s ../Jawas/jawasd; \
		cd /usr/local/Jawas/$$I/Sites && sudo ln -s ../Jawas/js/$(ARCH)_DBG.OBJ/libjs.so \
	; done
	sudo mkdir -p /usr/local/etc/rc.d
	sudo cp jawas.conf /usr/local/etc
	sudo cp jawas.sh /usr/local/etc/rc.d/jawas
	sudo mkdir -p /usr/local/sbin
	sudo cp jawas /usr/local/sbin
