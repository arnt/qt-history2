#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################

####### Directories

SRCDIR		= $(QTDIR)/src
INCDIR		= $(SRCDIR)/include
LIBDIR		= $(SRCDIR)/lib

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

####### Files

SOURCES		= moc.l		moc.y
LEXYACC		= lex.yy.c	moc.cpp
OBJECTS		= moc.o
TARGET		= moc

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS) $(LEXYACC)
	$(CC) $(OBJECTS) -o $(TARGET) -L$(LIBDIR) -lqtools
	strip moc
	cp moc $(QTDIR)/bin

rcs:: ./RCS/$$@,v

depend: $(SRCMETA)
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *BAK *~ *% #*
	/bin/rm -f $(LEXYACC) $(TARGET) y.output y.tab.h

####### Programs

lex.yy.c: moc.l
	flex moc.l
 
moc.cpp: moc.y
	yacc -d moc.y
	mv y.tab.c moc.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.

moc.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
moc.o: /usr/include/string.h /usr/include/features.h /usr/include/sys/cdefs.h
moc.o: /usr/local/lib/gcc-include/stddef.h /usr/include/malloc.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
moc.o: /usr/include/string.h /usr/include/features.h /usr/include/sys/cdefs.h
moc.o: /usr/local/lib/gcc-include/stddef.h
moc.o: /home/popcorn/home/hanord/qt/src/include/qdatetm.h
moc.o: /usr/include/ctype.h /usr/include/stdio.h /usr/include/libio.h
moc.o: /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h
moc.o: /usr/local/lib/gcc-include/float.h lex.yy.c /usr/include/malloc.h
