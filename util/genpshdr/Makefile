#############################################################################
# Qt Makefile - genpshdr
#############################################################################

####### Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
LFLAGS	=	-L$(LIBDIR) -lqtools
CC	=	gcc

####### Files

SOURCES	=	genpshdr.cpp
OBJECTS	=	genpshdr.o
TARGET	=	genpshdr

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *~ *% #*
	/bin/rm -f $(TARGET)


# DO NOT DELETE THIS LINE -- make depend depends on it.
