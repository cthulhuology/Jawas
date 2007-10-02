DEPTH = .

include config.mk

OTHER_LIBS += -Lfdlibm/$(OBJDIR) -lfdm 
INCLUDES += -I/usr/local/apache2/include/ -I/opt/local/include/apr-1
OPTIMIZER =  -ggdb

#
# XCFLAGS may be set in the environment or on the gmake command line
#
CFLAGS          += $(OPTIMIZER) $(OS_CFLAGS) $(DEFINES) $(INCLUDES) $(XCFLAGS)
LDFLAGS		= $(XLDFLAGS) 

ifndef NO_LIBM
LDFLAGS += -lm
endif

# For those platforms with the readline library installed.
DEFINES += -DEDITLINE
PROG_LIBS += -lreadline -ltermcap

# JS file lists
#
JS_HFILES =		\
jsarray.h	\
jsatom.h	\
jsbool.h	\
jsconfig.h	\
jscntxt.h	\
jsdate.h	\
jsemit.h	\
jsexn.h		\
jsfun.h		\
jsgc.h		\
jsinterp.h	\
jslibmath.h	\
jslock.h	\
jsmath.h	\
jsnum.h		\
jsobj.h		\
jsopcode.h      \
jsparse.h	\
jsarena.h	\
jsclist.h	\
jsdhash.h	\
jsdtoa.h	\
jshash.h	\
jslong.h	\
jsosdep.h	\
jstypes.h	\
jsprvtd.h	\
jspubtd.h	\
jsregexp.h	\
jsscan.h	\
jsscope.h	\
jsscript.h	\
jsstr.h		\
jsxdrapi.h	\
jsxml.h		\
$(NULL)

API_HFILES =		\
jsapi.h		\
jsdbgapi.h	\
$(NULL)

OTHER_HFILES =		\
jsbit.h		\
jscompat.h	\
jscpucfg.h	\
jsotypes.h	\
jsstddef.h	\
prmjtime.h	\
resource.h	\
jsopcode.tbl	\
js.msg		\
jsshell.msg	\
$(NULL)

ifndef PREBUILT_CPUCFG
OTHER_HFILES += $(OBJDIR)/jsautocfg.h
endif

HFILES = $(JS_HFILES) $(API_HFILES) $(OTHER_HFILES)

JS_CFILES =		\
jsapi.c		\
jsarena.c	\
jsarray.c	\
jsatom.c	\
jsbool.c	\
jscntxt.c	\
jsdate.c	\
jsdbgapi.c	\
jsdhash.c	\
jsdtoa.c	\
jsemit.c	\
jsexn.c		\
jsfun.c		\
jsgc.c		\
jshash.c	\
jsinterp.c	\
jslock.c	\
jslog2.c	\
jslong.c	\
jsmath.c	\
jsnum.c		\
jsobj.c		\
jsopcode.c      \
jsparse.c	\
jsprf.c		\
jsregexp.c	\
jsscan.c	\
jsscope.c	\
jsscript.c	\
jsstr.c		\
jsutil.c        \
jsxdrapi.c	\
jsxml.c		\
prmjtime.c	\
$(NULL)

PREDIRS	+= fdlibm

FDLIBM_LIBRARY = fdlibm/$(OBJDIR)/libfdm.a
#JSMATH_PRELINK	= $(OBJDIR)/jsmathtemp.o
# Flag for incremental linking
DASH_R		= -r

LIB_CFILES  = $(JS_CFILES)
LIB_ASFILES := $(wildcard *_$(OS_ARCH).s)
PROG_CFILES = js.c

LIBRARY = $(OBJDIR)/libjs.a
SHARED_LIBRARY = $(OBJDIR)/libjs.$(SO_SUFFIX)
PROGRAM = $(OBJDIR)/js
ifdef JS_PERLCONNECT
PROG_LIBS += $(PERLLDFLAGS)
endif

include rules.mk

MOZ_DEPTH = ../..
include jsconfig.mk

$(PROGRAM): $(PROG_OBJS) $(LIBRARY) # $(FDLIBM_LIBRARY)
	$(CC) -o $@ $(CFLAGS) $(PROG_OBJS) $(LIBRARY) $(LDFLAGS) $(OTHER_LIBS) \
	    $(PROG_LIBS)

ifndef PREBUILT_CPUCFG
$(HFILES) $(CFILES): $(OBJDIR)/jsautocfg.h

$(OBJDIR)/jsautocfg.h: $(OBJDIR)/jscpucfg
	rm -f $@
	$(OBJDIR)/jscpucfg > $@

$(OBJDIR)/jscpucfg: $(OBJDIR)/jscpucfg.o
	$(CC) -o $@ $(OBJDIR)/jscpucfg.o

# Look in OBJDIR to find jsautocfg.h
INCLUDES   += -I$(OBJDIR)

# Add to TARGETS for clobber rule
TARGETS += $(OBJDIR)/jsautocfg.h $(OBJDIR)/jscpucfg \
	$(OBJDIR)/jscpucfg.o # $(OBJDIR)/jsmathtemp.o
endif


# Note: generated headers must be built before descending
#       into fdlibm directory
$(FDLIBM_LIBRARY) :
	$(MAKE) -C fdlibm $(@F)

#
# Hardwire dependencies on jsopcode.tbl
#
jsopcode.h jsopcode.c: jsopcode.tbl

-include $(DEPENDENCIES)

TARNAME = jsref.tar
TARFILES = files `cat files`

SUFFIXES: .i
%.i: %.c
	$(CC) -C -E $(CFLAGS) $< > $*.i
