####### This section is automatically generated from
#######    /home/agulbra/qt/tutorial/Makefile

INCDIR	= ${QTDIR}/include
CFLAGS	= -O2 -fno-strength-reduce -W -Wtemplate-debugging -Wparentheses -Wuninitialized -Wchar-subscripts -Wformat -Wtrigraphs -Wcomment -Wswitch -Wunused -Wreturn-type -Wimplicit -Wpointer-arith -Wsynth -Wconversion -Wno-overloaded-virtual
LFLAGS	= 
CC	= gcc
MOC	= moc

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/tutorial/t2/Makefile,v $
#

####### Files

SOURCES =	main.cpp
OBJECTS =	main.o
TARGET	=	t2

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LFLAGS) -lm

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

clean:
	rm -f *.o *.bak *~ *% #*
	rm -f $(TARGET)


# DO NOT DELETE THIS LINE -- make depend depends on it.
