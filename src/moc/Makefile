#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################

#######	Directories

SRCDIR		= $(QTDIR)/src
INCDIR		= $(SRCDIR)/include
LIBDIR		= $(SRCDIR)/lib
CCINCDIR	= /usr/g++-include

#######	Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

#######	Files

SOURCES		= moc.l moc.y
LEXYACC		= lex.yy.c moc.C
OBJECTS		= moc.o
TARGET		= moc

#######	Implicit rules

.SUFFIXES:
.SUFFIXES: .C $(SUFFIXES)

.C.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

#######	Build rules

all: $(SOURCES) $(LEXYACC) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -L$(LIBDIR) -lqtools
	strip moc
	cp moc $(QTDIR)/bin

rcs:: ./RCS/$$@,v

showfiles:
	@echo $(HEADERS) $(SOURCES)

showheaders:
	@echo $(HEADERS)

showsources:
	@echo $(SOURCES)

depend:
	makedepend -I$(INCDIR) -I$(CCINCDIR) $(SOURCES)

clean:
	/bin/rm -f *.o core TRACEFS.LOG *.bak *BAK *~ *% #*
	/bin/rm -f $(LEXYACC) $(TARGET) y.output y.tab.h

#######	Programs

lex.yy.c: moc.l
	flex moc.l
 
moc.C: moc.y
	yacc -d moc.y
	mv y.tab.c moc.C
 

# DO NOT DELETE THIS LINE -- make depend depends on it.

moc.o: /home/popcorn/hanord/qt/src/include/qstring.h
moc.o: /home/popcorn/hanord/qt/src/include/qarray.h
moc.o: /home/popcorn/hanord/qt/src/include/qgarray.h
moc.o: /home/popcorn/hanord/qt/src/include/qglobal.h /usr/include/string.h
moc.o: /usr/include/features.h /usr/include/sys/cdefs.h
moc.o: /usr/local/lib/gcc-include/stddef.h /usr/include/malloc.h
moc.o: /home/popcorn/hanord/qt/src/include/qlist.h
moc.o: /home/popcorn/hanord/qt/src/include/qglist.h
moc.o: /home/popcorn/hanord/qt/src/include/qcollect.h
moc.o: /home/popcorn/hanord/qt/src/include/qglobal.h
moc.o: /home/popcorn/hanord/qt/src/include/qstring.h
moc.o: /home/popcorn/hanord/qt/src/include/qarray.h
moc.o: /home/popcorn/hanord/qt/src/include/qgarray.h /usr/include/string.h
moc.o: /usr/include/features.h /usr/include/sys/cdefs.h
moc.o: /usr/local/lib/gcc-include/stddef.h
moc.o: /home/popcorn/hanord/qt/src/include/qdatetm.h /usr/include/ctype.h
moc.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/stdlib.h
moc.o: /usr/include/errno.h /usr/include/linux/errno.h
moc.o: /usr/local/lib/gcc-include/float.h
