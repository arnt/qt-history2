####### This section was automatically generated from
#######    /home/hanord/qt/Makefile for building on
#######    linux-gcc-shared on Fri Feb 28 17:08:50 MET 1997

INCDIR = $(QTDIR)/include
CFLAGS = -O2 -fno-strength-reduce -Wall -W
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CC = gcc
MOC = moc
SHELL =	/bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/extensions/nsplugin/src/Makefile,v $
#

####### Compiler, tools and options

INCPATH	=	-I$(INCDIR) -I/usr/X11R6/include
AR	=	ar cq
RANLIB	=	ranlib

####### Files

HEADERS =	qnp.h
SOURCES =	qnpbind.cpp \
		npunix.c
OBJECTS =	qnpbind.o \
		npunix.o
SRCMOC	=	moc_qnp.cpp
OBJMOC	=	moc_qnp.o
TARGET	=	qnp
TARGETA	=	libqnp.a

####### Implicit rules

.SUFFIXES: .cpp .C

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all: staticlib 

install: staticlib
	cp -f -d $(TARGETA) $(QTDIR)/lib
	SDIR=$$PWD; cd $(QTDIR)/include; \
	for i in $(HEADERS); \
	do \
	    ln -f -s $$SDIR/$$i .; \
	done

staticlib: $(TARGETA)

$(TARGETA): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGETA)
	$(AR) $(TARGETA) $(OBJECTS) $(OBJMOC)
	$(RANLIB) $(TARGETA)

moc: $(SRCMOC)

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC)
	-rm -f $(TARGETA) 
	-rm -f *~ core

####### Compile

qnpbind.o: qnpbind.cpp \
		qnp.h \
		npapi.h \
		jri.h \
		jritypes.h \
		jri_md.h

npunix.o: npunix.c \
		npapi.h \
		jri.h \
		jritypes.h \
		jri_md.h \
		npupp.h

moc_qnp.o: moc_qnp.cpp \
		qnp.h

moc_qnp.cpp: qnp.h
	$(MOC) qnp.h -o moc_qnp.cpp
