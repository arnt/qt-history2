####### This section is automatically generated from
#######    /home/hanord/qt/examples/Makefile

INCDIR	=	$(QTDIR)/include
CFLAGS	=	-O -g -W -Wall
LFLAGS	=	-L$(QTDIR)/lib -lqt
CC	=	gcc
MOC	=	moc

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/tests/listview/Makefile,v $
#

####### Files

SOURCES	=	lv.cpp
OBJECTS	=	lv.o	
SRCMETA	=	lv.moc	
TARGET	=	lv

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(SRCMETA) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo widgets.cpp Makefile

clean:
	-rm -f *.o *.bak *~ *% #*
	-rm -f $(SRCMETA) $(TARGET)

####### Meta objects

lv.moc: lv.cpp
	$(MOC) lv.cpp -o lv.moc

# DO NOT DELETE THIS LINE -- make depend depends on it.
