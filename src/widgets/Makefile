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
		  qpushbt.h     qradiobt.h
SOURCES		= qbutton.cpp	qchkbox.cpp	qclock.cpp	qlcdnum.cpp   \
		  qpushbt.cpp   qradiobt.cpp
OBJECTS		= qbutton.o	qchkbox.o	qclock.o	qlcdnum.o     \
		  qpushbt.o     qradiobt.o
SRCMETA		= mbutton.cpp	mchkbox.cpp	mclock.cpp	mlcdnum.cpp   \
		  mpushbt.cpp   mradiobt.cpp
OBJMETA		= mbutton.o	mchkbox.o	mclock.o	mlcdnum.o     \
		  mpushbt.o	mradiobt.o
TARGET		= $(LIBDIR)/libqwidgets.a

#######	Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

#######	Build rules

all: $(HEADERS) $(SOURCES) $(SRCMETA) $(TARGET)

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

depend:
	makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *BAK *~ *% #*
	/bin/rm -f $(SRCMETA)

#######	Meta classes

$(OBJMETA): $(SRCMETA)

mbutton.cpp: qbutton.h
	$(QTDIR)/bin/moc qbutton.h -o mbutton.cpp

mchkbox.cpp: qchkbox.h
	$(QTDIR)/bin/moc qchkbox.h -o mchkbox.cpp

mclock.cpp: qclock.h
	$(QTDIR)/bin/moc qclock.h -o mclock.cpp

mlcdnum.cpp: qlcdnum.h
	$(QTDIR)/bin/moc qlcdnum.h -o mlcdnum.cpp

mpushbt.cpp: qpushbt.h
	$(QTDIR)/bin/moc qpushbt.h -o mpushbt.cpp

mradiobt.cpp: qradiobt.h
	$(QTDIR)/bin/moc qradiobt.h -o mradiobt.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.

qbutton.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qpixmap.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qpaintd.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qwindefs.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qobjdefs.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qsize.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qpoint.h qbutton.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qwidget.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qobject.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qrect.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qcolor.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qfont.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qbutton.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qbutton.o: /usr/include/string.h /usr/include/features.h
qbutton.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
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
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qchkbox.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qchkbox.o: /usr/include/string.h /usr/include/features.h
qchkbox.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
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
qclock.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qclock.o: /usr/include/string.h /usr/include/features.h
qclock.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qtimer.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qdatetm.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qclock.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qclock.o: /usr/include/stdio.h /usr/include/libio.h
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
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qlcdnum.o: /usr/include/string.h /usr/include/features.h
qlcdnum.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qbitarry.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qlcdnum.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qlcdnum.o: /usr/include/stdio.h /usr/include/libio.h
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
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qpushbt.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qpushbt.o: /usr/include/string.h /usr/include/features.h
qpushbt.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
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
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qcursor.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qradiobt.o: /usr/include/string.h /usr/include/features.h
qradiobt.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qevent.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpainter.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qfontmet.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qregion.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpen.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qbrush.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpntarry.h
qradiobt.o: /home/popcorn/home/hanord/qt/src/include/qpixmap.h
