#############################################################################
# Qt Makefile - widgets
#############################################################################

#######	Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib

#######	Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

#######	Files

HEADERS		= qbutton.h	qbttngrp.h	qchkbox.h	qlabel.h      \
		  qlcdnum.h     \
		  qlined.h	qlistbox.h	qmenubar.h	qmenudta.h    \
		  qpopmenu.h	qpushbt.h	qradiobt.h	qrangect.h    \
		  qscrbar.h	qtablew.h	qframe.h	qgrpbox.h     \
		  qcombo.h	qmsgbox.h
SOURCES		= qbutton.cpp	qbttngrp.cpp	qchkbox.cpp	qlabel.cpp    \
		  qlcdnum.cpp   \
		  qlined.cpp	qlistbox.cpp	qmenubar.cpp	qmenudta.cpp  \
		  qpopmenu.cpp	qpushbt.cpp	qradiobt.cpp	qrangect.cpp  \
		  qscrbar.cpp	qtablew.cpp	qframe.cpp	qgrpbox.cpp   \
		  qcombo.cpp	qmsgbox.cpp
OBJECTS		= qbutton.o	qbttngrp.o	qchkbox.o	qlabel.o      \
		  qlcdnum.o     \
		  qlined.o	qlistbox.o	qmenubar.o	qmenudta.o    \
		  qpopmenu.o	qpushbt.o	qradiobt.o	qrangect.o    \
		  qscrbar.o	qtablew.o	qframe.o	qgrpbox.o     \
		  qcombo.o	qmsgbox.o
SRCMETA		= mbutton.cpp	mbttngrp.cpp	mchkbox.cpp	mlabel.cpp    \
		  mlcdnum.cpp   \
		  mlined.cpp	mlistbox.cpp	mmenubar.cpp	mpopmenu.cpp  \
		  mpushbt.cpp	mradiobt.cpp	mscrbar.cpp	mtablew.cpp   \
		  mframe.cpp	mgrpbox.cpp     mcombo.cpp	mmsgbox.cpp
OBJMETA		= mbutton.o	mbttngrp.o	mchkbox.o	mlabel.o      \
		  mlcdnum.o     \
		  mlined.o	mlistbox.o	mmenubar.o	mpopmenu.o    \
		  mpushbt.o	mradiobt.o	mscrbar.o	mtablew.o     \
		  mframe.o	mgrpbox.o	mcombo.o	mmsgbox.o
TARGET		= $(LIBDIR)/libqwidgets.a

#######	Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

#######	Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(OBJECTS) $(OBJMETA)

$(TARGET): $(OBJECTS) $(OBJMETA)
	/bin/rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS) $(OBJMETA)
	ranlib $(TARGET)

rcs:: ./RCS/$$@,v

showfiles:
	@echo $(HEADERS) $(SOURCES)

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
	/bin/true # tell make it worked

#######	Meta classes

mbutton.cpp: qbutton.h
	$(QTDIR)/bin/moc qbutton.h -o mbutton.cpp

mbttngrp.cpp: qbttngrp.h
	$(QTDIR)/bin/moc qbttngrp.h -o mbttngrp.cpp

mchkbox.cpp: qchkbox.h
	$(QTDIR)/bin/moc qchkbox.h -o mchkbox.cpp

mcombo.cpp: qcombo.h
	$(QTDIR)/bin/moc qcombo.h -o mcombo.cpp

mframe.cpp: qframe.h
	$(QTDIR)/bin/moc qframe.h -o mframe.cpp

mgrpbox.cpp: qgrpbox.h
	$(QTDIR)/bin/moc qgrpbox.h -o mgrpbox.cpp

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

mmsgbox.cpp: qmsgbox.h
	$(QTDIR)/bin/moc qmsgbox.h -o mmsgbox.cpp

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


# DO NOT DELETE THIS LINE -- make depend depends on it.

