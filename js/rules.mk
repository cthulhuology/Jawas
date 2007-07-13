#
# JSRef GNUmake makefile rules
#

LIB_OBJS  = $(addprefix $(OBJDIR)/, $(LIB_CFILES:.c=.o))
LIB_OBJS  += $(addprefix $(OBJDIR)/, $(LIB_ASFILES:.s=.o))
PROG_OBJS = $(addprefix $(OBJDIR)/, $(PROG_CFILES:.c=.o))

CFILES = $(LIB_CFILES) $(PROG_CFILES)
OBJS   = $(LIB_OBJS) $(PROG_OBJS)

TARGETS += $(LIBRARY) $(SHARED_LIBRARY) $(PROGRAM) $(MODULE)

all:
	+$(LOOP_OVER_PREDIRS) 
ifneq "$(strip $(TARGETS))" ""
	$(MAKE) -f Makefile $(TARGETS)
endif
	+$(LOOP_OVER_DIRS)

$(OBJDIR)/%: %.c
	@$(MAKE_OBJDIR)
	$(CC) -o $@ $(CFLAGS) $*.c $(LDFLAGS)

# This rule must come before the rule with no dep on header
$(OBJDIR)/%.o: %.c %.h
	@$(MAKE_OBJDIR)
	$(CC) -o $@ -c $(CFLAGS) $*.c


$(OBJDIR)/%.o: %.c
	@$(MAKE_OBJDIR)
	$(CC) -o $@ -c $(CFLAGS) $*.c

$(OBJDIR)/%.o: %.s
	@$(MAKE_OBJDIR)
	$(AS) -o $@ $(ASFLAGS) $*.s

# This rule must come before rule with no dep on header
$(OBJDIR)/%.obj: %.c %.h
	@$(MAKE_OBJDIR)
	$(CC) -Fo$(OBJDIR)/ -c $(CFLAGS) $(JSDLL_CFLAGS) $*.c

$(OBJDIR)/%.obj: %.c
	@$(MAKE_OBJDIR)
	$(CC) -Fo$(OBJDIR)/ -c $(CFLAGS) $(JSDLL_CFLAGS) $*.c

$(OBJDIR)/js.obj: js.c
	@$(MAKE_OBJDIR)
	$(CC) -Fo$(OBJDIR)/ -c $(CFLAGS) $<

$(LIBRARY): $(LIB_OBJS)
	$(AR) rv $@ $?
	$(RANLIB) $@

$(SHARED_LIBRARY): $(LIB_OBJS)
	$(MKSHLIB) -o $@ $(LIB_OBJS) $(LDFLAGS) $(OTHER_LIBS)

define MAKE_OBJDIR
if test ! -d $(@D); then rm -rf $(@D); mkdir -p $(@D); fi
endef

ifdef DIRS
LOOP_OVER_DIRS		=					\
	@for d in $(DIRS); do					\
		if test -d $$d; then				\
			set -e;			\
			echo "cd $$d; $(MAKE) -f Makefile $@"; 		\
			cd $$d; $(MAKE) -f Makefile $@; cd ..;	\
			set +e;					\
		else						\
			echo "Skipping non-directory $$d...";	\
		fi;						\
	done
endif

ifdef PREDIRS
LOOP_OVER_PREDIRS	=					\
	@for d in $(PREDIRS); do				\
		if test -d $$d; then				\
			set -e;			\
			echo "cd $$d; $(MAKE) -f Makefile $@"; 		\
			cd $$d; $(MAKE) -f Makefile $@; cd ..;	\
			set +e;					\
		else						\
			echo "Skipping non-directory $$d...";	\
		fi;						\
	done
endif

export:
	+$(LOOP_OVER_PREDIRS)	
	mkdir -p $(DIST)/include $(DIST)/$(LIBDIR) $(DIST)/bin
ifneq "$(strip $(HFILES))" ""
	$(CP) $(HFILES) $(DIST)/include
endif
ifneq "$(strip $(LIBRARY))" ""
	$(CP) $(LIBRARY) $(DIST)/$(LIBDIR)
endif
ifneq "$(strip $(JARS))" ""
	$(CP) $(JARS) $(DIST)/$(LIBDIR)
endif
ifneq "$(strip $(SHARED_LIBRARY))" ""
	$(CP) $(SHARED_LIBRARY) $(DIST)/$(LIBDIR)
endif
ifneq "$(strip $(PROGRAM))" ""
	$(CP) $(PROGRAM) $(DIST)/bin
endif
	+$(LOOP_OVER_DIRS)

clean:
	rm -rf $(OBJS)
	@cd fdlibm; $(MAKE) -f Makefile clean

clobber:
	rm -rf $(OBJS) $(TARGETS) $(DEPENDENCIES)
	@cd fdlibm; $(MAKE) -f Makefile clobber

depend:
	gcc -MM $(CFLAGS) $(LIB_CFILES)

tar:
	tar cvf $(TARNAME) $(TARFILES)
	gzip $(TARNAME)

