#############################################################################
# Qt Makefile - kernel
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

HEADERS		= qapp.h	qbrush.h	qcolor.h	qconnect.h    \
		  qcursor.h	qevent.h	qfont.h		qfontmet.h    \
		  qkeycode.h	qmetafil.h	qmetaobj.h	qobjcoll.h    \
		  qobjdefs.h	qobject.h	qpaintd.h	qpainter.h    \
		  qpart.h	qpen.h		qpixmap.h	qpntarry.h    \
		  qpoint.h	qrect.h		qregion.h	qsize.h	      \
		  qtimer.h	qview.h		qwidget.h	qwindefs.h    \
		  qwininfo.h
SOURCES		= qapp.cpp	qapp_x11.cpp	qcol_x11.cpp	qcolor.cpp    \
		  qconnect.cpp	qcur_x11.cpp	qcursor.cpp	qfnt_x11.cpp  \
		  qmetafil.cpp	qmetaobj.cpp	qmtf_x11.cpp	qobject.cpp   \
		  qpainter.cpp	qpart.cpp	qpmp_x11.cpp	qpntarry.cpp  \
		  qpoint.cpp	qptd_x11.cpp	qptr_x11.cpp	qrect.cpp     \
		  qrgn_x11.cpp	qsize.cpp	qtimer.cpp	qview.cpp     \
		  qwid_x11.cpp	qwidget.cpp	qwininfo.cpp
OBJECTS		= qapp.o	qapp_x11.o	qcol_x11.o	qcolor.o      \
		  qconnect.o	qcur_x11.o	qcursor.o	qfnt_x11.o    \
		  qmetafil.o	qmetaobj.o	qmtf_x11.o	qobject.o     \
		  qpainter.o	qpart.o		qpmp_x11.o	qpntarry.o    \
		  qpoint.o	qptd_x11.o	qptr_x11.o	qrect.o	      \
		  qrgn_x11.o	qsize.o		qtimer.o	qview.o	      \
		  qwid_x11.o	qwidget.o	qwininfo.o
