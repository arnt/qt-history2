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

HEADERS		= qbutton.h	qchkbox.h	qlcdnum.h	qlined.h      \
		  qlistbox.h	qmenubar.h	qmenudta.h	qpopmenu.h    \
		  qpushbt.h	qradiobt.h	qrangect.h	qscrbar.h     \
		  qtablew.h
SOURCES		= qbutton.cpp	qchkbox.cpp	qlcdnum.cpp	qlined.cpp    \
		  qlistbox.cpp	qmenubar.cpp	qmenudta.cpp	qpopmenu.cpp  \
		  qpushbt.cpp	qradiobt.cpp	qrangect.cpp	qscrbar.cpp   \
		  qtablew.cpp
OBJECTS		= qbutton.o	qchkbox.o	qlcdnum.o	qlined.o      \
		  qlistbox.o	qmenubar.o	qmenudta.o	qpopmenu.o    \
		  qpushbt.o	qradiobt.o	qrangect.o	qscrbar.o     \
		  qtablew.o
SRCMETA		= mbutton.cpp	mchkbox.cpp	mlcdnum.cpp	mlined.cpp    \
		  mlistbox.cpp	mmenubar.cpp	mpopmenu.cpp	mpushbt.cpp   \
		  mradiobt.cpp	mscrbar.cpp	mtablew.cpp
OBJMETA		= mbutton.o	mchkbox.o	mlcdnum.o	mlined.o      \
		  mlistbox.o	mmenubar.o	mpopmenu.o	mpushbt.o     \
		  mradiobt.o	mscrbar.o	mtablew.o
TARGET		= $(LIBDIR)/libqwidgets.a

#######	Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

#######	Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

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

#######	Meta classes

mbutton.cpp: qbutton.h
	$(QTDIR)/bin/moc qbutton.h -o mbutton.cpp

mchkbox.cpp: qchkbox.h
	$(QTDIR)/bin/moc qchkbox.h -o mchkbox.cpp

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


# DO NOT DELETE THIS LINE -- make depend depends on it.

