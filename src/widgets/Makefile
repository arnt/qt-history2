#############################################################################
# Qt Makefile - widgets
#############################################################################

#######	Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

#######	Compiler

CFLAGS	=	-O2 -DCHECK_MEMORY
CC	=	gcc

#######	Files

HEADERS =	qbttngrp.h	qbutton.h	qchkbox.h	qcombo.h      \
		qframe.h	qgrpbox.h	qiconbt.h	qlabel.h      \
		qlcdnum.h	qlined.h	qlistbox.h	qmenubar.h    \
		qmenudta.h	qpopmenu.h	qpushbt.h	qicon.h \
		qradiobt.h	qrangect.h	qscrbar.h	qtablew.h
SOURCES =	qbttngrp.cpp	qbutton.cpp	qchkbox.cpp	qcombo.cpp    \
		qframe.cpp	qgrpbox.cpp	qiconbt.cpp	qlabel.cpp    \
		qlcdnum.cpp	qlined.cpp	qlistbox.cpp	qmenubar.cpp  \
		qmenudta.cpp	qpopmenu.cpp	qpushbt.cpp	qicon.cpp \
		qradiobt.cpp	qrangect.cpp	qscrbar.cpp	qtablew.cpp
OBJECTS =	qbttngrp.o	qbutton.o	qchkbox.o	qcombo.o      \
		qframe.o	qgrpbox.o	qiconbt.o	qlabel.o      \
		qlcdnum.o	qlined.o	qlistbox.o	qmenubar.o    \
		qmenudta.o	qpopmenu.o	qpushbt.o	qicon.o \
		qradiobt.o	qrangect.o	qscrbar.o	qtablew.o  
SRCMETA =	mbttngrp.cpp	mbutton.cpp	mchkbox.cpp	mcombo.cpp    \
		mframe.cpp	mgrpbox.cpp	miconbt.cpp	mlabel.cpp    \
		mlcdnum.cpp	mlined.cpp	mlistbox.cpp	mmenubar.cpp  \
		mpopmenu.cpp	mpushbt.cpp	mradiobt.cpp	micon.cpp \
		mscrbar.cpp	mtablew.cpp
OBJMETA =	mbttngrp.o	mbutton.o	mchkbox.o	mcombo.o      \
		mframe.o	mgrpbox.o	miconbt.o	mlabel.o      \
		mlcdnum.o	mlined.o	mlistbox.o	mmenubar.o    \
		mpopmenu.o	mpushbt.o	mradiobt.o	micon.o \
		mscrbar.o	mtablew.o
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

mbttngrp.cpp: qbttngrp.h
	$(QTDIR)/bin/moc qbttngrp.h -o mbttngrp.cpp

mbutton.cpp: qbutton.h
	$(QTDIR)/bin/moc qbutton.h -o mbutton.cpp

mchkbox.cpp: qchkbox.h
	$(QTDIR)/bin/moc qchkbox.h -o mchkbox.cpp

mcombo.cpp: qcombo.h
	$(QTDIR)/bin/moc qcombo.h -o mcombo.cpp

mframe.cpp: qframe.h
	$(QTDIR)/bin/moc qframe.h -o mframe.cpp

mgrpbox.cpp: qgrpbox.h
	$(QTDIR)/bin/moc qgrpbox.h -o mgrpbox.cpp

miconbt.cpp: qiconbt.h
	$(QTDIR)/bin/moc qiconbt.h -o miconbt.cpp

mlabel.cpp: qlabel.h
	$(QTDIR)/bin/moc qlabel.h -o mlabel.cpp

mlcdnum.cpp: qlcdnum.h
	$(QTDIR)/bin/moc qlcdnum.h -o mlcdnum.cpp

mlined.cpp: qlined.h
	$(QTDIR)/bin/moc qlined.h -o mlined.cpp

mlistbox.cpp: qlistbox.h
	$(QTDIR)/bin/moc qlistbox.h -o mlistbox.cpp

mmenubar.cpp: qmenubar.h
	$(QTDIR)/bin/moc qmenubar.h -o mmenubar.cpp

mpopmenu.cpp: qpopmenu.h
	$(QTDIR)/bin/moc qpopmenu.h -o mpopmenu.cpp

mpushbt.cpp: qpushbt.h
	$(QTDIR)/bin/moc qpushbt.h -o mpushbt.cpp

mradiobt.cpp: qradiobt.h
	$(QTDIR)/bin/moc qradiobt.h -o mradiobt.cpp

mscrbar.cpp: qscrbar.h
	$(QTDIR)/bin/moc qscrbar.h -o mscrbar.cpp

mtablew.cpp: qtablew.h
	$(QTDIR)/bin/moc qtablew.h -o mtablew.cpp

micon.cpp: qicon.h
	$(QTDIR)/bin/moc qicon.h -o micon.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.

