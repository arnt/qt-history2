#############################################################################
# Qt Makefile - kernel
#############################################################################

####### Directories

SRCDIR		= $(QTDIR)/src
INCDIR		= $(SRCDIR)/include
LIBDIR		= $(SRCDIR)/lib

####### Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

####### Files

HEADERS		= qapp.h	qbitmap.h	qbrush.h	qcolor.h      \
		  qconnect.h	qcursor.h	qevent.h	qfont.h	      \
		  qfontmet.h	qkeycode.h	qmetafil.h	qmetaobj.h    \
		  qobjcoll.h	qobjdefs.h	qobject.h	qpaintd.h     \
		  qpaintdc.h	qpainter.h	qpart.h		qpen.h	      \
		  qpixmap.h	qpntarry.h	qpoint.h	qprinter.h    \
		  qprtdrv.h	qprtstrm.h	qrect.h		qregion.h     \
		  qsize.h	qtimer.h	qview.h		qwidget.h     \
		  qwindefs.h	qwininfo.h	qwxfmat.h
SOURCES		= qapp.cpp	qapp_x11.cpp	qcol_x11.cpp	qcolor.cpp    \
		  qconnect.cpp	qcur_x11.cpp	qcursor.cpp	qfnt_x11.cpp  \
		  qmetafil.cpp	qmetaobj.cpp	qmtf_x11.cpp	qobject.cpp   \
		  qpaintdc.cpp	qpainter.cpp	qpart.cpp	qpdc_x11.cpp  \
		  qpmp_x11.cpp	qpntarry.cpp	qpoint.cpp	qprinter.cpp  \
		  qprtstrm.cpp	qptd_x11.cpp	qptr_x11.cpp	qrect.cpp     \
		  qrgn_x11.cpp	qsize.cpp	qtimer.cpp	qview.cpp     \
		  qwid_x11.cpp	qwidget.cpp	qwininfo.cpp	qwxfmat.cpp
OBJECTS		= qapp.o	qapp_x11.o	qcol_x11.o	qcolor.o      \
		  qconnect.o	qcur_x11.o	qcursor.o	qfnt_x11.o    \
		  qmetafil.o	qmetaobj.o	qmtf_x11.o	qobject.o     \
		  qpaintdc.o	qpainter.o	qpart.o		qpdc_x11.o    \
		  qpmp_x11.o	qpntarry.o	qpoint.o	qprinter.o    \
		  qprtstrm.o	qptd_x11.o	qptr_x11.o	qrect.o	      \
		  qrgn_x11.o	qsize.o		qtimer.o	qview.o	      \
		  qwid_x11.o	qwidget.o	qwininfo.o	qwxfmat.o
SRCMETA		= mtimer.cpp	mview.cpp	mwidget.cpp
OBJMETA		= mtimer.o	mview.o		mwidget.o
TARGET		= $(LIBDIR)/libqkernel.a

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

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

####### Meta classes

mtimer.cpp: qtimer.h
	$(QTDIR)/bin/moc qtimer.h -o mtimer.cpp

mview.cpp: qview.h
	$(QTDIR)/bin/moc qview.h -o mview.cpp

mwidget.cpp: qwidget.h
	$(QTDIR)/bin/moc qwidget.h -o mwidget.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.

