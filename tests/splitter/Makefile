####### This section is automatically generated from
#######    /home/agulbra/qt/examples/Makefile

INCDIR	= ${QTDIR}/include
CFLAGS	= -O -g -fno-strength-reduce -W -Wtemplate-debugging -Wparentheses -Wuninitialized -Wchar-subscripts -Wformat -Wtrigraphs -Wcomment -Wswitch -Wunused -Wreturn-type -Wimplicit -Wpointer-arith -Wsynth -Wconversion -Wno-overloaded-virtual
LFLAGS	= -lqt
CC	= gcc
MOC	= moc

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/tests/splitter/Makefile,v $
#

####### Files

SOURCES	=	split.cpp
OBJECTS	=	split.o
TARGET	=	split
MOCFILES=	split.moc
####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(MOCFILES) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LFLAGS)

split.moc: split.cpp
	moc -o split.moc split.cpp

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(SOURCES) $(HEADERS) Makefile

clean:
	-rm -f *.o *.bak *~ *% "#*"
	-rm -f $(TARGET) $(MOCFILES)

# DO NOT DELETE THIS LINE -- make depend depends on it.
