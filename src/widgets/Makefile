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

INTERNAL=	qheader.h 	qheader.cpp	qspinbox.h	qspinbox.cpp  \
		qvalidator.h	qvalidator.cpp	qscrollview.h	qscrollview.cpp
HEADERS =	qbttngrp.h	qbutton.h	qchkbox.h	qcombo.h      \
		qframe.h	qgrpbox.h	qlabel.h	qlcdnum.h     \
		qlined.h	qlistbox.h	qmenubar.h	qmenudta.h    \
		qpopmenu.h	qpushbt.h	qradiobt.h	qrangect.h    \
		qslider.h	qscrbar.h	qtablevw.h	qtooltip.h    \
		qmlined.h       qheader.h	qtabbar.h	qspinbox.h    \
		qprogbar.h	qscrollview.h	qvalidator.h
SOURCES =	qbttngrp.cpp	qbutton.cpp	qchkbox.cpp	qcombo.cpp    \
		qframe.cpp	qgrpbox.cpp	qlabel.cpp	qlcdnum.cpp   \
		qlined.cpp	qlistbox.cpp	qmenubar.cpp	qmenudta.cpp  \
		qpopmenu.cpp	qpushbt.cpp	qradiobt.cpp	qrangect.cpp  \
		qslider.cpp	qscrbar.cpp	qtablevw.cpp	qtooltip.cpp  \
		qmlined.cpp     qheader.cpp	qtabbar.cpp	qspinbox.cpp  \
		qprogbar.cpp	qscrollview.cpp	qvalidator.cpp
OBJECTS =	qbttngrp.o	qbutton.o	qchkbox.o	qcombo.o      \
		qframe.o	qgrpbox.o	qlabel.o	qlcdnum.o     \
		qlined.o	qlistbox.o	qmenubar.o	qmenudta.o    \
		qpopmenu.o	qpushbt.o	qradiobt.o	qrangect.o    \
		qslider.o	qscrbar.o	qtablevw.o	qtooltip.o    \
		qmlined.o       qheader.o	qtabbar.o	qspinbox.o    \
		qprogbar.o	qscrollview.o	# qvalidator.o
SRCMETA =	mbttngrp.cpp	mbutton.cpp	mchkbox.cpp	mcombo.cpp    \
		mframe.cpp	mgrpbox.cpp	mlabel.cpp	mlcdnum.cpp   \
		mlined.cpp	mlistbox.cpp	mmenubar.cpp	mpopmenu.cpp  \
		mpushbt.cpp	mradiobt.cpp	mscrbar.cpp	mtablevw.cpp  \
		mmlined.cpp	mtooltip.cpp	mslider.cpp	mheader.cpp   \
		mtabbar.cpp	mspinbox.cpp	mprogbar.cpp	mscrollview.cpp
OBJMETA =	mbttngrp.o	mbutton.o	mchkbox.o	mcombo.o      \
		mframe.o	mgrpbox.o	mlabel.o	mlcdnum.o     \
		mlined.o	mlistbox.o	mmenubar.o	mpopmenu.o    \
		mpushbt.o	mradiobt.o	mscrbar.o	mtablevw.o    \
		mmlined.o	mtooltip.o	mslider.o	mheader.o     \
		mtabbar.o	mspinbox.o	mprogbar.o	mscrollview.o

TARGET = $(LIBDIR)/libqwidgets.a

#######	Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

#######	Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(HEADERS) $(SOURCES) $(OBJECTS) $(OBJMETA)

$(TARGET): $(OBJECTS) $(OBJMETA)
	rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS) $(OBJMETA)
	ranlib $(TARGET)

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/widgets/-'` $(INCDIR)

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

showinternal:
	@echo $(INTERNAL)

showheaders:
	@echo $(HEADERS)

showsources:
	@echo $(SOURCES)

moc: $(SRCMETA)

depend: $(SRCMETA)
	gcc -I$(INCDIR) -M $(SOURCES) $(SRCMETA) > .depend

clean:
	rm -f *.o .depend *.bak *~ *% #*
	rm -f $(SRCMETA)

#######	Meta classes

mbttngrp.cpp: qbttngrp.h
	moc qbttngrp.h -o mbttngrp.cpp

mbutton.cpp: qbutton.h
	moc qbutton.h -o mbutton.cpp

mchkbox.cpp: qchkbox.h
	moc qchkbox.h -o mchkbox.cpp

mcombo.cpp: qcombo.h
	moc qcombo.h -o mcombo.cpp

mframe.cpp: qframe.h
	moc qframe.h -o mframe.cpp

mgrpbox.cpp: qgrpbox.h
	moc qgrpbox.h -o mgrpbox.cpp

mlabel.cpp: qlabel.h
	moc qlabel.h -o mlabel.cpp

mheader.cpp: qheader.h
	moc qheader.h -o mheader.cpp

mlcdnum.cpp: qlcdnum.h
	moc qlcdnum.h -o mlcdnum.cpp

mlined.cpp: qlined.h
	moc qlined.h -o mlined.cpp

mlistbox.cpp: qlistbox.h
	moc qlistbox.h -o mlistbox.cpp

mmenubar.cpp: qmenubar.h
	moc qmenubar.h -o mmenubar.cpp

mmlined.cpp: qmlined.h
	moc qmlined.h -o mmlined.cpp

mpopmenu.cpp: qpopmenu.h
	moc qpopmenu.h -o mpopmenu.cpp

mpushbt.cpp: qpushbt.h
	moc qpushbt.h -o mpushbt.cpp

mradiobt.cpp: qradiobt.h
	moc qradiobt.h -o mradiobt.cpp

mscrbar.cpp: qscrbar.h
	moc qscrbar.h -o mscrbar.cpp

mslider.cpp: qslider.h
	moc qslider.h -o mslider.cpp

mtablevw.cpp: qtablevw.h
	moc qtablevw.h -o mtablevw.cpp

mtooltip.cpp: qtooltip.h
	moc qtooltip.h -o mtooltip.cpp

mtabbar.cpp: qtabbar.h
	moc qtabbar.h -o mtabbar.cpp

mspinbox.cpp: qspinbox.h
	moc qspinbox.h -o mspinbox.cpp

mprogbar.cpp: qprogbar.h
	moc qprogbar.h -o mprogbar.cpp

mscrollview.cpp: qscrollview.h
	moc qscrollview.h -o mscrollview.cpp

# mvalidator.cpp: qvalidator.h
# 	moc qvalidator.h -o mvalidator.cpp

-include .depend