qbutton.o: /home/popcorn/home/hanord/qt/include/qdict.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qpixmap.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qrect.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qsize.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qpoint.h qbutton.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qwidget.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qobject.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qstring.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qarray.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qshared.h
qbutton.o: /usr/include/string.h /usr/include/features.h
qbutton.o: /usr/include/sys/cdefs.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qfont.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qevent.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qregion.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qpen.h
qbutton.o: /home/popcorn/home/hanord/qt/include/qbrush.h
qchkbox.o: qchkbox.h qbutton.h /home/popcorn/home/hanord/qt/include/qwidget.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qobject.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qstring.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qarray.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qshared.h
qchkbox.o: /usr/include/string.h /usr/include/features.h
qchkbox.o: /usr/include/sys/cdefs.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qrect.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qsize.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qfont.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qevent.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qregion.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qpen.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qbrush.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qpntarry.h
qchkbox.o: /home/popcorn/home/hanord/qt/include/qpixmap.h
qlcdnum.o: qlcdnum.h /home/popcorn/home/hanord/qt/include/qwidget.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qobject.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qstring.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qarray.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qshared.h
qlcdnum.o: /usr/include/string.h /usr/include/features.h
qlcdnum.o: /usr/include/sys/cdefs.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qrect.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qsize.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qfont.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qevent.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qbitarry.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qregion.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qpen.h
qlcdnum.o: /home/popcorn/home/hanord/qt/include/qbrush.h /usr/include/stdio.h
qlcdnum.o: /usr/include/libio.h
qlined.o: qlined.h /home/popcorn/home/hanord/qt/include/qwidget.h
qlined.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qlined.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qlined.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qlined.o: /home/popcorn/home/hanord/qt/include/qobject.h
qlined.o: /home/popcorn/home/hanord/qt/include/qstring.h
qlined.o: /home/popcorn/home/hanord/qt/include/qarray.h
qlined.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qlined.o: /home/popcorn/home/hanord/qt/include/qshared.h
qlined.o: /usr/include/string.h /usr/include/features.h
qlined.o: /usr/include/sys/cdefs.h
qlined.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qlined.o: /home/popcorn/home/hanord/qt/include/qrect.h
qlined.o: /home/popcorn/home/hanord/qt/include/qsize.h
qlined.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qlined.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qlined.o: /home/popcorn/home/hanord/qt/include/qfont.h
qlined.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qlined.o: /home/popcorn/home/hanord/qt/include/qevent.h
qlined.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qlined.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qlined.o: /home/popcorn/home/hanord/qt/include/qregion.h
qlined.o: /home/popcorn/home/hanord/qt/include/qpen.h
qlined.o: /home/popcorn/home/hanord/qt/include/qbrush.h
qlined.o: /home/popcorn/home/hanord/qt/include/qpixmap.h
qlined.o: /home/popcorn/home/hanord/qt/include/qkeycode.h
qlistbox.o: qlistbox.h qtablew.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qwidget.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qobject.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qstring.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qarray.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qshared.h
qlistbox.o: /usr/include/string.h /usr/include/features.h
qlistbox.o: /usr/include/sys/cdefs.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qrect.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qsize.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qfont.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qevent.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qstrlist.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qlist.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qglist.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qregion.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qpen.h
qlistbox.o: /home/popcorn/home/hanord/qt/include/qbrush.h
qmenubar.o: qmenubar.h qpopmenu.h qtablew.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qwidget.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qobject.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qstring.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qarray.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qshared.h
qmenubar.o: /usr/include/string.h /usr/include/features.h
qmenubar.o: /usr/include/sys/cdefs.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qrect.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qsize.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qfont.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qevent.h qmenudta.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qregion.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qpen.h
qmenubar.o: /home/popcorn/home/hanord/qt/include/qbrush.h qscrbar.h
qmenubar.o: qrangect.h /home/popcorn/home/hanord/qt/include/qapp.h
qmenudta.o: qmenudta.h /home/popcorn/home/hanord/qt/include/qglobal.h
qmenudta.o: qpopmenu.h qtablew.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qwidget.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qobject.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qstring.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qarray.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qshared.h
qmenudta.o: /usr/include/string.h /usr/include/features.h
qmenudta.o: /usr/include/sys/cdefs.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qrect.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qsize.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qfont.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qmenudta.o: /home/popcorn/home/hanord/qt/include/qevent.h
qpopmenu.o: qpopmenu.h qtablew.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qwidget.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qobject.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qstring.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qarray.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qshared.h
qpopmenu.o: /usr/include/string.h /usr/include/features.h
qpopmenu.o: /usr/include/sys/cdefs.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qrect.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qsize.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qfont.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qevent.h qmenudta.h
qpopmenu.o: qmenubar.h /home/popcorn/home/hanord/qt/include/qpainter.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qregion.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qpen.h
qpopmenu.o: /home/popcorn/home/hanord/qt/include/qbrush.h qscrbar.h
qpopmenu.o: qrangect.h /home/popcorn/home/hanord/qt/include/qapp.h
qpushbt.o: qpushbt.h qbutton.h /home/popcorn/home/hanord/qt/include/qwidget.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qobject.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qstring.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qarray.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qshared.h
qpushbt.o: /usr/include/string.h /usr/include/features.h
qpushbt.o: /usr/include/sys/cdefs.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qrect.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qsize.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qfont.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qevent.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qregion.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qpen.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qbrush.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qpntarry.h
qpushbt.o: /home/popcorn/home/hanord/qt/include/qpixmap.h
qradiobt.o: qradiobt.h qbutton.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qwidget.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qobject.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qstring.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qarray.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qshared.h
qradiobt.o: /usr/include/string.h /usr/include/features.h
qradiobt.o: /usr/include/sys/cdefs.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qrect.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qsize.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qfont.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qevent.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qregion.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qpen.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qbrush.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qpntarry.h
qradiobt.o: /home/popcorn/home/hanord/qt/include/qpixmap.h
qrangect.o: qrangect.h
qscrbar.o: qscrbar.h /home/popcorn/home/hanord/qt/include/qwidget.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qobject.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qstring.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qarray.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qshared.h
qscrbar.o: /usr/include/string.h /usr/include/features.h
qscrbar.o: /usr/include/sys/cdefs.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qrect.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qsize.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qfont.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qevent.h qrangect.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qregion.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qpen.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qbrush.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qpntarry.h
qscrbar.o: /home/popcorn/home/hanord/qt/include/qwmatrix.h
qtablew.o: qtablew.h /home/popcorn/home/hanord/qt/include/qwidget.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qobject.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qstring.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qarray.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qshared.h
qtablew.o: /usr/include/string.h /usr/include/features.h
qtablew.o: /usr/include/sys/cdefs.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qrect.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qsize.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qpoint.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qcolor.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qfont.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qcursor.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qevent.h qscrbar.h qrangect.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qpainter.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qfontmet.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qregion.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qpen.h
qtablew.o: /home/popcorn/home/hanord/qt/include/qbrush.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qobject.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qstring.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qarray.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qshared.h
mbutton.o: /usr/include/string.h /usr/include/features.h
mbutton.o: /usr/include/sys/cdefs.h qbutton.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qrect.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qsize.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qfont.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mbutton.o: /home/popcorn/home/hanord/qt/include/qevent.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qobject.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qstring.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qarray.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qshared.h
mchkbox.o: /usr/include/string.h /usr/include/features.h
mchkbox.o: /usr/include/sys/cdefs.h qchkbox.h qbutton.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qrect.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qsize.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qfont.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mchkbox.o: /home/popcorn/home/hanord/qt/include/qevent.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qobject.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qstring.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qarray.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qshared.h
mlcdnum.o: /usr/include/string.h /usr/include/features.h
mlcdnum.o: /usr/include/sys/cdefs.h qlcdnum.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qrect.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qsize.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qfont.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qevent.h
mlcdnum.o: /home/popcorn/home/hanord/qt/include/qbitarry.h
mlined.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mlined.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mlined.o: /home/popcorn/home/hanord/qt/include/qobject.h
mlined.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mlined.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mlined.o: /home/popcorn/home/hanord/qt/include/qstring.h
mlined.o: /home/popcorn/home/hanord/qt/include/qarray.h
mlined.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mlined.o: /home/popcorn/home/hanord/qt/include/qshared.h
mlined.o: /usr/include/string.h /usr/include/features.h
mlined.o: /usr/include/sys/cdefs.h qlined.h
mlined.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mlined.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mlined.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mlined.o: /home/popcorn/home/hanord/qt/include/qrect.h
mlined.o: /home/popcorn/home/hanord/qt/include/qsize.h
mlined.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mlined.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mlined.o: /home/popcorn/home/hanord/qt/include/qfont.h
mlined.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mlined.o: /home/popcorn/home/hanord/qt/include/qevent.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qobject.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qstring.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qarray.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qshared.h
mlistbox.o: /usr/include/string.h /usr/include/features.h
mlistbox.o: /usr/include/sys/cdefs.h qlistbox.h qtablew.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qrect.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qsize.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qfont.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qevent.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qstrlist.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qlist.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qglist.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qcollect.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qdstream.h
mlistbox.o: /home/popcorn/home/hanord/qt/include/qiodev.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qobject.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qstring.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qarray.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qshared.h
mmenubar.o: /usr/include/string.h /usr/include/features.h
mmenubar.o: /usr/include/sys/cdefs.h qmenubar.h qpopmenu.h qtablew.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qrect.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qsize.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qfont.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mmenubar.o: /home/popcorn/home/hanord/qt/include/qevent.h qmenudta.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qobject.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qstring.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qarray.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qshared.h
mpopmenu.o: /usr/include/string.h /usr/include/features.h
mpopmenu.o: /usr/include/sys/cdefs.h qpopmenu.h qtablew.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qrect.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qsize.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qfont.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mpopmenu.o: /home/popcorn/home/hanord/qt/include/qevent.h qmenudta.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qobject.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qstring.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qarray.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qshared.h
mpushbt.o: /usr/include/string.h /usr/include/features.h
mpushbt.o: /usr/include/sys/cdefs.h qpushbt.h qbutton.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qrect.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qsize.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qfont.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mpushbt.o: /home/popcorn/home/hanord/qt/include/qevent.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qobject.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qstring.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qarray.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qshared.h
mradiobt.o: /usr/include/string.h /usr/include/features.h
mradiobt.o: /usr/include/sys/cdefs.h qradiobt.h qbutton.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qrect.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qsize.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qfont.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mradiobt.o: /home/popcorn/home/hanord/qt/include/qevent.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qobject.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qstring.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qarray.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qshared.h
mscrbar.o: /usr/include/string.h /usr/include/features.h
mscrbar.o: /usr/include/sys/cdefs.h qscrbar.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qrect.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qsize.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qfont.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mscrbar.o: /home/popcorn/home/hanord/qt/include/qevent.h qrangect.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qmetaobj.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qconnect.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qobject.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qobjdefs.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qstring.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qarray.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qshared.h
mtablew.o: /usr/include/string.h /usr/include/features.h
mtablew.o: /usr/include/sys/cdefs.h qtablew.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qwidget.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qwindefs.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qpaintd.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qrect.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qsize.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qpoint.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qcolor.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qfont.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qcursor.h
mtablew.o: /home/popcorn/home/hanord/qt/include/qevent.h
