####### This section was automatically generated from
#######    /home/hanord/qt/Makefile for building on
#######    linux-gcc-shared on Fri Feb 28 17:08:52 MET 1997

INCDIR = $(QTDIR)/include
CFLAGS = -g -fno-strength-reduce -Wall -W
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CC = gcc
MOC = $(QTDIR)/bin/moc
SHELL =	/bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/extensions/imageio/src/Makefile,v $

INCPATH	=	-I$(INCDIR)
LINK	=	$(CC) 
LFLAGS	=	-shared -Wl,-soname,libqimgio.so
LIBS	=	-L$(QTDIR)/lib -lqt -L/usr/X11R6/lib -lX11
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
TARGET	=	libqimgio.so.0.1
TARGETA	=	libqimgio.a
TARGETD	=	libqimgio.so.0.1
TARGET0	=	libqimgio.so
TARGET1	=	libqimgio.so.0

####### Implicit rules

.SUFFIXES: .cpp .cxx .cc .C .c

.cpp.o:
	$(CC) -c $(CFLAGS) $(LIBCFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CC) -c $(CFLAGS) $(LIBCFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CC) -c $(CFLAGS) $(LIBCFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CC) -c $(CFLAGS) $(LIBCFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(LIBCFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all:  $(TARGET) include

$(TARGET): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGET) $(TARGET0) $(TARGET1)
	$(LINK) $(LFLAGS) -o $(TARGETD) $(OBJECTS) $(OBJMOC) $(LIBS)
	-ln -s $(TARGET) $(TARGET0)
	-ln -s $(TARGET) $(TARGET1)
	-rm -f ../../../lib/$(TARGET)
	-rm -f ../../../lib/$(TARGET0)
	-rm -f ../../../lib/$(TARGET1)
	-mv $(TARGET) $(TARGET0) $(TARGET1) ../../../lib/

staticlib: $(TARGETA) include

$(TARGETA): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGETA)
	$(AR) $(TARGETA) $(OBJECTS) $(OBJMOC)
	$(RANLIB) $(TARGETA)
	-mv $(TARGETA) ../../../lib

moc: $(SRCMOC)

include:
	( cd $(QTDIR)/include ; ln -s -f ../extensions/imageio/src/*.h . )

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC)
	-rm -f $(TARGETA) $(TARGETD) $(TARGET0) $(TARGET1)
	-rm -f *~ core

####### Compile

qimageio.o: qimageio.cpp

qjpegio.o: qjpegio.cpp

qpngio.o: qpngio.cpp

