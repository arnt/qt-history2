#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################

####### BEGIN
SYSCONF_CFLAGS_QT	=	-O2
SYSCONF_CC	=	g++
SYSCONF_YACCCFLAGS = -Wno-unused -Wno-parentheses
####### END

####### Compiler, tools and options

CC      =       $(SYSCONF_CC)
CFLAGS  =       $(SYSCONF_CFLAGS_QT) $(SYSCONF_YACCCFLAGS) $(SYSCONF_CFLAGS_X11) $(SYSCONF_CFLAGS_LIB)
LFLAGS  =       $(SYSCONF_LFLAGS_X11)
LIBS    =       $(SYSCONF_LIBS_X11)
MOC     =       $(SYSCONF_MOC)

####### Extra Inlcude Directories

INCPATH	= -I../tools

####### Compiler

# $Source: /tmp/cvs/qt/src/moc/GNUmakefile,v $

####### Lex and yacc

LEX	=	flex
YACC	=	yacc -d

####### Files

SOURCES =	moc.l		moc.y
LEXOUT	=	lex.yy.c
YACCOUT =	y.tab.c
YACCHDR =	y.tab.h
TMPCPP	=	mocgen.cpp
TOOLOBJ =	qbuffer.o	\
		qcollection.o	\
		qdatetime.o	\
		qdatastream.o	\
		qgarray.o	\
		qgdict.o	\
		qglist.o	\
		qglobal.o	\
		qgvector.o	\
		qiodevice.o	\
		qstring.o
OBJECTS =	mocgen.o
TARGET	=	moc

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I. $(INCPATH) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(TOOLOBJ) $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) $(TOOLOBJ) -o $(TARGET)

depend:
	makedepend $(INCPATH) $(SOURCES) 2> /dev/null

clean:
	-rm -f *.o *.bak *~ *% #*
	-rm -f $(TARGET) $(YACCOUT)

####### Process lex/yacc files

$(LEXOUT): moc.l
	$(LEX) moc.l

$(TMPCPP): moc.y $(LEXOUT)
	$(YACC) moc.y
	-rm -f $(TMPCPP)
	mv $(YACCOUT) $(TMPCPP)

####### Compile the .cpp files

mocgen.o: mocgen.cpp
	$(CC) -c $(CFLAGS) $(YACCCFLAGS) $(INCPATH) mocgen.cpp -o mocgen.o

qbuffer.o: ../tools/qbuffer.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qbuffer.cpp -o qbuffer.o

qcollection.o: ../tools/qcollection.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qcollection.cpp -o qcollection.o

qdatetime.o: ../tools/qdatetime.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qdatetime.cpp -o qdatetime.o

qdatastream.o: ../tools/qdatastream.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qdatastream.cpp -o qdatastream.o

qgarray.o: ../tools/qgarray.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qgarray.cpp -o qgarray.o

qgdict.o: ../tools/qgdict.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qgdict.cpp -o qgdict.o

qglist.o: ../tools/qglist.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qglist.cpp -o qglist.o

qglobal.o: ../tools/qglobal.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qglobal.cpp -o qglobal.o

qgvector.o: ../tools/qgvector.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qgvector.cpp -o qgvector.o

qiodevice.o: ../tools/qiodevice.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qiodevice.cpp -o qiodevice.o

qstring.o: ../tools/qstring.cpp
	$(CC) -c $(CFLAGS) $(INCPATH) ../tools/qstring.cpp -o qstring.o


# DO NOT DELETE THIS LINE -- make depend depends on it.
