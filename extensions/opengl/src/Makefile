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
MOC	=	moc
AR	=	ar q

####### Files

HEADERS =	qgl.h
SOURCES =	qgl.cpp
OBJECTS =	qgl.o
SRCMOC	=	moc_qgl.cpp
OBJMOC	=	moc_qgl.o
TARGET	=	libqgl.a

####### Implicit rules

.SUFFIXES: .cpp .c

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCPATH) $(LIBCFLAGS) -o $@ $<

####### Build rules

all: $(TARGET) include

$(TARGET): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGET)
	$(AR) $(TARGET) $(OBJECTS) $(OBJMOC)
	-rm -f $(QTDIR)/lib/$(TARGET)
	-mv $(TARGET)  $(QTDIR)/lib

moc: $(SRCMOC)

include:
	( cd $(QTDIR)/include ; ln -s -f ../extensions/opengl/src/*.h . )

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC)

####### Compile

qgl.o: qgl.cpp \
		qgl.h

moc_qgl.o: moc_qgl.cpp \
		qgl.h

moc_qgl.cpp: qgl.h
	$(MOC) qgl.h -o moc_qgl.cpp
