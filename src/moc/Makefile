#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

LEX		= flex
YACC		= yacc -d

####### Files

SOURCES		= moc.l		moc.y
LEX_FILE	= lex.yy.c
YACC_FILE	= y.tab.c
YACC_HDR_FILE	= y.tab.h
CPP_FILE	= moc.cpp
OBJECTS		= moc.o
TARGET		= moc

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -L$(LIBDIR) -lqtools
	strip moc
	cp moc $(QTDIR)/bin

rcs:: ./RCS/$$@,v

depend: $(SRCMETA)
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *BAK *~ *% #*
	/bin/rm -f $(TARGET) $(LEX_FILE) $(YACC_HDR_FILE) $(CPP_FILE)

####### Lex/yacc etc.

$(LEX_FILE): moc.l
	$(LEX) moc.l

$(CPP_FILE): moc.y $(LEX_FILE)
	$(YACC) moc.y
	mv $(YACC_FILE) $(CPP_FILE)


# DO NOT DELETE THIS LINE -- make depend depends on it.

moc.o: /home/popcorn/home/hanord/qt/include/qstring.h
moc.o: /home/popcorn/home/hanord/qt/include/qarray.h
moc.o: /home/popcorn/home/hanord/qt/include/qgarray.h
moc.o: /home/popcorn/home/hanord/qt/include/qshared.h
moc.o: /home/popcorn/home/hanord/qt/include/qglobal.h /usr/include/string.h
moc.o: /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/malloc.h
moc.o: /home/popcorn/home/hanord/qt/include/qlist.h
moc.o: /home/popcorn/home/hanord/qt/include/qglist.h
moc.o: /home/popcorn/home/hanord/qt/include/qcollect.h
moc.o: /home/popcorn/home/hanord/qt/include/qglobal.h
moc.o: /home/popcorn/home/hanord/qt/include/qstring.h
moc.o: /home/popcorn/home/hanord/qt/include/qarray.h
moc.o: /home/popcorn/home/hanord/qt/include/qgarray.h
moc.o: /home/popcorn/home/hanord/qt/include/qshared.h /usr/include/string.h
moc.o: /usr/include/features.h /usr/include/sys/cdefs.h
moc.o: /home/popcorn/home/hanord/qt/include/qdatetm.h /usr/include/ctype.h
moc.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/stdlib.h
moc.o: /usr/include/errno.h /usr/include/linux/errno.h lex.yy.c
moc.o: /usr/include/malloc.h
