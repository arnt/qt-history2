####### This section was automatically generated from
#######    /home/hanord/qt/Makefile for building on
#######    linux-gcc-shared on Fri Feb 28 17:08:50 MET 1997

INCDIR = $(QTDIR)/include
CFLAGS = -O2 -fno-strength-reduce -Wall -W
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CC = gcc
MOC = $(QTDIR)/bin/moc
SHELL =	/bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/extensions/opengl/examples/gear/Makefile,v $
#

####### Compiler, tools and options

INCPATH	=	-I$(INCDIR) -I/usr/X11R6/include
LIBS	=	$(LFLAGS) -lqgl
MOC	=	moc

####### Files

HEADERS =	
SOURCES =	gear.cpp
OBJECTS =	gear.o
SRCMOC	=	
OBJMOC	=	
TARGET	=	gear

####### Implicit rules

.SUFFIXES: .cpp .c

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all: $(TARGET) 

$(TARGET): $(OBJECTS) $(OBJMOC)
	$(CC) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)

moc: $(SRCMOC)

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC) $(TARGET)

####### Compile

gear.o: gear.cpp \
		$(QTDIR)/include/qgl.h \
		$(QTDIR)/include/qwidget.h \
		$(QTDIR)/include/qwindefs.h \
		$(QTDIR)/include/qobjdefs.h \
		$(QTDIR)/include/qglobal.h \
		$(QTDIR)/include/qobject.h \
		$(QTDIR)/include/qstring.h \
		$(QTDIR)/include/qarray.h \
		$(QTDIR)/include/qgarray.h \
		$(QTDIR)/include/qshared.h \
		$(QTDIR)/include/qgeneric.h \
		$(QTDIR)/include/qevent.h \
		$(QTDIR)/include/qrect.h \
		$(QTDIR)/include/qsize.h \
		$(QTDIR)/include/qpoint.h \
		$(QTDIR)/include/qpaintd.h \
		$(QTDIR)/include/qpalette.h \
		$(QTDIR)/include/qcolor.h \
		$(QTDIR)/include/qcursor.h \
		$(QTDIR)/include/qfont.h \
		$(QTDIR)/include/qfontmet.h \
		$(QTDIR)/include/qfontinf.h \
		$(QTDIR)/include/qapp.h

