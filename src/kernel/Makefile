#############################################################################
# Qt Makefile - kernel
#############################################################################

####### Directories

BASEDIR =	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
CC	=	gcc

####### Files

HEADERS =	qaccel.h	qapp.h		qbitmap.h	qbrush.h      \
		qclipbrd.h	qcolor.h	qconnect.h	qcursor.h     \
		qdialog.h	qdrawutl.h	qevent.h	qfont.h	      \
		qfontdta.h	qfontinf.h	qfontmet.h	qimage.h      \
		qkeycode.h	qmetaobj.h	qobjcoll.h	qobjdefs.h    \
		qobject.h	qpaintd.h	qpaintdc.h	qpainter.h    \
		qpalette.h	qpdevmet.h	qpen.h		qpicture.h    \
		qpixmap.h	qpmcache.h	qpntarry.h	qpoint.h      \
		qprinter.h	qpsprn.h	qrect.h		qregion.h     \
		qsignal.h	qsize.h		qsocknot.h	qtimer.h      \
		qwidcoll.h	qwidget.h	qwindefs.h	qwindow.h     \
		qwmatrix.h
SOURCES =	qaccel.cpp	qapp.cpp	qapp_x11.cpp	qbitmap.cpp   \
		qclb_x11.cpp	qclipbrd.cpp	qcol_x11.cpp	qcolor.cpp    \
		qconnect.cpp	qcur_x11.cpp	qcursor.cpp	qdialog.cpp   \
		qdrawutl.cpp	qevent.cpp	qfnt_x11.cpp	qfont.cpp     \
		qimage.cpp	qmetaobj.cpp	qobject.cpp	qpainter.cpp  \
		qpalette.cpp	qpdevmet.cpp	qpic_x11.cpp	qpicture.cpp  \
		qpixmap.cpp	qpm_x11.cpp	qpmcache.cpp	qpntarry.cpp  \
		qpoint.cpp	qprinter.cpp	qprn_x11.cpp	qpsprn.cpp    \
		qptd_x11.cpp	qptr_x11.cpp	qrect.cpp	qregion.cpp   \
		qrgn_x11.cpp	qsignal.cpp	qsize.cpp	qsocknot.cpp  \
		qtimer.cpp	qwid_x11.cpp	qwidget.cpp	qwindow.cpp   \
		qwmatrix.cpp
OBJECTS =	qaccel.o	qapp.o		qapp_x11.o	qbitmap.o     \
		qclb_x11.o	qclipbrd.o	qcol_x11.o	qcolor.o      \
		qconnect.o	qcur_x11.o	qcursor.o	qdialog.o     \
		qdrawutl.o	qevent.o	qfnt_x11.o	qfont.o	      \
		qimage.o	qmetaobj.o	qobject.o	qpainter.o    \
		qpalette.o	qpdevmet.o	qpic_x11.o	qpicture.o    \
		qpixmap.o	qpm_x11.o	qpmcache.o	qpntarry.o    \
		qpoint.o	qprinter.o	qprn_x11.o	qpsprn.o      \
		qptd_x11.o	qptr_x11.o	qrect.o		qregion.o     \
		qrgn_x11.o	qsignal.o	qsize.o		qsocknot.o    \
		qtimer.o	qwid_x11.o	qwidget.o	qwindow.o     \
		qwmatrix.o
SRCMETA =	maccel.cpp	mapp.cpp	mclipbrd.cpp	mdialog.cpp   \
		mtimer.cpp	mwidget.cpp	mwindow.cpp	msocknot.cpp
OBJMETA =	maccel.o	mapp.o		mclipbrd.o	mdialog.o     \
		mtimer.o	mwidget.o	mwindow.o	msocknot.o
TARGET	=	$(LIBDIR)/libqkernel.a

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(HEADERS) $(SOURCES) $(OBJECTS) $(OBJMETA)

$(TARGET): $(OBJECTS) $(OBJMETA)
	rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS) $(OBJMETA)
	ranlib $(TARGET)

showfiles:
	@echo $(HEADERS) $(SOURCES) Makefile

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/kernel/-'` $(INCDIR)

showheaders:
	@echo $(HEADERS)

showsources:
	@echo $(SOURCES)

depend: $(SRCMETA)
	makedepend -I$(INCDIR) $(SOURCES) $(SRCMETA) 2> /dev/null

clean:
	rm -f *.o *.bak *BAK *~ *% #*
	rm -f $(SRCMETA)

####### Meta classes

maccel.cpp: qaccel.h
	$(QTDIR)/bin/moc qaccel.h -o maccel.cpp

mapp.cpp: qapp.h
	$(QTDIR)/bin/moc qapp.h -o mapp.cpp

mclipbrd.cpp: qclipbrd.h
	$(QTDIR)/bin/moc qclipbrd.h -o mclipbrd.cpp

mdialog.cpp: qdialog.h
	$(QTDIR)/bin/moc qdialog.h -o mdialog.cpp

msocknot.cpp: qsocknot.h
	$(QTDIR)/bin/moc qsocknot.h -o msocknot.cpp

mtimer.cpp: qtimer.h
	$(QTDIR)/bin/moc qtimer.h -o mtimer.cpp

mwidget.cpp: qwidget.h
	$(QTDIR)/bin/moc qwidget.h -o mwidget.cpp

mwindow.cpp: qwindow.h
	$(QTDIR)/bin/moc qwindow.h -o mwindow.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.
