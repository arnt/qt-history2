####### This section is automatically generated from
#######    /home/hanord/qt/examples/Makefile

INCDIR	=	$(QTDIR)/include
CFLAGS	=	-O2
LFLAGS	=	-L$(QTDIR)/lib -lqt
CC	=	gcc
MOC	=	moc

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/etc/xshape/Makefile,v $
#

# Additional X libraries you'll need
XLIBS	=	-L/usr/X11R6/lib -lXext -lX11

####### Files

SOURCES	=	xshape.cpp
OBJECTS	=	xshape.o
TARGET	=	xshape

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LFLAGS) $(XLIBS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(SOURCES) $(HEADERS) Makefile

clean:
	-rm -f *.o *.bak *~ *% #*
	-rm -f $(TARGET)


# DO NOT DELETE THIS LINE -- make depend depends on it.
