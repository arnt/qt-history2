#############################################################################
# Makefile: Qt Tutorial
#############################################################################

####### Directories

BASEDIR =	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
LFLAGS	=	-L$(LIBDIR) -lqt -lX11 -lm
CC	=	gcc

####### Directories

SUBDIRS =	t1 t2 t3 t4 t5 t6 t7 t8 t9 t10 t11 t12 t13 t14

####### Targets

all: targets

targets:
	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i 'CC=$(CC)' 'CFLAGS=$(CFLAGS)' 'LFLAGS=$(LFLAGS)'; done

showdirs:
	@echo $(SUBDIRS)

showfiles:
	@echo README Makefile

depend:
	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i depend; done

clean:
	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done

fixmessage: clean
	co -l t{?,??}/*.{h,cpp}
	perl fixmessage t{?,??}/*.{h,cpp}
	ci -u '-mautomatically changed comment header' t{?,??}/*.{h,cpp}
