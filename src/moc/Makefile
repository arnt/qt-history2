#############################################################################
# Qt Makefile - Meta Object Compiler (moc)
#############################################################################

####### Directories

BASEDIR	= $(QTDIR)
INCDIR	= $(BASEDIR)/include
LIBDIR	= $(BASEDIR)/lib

####### Compiler

CFLAGS	= -O2
CC	= gcc

LEX	= flex
YACC	= yacc -d

####### Files

SOURCES	=	moc.l		moc.y
LEXOUT	=	lex.yy.c
YACCOUT =	y.tab.c
YACCHDR =	y.tab.h
TMPCPP	=	moc.cpp
OBJECTS	=	moc.o
TARGET	=	moc

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

depend:
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *BAK *~ *% #*
	/bin/rm -f $(TARGET) $(LEXOUT) $(YACCOUT) $(YACCHDR) $(TMPCPP)

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
	/bin/true	# tell make it worked

####### Lex/yacc etc.

$(LEXOUT): moc.l
	$(LEX) moc.l

$(TMPCPP): moc.y $(LEXOUT)
	$(YACC) moc.y
	mv $(YACCOUT) $(TMPCPP)


# DO NOT DELETE THIS LINE -- make depend depends on it.

moc.o: /home/popcorn/home/hanord/qt/include/qstring.h
moc.o: /home/popcorn/home/hanord/qt/include/qarray.h
moc.o: /home/popcorn/home/hanord/qt/include/qgarray.h
moc.o: /home/popcorn/home/hanord/qt/include/qshared.h
moc.o: /home/popcorn/home/hanord/qt/include/qglobal.h
moc.o: /home/popcorn/home/hanord/qt/include/qgeneric.h /usr/include/string.h
moc.o: /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/malloc.h
moc.o: /home/popcorn/home/hanord/qt/include/qlist.h
moc.o: /home/popcorn/home/hanord/qt/include/qglist.h
moc.o: /home/popcorn/home/hanord/qt/include/qcollect.h
moc.o: /home/popcorn/home/hanord/qt/include/qglobal.h
moc.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
moc.o: /home/popcorn/home/hanord/qt/include/qstring.h
moc.o: /home/popcorn/home/hanord/qt/include/qarray.h
moc.o: /home/popcorn/home/hanord/qt/include/qgarray.h
moc.o: /home/popcorn/home/hanord/qt/include/qshared.h /usr/include/string.h
moc.o: /usr/include/features.h /usr/include/sys/cdefs.h
moc.o: /home/popcorn/home/hanord/qt/include/qdatetm.h /usr/include/ctype.h
moc.o: /usr/include/endian.h /usr/include/bytesex.h
moc.o: /usr/include/linux/version.h /usr/include/asm/byteorder.h
moc.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
moc.o: /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h
moc.o: /usr/include/asm/errno.h /usr/include/alloca.h
