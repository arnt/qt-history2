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

INTERNAL=
HEADERS	=	qarray.h	qbitarry.h	qbuffer.h	qcache.h      \
		qcollect.h	qdatetm.h	qdict.h		qdir.h	      \
		qdstream.h	qfile.h		qfiledef.h	qfileinf.h    \
		qgarray.h	qgcache.h	qgdict.h	qgeneric.h    \
		qglist.h	qglobal.h	qgvector.h	qintcach.h    \
		qintdict.h	qiodev.h	qlist.h		qptrdict.h    \
		qqueue.h	qregexp.h	qshared.h	qstack.h      \
		qstring.h	qstrlist.h	qstrvec.h	qtstream.h    \
		qvector.h
SOURCES	=	qbitarry.cpp	qbuffer.cpp	qcollect.cpp	qdatetm.cpp   \
		qdir.cpp	qdstream.cpp	qfile.cpp	qfileinf.cpp  \
		qgarray.cpp	qgcache.cpp	qgdict.cpp	qglist.cpp    \
		qglobal.cpp	qgvector.cpp	qiodev.cpp	qregexp.cpp   \
		qstring.cpp	qtstream.cpp
OBJECTS	=	qbitarry.o	qbuffer.o	qcollect.o	qdatetm.o     \
		qdir.o		qdstream.o	qfile.o		qfileinf.o    \
		qgarray.o	qgcache.o	qgdict.o	qglist.o      \
		qglobal.o	qgvector.o	qiodev.o	qregexp.o     \
		qstring.o	qtstream.o
TARGET	=	$(LIBDIR)/libqtools.a

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(HEADERS) $(SOURCES) $(OBJECTS)

$(TARGET): $(OBJECTS)
	rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS)
	ranlib $(TARGET)

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/tools/-'` $(INCDIR)

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

showinternal:
	@echo $(BETA)

showheaders:
	@echo $(HEADERS)

showsources:
	@echo $(SOURCES)

moc:

depend:
	gcc -I$(INCDIR) -M $(SOURCES) > .depend

clean:
	rm -f *.o .depend *.bak *~ *% #*


-include .depend
