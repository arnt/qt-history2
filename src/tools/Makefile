#############################################################################
# Qt Makefile - tools
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2 -DCHECK_MEMORY
CC		= gcc

####### Files

HEADERS		= qarray.h	qbitarry.h	qbuffer.h	qcollect.h    \
		  qdatetm.h	qdict.h		qdstream.h	qfile.h	      \
		  qgarray.h	qgdict.h	qgeneric.h	qglist.h      \
		  qglobal.h	qgvector.h	qintdict.h	qiodev.h      \
		  	qlist.h		qmemchk.h	      \
		  qqueue.h	qshared.h	qstack.h      \
		  qstring.h	qstrlist.h	qstrvec.h	      \
		  	qtstream.h		qvector.h     \
		  qregexp.h     qgcache.h
SOURCES		= qbitarry.cpp	qbuffer.cpp	qcollect.cpp	qdatetm.cpp   \
		  qdstream.cpp	qfile.cpp	qgarray.cpp	qgdict.cpp    \
		  qglist.cpp	qglobal.cpp	qgvector.cpp	qiodev.cpp    \
		  	qmemchk.cpp	\
		  qstring.cpp		qtstream.cpp  \
		  	qregexp.cpp     qgcache.cpp
OBJECTS		= qbitarry.o	qbuffer.o	qcollect.o	qdatetm.o     \
		  qdstream.o	qfile.o		qgarray.o	qgdict.o      \
		  qglist.o	qglobal.o	qgvector.o	qiodev.o      \
		  	qmemchk.o     \
		  qstring.o		qtstream.o    \
		  	qregexp.o       qgcache.o
TARGET		= $(LIBDIR)/libqtools.a
MOCSUPPORT	= $(LIBDIR)/libmocsupport.a
MOCOBJECTS	= qcollect.o	qdatetm.o     	qgarray.o	qglist.o \
		  qglobal.o	qstring.o

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

mocsupport: $(MOCOBJECTS)
	/bin/rm -f $(MOCSUPPORT)
	ar q $(MOCSUPPORT) $(MOCOBJECTS)
	ranlib $(MOCSUPPORT)

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


