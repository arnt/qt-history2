####### This section is automatically generated from
#######    /home/agulbra/qt/examples/Makefile

INCDIR	= ${QTDIR}/include
CFLAGS	= -O -g -fno-strength-reduce -W -Wtemplate-debugging -Wparentheses -Wuninitialized -Wchar-subscripts -Wformat -Wtrigraphs -Wcomment -Wswitch -Wunused -Wreturn-type -Wimplicit -Wpointer-arith -Wsynth -Wconversion -Wno-overloaded-virtual
LFLAGS	= -lqt
CC	= gcc
MOC	= moc

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/tests/well/Makefile,v $
#

####### Files

SOURCES	=	well.cpp
OBJECTS	=	well.o
TARGET	=	well
MOCFILES=	well.moc
####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(MOCFILES) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LFLAGS)

well.moc: well.cpp
	moc -o well.moc well.cpp

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(SOURCES) $(HEADERS) Makefile

clean:
	-rm -f *.o *.bak *~ *% "#*"
	-rm -f $(TARGET) $(MOCFILES)

# DO NOT DELETE THIS LINE -- make depend depends on it.
