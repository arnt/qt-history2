#############################################################################
# Qt Makefile - tools
#############################################################################

####### Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
CC	=	gcc

####### Files

HEADERS	=	qarray.h	qbitarry.h	qbuffer.h	qcache.h      \
		qcollect.h	qdatetm.h	qdict.h		qdstream.h    \
		qfile.h		qgarray.h	qgcache.h	qgdict.h      \
		qgeneric.h	qglist.h	qglobal.h	qgvector.h    \
		qintcach.h	qintdict.h	qiodev.h	qlist.h	      \
		qqueue.h	qregexp.h	qshared.h     \
		qstack.h	qstring.h	qstrlist.h	qstrvec.h     \
		qtstream.h	qvector.h       qfileinf.h	qdir.h	      \
		qfiledef.h
SOURCES	=	qbitarry.cpp	qbuffer.cpp	qcollect.cpp	qdatetm.cpp   \
		qdstream.cpp	qfile.cpp	qgarray.cpp	qgcache.cpp   \
		qgdict.cpp	qglist.cpp	qglobal.cpp	qgvector.cpp  \
		qiodev.cpp	qregexp.cpp	qstring.cpp   \
		qtstream.cpp	qfileinf.cpp	qdir.cpp
OBJECTS	=	qbitarry.o	qbuffer.o	qcollect.o	qdatetm.o     \
		qdstream.o	qfile.o		qgarray.o	qgcache.o     \
		qgdict.o	qglist.o	qglobal.o	qgvector.o    \
		qiodev.o	qregexp.o	qstring.o     \
		qtstream.o	qfileinf.o	qdir.o
TARGET	=	$(LIBDIR)/libqtools.a

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(HEADERS) $(SOURCES) $(OBJECTS)

$(TARGET): $(OBJECTS)
	rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS)
	ranlib $(TARGET)

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/tools/-'` $(INCDIR)

showheaders:
	@echo $(HEADERS)

showsources:
	@echo $(SOURCES)

moc:

depend:
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	rm -f *.o *.bak *~ *% #*


# DO NOT DELETE THIS LINE -- make depend depends on it.
