#############################################################################
# Qt Makefile - tools
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib

####### Compiler

CFLAGS		= -O2 -DCHECK_MEMORY
CC		= gcc

####### Files

HEADERS		= qarray.h	qbitarry.h	qbuffer.h	qcache.h      \
		  qcollect.h	qdatetm.h	qdict.h		qdstream.h    \
		  qfile.h	qgarray.h	qgcache.h	qgdict.h      \
		  qgeneric.h	qglist.h	qglobal.h	qgvector.h    \
		  qintcach.h	qintdict.h	qiodev.h	qlist.h	      \
		  qmemchk.h	qqueue.h	qregexp.h	qshared.h     \
		  qstack.h	qstring.h	qstrlist.h	qstrvec.h     \
		  qtstream.h	qvector.h
SOURCES		= qbitarry.cpp	qbuffer.cpp	qcollect.cpp	qdatetm.cpp   \
		  qdstream.cpp	qfile.cpp	qgarray.cpp	qgcache.cpp   \
		  qgdict.cpp	qglist.cpp	qglobal.cpp	qgvector.cpp  \
		  qiodev.cpp	qmemchk.cpp	qregexp.cpp	qstring.cpp   \
		  qtstream.cpp
OBJECTS		= qbitarry.o	qbuffer.o	qcollect.o	qdatetm.o     \
		  qdstream.o	qfile.o		qgarray.o	qgcache.o     \
		  qgdict.o	qglist.o	qglobal.o	qgvector.o    \
		  qiodev.o	qmemchk.o	qregexp.o	qstring.o     \
		  qtstream.o
TARGET		= $(LIBDIR)/libqtools.a

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(HEADERS) $(SOURCES) $(OBJECTS)

$(TARGET): $(OBJECTS)
	/bin/rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS)
	ranlib $(TARGET)

rcs:: ./RCS/$$@,v

showfiles:
	@echo $(HEADERS) $(SOURCES)

showheaders:
	@echo $(HEADERS)

showsources:
	@echo $(SOURCES)

depend:
	makedepend -Y$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *BAK *~ *% #*

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


# DO NOT DELETE THIS LINE -- make depend depends on it.

qbitarry.o: qbitarry.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qbitarry.o: qgeneric.h qdstream.h qiodev.h
qbuffer.o: qbuffer.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h
qbuffer.o: qshared.h qgeneric.h
qcollect.o: qcollect.h qglobal.h
qdatetm.o: qdatetm.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qdatetm.o: qgeneric.h qdstream.h qiodev.h
qdstream.o: qdstream.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h
qdstream.o: qshared.h qgeneric.h
qfile.o: qfile.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h qshared.h
qfile.o: qgeneric.h qdatetm.h
qgarray.o: qgarray.h qshared.h qglobal.h
qgcache.o: qgcache.h qcollect.h qglobal.h qglist.h qgdict.h qlist.h
qgcache.o: qgeneric.h qdict.h qstring.h qarray.h qgarray.h qshared.h
qgdict.o: qgdict.h qcollect.h qglobal.h qlist.h qglist.h qgeneric.h qstring.h
qgdict.o: qarray.h qgarray.h qshared.h qdstream.h qiodev.h
qglist.o: qglist.h qcollect.h qglobal.h qgvector.h qdstream.h qiodev.h
qglobal.o: qglobal.h
qgvector.o: qgvector.h qcollect.h qglobal.h qglist.h qstring.h qarray.h
qgvector.o: qgarray.h qshared.h qgeneric.h qdstream.h qiodev.h
qiodev.o: qiodev.h qglobal.h
qmemchk.o: qmemchk.h qglobal.h
qregexp.o: qregexp.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qregexp.o: qgeneric.h
qstring.o: qstring.h qarray.h qgarray.h qshared.h qglobal.h qgeneric.h
qstring.o: qdstream.h qiodev.h
qtstream.o: qtstream.h qiodev.h qglobal.h qfile.h qstring.h qarray.h
qtstream.o: qgarray.h qshared.h qgeneric.h
