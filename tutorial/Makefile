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

SUBDIRS =	t1

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
