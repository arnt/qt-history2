####### This section was automatically generated from
#######    /home/hanord/qt/examples/Makefile for building on
#######     on Tue Apr  1 20:32:47 MET DST 1997

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
# $Source: /tmp/cvs/qt/tests/layout/Makefile,v $

####### Files

SOURCES	=	bex.cpp cex.cpp gex.cpp hex.cpp qex.cpp mex.cpp
OBJECTS	=	bex.o cex.o gex.o hex.o qex.o mex.o

TARGET	=	bex cex gex hex qex mex

####### Implicit rules

.SUFFIXES: .cpp .o $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

.o:
	$(CC) $@.o -o $@ $(LFLAGS)

####### Build rules

all: $(TARGET)

run: all
	for a in $(TARGET) ; do [ -f $$a -a -x $$a ] && (  ./$$a & ) ; done 

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(SOURCES) $(HEADERS) Makefile

clean:
	/bin/rm -f *.o *.bak *~ *% "#"*
	/bin/rm -f $(TARGET)


# DO NOT DELETE THIS LINE -- make depend depends on it.