qapp.o: qapp.h qwidget.h qwindefs.h qobjdefs.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobject.h
qapp.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qapp.o: /usr/include/string.h /usr/include/features.h
qapp.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qapp.o: qcursor.h qevent.h qobjcoll.h qmetaobj.h qconnect.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qapp_x11.o: qapp.h qwidget.h qwindefs.h qobjdefs.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobject.h
qapp_x11.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qapp_x11.o: /usr/include/string.h /usr/include/features.h
qapp_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qapp_x11.o: qcursor.h qevent.h qwininfo.h /usr/include/stdlib.h
qapp_x11.o: /usr/include/errno.h /usr/include/linux/errno.h
qapp_x11.o: /usr/local/lib/gcc-include/float.h /usr/include/signal.h
qapp_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qapp_x11.o: /usr/include/linux/signal.h /usr/include/X11/Xlib.h
qapp_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qapp_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qapp_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qapp_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qapp_x11.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
qapp_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qapp_x11.o: /usr/include/sys/time.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qstack.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qapp_x11.o: /usr/include/X11/keysymdef.h qkeycode.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qintdict.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qcol_x11.o: qcolor.h qwindefs.h qobjdefs.h
qcol_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qwininfo.h
qcol_x11.o: qsize.h qpoint.h /usr/include/X11/Xlib.h /usr/include/sys/types.h
qcol_x11.o: /usr/include/linux/types.h /usr/include/X11/X.h
qcol_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qcol_x11.o: /usr/local/lib/gcc-include/stddef.h /usr/include/X11/Xutil.h
qcol_x11.o: /usr/include/X11/Xos.h /usr/include/string.h
qcol_x11.o: /usr/include/features.h /usr/include/sys/cdefs.h
qcol_x11.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h
qcol_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qcol_x11.o: /usr/include/gnu/types.h /usr/include/sys/time.h
qcol_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qcol_x11.o: /home/popcorn/home/hanord/qt/src/include/qintdict.h
qcol_x11.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qcol_x11.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qcol_x11.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qcol_x11.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qcolor.o: qcolor.h qwindefs.h qobjdefs.h
qcolor.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qcolor.o: /home/popcorn/home/hanord/qt/src/include/qdstream.h
qcolor.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
qconnect.o: qconnect.h qobject.h qobjdefs.h
qconnect.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qcur_x11.o: qcursor.h qpoint.h qwindefs.h qobjdefs.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qapp.h
qcur_x11.o: qwidget.h qobject.h qpaintd.h qrect.h qsize.h qcolor.h qfont.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qcur_x11.o: /usr/include/string.h /usr/include/features.h
qcur_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qcur_x11.o: qevent.h qbitmap.h qpixmap.h /usr/include/X11/Xlib.h
qcur_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qcur_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qcur_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qcur_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qcur_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qcur_x11.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
qcur_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qcur_x11.o: /usr/include/sys/time.h /usr/include/X11/cursorfont.h
qcursor.o: qcursor.h qpoint.h qwindefs.h qobjdefs.h
qcursor.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qcursor.o: /home/popcorn/home/hanord/qt/src/include/qdstream.h
qcursor.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
qfnt_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qfnt_x11.o: /usr/include/linux/types.h /usr/include/X11/X.h
qfnt_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qfnt_x11.o: /usr/local/lib/gcc-include/stddef.h /usr/include/X11/Xutil.h
qfnt_x11.o: /usr/include/X11/Xos.h /usr/include/string.h
qfnt_x11.o: /usr/include/features.h /usr/include/sys/cdefs.h
qfnt_x11.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h
qfnt_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qfnt_x11.o: /usr/include/gnu/types.h /usr/include/sys/time.h
qfnt_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h qpainter.h
qfnt_x11.o: qpaintd.h qwindefs.h qobjdefs.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qfnt_x11.o: qsize.h qpoint.h qcolor.h qfontmet.h qfont.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h qregion.h
qfnt_x11.o: qpen.h qbrush.h /home/popcorn/home/hanord/qt/src/include/qdict.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qmetafil.o: qmetafil.h qpaintd.h qwindefs.h qobjdefs.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qmetafil.o: qsize.h qpoint.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qbuffer.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qmetafil.o: /usr/include/string.h /usr/include/features.h
qmetafil.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qmetafil.o: qpainter.h qcolor.h qfontmet.h qfont.h qregion.h qpen.h qbrush.h
qmetafil.o: qpntarry.h qwxfmat.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qfile.h
qmetafil.o: /usr/include/stdio.h /usr/include/libio.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qdstream.h
qmetaobj.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobjcoll.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qstrlist.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qmetaobj.o: /usr/include/string.h /usr/include/features.h
qmetaobj.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qdstream.h
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
qmtf_x11.o: qmetafil.h qpaintd.h qwindefs.h qobjdefs.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qmtf_x11.o: qsize.h qpoint.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qbuffer.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qmtf_x11.o: /usr/include/string.h /usr/include/features.h
qmtf_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qmtf_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qmtf_x11.o: /usr/include/linux/types.h /usr/include/X11/X.h
qmtf_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qmtf_x11.o: /usr/include/X11/Xutil.h /usr/include/X11/Xos.h
qmtf_x11.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h
qmtf_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qmtf_x11.o: /usr/include/gnu/types.h /usr/include/sys/time.h
qmtf_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qobject.o: qobject.h qobjdefs.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobjcoll.h
qobject.o: qmetaobj.h qconnect.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h qpart.h qview.h
qobject.o: qwidget.h qwindefs.h qpaintd.h qrect.h qsize.h qpoint.h qcolor.h
qobject.o: qfont.h /home/popcorn/home/hanord/qt/src/include/qstring.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qobject.o: /usr/include/string.h /usr/include/features.h
qobject.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qobject.o: qcursor.h qevent.h
qpainter.o: qpainter.h qpaintd.h qwindefs.h qobjdefs.h
qpainter.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qpainter.o: qsize.h qpoint.h qcolor.h qfontmet.h qfont.h
qpainter.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qpainter.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qpainter.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qpainter.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qpainter.o: /usr/include/string.h /usr/include/features.h
qpainter.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qpainter.o: qregion.h qpen.h qbrush.h qpntarry.h
qpart.o: qpart.h qobject.h qobjdefs.h
qpart.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobjcoll.h
qpart.o: qmetaobj.h qconnect.h
qpart.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qpart.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qpart.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qpart.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qpart.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qpart.o: /usr/include/ctype.h /usr/include/features.h
qpart.o: /usr/include/sys/cdefs.h
qpmp_x11.o: qbitmap.h qpixmap.h qpaintd.h qwindefs.h qobjdefs.h
qpmp_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qpmp_x11.o: qsize.h qpoint.h /usr/include/X11/Xlib.h /usr/include/sys/types.h
qpmp_x11.o: /usr/include/linux/types.h /usr/include/X11/X.h
qpmp_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qpmp_x11.o: /usr/local/lib/gcc-include/stddef.h /usr/include/X11/Xutil.h
qpmp_x11.o: /usr/include/X11/Xos.h /usr/include/string.h
qpmp_x11.o: /usr/include/features.h /usr/include/sys/cdefs.h
qpmp_x11.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h
qpmp_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qpmp_x11.o: /usr/include/gnu/types.h /usr/include/sys/time.h
qpmp_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h qwxfmat.h
qpntarry.o: qpntarry.h /home/popcorn/home/hanord/qt/src/include/qarray.h
qpntarry.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qpntarry.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qpntarry.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qpoint.h
qpntarry.o: qwindefs.h qobjdefs.h qrect.h qsize.h
qpntarry.o: /home/popcorn/home/hanord/qt/src/include/qdstream.h
qpntarry.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
qpoint.o: qpoint.h qwindefs.h qobjdefs.h
qpoint.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qpoint.o: /home/popcorn/home/hanord/qt/src/include/qdstream.h
qpoint.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
qprinter.o: qprinter.h qpaintd.h qwindefs.h qobjdefs.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qprinter.o: qsize.h qpoint.h qprtdrv.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h qpainter.h
qprinter.o: qcolor.h qfontmet.h qfont.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qprinter.o: /usr/include/string.h /usr/include/features.h
qprinter.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qprinter.o: qregion.h qpen.h qbrush.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qtstream.h
qprinter.o: /usr/include/stdio.h /usr/include/libio.h
qprinter.o: /home/popcorn/home/hanord/qt/src/include/qfile.h qpntarry.h
qprtstrm.o: qprtstrm.h qprinter.h qpaintd.h qwindefs.h qobjdefs.h
qprtstrm.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qprtstrm.o: qsize.h qpoint.h
qptd_x11.o: qpaintd.h qwindefs.h qobjdefs.h
qptd_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qptd_x11.o: qsize.h qpoint.h qwidget.h qobject.h qcolor.h qfont.h
qptd_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qptd_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qptd_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qptd_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qptd_x11.o: /usr/include/string.h /usr/include/features.h
qptd_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qptd_x11.o: qcursor.h qevent.h qpixmap.h /usr/include/X11/Xlib.h
qptd_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qptd_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qptd_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qptd_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qptd_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qptd_x11.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
qptd_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qptd_x11.o: /usr/include/sys/time.h
qptr_x11.o: qpainter.h qpaintd.h qwindefs.h qobjdefs.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qptr_x11.o: qsize.h qpoint.h qcolor.h qfontmet.h qfont.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qptr_x11.o: /usr/include/string.h /usr/include/features.h
qptr_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qptr_x11.o: qregion.h qpen.h qbrush.h qwidget.h qobject.h qcursor.h qevent.h
qptr_x11.o: qpntarry.h qwxfmat.h qbitmap.h qpixmap.h /usr/include/X11/Xlib.h
qptr_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qptr_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qptr_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qptr_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qptr_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qptr_x11.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
qptr_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qptr_x11.o: /usr/include/sys/time.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qrect.o: qrect.h qsize.h qpoint.h qwindefs.h qobjdefs.h
qrect.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qrect.o: /home/popcorn/home/hanord/qt/src/include/qdstream.h
qrect.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
qrgn_x11.o: qregion.h /home/popcorn/home/hanord/qt/src/include/qshared.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qrect.h
qrgn_x11.o: qsize.h qpoint.h qwindefs.h qobjdefs.h qpntarry.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qrgn_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qrgn_x11.o: /usr/include/linux/types.h /usr/include/X11/X.h
qrgn_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qrgn_x11.o: /usr/local/lib/gcc-include/stddef.h /usr/include/X11/Xutil.h
qrgn_x11.o: /usr/include/X11/Xos.h /usr/include/string.h
qrgn_x11.o: /usr/include/features.h /usr/include/sys/cdefs.h
qrgn_x11.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h
qrgn_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qrgn_x11.o: /usr/include/gnu/types.h /usr/include/sys/time.h
qrgn_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qsize.o: qsize.h qpoint.h qwindefs.h qobjdefs.h
qsize.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qsize.o: /home/popcorn/home/hanord/qt/src/include/qdstream.h
qsize.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
qtimer.o: qtimer.h qobject.h qobjdefs.h
qtimer.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qevent.h
qtimer.o: qwindefs.h qrect.h qsize.h qpoint.h
qview.o: qview.h qwidget.h qwindefs.h qobjdefs.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobject.h
qview.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qview.o: /usr/include/string.h /usr/include/features.h
qview.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qview.o: qcursor.h qevent.h qobjcoll.h qmetaobj.h qconnect.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qview.o: /usr/include/ctype.h
qwid_x11.o: qview.h qwidget.h qwindefs.h qobjdefs.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobject.h
qwid_x11.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qwid_x11.o: /usr/include/string.h /usr/include/features.h
qwid_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qwid_x11.o: qcursor.h qevent.h qapp.h qwininfo.h /usr/include/X11/Xlib.h
qwid_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qwid_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qwid_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qwid_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qwid_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qwid_x11.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
qwid_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qwid_x11.o: /usr/include/sys/time.h
qwidget.o: qview.h qwidget.h qwindefs.h qobjdefs.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobject.h
qwidget.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qwidget.o: /usr/include/string.h /usr/include/features.h
qwidget.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qwidget.o: qcursor.h qevent.h qapp.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qintdict.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qwininfo.o: qwininfo.h qsize.h qpoint.h qwindefs.h qobjdefs.h
qwininfo.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qwxfmat.o: qwxfmat.h qwindefs.h qobjdefs.h
qwxfmat.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qwxfmat.o: /home/popcorn/home/hanord/qt/src/include/qdstream.h
qwxfmat.o: /home/popcorn/home/hanord/qt/src/include/qiodev.h
mtimer.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mtimer.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qtimer.h
mview.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mview.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qview.h qwidget.h
mview.o: qwindefs.h qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
mview.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mview.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mview.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mview.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mview.o: /usr/include/string.h /usr/include/features.h
mview.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mview.o: qcursor.h qevent.h
mwidget.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mwidget.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qwidget.h
mwidget.o: qwindefs.h qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
mwidget.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
mwidget.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
mwidget.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
mwidget.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
mwidget.o: /usr/include/string.h /usr/include/features.h
mwidget.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
mwidget.o: qcursor.h qevent.h
