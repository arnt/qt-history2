#############################################################################
# Qt Makefile - Tutorial 6
#############################################################################

####### Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
LFLAGS	=	-L$(LIBDIR) -lqt -lX11
CC	=	gcc

####### Files

HEADERS	=
SOURCES	=
OBJECTS	=	t6.o
SRCMETA	=
OBJMETA	=
TARGET	=	t6

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: qtdirtest $(TARGET)

qtdirtest:
	@if [ "$(QTDIR)" = "" ]; then			\
	    make "QTDIR=/usr/local/qt";			\
	    exit;					\
	fi

$(TARGET): $(OBJECTS) $(OBJMETA)
	$(CC) $(OBJECTS) $(OBJMETA) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *~ *% #*
	/bin/rm -f $(SRCMETA) $(TARGET)

# DO NOT DELETE THIS LINE -- make depend depends on it.
