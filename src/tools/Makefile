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
		  qqueue.h	qres.h		qshared.h	qstack.h      \
		  qstring.h	qstrlist.h	qstrvec.h	      \
		  	qtstream.h		qvector.h     \
		  qregexp.h     qgcache.h
SOURCES		= qbitarry.cpp	qbuffer.cpp	qcollect.cpp	qdatetm.cpp   \
		  qdstream.cpp	qfile.cpp	qgarray.cpp	qgdict.cpp    \
		  qglist.cpp	qglobal.cpp	qgvector.cpp	qiodev.cpp    \
		  	qmemchk.cpp		qres.cpp      \
		  qstring.cpp		qtstream.cpp  \
		  	qregexp.cpp     qgcache.cpp
OBJECTS		= qbitarry.o	qbuffer.o	qcollect.o	qdatetm.o     \
		  qdstream.o	qfile.o		qgarray.o	qgdict.o      \
		  qglist.o	qglobal.o	qgvector.o	qiodev.o      \
		  	qmemchk.o		qres.o	      \
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

qbitarry.o: qbitarry.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qbitarry.o: qgeneric.h /usr/include/string.h /usr/include/features.h
qbitarry.o: /usr/include/sys/cdefs.h qdstream.h qiodev.h
qbuffer.o: qbuffer.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h
qbuffer.o: qshared.h qgeneric.h /usr/include/string.h /usr/include/features.h
qbuffer.o: /usr/include/sys/cdefs.h /usr/include/stdlib.h
qbuffer.o: /usr/include/errno.h /usr/include/linux/errno.h
qbuffer.o: /usr/include/alloca.h
qcollect.o: qcollect.h qglobal.h
qdatetm.o: qdatetm.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qdatetm.o: qgeneric.h /usr/include/string.h /usr/include/features.h
qdatetm.o: /usr/include/sys/cdefs.h qdstream.h qiodev.h /usr/include/stdio.h
qdatetm.o: /usr/include/libio.h /usr/include/_G_config.h /usr/include/time.h
qdstream.o: qdstream.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h
qdstream.o: qshared.h qgeneric.h /usr/include/string.h
qdstream.o: /usr/include/features.h /usr/include/sys/cdefs.h
qdstream.o: /usr/include/stdio.h /usr/include/libio.h
qdstream.o: /usr/include/_G_config.h /usr/include/ctype.h
qdstream.o: /usr/include/stdlib.h /usr/include/errno.h
qdstream.o: /usr/include/linux/errno.h /usr/include/alloca.h
qfile.o: qfile.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h qshared.h
qfile.o: qgeneric.h /usr/include/string.h /usr/include/features.h
qfile.o: /usr/include/sys/cdefs.h /usr/include/stdio.h /usr/include/libio.h
qfile.o: /usr/include/_G_config.h qdatetm.h /usr/include/sys/types.h
qfile.o: /usr/include/linux/types.h /usr/include/asm/types.h
qfile.o: /usr/include/sys/stat.h /usr/include/linux/stat.h
qfile.o: /usr/include/fcntl.h /usr/include/gnu/types.h
qfile.o: /usr/include/linux/fcntl.h /usr/include/errno.h
qfile.o: /usr/include/linux/errno.h /usr/include/limits.h
qfile.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
qfile.o: /usr/include/posix2_lim.h
qgarray.o: qgarray.h qshared.h qglobal.h /usr/include/string.h
qgarray.o: /usr/include/features.h /usr/include/sys/cdefs.h
qgarray.o: /usr/include/stdlib.h /usr/include/errno.h
qgarray.o: /usr/include/linux/errno.h /usr/include/alloca.h
qgarray.o: /usr/include/malloc.h
qgdict.o: qgdict.h qcollect.h qglobal.h qlist.h qglist.h qgeneric.h qstring.h
qgdict.o: qarray.h qgarray.h qshared.h /usr/include/string.h
qgdict.o: /usr/include/features.h /usr/include/sys/cdefs.h qdstream.h
qgdict.o: qiodev.h /usr/include/ctype.h
qglist.o: qglist.h qcollect.h qglobal.h qgvector.h qdstream.h qiodev.h
qglobal.o: qglobal.h /usr/include/stdio.h /usr/include/libio.h
qglobal.o: /usr/include/_G_config.h /usr/include/stdlib.h
qglobal.o: /usr/include/features.h /usr/include/sys/cdefs.h
qglobal.o: /usr/include/errno.h /usr/include/linux/errno.h
qglobal.o: /usr/include/alloca.h
qgvector.o: qgvector.h qcollect.h qglobal.h qglist.h qstring.h qarray.h
qgvector.o: qgarray.h qshared.h qgeneric.h /usr/include/string.h
qgvector.o: /usr/include/features.h /usr/include/sys/cdefs.h qdstream.h
qgvector.o: qiodev.h /usr/include/stdlib.h /usr/include/errno.h
qgvector.o: /usr/include/linux/errno.h /usr/include/alloca.h
qgvector.o: /usr/include/malloc.h
qiodev.o: qiodev.h qglobal.h
qmemchk.o: qmemchk.h qglobal.h
qres.o: qres.h qstrvec.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qres.o: qgeneric.h /usr/include/string.h /usr/include/features.h
qres.o: /usr/include/sys/cdefs.h qvector.h qgvector.h qcollect.h qdstream.h
qres.o: qiodev.h qstrlist.h qlist.h qglist.h qdict.h qgdict.h
qres.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
qres.o: /usr/include/ctype.h
qstring.o: qstring.h qarray.h qgarray.h qshared.h qglobal.h qgeneric.h
qstring.o: /usr/include/string.h /usr/include/features.h
qstring.o: /usr/include/sys/cdefs.h qdstream.h qiodev.h /usr/include/stdio.h
qstring.o: /usr/include/libio.h /usr/include/_G_config.h
qstring.o: /usr/include/stdlib.h /usr/include/errno.h
qstring.o: /usr/include/linux/errno.h /usr/include/alloca.h
qstring.o: /usr/include/ctype.h
qtstream.o: qtstream.h qiodev.h qglobal.h /usr/include/stdio.h
qtstream.o: /usr/include/libio.h /usr/include/_G_config.h qfile.h qstring.h
qtstream.o: qarray.h qgarray.h qshared.h qgeneric.h /usr/include/string.h
qtstream.o: /usr/include/features.h /usr/include/sys/cdefs.h
qtstream.o: /usr/include/ctype.h /usr/include/stdlib.h /usr/include/errno.h
qtstream.o: /usr/include/linux/errno.h /usr/include/alloca.h
qregexp.o: qregexp.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qregexp.o: qgeneric.h /usr/include/string.h /usr/include/features.h
qregexp.o: /usr/include/sys/cdefs.h /usr/include/ctype.h
qregexp.o: /usr/include/malloc.h
qgcache.o: qgcache.h qcollect.h qglobal.h qglist.h qgdict.h qlist.h
qgcache.o: qgeneric.h qdict.h qstring.h qarray.h qgarray.h qshared.h
qgcache.o: /usr/include/string.h /usr/include/features.h
qgcache.o: /usr/include/sys/cdefs.h
