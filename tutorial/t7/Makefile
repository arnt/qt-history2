####### This section is automatically generated from
#######    /home/agulbra/qt/tutorial/Makefile

INCDIR	= ${QTDIR}/include
CFLAGS	= -O2 -fno-strength-reduce -W -Wtemplate-debugging -Wparentheses -Wuninitialized -Wchar-subscripts -Wformat -Wtrigraphs -Wcomment -Wswitch -Wunused -Wreturn-type -Wimplicit -Wpointer-arith -Wsynth -Wconversion -Wno-overloaded-virtual
LFLAGS	= 
CC	= gcc
MOC	= moc

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/tutorial/t7/Makefile,v $
#

####### Files

HEADERS =	lcdrange.h
SOURCES =	lcdrange.cpp	main.cpp
OBJECTS =	lcdrange.o	main.o
SRCMETA =	mlcdrang.cpp
OBJMETA =	mlcdrang.o
TARGET	=	t7

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS) $(OBJMETA)
	$(CC) $(OBJECTS) $(OBJMETA) -o $(TARGET) $(LFLAGS) -lm

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

clean:
	rm -f *.o *.bak *~ *% #*
	rm -f $(SRCMETA) $(TARGET)

####### Meta classes

mlcdrang.cpp: lcdrange.h
	$(MOC) -o $@ $<


# DO NOT DELETE THIS LINE -- make depend depends on it.
