####### This section was automatically generated from
#######    /home/hanord/qt/Makefile for building on
#######    linux-gcc-shared on Fri Feb 28 17:08:52 MET 1997

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
# $Source: /tmp/cvs/qt/extensions/opengl/src/Makefile,v $

####### Compiler, tools and options

INCPATH =	-I$(INCDIR) -I/usr/X11R6/include
LINK	=	$(CC) -shared
LIBS	=	$(LFLAGS) -L/usr/X11R6/lib -lMesaGL -lXext -lm
MOC	=	moc

####### Files

HEADERS =	qgl.h
SOURCES =	qgl.cpp
OBJECTS =	qgl.o
SRCMOC	=	moc_qgl.cpp
OBJMOC	=	moc_qgl.o
VERSION =	0.9
VER_MAJ =	0
VER_MIN =	9
TARGET	=	libqgl.so.0.9

####### Implicit rules

.SUFFIXES: .cpp .c

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCPATH) $(LIBCFLAGS) -o $@ $<

####### Build rules

all: $(TARGET) include

$(TARGET): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGET)
	$(LINK) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)
	-rm -f $(QTDIR)/lib/$(TARGET)
	-mv $(TARGET)  $(QTDIR)/lib
	( cd $(QTDIR)/lib ; \
	  rm -f libqgl.so ; \
	  ln -s libqgl.so.$(VERSION) libqgl.so ; \
	  rm -f libqgl.so.$(VER_MAJ) ; \
	  ln -s libqgl.so.$(VERSION) libqgl.so.$(VER_MAJ) )

moc: $(SRCMOC)

include:
	( cd $(QTDIR)/include ; ln -s -f ../extensions/opengl/src/*.h . )

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC)

####### Compile

qgl.o: qgl.cpp \
		qgl.h \
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
		$(QTDIR)/include/qfontinf.h

moc_qgl.o: moc_qgl.cpp \
		qgl.h \
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
		$(QTDIR)/include/qfontinf.h

moc_qgl.cpp: qgl.h
	$(MOC) qgl.h -o moc_qgl.cpp

