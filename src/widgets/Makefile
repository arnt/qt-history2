#############################################################################
# Qt Makefile - widgets
#############################################################################

#######	Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

#######	Compiler

CFLAGS	=	-O2
CC	=	gcc

#######	Files

HEADERS =	qbttngrp.h	qbutton.h	qchkbox.h	qcombo.h      \
		qframe.h	qgrpbox.h	qlabel.h	qlcdnum.h     \
		qlined.h	qlistbox.h	qmenubar.h	qmenudta.h    \
		qpopmenu.h	qpushbt.h	qradiobt.h	qrangect.h    \
		qscrbar.h	qtablevw.h	qgeom.h		qbasic.h
SOURCES =	qbttngrp.cpp	qbutton.cpp	qchkbox.cpp	qcombo.cpp    \
		qframe.cpp	qgrpbox.cpp	qlabel.cpp	qlcdnum.cpp   \
		qlined.cpp	qlistbox.cpp	qmenubar.cpp	qmenudta.cpp  \
		qpopmenu.cpp	qpushbt.cpp	qradiobt.cpp	qrangect.cpp  \
		qscrbar.cpp	qtablevw.cpp	qgeom.cpp	qbasic.cpp
OBJECTS =	qbttngrp.o	qbutton.o	qchkbox.o	qcombo.o      \
		qframe.o	qgrpbox.o	qlabel.o	qlcdnum.o     \
		qlined.o	qlistbox.o	qmenubar.o	qmenudta.o    \
		qpopmenu.o	qpushbt.o	qradiobt.o	qrangect.o    \
		qscrbar.o	qtablevw.o	qgeom.o		qbasic.o
SRCMETA =	mbttngrp.cpp	mbutton.cpp	mchkbox.cpp	mcombo.cpp    \
		mframe.cpp	mgrpbox.cpp	mlabel.cpp	mlcdnum.cpp   \
		mlined.cpp	mlistbox.cpp	mmenubar.cpp	mpopmenu.cpp  \
		mpushbt.cpp	mradiobt.cpp	mscrbar.cpp	mtablevw.cpp  \
		mgeom.cpp	mbasic.cpp
OBJMETA =	mbttngrp.o	mbutton.o	mchkbox.o	mcombo.o      \
		mframe.o	mgrpbox.o	mlabel.o	mlcdnum.o     \
		mlined.o	mlistbox.o	mmenubar.o	mpopmenu.o    \
		mpushbt.o	mradiobt.o	mscrbar.o	mtablevw.o    \
		mgeom.o		mbasic.o
TARGET = $(LIBDIR)/libqwidgets.a

#######	Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

#######	Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(HEADERS) $(SOURCES) $(OBJECTS) $(OBJMETA)

$(TARGET): $(OBJECTS) $(OBJMETA)
	/bin/rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS) $(OBJMETA)
	ranlib $(TARGET)

rcs:: ./RCS/$$@,v

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/widgets/-'` $(INCDIR)

showheaders:
	@echo $(HEADERS)

showsources:
	@echo $(SOURCES)

depend: $(SRCMETA)
	makedepend -I$(INCDIR) $(SOURCES) $(SRCMETA) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *BAK *~ *% #*
	/bin/rm -f $(SRCMETA)

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

#######	Meta classes
