#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

LEX		= flex
YACC		= yacc -d

####### Files

SOURCES		= moc.l		moc.y
LEX_FILE	= lex.yy.c
YACC_FILE	= y.tab.c
YACC_HDR_FILE	= y.tab.h
CPP_FILE	= moc.cpp
TOOLOBJECTS	= qbuffer.o	\
		  qcollect.o	\
		  qdatetm.o	\
		  qdstream.o	\
		  qgarray.o	\
		  qglist.o	\
		  qglobal.o	\
		  qgvector.o	\
		  qiodev.o	\
		  qstring.o
OBJECTS		= moc.o
TARGET		= moc

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(SOURCES) $(TARGET)

obj: $(HEADERS) $(SOURCES) $(OBJECTS)

$(TARGET): $(OBJECTS) $(TOOLOBJECTS)
	$(CC) $(OBJECTS) $(TOOLOBJECTS) -o $(TARGET)

rcs:: ./RCS/$$@,v

include: # nothing yet

depend: $(SRCMETA)
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *BAK *~ *% #*
	/bin/rm -f $(TARGET) $(LEX_FILE) $(YACC_HDR_FILE) $(CPP_FILE)

# 'make checkout' updates files that MUST be old
# 'make checkout CODATE=-d1994/12/24' recreates 1994/12/24
# 'make checkout CODATE=-d2000/1/1 updates all files
checkout:
	@for rcs in RCS/*,v ; do \
	    file=`basename $$rcs ,v` ;\
	    [ -n "${CODATE}" ] || \
	    [ '!' -f $$file ] || \
	    ( [ '!' -w $$file ] && [ $$rcs -nt $$file ] ) && \
	    co ${CODATE} $$file ;\
	done; \
	/bin/true # tell make it worked

####### Lex/yacc etc.

$(LEX_FILE): moc.l
	$(LEX) moc.l

$(CPP_FILE): moc.y $(LEX_FILE)
	$(YACC) moc.y
	mv $(YACC_FILE) $(CPP_FILE)

####### Tools

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
