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
SOURCES =	qnp.cpp
OBJECTS =	qnp.o
SRCMOC	=	moc_qnp.cpp
OBJMOC	=	moc_qnp.o
TARGET	=	libqnp.a
TARGETA	=	libqnp.a

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all:  $(TARGET) include

$(TARGET): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGET)
	$(AR) $(TARGET) $(OBJECTS) $(OBJMOC)
	$(RANLIB) $(TARGET)
	-mv $(TARGET) ../../../lib

tmake: Makefile

Makefile: qnp.pro
	tmake qnp.pro -nodepend -o Makefile

moc: $(SRCMOC)

include:
	( cd $(QTDIR)/include ; ln -s -f ../extensions/nsplugin/src/*.h . )

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC)
	-rm -f $(TARGET)
	-rm -f *~ core

####### Compile

qnp.o: qnp.cpp

moc_qnp.o: moc_qnp.cpp \
		qnp.h

moc_qnp.cpp: qnp.h
	$(MOC) qnp.h -o moc_qnp.cpp

