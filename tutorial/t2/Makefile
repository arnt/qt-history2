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
# $Source: /tmp/cvs/qt/tutorial/t2/Makefile,v $
#

####### Files

SOURCES =	main.cpp
OBJECTS =	main.o
TARGET	=	t2

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LFLAGS) -lm

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

clean:
	rm -f *.o *.bak *~ *% #*
	rm -f $(TARGET)


# DO NOT DELETE THIS LINE -- make depend depends on it.
