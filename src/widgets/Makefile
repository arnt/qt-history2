#############################################################################
# Qt Makefile - widgets
#############################################################################

#######	Directories

SRCDIR		= $(QTDIR)/src
INCDIR		= $(SRCDIR)/include
LIBDIR		= $(SRCDIR)/lib

#######	Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

#######	Files

HEADERS		= qbutton.h	qchkbox.h	qclock.h	qlcdnum.h     \
		  qlined.h	qpushbt.h	qradiobt.h	qrangect.h    \
		  qscrbar.h
SOURCES		= qbutton.cpp	qchkbox.cpp	qclock.cpp	qlcdnum.cpp   \
		  qlined.cpp	qpushbt.cpp	qradiobt.cpp	qrangect.cpp  \
		  qscrbar.cpp
OBJECTS		= qbutton.o	qchkbox.o	qclock.o	qlcdnum.o     \
		  qpushbt.o	qradiobt.o	qlined.o	qscrbar.o     \
		  qrangect.o
SRCMETA		= mbutton.cpp	mchkbox.cpp	mclock.cpp	mlcdnum.cpp   \
		  mpushbt.cpp	mradiobt.cpp	mlined.cpp	mscrbar.cpp
OBJMETA		= mbutton.o	mchkbox.o	mclock.o	mlcdnum.o     \
		  mpushbt.o	mradiobt.o	mlined.o	mscrbar.o
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

mclock.cpp: qclock.h
	$(QTDIR)/bin/moc qclock.h -o mclock.cpp

mlcdnum.cpp: qlcdnum.h
	$(QTDIR)/bin/moc qlcdnum.h -o mlcdnum.cpp

mlined.cpp: qlined.h
	$(QTDIR)/bin/moc qlined.h -o mlined.cpp

mpushbt.cpp: qpushbt.h
	$(QTDIR)/bin/moc qpushbt.h -o mpushbt.cpp

mradiobt.cpp: qradiobt.h
	$(QTDIR)/bin/moc qradiobt.h -o mradiobt.cpp

mscrbar.cpp: qscrbar.h
	$(QTDIR)/bin/moc qscrbar.h -o mscrbar.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.

qbutton.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qpixmap.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h qbutton.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qbutton.o: /usr/include/string.h /usr/include/features.h
qbutton.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qchkbox.o: qchkbox.h qbutton.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qchkbox.o: /usr/include/string.h /usr/include/features.h
qchkbox.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qpntarry.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qpixmap.h
qclock.o: qclock.h /home/popcorn/home/hanord/qt/src/include/qview.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qclock.o: /usr/include/string.h /usr/include/features.h
qclock.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qtimer.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qdatetm.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qlcdnum.o: qlcdnum.h /home/popcorn/home/hanord/qt/src/include/qwidget.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qlcdnum.o: /usr/include/string.h /usr/include/features.h
qlcdnum.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qbitarry.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qlcdnum.o: /usr/include/stdio.h /usr/include/libio.h
qlined.o: qlined.h /home/popcorn/home/hanord/qt/src/include/qwidget.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qlined.o: /usr/include/string.h /usr/include/features.h
qlined.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qpixmap.h
qlined.o: /home/popcorn/home/hanord/qt/src/include/qkeycode.h
qpushbt.o: qpushbt.h qbutton.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qpushbt.o: /usr/include/string.h /usr/include/features.h
qpushbt.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qpntarry.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qpixmap.h
qradiobt.o: qradiobt.h qbutton.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qradiobt.o: /usr/include/string.h /usr/include/features.h
qradiobt.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpntarry.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpixmap.h
qrangect.o: qrangect.h
qscrbar.o: qscrbar.h /home/popcorn/home/hanord/qt/src/include/qwidget.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qscrbar.o: /usr/include/string.h /usr/include/features.h
qscrbar.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qevent.h qrangect.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qpntarry.h
qscrbar.o: /home/popcorn/home/hanord/qt/src/include/qwxfmat.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qmetaobj.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qconnect.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qbutton.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mbutton.o: /usr/include/string.h /usr/include/features.h
mbutton.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
mbutton.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qmetaobj.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qconnect.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qchkbox.h
mchkbox.o: qbutton.h /home/popcorn/home/hanord/qt/src/include/qwidget.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mchkbox.o: /usr/include/string.h /usr/include/features.h
mchkbox.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
mchkbox.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qmetaobj.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qconnect.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qclock.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qview.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mclock.o: /usr/include/string.h /usr/include/features.h
mclock.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
mclock.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qmetaobj.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qconnect.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qlcdnum.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mlcdnum.o: /usr/include/string.h /usr/include/features.h
mlcdnum.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
mlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qbitarry.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qmetaobj.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qconnect.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qpushbt.h
mpushbt.o: qbutton.h /home/popcorn/home/hanord/qt/src/include/qwidget.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mpushbt.o: /usr/include/string.h /usr/include/features.h
mpushbt.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
mpushbt.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qmetaobj.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qconnect.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qradiobt.h
mradiobt.o: qbutton.h /home/popcorn/home/hanord/qt/src/include/qwidget.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mradiobt.o: /usr/include/string.h /usr/include/features.h
mradiobt.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
mradiobt.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qmetaobj.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qconnect.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qlined.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mlined.o: /usr/include/string.h /usr/include/features.h
mlined.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
mlined.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qmetaobj.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qconnect.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qscrbar.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mscrbar.o: /usr/include/string.h /usr/include/features.h
mscrbar.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
mscrbar.o: /home/popcorn/home/hanord/qt/src/include/qevent.h qrangect.h
