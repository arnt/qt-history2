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
# $Source: /tmp/cvs/qt/extensions/nsplugin/examples/qtimage/Makefile,v $
#

####### Compiler, tools and options

INCPATH	=	-I$(INCDIR)
CFLAGSX =	$(CFLAGS) $(LIBCFLAGS) -DPNG_SUPPORT
LINK	=	gcc
LFLAGS	=	-shared -Wl,-soname,$(TARGET)
LIBS	=	-L$(QTDIR)/lib -L/usr/X11/lib -lqnp -lqt -lXt -lX11 -lm

####### Files

HEADERS =	
SOURCES =	qtimage.cpp 
OBJECTS =	qtimage.o
SRCMOC	=	
OBJMOC	=	
TARGETD	=	qtimage.so

####### Implicit rules

.SUFFIXES: .cpp .c .C

.cpp.o:
	$(CC) -c $(CFLAGSX) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGSX) $(INCPATH) -o $@ $<

.C.o:
	$(CC) -c $(CFLAGSX) $(INCPATH) -o $@ $<

####### Build rules

all: $(TARGETD) 

install: $(TARGETD)
	cp -f $(TARGETD) $(HOME)/.netscape/plugins/$(TARGETD)

test: $(TARGETD)
	gcc $(TARGETD) harness.cpp $(LIBS)

$(TARGETD): $(OBJECTS) $(OBJMOC)
	-rm -f $(TARGETD)
	$(LINK) $(LFLAGS) -o $(TARGETD) $(OBJECTS) $(OBJMOC) $(LIBS)

moc: $(SRCMOC)

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC) $(TARGETD)
	-rm -f *~ core

####### Compile

qtimage.o: qtimage.cpp

