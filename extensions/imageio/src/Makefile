####### This section was automatically generated from
#######    /tmp/qtdist-warwick2/qt-1.3beta1/Makefile for building on
#######    linux-gcc-shared on Tue Jul  8 16:26:18 MET DST 1997

INCDIR = $(QTDIR)/include
CFLAGS = -O2 -fno-strength-reduce -Wall -W -I/usr/X11R6/include
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CC = gcc
MOC = $(QTDIR)/bin/moc
SHELL = /bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/extensions/imageio/src/Makefile,v $
#

LINK	=	gcc
LSHR	=	-shared -Wl,-soname,libqimgio.so
LIBS	=	-lqt -L/usr/X11R6/lib -lX11 -ljpeg -lpng -lz -lm
AR	=	ar cq
RANLIB	=	ranlib

####### Files

HEADERS =	qimageio.h \
		qjpegio.h \
		qpngio.h
SOURCES =	qimageio.cpp \
		qjpegio.cpp \
		qpngio.cpp
OBJECTS =	qimageio.o \
		qjpegio.o \
		qpngio.o
SRCMOC	=	
OBJMOC	=	
TARGET	=	qimgio
TARGETA	=	libqimgio.a
TARGETD	=	libqimgio.so.0.1
TARGET0	=	libqimgio.so
TARGET1	=	libqimgio.so.0

####### Implicit rules

.SUFFIXES: .cpp .c .C

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) -o $@ $<

.C.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) -o $@ $<

####### Build rules

all: sharedlib 

sharedlib: $(TARGETD)

install: sharedlib
	cp -f -d $(TARGET0) $(TARGET1) $(TARGETD) $(QTDIR)/lib
	SDIR=$$PWD; cd $(QTDIR)/include; \
	for i in $(HEADERS); \
	do \
	    ln -f -s $$SDIR/$$i .; \
	done

$(TARGETD): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGETD) $(TARGET0) $(TARGET1)
	$(LINK) $(LSHR) $(LFLAGS) -o $(TARGETD) $(OBJECTS) $(OBJMOC) $(LIBS)
	-ln -s $(TARGETD) $(TARGET0)
	-ln -s $(TARGETD) $(TARGET1)

staticlib: $(TARGETA)

$(TARGETA): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGETA)
	$(AR) $(TARGETA) $(OBJECTS) $(OBJMOC)
	$(RANLIB) $(TARGETA)

moc: $(SRCMOC)

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC)
	-rm -f $(TARGETA) $(TARGETD) $(TARGET0) $(TARGET1)
	-rm -f *~ core

####### Compile

qimageio.o: qimageio.cpp \
		qjpegio.h \
		qpngio.h

qjpegio.o: qjpegio.cpp

qpngio.o: qpngio.cpp