qbutton.o: qbutton.h /home/globoid/home/eiriken/qt/include/qwidget.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qobject.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qstring.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qarray.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qshared.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qbutton.o: /usr/include/string.h /usr/include/features.h
qbutton.o: /usr/include/sys/cdefs.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qrect.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qsize.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qfont.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qevent.h qbttngrp.h
qbutton.o: qgrpbox.h qframe.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qregion.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qpen.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qbutton.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qbutton.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qbttngrp.o: qbttngrp.h qgrpbox.h qframe.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qwidget.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qobject.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qstring.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qarray.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qshared.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qbttngrp.o: /usr/include/string.h /usr/include/features.h
qbttngrp.o: /usr/include/sys/cdefs.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qrect.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qsize.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qfont.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qevent.h qbutton.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qlist.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qglist.h
qbttngrp.o: /home/globoid/home/eiriken/qt/include/qcollect.h
qchkbox.o: qchkbox.h qbutton.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qwidget.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qobject.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qstring.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qarray.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qshared.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qchkbox.o: /usr/include/string.h /usr/include/features.h
qchkbox.o: /usr/include/sys/cdefs.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qrect.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qsize.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qfont.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qevent.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qregion.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qpen.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
qchkbox.o: /home/globoid/home/eiriken/qt/include/qpmcache.h
qlabel.o: qlabel.h qframe.h /home/globoid/home/eiriken/qt/include/qwidget.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qobject.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qstring.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qarray.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qshared.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qlabel.o: /usr/include/string.h /usr/include/features.h
qlabel.o: /usr/include/sys/cdefs.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qrect.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qsize.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qfont.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qevent.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qregion.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qpen.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qlabel.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qlabel.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qlcdnum.o: qlcdnum.h qframe.h /home/globoid/home/eiriken/qt/include/qwidget.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qobject.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qstring.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qarray.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qshared.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qlcdnum.o: /usr/include/string.h /usr/include/features.h
qlcdnum.o: /usr/include/sys/cdefs.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qrect.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qsize.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qfont.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qevent.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qbitarry.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qregion.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qpen.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qlcdnum.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qlcdnum.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
qlined.o: qlined.h /home/globoid/home/eiriken/qt/include/qwidget.h
qlined.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qlined.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qlined.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qlined.o: /home/globoid/home/eiriken/qt/include/qobject.h
qlined.o: /home/globoid/home/eiriken/qt/include/qstring.h
qlined.o: /home/globoid/home/eiriken/qt/include/qarray.h
qlined.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qlined.o: /home/globoid/home/eiriken/qt/include/qshared.h
qlined.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qlined.o: /usr/include/string.h /usr/include/features.h
qlined.o: /usr/include/sys/cdefs.h
qlined.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qlined.o: /home/globoid/home/eiriken/qt/include/qrect.h
qlined.o: /home/globoid/home/eiriken/qt/include/qsize.h
qlined.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qlined.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qlined.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qlined.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qlined.o: /home/globoid/home/eiriken/qt/include/qfont.h
qlined.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qlined.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qlined.o: /home/globoid/home/eiriken/qt/include/qevent.h
qlined.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qlined.o: /home/globoid/home/eiriken/qt/include/qregion.h
qlined.o: /home/globoid/home/eiriken/qt/include/qpen.h
qlined.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qlined.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qlined.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qlined.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
qlined.o: /home/globoid/home/eiriken/qt/include/qkeycode.h
qlistbox.o: qlistbox.h qtablew.h qframe.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qwidget.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qobject.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qstring.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qarray.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qshared.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qlistbox.o: /usr/include/string.h /usr/include/features.h
qlistbox.o: /usr/include/sys/cdefs.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qrect.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qsize.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qfont.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qevent.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qregion.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qpen.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qstrlist.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qlist.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qglist.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qcollect.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qdstream.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qiodev.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qkeycode.h
qlistbox.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
qmenubar.o: qmenubar.h qpopmenu.h qtablew.h qframe.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qwidget.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qobject.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qstring.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qarray.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qshared.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qmenubar.o: /usr/include/string.h /usr/include/features.h
qmenubar.o: /usr/include/sys/cdefs.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qrect.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qsize.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qfont.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qevent.h qmenudta.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qsignal.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qlist.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qglist.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qcollect.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qaccel.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qkeycode.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qregion.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qpen.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qmenubar.o: /home/globoid/home/eiriken/qt/include/qapp.h /usr/include/ctype.h
qmenudta.o: qmenudta.h /home/globoid/home/eiriken/qt/include/qglobal.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qstring.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qarray.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qshared.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qmenudta.o: /usr/include/string.h /usr/include/features.h
qmenudta.o: /usr/include/sys/cdefs.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qrect.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qsize.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qsignal.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qobject.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qlist.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qglist.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qcollect.h qpopmenu.h
qmenudta.o: qtablew.h qframe.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qwidget.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qfont.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qevent.h
qmenudta.o: /home/globoid/home/eiriken/qt/include/qapp.h
qpopmenu.o: qpopmenu.h qtablew.h qframe.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qwidget.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qobject.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qstring.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qarray.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qshared.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qpopmenu.o: /usr/include/string.h /usr/include/features.h
qpopmenu.o: /usr/include/sys/cdefs.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qrect.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qsize.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qfont.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qevent.h qmenudta.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qsignal.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qlist.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qglist.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qcollect.h qmenubar.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qaccel.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qkeycode.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qregion.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qpen.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qpopmenu.o: /home/globoid/home/eiriken/qt/include/qpntarry.h qscrbar.h
qpopmenu.o: qrangect.h /home/globoid/home/eiriken/qt/include/qapp.h
qpushbt.o: qpushbt.h qbutton.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qwidget.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qobject.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qstring.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qarray.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qshared.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qpushbt.o: /usr/include/string.h /usr/include/features.h
qpushbt.o: /usr/include/sys/cdefs.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qrect.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qsize.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qfont.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qevent.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qregion.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qpen.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
qpushbt.o: /home/globoid/home/eiriken/qt/include/qpmcache.h
qradiobt.o: qradiobt.h qbutton.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qwidget.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qobject.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qstring.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qarray.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qshared.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qradiobt.o: /usr/include/string.h /usr/include/features.h
qradiobt.o: /usr/include/sys/cdefs.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qrect.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qsize.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qfont.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qevent.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qregion.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qpen.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
qradiobt.o: /home/globoid/home/eiriken/qt/include/qpmcache.h
qrangect.o: qrangect.h
qscrbar.o: qscrbar.h /home/globoid/home/eiriken/qt/include/qwidget.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qobject.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qstring.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qarray.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qshared.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qscrbar.o: /usr/include/string.h /usr/include/features.h
qscrbar.o: /usr/include/sys/cdefs.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qrect.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qsize.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qfont.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qevent.h qrangect.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qregion.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qpen.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qscrbar.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qtablew.o: qtablew.h qframe.h /home/globoid/home/eiriken/qt/include/qwidget.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qobject.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qstring.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qarray.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qshared.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qtablew.o: /usr/include/string.h /usr/include/features.h
qtablew.o: /usr/include/sys/cdefs.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qrect.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qsize.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qfont.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qevent.h qscrbar.h
qtablew.o: qrangect.h /home/globoid/home/eiriken/qt/include/qpainter.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qregion.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qpen.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qtablew.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qtablew.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qframe.o: qframe.h /home/globoid/home/eiriken/qt/include/qwidget.h
qframe.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qframe.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qframe.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qframe.o: /home/globoid/home/eiriken/qt/include/qobject.h
qframe.o: /home/globoid/home/eiriken/qt/include/qstring.h
qframe.o: /home/globoid/home/eiriken/qt/include/qarray.h
qframe.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qframe.o: /home/globoid/home/eiriken/qt/include/qshared.h
qframe.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qframe.o: /usr/include/string.h /usr/include/features.h
qframe.o: /usr/include/sys/cdefs.h
qframe.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qframe.o: /home/globoid/home/eiriken/qt/include/qrect.h
qframe.o: /home/globoid/home/eiriken/qt/include/qsize.h
qframe.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qframe.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qframe.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qframe.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qframe.o: /home/globoid/home/eiriken/qt/include/qfont.h
qframe.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qframe.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qframe.o: /home/globoid/home/eiriken/qt/include/qevent.h
qframe.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qframe.o: /home/globoid/home/eiriken/qt/include/qregion.h
qframe.o: /home/globoid/home/eiriken/qt/include/qpen.h
qframe.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qframe.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qframe.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qgrpbox.o: qgrpbox.h qframe.h /home/globoid/home/eiriken/qt/include/qwidget.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qobject.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qstring.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qarray.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qshared.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qgrpbox.o: /usr/include/string.h /usr/include/features.h
qgrpbox.o: /usr/include/sys/cdefs.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qrect.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qsize.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qfont.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qevent.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qregion.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qpen.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qgrpbox.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qcombo.o: qcombo.h /home/globoid/home/eiriken/qt/include/qwidget.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qglobal.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qobject.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qstring.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qarray.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qgarray.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qshared.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
qcombo.o: /usr/include/string.h /usr/include/features.h
qcombo.o: /usr/include/sys/cdefs.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qrect.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qsize.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qpoint.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qpalette.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qcolor.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qfont.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qcursor.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qevent.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qpainter.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qregion.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qpen.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qbrush.h
qcombo.o: /home/globoid/home/eiriken/qt/include/q2matrix.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qpntarry.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qkeycode.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qstrlist.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qlist.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qglist.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qcollect.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qdstream.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qiodev.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qpixmap.h qpopmenu.h
qcombo.o: qtablew.h qframe.h qmenudta.h
qcombo.o: /home/globoid/home/eiriken/qt/include/qsignal.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qobject.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qstring.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qarray.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qshared.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mbutton.o: /usr/include/string.h /usr/include/features.h
mbutton.o: /usr/include/sys/cdefs.h qbutton.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qrect.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qsize.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qfont.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mbutton.o: /home/globoid/home/eiriken/qt/include/qevent.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qobject.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qstring.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qarray.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qshared.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mbttngrp.o: /usr/include/string.h /usr/include/features.h
mbttngrp.o: /usr/include/sys/cdefs.h qbttngrp.h qgrpbox.h qframe.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qrect.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qsize.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qfont.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mbttngrp.o: /home/globoid/home/eiriken/qt/include/qevent.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qobject.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qstring.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qarray.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qshared.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mchkbox.o: /usr/include/string.h /usr/include/features.h
mchkbox.o: /usr/include/sys/cdefs.h qchkbox.h qbutton.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qrect.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qsize.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qfont.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mchkbox.o: /home/globoid/home/eiriken/qt/include/qevent.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qobject.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qstring.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qarray.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qshared.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mlabel.o: /usr/include/string.h /usr/include/features.h
mlabel.o: /usr/include/sys/cdefs.h qlabel.h qframe.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qrect.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qsize.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qfont.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mlabel.o: /home/globoid/home/eiriken/qt/include/qevent.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qobject.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qstring.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qarray.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qshared.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mlcdnum.o: /usr/include/string.h /usr/include/features.h
mlcdnum.o: /usr/include/sys/cdefs.h qlcdnum.h qframe.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qrect.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qsize.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qfont.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qevent.h
mlcdnum.o: /home/globoid/home/eiriken/qt/include/qbitarry.h
mlined.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mlined.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mlined.o: /home/globoid/home/eiriken/qt/include/qobject.h
mlined.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mlined.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mlined.o: /home/globoid/home/eiriken/qt/include/qstring.h
mlined.o: /home/globoid/home/eiriken/qt/include/qarray.h
mlined.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mlined.o: /home/globoid/home/eiriken/qt/include/qshared.h
mlined.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mlined.o: /usr/include/string.h /usr/include/features.h
mlined.o: /usr/include/sys/cdefs.h qlined.h
mlined.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mlined.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mlined.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mlined.o: /home/globoid/home/eiriken/qt/include/qrect.h
mlined.o: /home/globoid/home/eiriken/qt/include/qsize.h
mlined.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mlined.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mlined.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mlined.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mlined.o: /home/globoid/home/eiriken/qt/include/qfont.h
mlined.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mlined.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mlined.o: /home/globoid/home/eiriken/qt/include/qevent.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qobject.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qstring.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qarray.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qshared.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mlistbox.o: /usr/include/string.h /usr/include/features.h
mlistbox.o: /usr/include/sys/cdefs.h qlistbox.h qtablew.h qframe.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qrect.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qsize.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qfont.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mlistbox.o: /home/globoid/home/eiriken/qt/include/qevent.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qobject.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qstring.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qarray.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qshared.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mmenubar.o: /usr/include/string.h /usr/include/features.h
mmenubar.o: /usr/include/sys/cdefs.h qmenubar.h qpopmenu.h qtablew.h qframe.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qrect.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qsize.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qfont.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qevent.h qmenudta.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qsignal.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qlist.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qglist.h
mmenubar.o: /home/globoid/home/eiriken/qt/include/qcollect.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qobject.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qstring.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qarray.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qshared.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mpopmenu.o: /usr/include/string.h /usr/include/features.h
mpopmenu.o: /usr/include/sys/cdefs.h qpopmenu.h qtablew.h qframe.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qrect.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qsize.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qfont.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qevent.h qmenudta.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qpixmap.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qsignal.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qlist.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qglist.h
mpopmenu.o: /home/globoid/home/eiriken/qt/include/qcollect.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qobject.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qstring.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qarray.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qshared.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mpushbt.o: /usr/include/string.h /usr/include/features.h
mpushbt.o: /usr/include/sys/cdefs.h qpushbt.h qbutton.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qrect.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qsize.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qfont.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mpushbt.o: /home/globoid/home/eiriken/qt/include/qevent.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qobject.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qstring.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qarray.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qshared.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mradiobt.o: /usr/include/string.h /usr/include/features.h
mradiobt.o: /usr/include/sys/cdefs.h qradiobt.h qbutton.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qrect.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qsize.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qfont.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mradiobt.o: /home/globoid/home/eiriken/qt/include/qevent.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qobject.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qstring.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qarray.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qshared.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mscrbar.o: /usr/include/string.h /usr/include/features.h
mscrbar.o: /usr/include/sys/cdefs.h qscrbar.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qrect.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qsize.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qfont.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mscrbar.o: /home/globoid/home/eiriken/qt/include/qevent.h qrangect.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qobject.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qstring.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qarray.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qshared.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mtablew.o: /usr/include/string.h /usr/include/features.h
mtablew.o: /usr/include/sys/cdefs.h qtablew.h qframe.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qrect.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qsize.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qfont.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mtablew.o: /home/globoid/home/eiriken/qt/include/qevent.h
mframe.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mframe.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mframe.o: /home/globoid/home/eiriken/qt/include/qobject.h
mframe.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mframe.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mframe.o: /home/globoid/home/eiriken/qt/include/qstring.h
mframe.o: /home/globoid/home/eiriken/qt/include/qarray.h
mframe.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mframe.o: /home/globoid/home/eiriken/qt/include/qshared.h
mframe.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mframe.o: /usr/include/string.h /usr/include/features.h
mframe.o: /usr/include/sys/cdefs.h qframe.h
mframe.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mframe.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mframe.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mframe.o: /home/globoid/home/eiriken/qt/include/qrect.h
mframe.o: /home/globoid/home/eiriken/qt/include/qsize.h
mframe.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mframe.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mframe.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mframe.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mframe.o: /home/globoid/home/eiriken/qt/include/qfont.h
mframe.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mframe.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mframe.o: /home/globoid/home/eiriken/qt/include/qevent.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qobject.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qstring.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qarray.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qshared.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mgrpbox.o: /usr/include/string.h /usr/include/features.h
mgrpbox.o: /usr/include/sys/cdefs.h qgrpbox.h qframe.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qrect.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qsize.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qfont.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mgrpbox.o: /home/globoid/home/eiriken/qt/include/qevent.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qobject.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qstring.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qarray.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qshared.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mcombo.o: /usr/include/string.h /usr/include/features.h
mcombo.o: /usr/include/sys/cdefs.h qcombo.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qrect.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qsize.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qfont.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mcombo.o: /home/globoid/home/eiriken/qt/include/qevent.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qmetaobj.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qconnect.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qobject.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qobjdefs.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qglobal.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qstring.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qarray.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qgarray.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qshared.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qgeneric.h
mmsgbox.o: /usr/include/string.h /usr/include/features.h
mmsgbox.o: /usr/include/sys/cdefs.h qmsgbox.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qdialog.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qview.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qwidget.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qwindefs.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qpaintd.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qrect.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qsize.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qpoint.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qpalette.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qcolor.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qfontmet.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qfont.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qfontinf.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qcursor.h
mmsgbox.o: /home/globoid/home/eiriken/qt/include/qevent.h
