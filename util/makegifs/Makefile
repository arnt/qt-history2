####### This section was automatically generated from
#######    /home/hanord/qt/util/Makefile for building on
#######     on ...

INCDIR = $(QTDIR)/include
CFLAGS =  -g -Wall -W -I$(QTDIR)/examples/life
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CC = gcc
MOC = moc
SHELL =	/bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/util/makegifs/Makefile,v $
#

####### Files

SOURCES	=	makeppms.cpp $(QTDIR)/examples/life/life.cpp
OBJECTS	=	makeppms.o life.o
SRCMETA	=	moc_life.cpp
OBJMETA =	moc_life.o
TARGET	=	makeppms

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all:
	@echo "make gifs  - generate widget gifs in $(QTDIR)/gif/"
	@echo "make $(TARGET)  - generate ppm generator"

gifs: $(TARGET)
	./$(TARGET)
	for i in *.ppm; do ppmquant 256 $$i | \
	    ppmtogif >$(QTDIR)/gif/$${i%ppm}gif; rm $$i; done

$(TARGET): $(SRCMETA) $(OBJECTS) $(OBJMETA)
	$(CC) $(OBJECTS) $(OBJMETA) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	-rm -f *.o *.bak *~ *% #*
	-rm -f $(SRCMETA) $(TARGET)

####### Explicit dependencies

life.o: $(QTDIR)/examples/life/life.cpp
	$(CC) -o life.o -c $(CFLAGS) -I$(INCDIR) $(QTDIR)/examples/life/life.cpp

####### Meta objects

moc_life.cpp: $(QTDIR)/examples/life/life.h
	$(MOC) $(QTDIR)/examples/life/life.h -o moc_life.cpp

# DO NOT DELETE THIS LINE -- make depend depends on it.

