#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################

####### Directories

BASEDIR =	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
CC	=	gcc

LEX	=	flex
YACC	=	yacc -d

####### Files

SOURCES =	moc.l		moc.y
LEXOUT	=	lex.yy.c
YACCOUT =	y.tab.c
YACCHDR =	y.tab.h
TMPCPP	=	moc.cpp
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
OBJECTS =	moc.o
TARGET	=	moc

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) $(TOOLOBJ)
	$(CC) $(OBJECTS) $(TOOLOBJ) -o $(TARGET)

include:

depend:
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	rm -f *.o *.bak *.BAK *~ *% #*
	rm -f $(TARGET) $(LEXOUT) $(YACCOUT) $(YACCHDR) $(TMPCPP)

####### Lex/yacc etc.

$(LEXOUT): moc.l
	$(LEX) moc.l

$(TMPCPP): moc.y $(LEXOUT)
	$(YACC) moc.y
	mv $(YACCOUT) $(TMPCPP)

$(OBJ): $(TMPCPP)
	$(CC) -c $(CFLAGS) $(FLEXFLAGS) -I$(INCDIR) $<

####### Tools needed for moc

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
