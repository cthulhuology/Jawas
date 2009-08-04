# jsconfig.mk

ifndef OBJDIR
  ifdef OBJDIR_NAME
    OBJDIR = $(OBJDIR_NAME)
  endif
endif

.FORCE:
