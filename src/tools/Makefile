#############################################################################
# Qt Makefile - tools
#############################################################################

#######	Directories

SRCDIR		= $(QTDIR)/src
INCDIR		= $(SRCDIR)/include
LIBDIR		= $(SRCDIR)/lib

#######	Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

#######	Files

HEADERS		= qarray.h	qbitarry.h	qbuffer.h	qcollect.h    \
		  qdatetm.h	qdict.h		qdstream.h	qfile.h	      \
		  qgarray.h	qgdict.h	qgeneric.h	qglist.h      \
		  qglobal.h	qgvector.h	qintdict.h	qiodev.h      \
		  qipcomm.h	qlist.h		qnpipe.h	qqueue.h      \
		  qres.h	qshared.h	qstack.h	qstring.h     \
		  qstrlist.h	qstrvec.h	qtcomm.h	qtcpcomm.h    \
		  qtracefs.h	qtstream.h	qudpcomm.h	qvector.h
SOURCES		= qbitarry.cpp	qbuffer.cpp	qcollect.cpp	qdatetm.cpp   \
		  qdstream.cpp	qfile.cpp	qgarray.cpp	qgdict.cpp    \
		  qglist.cpp	qglobal.cpp	qgvector.cpp	qiodev.cpp    \
		  qipcomm.cpp	qnpipe.cpp	qres.cpp	qstring.cpp   \
		  qtcomm.cpp	qtcpcomm.cpp	qtracefs.cpp	qtstream.cpp  \
		  qudpcomm.cpp
OBJECTS		= qbitarry.o	qbuffer.o	qcollect.o	qdatetm.o     \
		  qdstream.o	qfile.o		qgarray.o	qgdict.o      \
		  qglist.o	qglobal.o	qgvector.o	qiodev.o      \
		  qipcomm.o	qnpipe.o	qres.o		qstring.o     \
		  qtcomm.o	qtcpcomm.o	qtracefs.o	qtstream.o    \
		  qudpcomm.o
TARGET		= $(LIBDIR)/libqtools.a

#######	Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

#######	Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

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
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *BAK *~ *% #*


# DO NOT DELETE THIS LINE -- make depend depends on it.

