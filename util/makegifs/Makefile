####### This section was automatically generated from
#######    /home/hanord/qt/util/Makefile for building on
#######     on ...

INCDIR = $(QTDIR)/include
CFLAGS =  -g -Wall -W -I$(QTDIR)/examples/life
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CXX = g++
MOC = moc
SHELL =	/bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/util/makegifs/Makefile,v $
#

####### Files

SOURCES	=	makeppms.cpp $(QTDIR)/examples/life/life.cpp $(QTDIR)/examples/dirview/dirview.cpp
OBJECTS	=	makeppms.o life.o dirview.o
SRCMETA	=	moc_life.cpp moc_dirview.cpp makeppms.moc
OBJMETA =	moc_life.o moc_dirview.o
TARGET	=	makeppms

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CXX) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all:	$(TARGET)

pngs: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRCMETA) $(OBJECTS) $(OBJMETA)
	$(CXX) $(OBJECTS) $(OBJMETA) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	-rm -f *.o *.bak *~ *% *-[wm].ppm
	-rm -f $(SRCMETA) $(TARGET)

####### Explicit dependencies

life.o: $(QTDIR)/examples/life/life.cpp
	$(CXX) -o life.o -c $(CFLAGS) -I$(INCDIR) $(QTDIR)/examples/life/life.cpp
dirview.o: $(QTDIR)/examples/dirview/dirview.cpp
	$(CXX) -o dirview.o -c $(CFLAGS) -I$(INCDIR) $(QTDIR)/examples/dirview/dirview.cpp

####### Meta objects

moc_life.cpp: $(QTDIR)/examples/life/life.h
	$(MOC) $(QTDIR)/examples/life/life.h -o moc_life.cpp
moc_dirview.cpp: $(QTDIR)/examples/dirview/dirview.h
	$(MOC) $(QTDIR)/examples/dirview/dirview.h -o moc_dirview.cpp
makeppms.moc: makeppms.cpp
	$(MOC) makeppms.cpp -o makeppms.moc

# DO NOT DELETE THIS LINE -- make depend depends on it.

