#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################

####### Directories

INCDIR	=	../tools

####### Compiler

CFLAGS	=	-g
CC	=	gcc

####### Extra flags for yacc-written code

YACCCFLAGS = -Wno-unused -Wno-parentheses

# $Source: /tmp/cvs/qt/src/moc/Makefile,v $

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
		qcollect.o	\
		qdatetm.o	\
		qdstream.o	\
		qgarray.o	\
		qgdict.o	\
		qglist.o	\
		qglobal.o	\
		qgvector.o	\
		qiodev.o	\
		qstring.o
OBJECTS =	mocgen.o
TARGET	=	moc

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I. -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(TOOLOBJ) $(OBJECTS)
	$(CC) $(OBJECTS) $(TOOLOBJ) -o $(TARGET)

depend:
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	-rm -f *.o *.bak *~ *% #*
	-rm -f $(TARGET) $(YACCOUT) $(YACCHDR)

####### Process lex/yacc files

$(LEXOUT): moc.l
	$(LEX) moc.l

$(TMPCPP): moc.y $(LEXOUT)
	$(YACC) moc.y
	-rm -f $(TMPCPP)
	mv $(YACCOUT) $(TMPCPP)

####### Compile the .cpp files

mocgen.o: mocgen.cpp
	$(CC) -c $(CFLAGS) $(YACCCFLAGS) -I$(INCDIR) mocgen.cpp -o mocgen.o

qbuffer.o: ../tools/qbuffer.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qbuffer.cpp -o qbuffer.o

qcollect.o: ../tools/qcollect.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qcollect.cpp -o qcollect.o

qdatetm.o: ../tools/qdatetm.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qdatetm.cpp -o qdatetm.o

qdstream.o: ../tools/qdstream.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qdstream.cpp -o qdstream.o

qgarray.o: ../tools/qgarray.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qgarray.cpp -o qgarray.o

qgdict.o: ../tools/qgdict.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qgdict.cpp -o qgdict.o

qglist.o: ../tools/qglist.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qglist.cpp -o qglist.o

qglobal.o: ../tools/qglobal.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qglobal.cpp -o qglobal.o

qgvector.o: ../tools/qgvector.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qgvector.cpp -o qgvector.o

qiodev.o: ../tools/qiodev.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qiodev.cpp -o qiodev.o

qstring.o: ../tools/qstring.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) ../tools/qstring.cpp -o qstring.o


# DO NOT DELETE THIS LINE -- make depend depends on it.
