#############################################################################
# Qt Makefile - examples
#############################################################################

####### Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
LFLAGS	=	-L$(LIBDIR) -lqt -lX11
CC	=	gcc

####### Directories

SUBDIRS =	t1 t2 t3 t4 t5 t6 t7 t8 t9 t10 t11 t12 t13 t14

####### Targets

all: qtdirtest targets

qtdirtest:
	@if [ "$(QTDIR)" = "" ]; then			\
	    make "QTDIR=/usr/local/qt";			\
	    exit;					\
	fi

targets:
	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i 'CFLAGS=$(CFLAGS)' 'LFLAGS=$(LFLAGS)' 'CC=$(CC)'; done

showdirs:
	@echo $(SUBDIRS)

depend:
	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i depend 'CFLAGS=$(CFLAGS)'; done

clean:
	set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done

fixmessage: clean
	co -l t{?,??}/*.{h,cpp}
	perl fixmessage t{?,??}/*.{h,cpp}
	ci -u '-mautomaticall changed commend header' t{?,??}/*.{h,cpp}

