####### This section was automatically generated from
#######    /home/hanord/qt/tutorial/Makefile for building on
#######     on Tue Apr  1 19:27:48 MET DST 1997

INCDIR = $(QTDIR)/include
CFLAGS =  -O2 -fno-strength-reduce -Wall -W
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CC = gcc
MOC = moc
SHELL =	/bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/tutorial/t14/Makefile,v $
#

####### Files

HEADERS =	lcdrange.h	cannon.h	gamebrd.h
SOURCES =	lcdrange.cpp	cannon.cpp	gamebrd.cpp	main.cpp
OBJECTS =	lcdrange.o	cannon.o	gamebrd.o	main.o
SRCMETA =	mlcdrang.cpp	mcannon.cpp	mgamebr.cpp
OBJMETA =	mlcdrang.o	mcannon.o	mgamebr.o
TARGET	=	t14

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS) $(OBJMETA)
	$(CC) $(OBJECTS) $(OBJMETA) -o $(TARGET) $(LFLAGS) -lm

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

clean:
	rm -f *.o *.bak *~ *% #*
	rm -f $(SRCMETA) $(TARGET)

####### Meta classes

mlcdrang.cpp: lcdrange.h
	$(MOC) -o $@ lcdrange.h

mcannon.cpp: cannon.h
	$(MOC) -o $@ cannon.h

mgamebr.cpp: gamebrd.h
	$(MOC) -o $@ gamebrd.h


# DO NOT DELETE THIS LINE -- make depend depends on it.
