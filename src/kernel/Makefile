#############################################################################
# Qt Makefile - kernel
#############################################################################

####### Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

####### Files

HEADERS		= qapp.h	qbitmap.h	qbrush.h	qcolor.h      \
		  qconnect.h	qcursor.h	qevent.h	qfont.h	      \
		  qfontdta.h	qfontinf.h	qfontmet.h		      \
		  qkeycode.h	qmetaobj.h	qobjcoll.h	qobjdefs.h    \
		  qobject.h	qpaintd.h	qpaintdc.h	qpainter.h    \
		  qpdevmet.h	qpen.h		qpicture.h	qpixmap.h     \
		  qpntarry.h	qpoint.h	qprinter.h	     \
		  	qrect.h		qregion.h	qsignal.h     \
		  qsize.h	qtimer.h	qwindow.h		qwidcoll.h    \
		  qwidget.h	qwindefs.h	q2matrix.h	qpalette.h    \
		  qaccel.h	qdialog.h	qimage.h	qpmcache.h
SOURCES		= qapp.cpp	qapp_x11.cpp	qbitmap.cpp	qcol_x11.cpp  \
		  qcolor.cpp	qconnect.cpp	qcur_x11.cpp	qcursor.cpp   \
		  qevent.cpp	qfnt_x11.cpp	qfont.cpp		      \
		  qmetaobj.cpp	qobject.cpp	qpainter.cpp	qpdevmet.cpp  \
		  qpic_x11.cpp	qpicture.cpp	qpixmap.cpp	qpm_x11.cpp  \
		  qpntarry.cpp	qpoint.cpp	qprinter.cpp	\
		  qptd_x11.cpp	qptr_x11.cpp	qrect.cpp	qrgn_x11.cpp  \
		  qsignal.cpp	qsize.cpp	qtimer.cpp	qwindow.cpp     \
		  qwid_x11.cpp	qwidget.cpp	q2matrix.cpp	qpalette.cpp  \
		  qaccel.cpp	qdialog.cpp	qimage.cpp	qpmcache.cpp
OBJECTS		= qapp.o	qapp_x11.o	qbitmap.o	qcol_x11.o    \
		  qcolor.o	qconnect.o	qcur_x11.o	qcursor.o     \
		  qevent.o	qfnt_x11.o	qfont.o			      \
		  qmetaobj.o	qobject.o	qpainter.o	qpdevmet.o    \
		  qpic_x11.o	qpicture.o	qpixmap.o	qpm_x11.o    \
		  qpntarry.o	qpoint.o	qprinter.o	    \
		  qptd_x11.o	qptr_x11.o	qrect.o		qrgn_x11.o    \
		  qsignal.o	qsize.o		qtimer.o	qwindow.o	      \
		  qwid_x11.o	qwidget.o	q2matrix.o	qpalette.o    \
		  qaccel.o	qdialog.o	qimage.o	qpmcache.o
SRCMETA		= maccel.cpp	mdialog.cpp	mtimer.cpp	mwindow.cpp     \
		  mwidget.cpp
OBJMETA		= maccel.o	mdialog.o	mtimer.o	mwindow.o	      \
		  mwidget.o
TARGET		= $(LIBDIR)/libqkernel.a

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(HEADERS) $(SOURCES) $(OBJECTS) $(OBJMETA) $(SRCMETA)

$(TARGET): $(OBJECTS) $(OBJMETA)
	/bin/rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS) $(OBJMETA)
	ranlib $(TARGET)

rcs:: ./RCS/$$@,v

showfiles:
	@echo $(HEADERS) $(SOURCES)

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/kernel/-'` $(INCDIR)

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

####### Meta classes

maccel.cpp: qaccel.h
	$(QTDIR)/bin/moc qaccel.h -o maccel.cpp

mdialog.cpp: qdialog.h
	$(QTDIR)/bin/moc qdialog.h -o mdialog.cpp

mtimer.cpp: qtimer.h
	$(QTDIR)/bin/moc qtimer.h -o mtimer.cpp

mwindow.cpp: qwindow.h
	$(QTDIR)/bin/moc qwindow.h -o mwindow.cpp

mwidget.cpp: qwidget.h
	$(QTDIR)/bin/moc qwidget.h -o mwidget.cpp


# DO NOT DELETE