qbitarry.o: qbitarry.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qbitarry.o: /usr/include/string.h /usr/include/features.h
qbitarry.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qbitarry.o: qdstream.h qiodev.h
qbuffer.o: qbuffer.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h
qbuffer.o: qshared.h /usr/include/string.h /usr/include/features.h
qbuffer.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qbuffer.o: /usr/include/stdlib.h /usr/include/errno.h
qbuffer.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qcollect.o: qcollect.h qglobal.h
qdatetm.o: qdatetm.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qdatetm.o: /usr/include/string.h /usr/include/features.h
qdatetm.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qdatetm.o: qdstream.h qiodev.h /usr/include/stdio.h /usr/include/libio.h
qdatetm.o: /usr/include/time.h
qdstream.o: qdstream.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h
qdstream.o: qshared.h /usr/include/string.h /usr/include/features.h
qdstream.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qdstream.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/ctype.h
qdstream.o: /usr/include/stdlib.h /usr/include/errno.h
qdstream.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qfile.o: qfile.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h qshared.h
qfile.o: /usr/include/string.h /usr/include/features.h
qfile.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qfile.o: /usr/include/stdio.h /usr/include/libio.h qdatetm.h
qfile.o: /usr/include/sys/types.h /usr/include/linux/types.h
qfile.o: /usr/include/sys/stat.h /usr/include/linux/stat.h
qfile.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h /usr/include/errno.h
qfile.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/limits.h
qfile.o: /usr/local/lib/gcc-include/syslimits.h
qgarray.o: qgarray.h qshared.h qglobal.h /usr/include/string.h
qgarray.o: /usr/include/features.h /usr/include/sys/cdefs.h
qgarray.o: /usr/local/lib/gcc-include/stddef.h /usr/include/stdlib.h
qgarray.o: /usr/include/errno.h /usr/include/linux/errno.h
qgarray.o: /usr/local/lib/gcc-include/float.h
qgdict.o: qgdict.h qcollect.h qglobal.h qlist.h qglist.h qstring.h qarray.h
qgdict.o: qgarray.h qshared.h /usr/include/string.h /usr/include/features.h
qgdict.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qgdict.o: qdstream.h qiodev.h /usr/include/ctype.h
qglist.o: qglist.h qcollect.h qglobal.h qgvector.h qdstream.h qiodev.h
qglobal.o: qglobal.h /usr/include/stdio.h /usr/include/libio.h
qglobal.o: /usr/include/sys/cdefs.h /usr/include/features.h
qglobal.o: /usr/local/lib/gcc-include/stdarg.h /usr/include/stdlib.h
qglobal.o: /usr/local/lib/gcc-include/stddef.h /usr/include/errno.h
qglobal.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qgvector.o: qgvector.h qcollect.h qglobal.h qglist.h qstring.h qarray.h
qgvector.o: qgarray.h qshared.h /usr/include/string.h /usr/include/features.h
qgvector.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qgvector.o: qdstream.h qiodev.h /usr/include/stdlib.h /usr/include/errno.h
qgvector.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qiodev.o: qiodev.h qglobal.h
qipcomm.o: qipcomm.h qtcomm.h qiodev.h qglobal.h qbuffer.h qstring.h qarray.h
qipcomm.o: qgarray.h qshared.h /usr/include/string.h /usr/include/features.h
qipcomm.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qipcomm.o: /usr/include/stdlib.h /usr/include/errno.h
qipcomm.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qnpipe.o: qnpipe.h qtcomm.h qiodev.h qglobal.h qbuffer.h qstring.h qarray.h
qnpipe.o: qgarray.h qshared.h /usr/include/string.h /usr/include/features.h
qnpipe.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qres.o: qres.h qstrvec.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qres.o: /usr/include/string.h /usr/include/features.h
qres.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qres.o: qvector.h qgvector.h qcollect.h qdstream.h qiodev.h qstrlist.h
qres.o: qlist.h qglist.h qdict.h qgdict.h /usr/include/stdio.h
qres.o: /usr/include/libio.h /usr/include/ctype.h
qstring.o: qstring.h qarray.h qgarray.h qshared.h qglobal.h
qstring.o: /usr/include/string.h /usr/include/features.h
qstring.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qstring.o: qdstream.h qiodev.h /usr/include/stdio.h /usr/include/libio.h
qstring.o: /usr/local/lib/gcc-include/stdarg.h /usr/include/stdlib.h
qstring.o: /usr/include/errno.h /usr/include/linux/errno.h
qstring.o: /usr/local/lib/gcc-include/float.h /usr/include/ctype.h
qtcomm.o: qtcomm.h qiodev.h qglobal.h qbuffer.h qstring.h qarray.h qgarray.h
qtcomm.o: qshared.h /usr/include/string.h /usr/include/features.h
qtcomm.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qtcpcomm.o: qtcpcomm.h qipcomm.h qtcomm.h qiodev.h qglobal.h qbuffer.h
qtcpcomm.o: qstring.h qarray.h qgarray.h qshared.h /usr/include/string.h
qtcpcomm.o: /usr/include/features.h /usr/include/sys/cdefs.h
qtcpcomm.o: /usr/local/lib/gcc-include/stddef.h
qtracefs.o: qtracefs.h
qtstream.o: qtstream.h qiodev.h qglobal.h qstring.h qarray.h qgarray.h
qtstream.o: qshared.h /usr/include/string.h /usr/include/features.h
qtstream.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qtstream.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/ctype.h
qtstream.o: /usr/include/stdlib.h /usr/include/errno.h
qtstream.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qudpcomm.o: qudpcomm.h qipcomm.h qtcomm.h qiodev.h qglobal.h qbuffer.h
qudpcomm.o: qstring.h qarray.h qgarray.h qshared.h /usr/include/string.h
qudpcomm.o: /usr/include/features.h /usr/include/sys/cdefs.h
qudpcomm.o: /usr/local/lib/gcc-include/stddef.h
