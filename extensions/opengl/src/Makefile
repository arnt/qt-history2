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

# Include path for Mesa.
GL_INC	=	-I/usr/X11R6/include

INCOPT =	-I$(INCDIR) $(GL_INC)
MOC	=	moc
AR	=	ar cqs

####### Files

HEADERS =	qgl.h
SOURCES =	qgl.cpp
OBJECTS =	qgl.o
SRCMOC	=	moc_qgl.cpp
OBJMOC	=	moc_qgl.o
TARGET	=	../../../lib/libqgl.a

####### Implicit rules

.SUFFIXES: .cpp .c

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCOPT) -o $@ $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGET)
	$(AR) $(TARGET) $(OBJECTS) $(OBJMOC)

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