SRCMETA		= mtimer.cpp	mview.cpp	mwidget.cpp
OBJMETA		= mtimer.o	mview.o		mwidget.o
TARGET		= $(LIBDIR)/libqkernel.a

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
qapp.o: /home/popcorn/home/hanord/qt/src/include/qshared.h qcursor.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qapp.o: /usr/include/string.h /usr/include/features.h
qapp.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h qevent.h
qapp.o: qobjcoll.h qmetaobj.h qconnect.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qapp.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qapp_x11.o: qapp.h qwidget.h qwindefs.h qobjdefs.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobject.h
qapp_x11.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h qcursor.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qapp_x11.o: /usr/include/string.h /usr/include/features.h
qapp_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qapp_x11.o: qevent.h qwininfo.h /usr/include/stdlib.h /usr/include/errno.h
qapp_x11.o: /usr/include/linux/errno.h /usr/local/lib/gcc-include/float.h
qapp_x11.o: /usr/include/signal.h /usr/include/sys/types.h
qapp_x11.o: /usr/include/linux/types.h /usr/include/linux/signal.h
qapp_x11.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
qapp_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qapp_x11.o: /usr/include/X11/Xutil.h /usr/include/X11/Xos.h
qapp_x11.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h
qapp_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qapp_x11.o: /usr/include/gnu/types.h /usr/include/sys/time.h
qapp_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qstack.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qapp_x11.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qapp_x11.o: /usr/include/X11/keysymdef.h qkeycode.h
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
qcolor.o: qcolor.h qwindefs.h qobjdefs.h
qcolor.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qcolor.o: /home/popcorn/home/hanord/qt/src/include/qstream.h
qconnect.o: qconnect.h qobject.h qobjdefs.h
qconnect.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qcur_x11.o: qcursor.h qpoint.h qwindefs.h qobjdefs.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qapp.h
qcur_x11.o: qwidget.h qobject.h qpaintd.h qrect.h qsize.h qcolor.h qfont.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qcur_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qcur_x11.o: /usr/include/string.h /usr/include/features.h
qcur_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qcur_x11.o: qevent.h /usr/include/X11/Xlib.h /usr/include/sys/types.h
qcur_x11.o: /usr/include/linux/types.h /usr/include/X11/X.h
qcur_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qcur_x11.o: /usr/include/X11/Xutil.h /usr/include/X11/Xos.h
qcur_x11.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h
qcur_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qcur_x11.o: /usr/include/gnu/types.h /usr/include/sys/time.h
qcur_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qcur_x11.o: /usr/include/X11/cursorfont.h
qcursor.o: qcursor.h qpoint.h qwindefs.h qobjdefs.h
qcursor.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qcursor.o: /home/popcorn/home/hanord/qt/src/include/qstream.h
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
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qcolor.h
qfnt_x11.o: qfontmet.h qfont.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h qregion.h
qfnt_x11.o: qrect.h qsize.h qpoint.h qpen.h qbrush.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qmetafil.o: qmetafil.h qpaintd.h qwindefs.h qobjdefs.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qbuffer.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qstream.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qmetafil.o: /usr/include/string.h /usr/include/features.h
qmetafil.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qmetafil.o: qpainter.h qcolor.h qfontmet.h qfont.h qregion.h qrect.h qsize.h
qmetafil.o: qpoint.h qpen.h qbrush.h qpntarry.h
qmetafil.o: /home/popcorn/home/hanord/qt/src/include/qfile.h
qmetafil.o: /usr/include/stdio.h /usr/include/libio.h
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
qmetaobj.o: /home/popcorn/home/hanord/qt/src/include/qstream.h
qmtf_x11.o: qmetafil.h qpaintd.h qwindefs.h qobjdefs.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qbuffer.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qstream.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qmtf_x11.o: /usr/include/string.h /usr/include/features.h
qmtf_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qmtf_x11.o: qpainter.h qcolor.h qfontmet.h qfont.h qregion.h qrect.h qsize.h
qmtf_x11.o: qpoint.h qpen.h qbrush.h qpntarry.h
qmtf_x11.o: /home/popcorn/home/hanord/qt/src/include/qfile.h
qmtf_x11.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/X11/Xlib.h
qmtf_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qmtf_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qmtf_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qmtf_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qmtf_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qmtf_x11.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
qmtf_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qmtf_x11.o: /usr/include/sys/time.h
qobject.o: qobject.h qobjdefs.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobjcoll.h
qobject.o: qmetaobj.h qconnect.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h qpart.h qview.h
qobject.o: qwidget.h qwindefs.h qpaintd.h qrect.h qsize.h qpoint.h qcolor.h
qobject.o: qfont.h /home/popcorn/home/hanord/qt/src/include/qshared.h
qobject.o: qcursor.h /home/popcorn/home/hanord/qt/src/include/qstring.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qobject.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qobject.o: /usr/include/string.h /usr/include/features.h
qobject.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qobject.o: qevent.h
qpainter.o: qpainter.h qpaintd.h qwindefs.h qobjdefs.h
qpainter.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qcolor.h
qpainter.o: qfontmet.h qfont.h
qpainter.o: /home/popcorn/home/hanord/qt/src/include/qshared.h qregion.h
qpainter.o: qrect.h qsize.h qpoint.h qpen.h qbrush.h qpntarry.h
qpainter.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qpainter.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
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
qpmp_x11.o: qpixmap.h qpaintd.h qwindefs.h qobjdefs.h
qpmp_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qsize.h
qpmp_x11.o: qpoint.h /usr/include/X11/Xlib.h /usr/include/sys/types.h
qpmp_x11.o: /usr/include/linux/types.h /usr/include/X11/X.h
qpmp_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qpmp_x11.o: /usr/local/lib/gcc-include/stddef.h /usr/include/X11/Xutil.h
qpmp_x11.o: /usr/include/X11/Xos.h /usr/include/string.h
qpmp_x11.o: /usr/include/features.h /usr/include/sys/cdefs.h
qpmp_x11.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h
qpmp_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qpmp_x11.o: /usr/include/gnu/types.h /usr/include/sys/time.h
qpmp_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qpntarry.o: qpntarry.h /home/popcorn/home/hanord/qt/src/include/qarray.h
qpntarry.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qpntarry.o: /home/popcorn/home/hanord/qt/src/include/qshared.h
qpntarry.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qpoint.h
qpntarry.o: qwindefs.h qobjdefs.h
qpntarry.o: /home/popcorn/home/hanord/qt/src/include/qstream.h
qpoint.o: qpoint.h qwindefs.h qobjdefs.h
qpoint.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qpoint.o: /home/popcorn/home/hanord/qt/src/include/qstream.h
qptd_x11.o: qpaintd.h qwindefs.h qobjdefs.h
qptd_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qptd_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qptd_x11.o: /usr/include/linux/types.h /usr/include/X11/X.h
qptd_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qptd_x11.o: /usr/local/lib/gcc-include/stddef.h /usr/include/X11/Xutil.h
qptd_x11.o: /usr/include/X11/Xos.h /usr/include/string.h
qptd_x11.o: /usr/include/features.h /usr/include/sys/cdefs.h
qptd_x11.o: /usr/include/fcntl.h /usr/include/linux/fcntl.h
qptd_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qptd_x11.o: /usr/include/gnu/types.h /usr/include/sys/time.h
qptd_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qptr_x11.o: qpainter.h qpaintd.h qwindefs.h qobjdefs.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qcolor.h
qptr_x11.o: qfontmet.h qfont.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h qregion.h
qptr_x11.o: qrect.h qsize.h qpoint.h qpen.h qbrush.h qwidget.h qobject.h
qptr_x11.o: qcursor.h /home/popcorn/home/hanord/qt/src/include/qstring.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qptr_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qptr_x11.o: /usr/include/string.h /usr/include/features.h
qptr_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qptr_x11.o: qevent.h qpntarry.h qpixmap.h /usr/include/X11/Xlib.h
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
qrect.o: qrect.h qsize.h qpoint.h qwindefs.h qobjdefs.h
qrect.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
qrect.o: /home/popcorn/home/hanord/qt/src/include/qstream.h
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
qsize.o: /home/popcorn/home/hanord/qt/src/include/qstream.h
qtimer.o: qtimer.h qobject.h qobjdefs.h
qtimer.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qevent.h
qtimer.o: qwindefs.h qrect.h qsize.h qpoint.h
qview.o: qview.h qwidget.h qwindefs.h qobjdefs.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobject.h
qview.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qshared.h qcursor.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qview.o: /usr/include/string.h /usr/include/features.h
qview.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qview.o: qevent.h qobjcoll.h qmetaobj.h qconnect.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qlist.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qglist.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qdict.h
qview.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qview.o: /usr/include/ctype.h
qwid_x11.o: qview.h qwidget.h qwindefs.h qobjdefs.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h qobject.h
qwid_x11.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfont.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qshared.h qcursor.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qwid_x11.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qwid_x11.o: /usr/include/string.h /usr/include/features.h
qwid_x11.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qwid_x11.o: qevent.h qapp.h qwininfo.h /usr/include/X11/Xlib.h
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
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qshared.h qcursor.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qstring.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qarray.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qgarray.h
qwidget.o: /usr/include/string.h /usr/include/features.h
qwidget.o: /usr/include/sys/cdefs.h /usr/local/lib/gcc-include/stddef.h
qwidget.o: qevent.h qapp.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qintdict.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qgdict.h
qwidget.o: /home/popcorn/home/hanord/qt/src/include/qcollect.h
qwininfo.o: qwininfo.h qsize.h qpoint.h qwindefs.h qobjdefs.h
qwininfo.o: /home/popcorn/home/hanord/qt/src/include/qglobal.h
