#############################################################################
# Makefile: Qt Tutorial 8
#############################################################################

####### Directories

BASEDIR =	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
LFLAGS	=	-L$(LIBDIR) -lqt -lX11 -lm
CC	=	gcc

####### Files

HEADERS =	lcdrange.h	cannon.h
SOURCES =	lcdrange.cpp	cannon.cpp	main.cpp
OBJECTS =	lcdrange.o	cannon.o	main.o
SRCMETA =	mlcdrang.cpp	mcannon.cpp
OBJMETA =	mlcdrang.o	mcannon.o
TARGET	=	t8

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS) $(OBJMETA)
	$(CC) $(OBJECTS) $(OBJMETA) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

clean:
	/bin/rm -f *.o *.bak *~ *% #*
	/bin/rm -f $(SRCMETA) $(TARGET)

####### Meta classes

mlcdrang.cpp: lcdrange.h
	moc -o $@ $<

mcannon.cpp: cannon.h
	moc -o $@ $<


# DO NOT DELETE THIS LINE -- make depend depends on it.
