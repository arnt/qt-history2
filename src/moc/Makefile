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
YACC		= yacc -t -v -d

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

depend: $(SRCMETA) $(TARGET)
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *BAK *~ *% #*
	/bin/rm -f $(TARGET) $(LEX_FILE) $(YACC_HDR_FILE) $(CPP_FILE)

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

####### Lex/yacc etc.

$(LEX_FILE): moc.l
	$(LEX) moc.l

$(CPP_FILE): moc.y $(LEX_FILE)
	$(YACC) moc.y
	mv $(YACC_FILE) $(CPP_FILE)


# DO NOT DELETE THIS LINE -- make depend depends on it.


