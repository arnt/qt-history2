####### This section was automatically generated from
#######    /home/hanord/qt/util/Makefile for building on
#######     on ...

INCDIR = $(QTDIR)/include
CFLAGS =  -g -Wall -W
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

SOURCES	=	makeppms.cpp
OBJECTS	=	makeppms.o
SRCMETA	=	
OBJMETA =	
TARGET	=	makeppms

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all:
	@echo "make gifs  - generate widget gifs in $(QTDIR)/gif/"

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

####### Meta objects

# DO NOT DELETE THIS LINE -- make depend depends on it.

