#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################

####### Directories

BASEDIR =	$(QTDIR)
INCDIR	=	../tools
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
CC	=	gcc

####### Lex and yacc

LEX	=	flex
YACC	=	yacc -d

####### Files

SOURCES =	moc.l		moc.y
LEXOUT	=	lex.yy.c
YACCOUT =	y.tab.c
YACCHDR =	y.tab.h
TMPCPP	=	moc_gen.cpp
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
OBJECTS =	moc_gen.o
TARGET	=	moc

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I. -I$(INCDIR) $<

####### Build rules

all: $(TARGET)

$(TARGET): $(OBJECTS) $(TOOLOBJ)
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
	mv $(YACCOUT) $(TMPCPP)

####### External files used by the moc

qbuffer.o: ../tools/qbuffer.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qcollect.o: ../tools/qcollect.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qdatetm.o: ../tools/qdatetm.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qdstream.o: ../tools/qdstream.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qgarray.o: ../tools/qgarray.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qgdict.o: ../tools/qgdict.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qglist.o: ../tools/qglist.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qglobal.o: ../tools/qglobal.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qgvector.o: ../tools/qgvector.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qiodev.o: ../tools/qiodev.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@

qstring.o: ../tools/qstring.cpp
	$(CC) -c $(CFLAGS) -I$(INCDIR) $? -o $@


# DO NOT DELETE THIS LINE -- make depend depends on it.
