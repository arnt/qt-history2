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
		  qregexp.h     qgcache.h	qintcach.h 	qcache.h
SOURCES		= qbitarry.cpp	qbuffer.cpp	qcollect.cpp	qdatetm.cpp   \
		  qdstream.cpp	qfile.cpp	qgarray.cpp	qgdict.cpp    \
		  qglist.cpp	qglobal.cpp	qgvector.cpp	qiodev.cpp    \
		  	qmemchk.cpp		\
		  qstring.cpp		qtstream.cpp  \
		  	qregexp.cpp     qgcache.cpp
OBJECTS		= qbitarry.o	qbuffer.o	qcollect.o	qdatetm.o     \
		  qdstream.o	qfile.o		qgarray.o	qgdict.o      \
		  qglist.o	qglobal.o	qgvector.o	qiodev.o      \
		  	qmemchk.o	\
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

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/tools/-'` $(INCDIR)

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


# DO NOT DELETE
qbitarry.o: qbitarry.cpp qbitarry.h qstring.h qarray.h qgarray.h \
 qshared.h qglobal.h qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qdstream.h \
 qiodev.h
qbuffer.o: qbuffer.cpp qbuffer.h qiodev.h qglobal.h qstring.h qarray.h \
 qgarray.h qshared.h qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h
qcollect.o: qcollect.cpp qcollect.h qglobal.h
qdatetm.o: qdatetm.cpp qdatetm.h qstring.h qarray.h qgarray.h \
 qshared.h qglobal.h qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qdstream.h \
 qiodev.h /usr/include/stdio.h /usr/include/libio.h \
 /usr/include/_G_config.h /usr/include/time.h /usr/include/sys/time.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/linux/time.h /usr/include/sys/types.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/gnu/types.h /usr/include/confname.h
qdstream.o: qdstream.cpp qdstream.h qiodev.h qglobal.h qstring.h \
 qarray.h qgarray.h qshared.h qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/asm/types.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/linux/sockios.h /usr/include/linux/in.h
qfile.o: qfile.cpp qfile.h qiodev.h qglobal.h qstring.h qarray.h \
 qgarray.h qshared.h qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h \
 qdatetm.h /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/asm/types.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/errno.h \
 /usr/include/linux/errno.h /usr/include/asm/errno.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h
qgarray.o: qgarray.cpp qgarray.h qshared.h qglobal.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/malloc.h
qgdict.o: qgdict.cpp qgdict.h qcollect.h qglobal.h qlist.h qglist.h \
 qgeneric.h qstring.h qarray.h qgarray.h qshared.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qdstream.h \
 qiodev.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h /usr/include/linux/version.h \
 /usr/include/asm/byteorder.h
qglist.o: qglist.cpp qglist.h qcollect.h qglobal.h qgvector.h \
 qdstream.h qiodev.h
qglobal.o: qglobal.cpp qglobal.h /usr/include/stdio.h \
 /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/libio.h \
 /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stdarg.h \
 /usr/include/stdlib.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h
qgvector.o: qgvector.cpp qgvector.h qcollect.h qglobal.h qglist.h \
 qstring.h qarray.h qgarray.h qshared.h qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qdstream.h \
 qiodev.h /usr/include/stdlib.h /usr/include/errno.h \
 /usr/include/linux/errno.h /usr/include/asm/errno.h \
 /usr/include/alloca.h /usr/include/malloc.h
qiodev.o: qiodev.cpp qiodev.h qglobal.h
qmemchk.o: qmemchk.cpp qmemchk.h qglobal.h
qstring.o: qstring.cpp qstring.h qarray.h qgarray.h qshared.h \
 qglobal.h qgeneric.h /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qdstream.h \
 qiodev.h /usr/include/stdio.h /usr/include/libio.h \
 /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/ctype.h \
 /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h
qtstream.o: qtstream.cpp qtstream.h qiodev.h qglobal.h \
 /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/libio.h /usr/include/_G_config.h qfile.h qstring.h \
 qarray.h qgarray.h qshared.h qgeneric.h /usr/include/string.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h
qregexp.o: qregexp.cpp qregexp.h qstring.h qarray.h qgarray.h \
 qshared.h qglobal.h qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h \
 /usr/include/malloc.h
qgcache.o: qgcache.cpp qgcache.h qcollect.h qglobal.h qglist.h \
 qgdict.h qlist.h qgeneric.h qdict.h qstring.h qarray.h qgarray.h \
 qshared.h /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h
