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
		  qdatetm.h	qdict.h		qfile.h		qgarray.h     \
		  qgdict.h	qglist.h	qglobal.h	qgvector.h    \
		  qintdict.h	qlist.h		qqueue.h	qres.h	      \
		  qshared.h	qstack.h	qstream.h	qstring.h     \
		  qstrlist.h	qstrvec.h	qtracefs.h	qvector.h
SOURCES		= qbitarry.cpp	qbuffer.cpp	qcollect.cpp	qdatetm.cpp   \
		  qfile.cpp	qgarray.cpp	qgdict.cpp	qglist.cpp    \
		  qglobal.cpp	qgvector.cpp	qres.cpp	qstream.cpp   \
		  qstring.cpp	qtracefs.cpp
OBJECTS		= qbitarry.o	qbuffer.o	qcollect.o	qdatetm.o     \
		  qfile.o	qgarray.o	qgdict.o	qglist.o      \
		  qglobal.o	qgvector.o	qres.o		qstream.o     \
		  qstring.o	qtracefs.o
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
qbitarry.o: qstream.h
qbuffer.o: qbuffer.h qstream.h qglobal.h qstring.h qarray.h qgarray.h
qbuffer.o: qshared.h /usr/include/string.h /usr/include/features.h
qbuffer.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qbuffer.o: /usr/include/stdlib.h /usr/include/errno.h
qbuffer.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qcollect.o: qcollect.h qglobal.h
qdatetm.o: qdatetm.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qdatetm.o: /usr/include/string.h /usr/include/features.h
qdatetm.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qdatetm.o: qstream.h /usr/include/stdio.h /usr/include/libio.h
qdatetm.o: /usr/include/time.h
qfile.o: qfile.h qstream.h qglobal.h qstring.h qarray.h qgarray.h qshared.h
qfile.o: /usr/include/string.h /usr/include/features.h
qfile.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qfile.o: /usr/include/stdio.h /usr/include/libio.h
qfile.o: /usr/local/lib/gcc-include/limits.h
qfile.o: /usr/local/lib/gcc-include/syslimits.h
qgarray.o: qgarray.h qshared.h qglobal.h /usr/include/string.h
qgarray.o: /usr/include/features.h /usr/include/sys/cdefs.h
qgarray.o: /usr/local/lib/gcc-include/stddef.h /usr/include/stdlib.h
qgarray.o: /usr/include/errno.h /usr/include/linux/errno.h
qgarray.o: /usr/local/lib/gcc-include/float.h
qgdict.o: qgdict.h qcollect.h qglobal.h qlist.h qglist.h qstring.h qarray.h
qgdict.o: qgarray.h qshared.h /usr/include/string.h /usr/include/features.h
qgdict.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qgdict.o: qstream.h /usr/include/ctype.h
qglist.o: qglist.h qcollect.h qglobal.h qgvector.h qstream.h
qglobal.o: qglobal.h /usr/include/stdio.h /usr/include/libio.h
qglobal.o: /usr/include/sys/cdefs.h /usr/include/features.h
qglobal.o: /usr/local/lib/gcc-include/stdarg.h /usr/include/stdlib.h
qglobal.o: /usr/local/lib/gcc-include/stddef.h /usr/include/errno.h
qglobal.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qgvector.o: qgvector.h qcollect.h qglobal.h qglist.h qstring.h qarray.h
qgvector.o: qgarray.h qshared.h /usr/include/string.h /usr/include/features.h
qgvector.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qgvector.o: qstream.h /usr/include/stdlib.h /usr/include/errno.h
qgvector.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qres.o: qres.h qstrvec.h qstring.h qarray.h qgarray.h qshared.h qglobal.h
qres.o: /usr/include/string.h /usr/include/features.h
qres.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qres.o: qvector.h qgvector.h qcollect.h qstream.h qstrlist.h qlist.h qglist.h
qres.o: qdict.h qgdict.h /usr/include/stdio.h /usr/include/libio.h
qres.o: /usr/include/ctype.h
qstream.o: qstream.h qglobal.h qstring.h qarray.h qgarray.h qshared.h
qstream.o: /usr/include/string.h /usr/include/features.h
qstream.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qstream.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/ctype.h
qstream.o: /usr/include/stdlib.h /usr/include/errno.h
qstream.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qstring.o: qstring.h qarray.h qgarray.h qshared.h qglobal.h
qstring.o: /usr/include/string.h /usr/include/features.h
qstring.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qstring.o: /usr/include/stdio.h /usr/include/libio.h
qstring.o: /usr/local/lib/gcc-include/stdarg.h /usr/include/stdlib.h
qstring.o: /usr/include/errno.h /usr/include/linux/errno.h
qstring.o: /usr/local/lib/gcc-include/float.h /usr/include/ctype.h qstream.h
qtracefs.o: qtracefs.h
