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
		  qfontdta.h	qfontinf.h	qfontmet.h	qkeycode.h    \
		  qmetaobj.h    qobjcoll.h	qobjdefs.h	qobject.h     \
		  qpaintd.h	qpaintdc.h	qpainter.h	qpdevmet.h    \
		  qpen.h	qpicture.h	qpixmap.h	qpntarry.h    \
		  qpoint.h	qprinter.h	qprtdrv.h	qprtstrm.h    \
		  qrect.h	qregion.h	qsignal.h	qsize.h       \
		  qtimer.h	qview.h		qwidcoll.h	qwidget.h     \
		  qwindefs.h	qwmatrix.h	qimage.h
SOURCES		= qapp.cpp	qapp_x11.cpp	qcol_x11.cpp	qcolor.cpp    \
		  qconnect.cpp	qcur_x11.cpp	qcursor.cpp	qevent.cpp    \
		  qfnt_x11.cpp	qfont.cpp	qmetaobj.cpp	qobject.cpp   \
		  qpainter.cpp	qpdevmet.cpp	qpic_x11.cpp	qpicture.cpp  \
		  qpmp_x11.cpp	qpntarry.cpp	qpoint.cpp	qprinter.cpp  \
		  qprtstrm.cpp	qptd_x11.cpp	qptr_x11.cpp	qrect.cpp     \
		  qrgn_x11.cpp	qsignal.cpp	qsize.cpp	qtimer.cpp    \
		  qview.cpp	qwid_x11.cpp	qwidget.cpp	qwmatrix.cpp  \
		  qimage.cpp
OBJECTS		= qapp.o	qapp_x11.o	qcol_x11.o	qcolor.o      \
		  qconnect.o	qcur_x11.o	qcursor.o	qevent.o      \
		  qfnt_x11.o	qfont.o		qmetaobj.o	qobject.o     \
		  qpainter.o	qpdevmet.o	qpic_x11.o	qpicture.o    \
		  qpmp_x11.o	qpntarry.o	qpoint.o	qprinter.o    \
		  qprtstrm.o	qptd_x11.o	qptr_x11.o	qrect.o	      \
		  qrgn_x11.o	qsignal.o	qsize.o		qtimer.o      \
		  qview.o	qwid_x11.o	qwidget.o	qwmatrix.o    \
		  qimage.o
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
qapp.o: /home/popcorn/home/hanord/qt/include/qglobal.h qobject.h
qapp.o: /home/popcorn/home/hanord/qt/include/qstring.h
qapp.o: /home/popcorn/home/hanord/qt/include/qarray.h
qapp.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qapp.o: /home/popcorn/home/hanord/qt/include/qshared.h
qapp.o: /home/popcorn/home/hanord/qt/include/qgeneric.h /usr/include/string.h
qapp.o: /usr/include/features.h /usr/include/sys/cdefs.h qpaintd.h qrect.h
qapp.o: qsize.h qpoint.h qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h
qapp.o: qevent.h qobjcoll.h qmetaobj.h qconnect.h
qapp.o: /home/popcorn/home/hanord/qt/include/qlist.h
qapp.o: /home/popcorn/home/hanord/qt/include/qglist.h
qapp.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qapp.o: /home/popcorn/home/hanord/qt/include/qdict.h
qapp.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qapp_x11.o: qapp.h qwidget.h qwindefs.h qobjdefs.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qobject.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qapp_x11.o: /usr/include/string.h /usr/include/features.h
qapp_x11.o: /usr/include/sys/cdefs.h qpaintd.h qrect.h qsize.h qpoint.h
qapp_x11.o: qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h
qapp_x11.o: qwidcoll.h /home/popcorn/home/hanord/qt/include/qlist.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qglist.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qintdict.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qapp_x11.o: /usr/include/stdlib.h /usr/include/errno.h
qapp_x11.o: /usr/include/linux/errno.h /usr/include/alloca.h
qapp_x11.o: /usr/include/signal.h /usr/include/sys/types.h
qapp_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qapp_x11.o: /usr/include/linux/signal.h /usr/include/X11/Xlib.h
qapp_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qapp_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qapp_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qapp_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qapp_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qapp_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qapp_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qapp_x11.o: /usr/include/sys/param.h /usr/include/limits.h
qapp_x11.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
qapp_x11.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
qapp_x11.o: /usr/include/asm/param.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qmemchk.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qstack.h
qapp_x11.o: /usr/include/X11/keysymdef.h qkeycode.h
qcol_x11.o: qcolor.h qwindefs.h qobjdefs.h
qcol_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qcol_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qcol_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qcol_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qcol_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qcol_x11.o: /usr/include/X11/Xos.h /usr/include/string.h
qcol_x11.o: /usr/include/features.h /usr/include/sys/cdefs.h
qcol_x11.o: /usr/include/fcntl.h /usr/include/gnu/types.h
qcol_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qcol_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qcol_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qcol_x11.o: /usr/include/sys/time.h /usr/include/sys/param.h
qcol_x11.o: /usr/include/limits.h /usr/include/posix1_lim.h
qcol_x11.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
qcol_x11.o: /usr/include/linux/param.h /usr/include/asm/param.h
qcol_x11.o: /home/popcorn/home/hanord/qt/include/qintdict.h
qcol_x11.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qcol_x11.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qcol_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qcolor.o: qcolor.h qwindefs.h qobjdefs.h
qcolor.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qcolor.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qcolor.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qconnect.o: qconnect.h qobject.h qobjdefs.h
qconnect.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qconnect.o: /home/popcorn/home/hanord/qt/include/qstring.h
qconnect.o: /home/popcorn/home/hanord/qt/include/qarray.h
qconnect.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qconnect.o: /home/popcorn/home/hanord/qt/include/qshared.h
qconnect.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qconnect.o: /usr/include/string.h /usr/include/features.h
qconnect.o: /usr/include/sys/cdefs.h
qcur_x11.o: qcursor.h qpoint.h qwindefs.h qobjdefs.h
qcur_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qcur_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h qapp.h qwidget.h
qcur_x11.o: qobject.h /home/popcorn/home/hanord/qt/include/qstring.h
qcur_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qcur_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qcur_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qcur_x11.o: /usr/include/string.h /usr/include/features.h
qcur_x11.o: /usr/include/sys/cdefs.h qpaintd.h qrect.h qsize.h qcolor.h
qcur_x11.o: qfontmet.h qfont.h qfontinf.h qevent.h qbitmap.h qpixmap.h
qcur_x11.o: /home/popcorn/home/hanord/qt/include/qmemchk.h
qcur_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qcur_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qcur_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qcur_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qcur_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qcur_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qcur_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qcur_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qcur_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qcur_x11.o: /usr/include/sys/param.h /usr/include/limits.h
qcur_x11.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
qcur_x11.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
qcur_x11.o: /usr/include/asm/param.h /usr/include/X11/cursorfont.h
qcursor.o: qcursor.h qpoint.h qwindefs.h qobjdefs.h
qcursor.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qcursor.o: /home/popcorn/home/hanord/qt/include/qshared.h
qcursor.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qcursor.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qevent.o: qevent.h qwindefs.h qobjdefs.h
qevent.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qevent.o: qpoint.h
qfnt_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qfnt_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qfnt_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qfnt_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qfnt_x11.o: /usr/include/X11/Xos.h /usr/include/string.h
qfnt_x11.o: /usr/include/features.h /usr/include/sys/cdefs.h
qfnt_x11.o: /usr/include/fcntl.h /usr/include/gnu/types.h
qfnt_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qfnt_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qfnt_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qfnt_x11.o: /usr/include/sys/time.h /usr/include/sys/param.h
qfnt_x11.o: /usr/include/limits.h /usr/include/posix1_lim.h
qfnt_x11.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
qfnt_x11.o: /usr/include/linux/param.h /usr/include/asm/param.h
qfnt_x11.o: /usr/include/X11/Xatom.h qfont.h qwindefs.h qobjdefs.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h qfontdta.h
qfnt_x11.o: qfontmet.h qfontinf.h qpainter.h qpaintd.h qrect.h qsize.h
qfnt_x11.o: qpoint.h qcolor.h qregion.h qpen.h qbrush.h qwmatrix.h qpntarry.h
qfnt_x11.o: /usr/include/ctype.h /usr/include/stdlib.h /usr/include/errno.h
qfnt_x11.o: /usr/include/linux/errno.h /usr/include/alloca.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qdict.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qfont.o: qfont.h qwindefs.h qobjdefs.h
qfont.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qfont.o: /home/popcorn/home/hanord/qt/include/qstring.h
qfont.o: /home/popcorn/home/hanord/qt/include/qarray.h
qfont.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qfont.o: /home/popcorn/home/hanord/qt/include/qshared.h
qfont.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qfont.o: /usr/include/string.h /usr/include/features.h
qfont.o: /usr/include/sys/cdefs.h qfontdta.h qfontmet.h qfontinf.h qpainter.h
qfont.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qregion.h qpen.h
qfont.o: qbrush.h qwmatrix.h qpntarry.h qwidcoll.h qwidget.h qobject.h
qfont.o: qcursor.h qevent.h /home/popcorn/home/hanord/qt/include/qlist.h
qfont.o: /home/popcorn/home/hanord/qt/include/qglist.h
qfont.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qfont.o: /home/popcorn/home/hanord/qt/include/qintdict.h
qfont.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qfont.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qfont.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qmetaobj.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qstring.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qarray.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qshared.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qmetaobj.o: /usr/include/string.h /usr/include/features.h
qmetaobj.o: /usr/include/sys/cdefs.h qobjcoll.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qlist.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qglist.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qdict.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qstrlist.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qmetaobj.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qobject.o: qobject.h qobjdefs.h
qobject.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qobject.o: /home/popcorn/home/hanord/qt/include/qstring.h
qobject.o: /home/popcorn/home/hanord/qt/include/qarray.h
qobject.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qobject.o: /home/popcorn/home/hanord/qt/include/qshared.h
qobject.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qobject.o: /usr/include/string.h /usr/include/features.h
qobject.o: /usr/include/sys/cdefs.h qobjcoll.h qmetaobj.h qconnect.h
qobject.o: /home/popcorn/home/hanord/qt/include/qlist.h
qobject.o: /home/popcorn/home/hanord/qt/include/qglist.h
qobject.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qobject.o: /home/popcorn/home/hanord/qt/include/qdict.h
qobject.o: /home/popcorn/home/hanord/qt/include/qgdict.h /usr/include/ctype.h
qpainter.o: qpainter.h qpaintd.h qwindefs.h qobjdefs.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qpainter.o: qpoint.h qcolor.h qfontmet.h qfont.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qstring.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qarray.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qshared.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qpainter.o: /usr/include/string.h /usr/include/features.h
qpainter.o: /usr/include/sys/cdefs.h qfontinf.h qregion.h qpen.h qbrush.h
qpainter.o: qwmatrix.h qpntarry.h qpaintdc.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qstack.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qglist.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qpdevmet.o: qpdevmet.h qwindefs.h qobjdefs.h
qpdevmet.o: /home/popcorn/home/hanord/qt/include/qglobal.h qpaintd.h qrect.h
qpdevmet.o: qsize.h qpoint.h qpaintdc.h
qpic_x11.o: qpicture.h qpaintd.h qwindefs.h qobjdefs.h
qpic_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qpic_x11.o: qpoint.h /home/popcorn/home/hanord/qt/include/qbuffer.h
qpic_x11.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qpic_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qpic_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qpic_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qpic_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qpic_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qpic_x11.o: /usr/include/string.h /usr/include/features.h
qpic_x11.o: /usr/include/sys/cdefs.h /usr/include/X11/Xlib.h
qpic_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qpic_x11.o: /usr/include/asm/types.h /usr/include/X11/X.h
qpic_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qpic_x11.o: /usr/include/X11/Xutil.h /usr/include/X11/Xos.h
qpic_x11.o: /usr/include/fcntl.h /usr/include/gnu/types.h
qpic_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qpic_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qpic_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qpic_x11.o: /usr/include/sys/time.h /usr/include/sys/param.h
qpic_x11.o: /usr/include/limits.h /usr/include/posix1_lim.h
qpic_x11.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
qpic_x11.o: /usr/include/linux/param.h /usr/include/asm/param.h
qpicture.o: qpicture.h qpaintd.h qwindefs.h qobjdefs.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qpicture.o: qpoint.h /home/popcorn/home/hanord/qt/include/qbuffer.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qstring.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qarray.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qshared.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qpicture.o: /usr/include/string.h /usr/include/features.h
qpicture.o: /usr/include/sys/cdefs.h qpaintdc.h qpainter.h qcolor.h
qpicture.o: qfontmet.h qfont.h qfontinf.h qregion.h qpen.h qbrush.h
qpicture.o: qwmatrix.h qpntarry.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qfile.h /usr/include/stdio.h
qpicture.o: /usr/include/libio.h /usr/include/_G_config.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qpmp_x11.o: qbitmap.h qpixmap.h qpaintd.h qwindefs.h qobjdefs.h
qpmp_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qpmp_x11.o: qpoint.h qcolor.h qimage.h
qpmp_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qpmp_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qpmp_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qpmp_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qpmp_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qpmp_x11.o: /usr/include/string.h /usr/include/features.h
qpmp_x11.o: /usr/include/sys/cdefs.h qpainter.h qfontmet.h qfont.h qfontinf.h
qpmp_x11.o: qregion.h qpen.h qbrush.h qwmatrix.h qpntarry.h qpaintdc.h
qpmp_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qpmp_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qpmp_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qpmp_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qpmp_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qpmp_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qpmp_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qpmp_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qpmp_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qpmp_x11.o: /usr/include/sys/param.h /usr/include/limits.h
qpmp_x11.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
qpmp_x11.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
qpmp_x11.o: /usr/include/asm/param.h
qpntarry.o: qpntarry.h /home/popcorn/home/hanord/qt/include/qarray.h
qpntarry.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qpntarry.o: /home/popcorn/home/hanord/qt/include/qshared.h
qpntarry.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qpntarry.o: /home/popcorn/home/hanord/qt/include/qgeneric.h qpoint.h
qpntarry.o: qwindefs.h qobjdefs.h qrect.h qsize.h
qpntarry.o: /home/popcorn/home/hanord/qt/include/qbitarry.h
qpntarry.o: /home/popcorn/home/hanord/qt/include/qstring.h
qpntarry.o: /usr/include/string.h /usr/include/features.h
qpntarry.o: /usr/include/sys/cdefs.h
qpntarry.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qpntarry.o: /home/popcorn/home/hanord/qt/include/qiodev.h /usr/include/math.h
qpntarry.o: /usr/include/huge_val.h /usr/include/endian.h
qpntarry.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
qpoint.o: qpoint.h qwindefs.h qobjdefs.h
qpoint.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qpoint.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qpoint.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qprinter.o: qprinter.h qpaintd.h qwindefs.h qobjdefs.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qprinter.o: qpoint.h /home/popcorn/home/hanord/qt/include/qstring.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qarray.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qshared.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qprinter.o: /usr/include/string.h /usr/include/features.h
qprinter.o: /usr/include/sys/cdefs.h qprtdrv.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qiodev.h qpainter.h qcolor.h
qprinter.o: qfontmet.h qfont.h qfontinf.h qregion.h qpen.h qbrush.h
qprinter.o: qwmatrix.h qpntarry.h qpaintdc.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qdict.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qdatetm.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qstack.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qglist.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qtstream.h
qprinter.o: /usr/include/stdio.h /usr/include/libio.h
qprinter.o: /usr/include/_G_config.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qfile.h
qprtstrm.o: qprtstrm.h qprinter.h qpaintd.h qwindefs.h qobjdefs.h
qprtstrm.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qprtstrm.o: qpoint.h /home/popcorn/home/hanord/qt/include/qstring.h
qprtstrm.o: /home/popcorn/home/hanord/qt/include/qarray.h
qprtstrm.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qprtstrm.o: /home/popcorn/home/hanord/qt/include/qshared.h
qprtstrm.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qprtstrm.o: /usr/include/string.h /usr/include/features.h
qprtstrm.o: /usr/include/sys/cdefs.h
qptd_x11.o: qpaintd.h qwindefs.h qobjdefs.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qptd_x11.o: qpoint.h qwidget.h qobject.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qptd_x11.o: /usr/include/string.h /usr/include/features.h
qptd_x11.o: /usr/include/sys/cdefs.h qcolor.h qfontmet.h qfont.h qfontinf.h
qptd_x11.o: qcursor.h qevent.h qpixmap.h /usr/include/X11/Xlib.h
qptd_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qptd_x11.o: /usr/include/asm/types.h /usr/include/X11/X.h
qptd_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qptd_x11.o: /usr/include/X11/Xutil.h /usr/include/X11/Xos.h
qptd_x11.o: /usr/include/fcntl.h /usr/include/gnu/types.h
qptd_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qptd_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qptd_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qptd_x11.o: /usr/include/sys/time.h /usr/include/sys/param.h
qptd_x11.o: /usr/include/limits.h /usr/include/posix1_lim.h
qptd_x11.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
qptd_x11.o: /usr/include/linux/param.h /usr/include/asm/param.h
qptr_x11.o: qpainter.h qpaintd.h qwindefs.h qobjdefs.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qptr_x11.o: qpoint.h qcolor.h qfontmet.h qfont.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qptr_x11.o: /usr/include/string.h /usr/include/features.h
qptr_x11.o: /usr/include/sys/cdefs.h qfontinf.h qregion.h qpen.h qbrush.h
qptr_x11.o: qwmatrix.h qpntarry.h qpaintdc.h qwidget.h qobject.h qcursor.h
qptr_x11.o: qevent.h qbitmap.h qpixmap.h /usr/include/ctype.h
qptr_x11.o: /usr/include/malloc.h /usr/include/X11/Xlib.h
qptr_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qptr_x11.o: /usr/include/asm/types.h /usr/include/X11/X.h
qptr_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qptr_x11.o: /usr/include/X11/Xutil.h /usr/include/X11/Xos.h
qptr_x11.o: /usr/include/fcntl.h /usr/include/gnu/types.h
qptr_x11.o: /usr/include/linux/fcntl.h /usr/include/unistd.h
qptr_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qptr_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qptr_x11.o: /usr/include/sys/time.h /usr/include/sys/param.h
qptr_x11.o: /usr/include/limits.h /usr/include/posix1_lim.h
qptr_x11.o: /usr/include/linux/limits.h /usr/include/posix2_lim.h
qptr_x11.o: /usr/include/linux/param.h /usr/include/asm/param.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qlist.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qglist.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qdict.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qrect.o: qrect.h qsize.h qpoint.h qwindefs.h qobjdefs.h
qrect.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qrect.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qrect.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qrgn_x11.o: qregion.h /home/popcorn/home/hanord/qt/include/qshared.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qrgn_x11.o: qpoint.h qwindefs.h qobjdefs.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qrgn_x11.o: /usr/include/string.h /usr/include/features.h
qrgn_x11.o: /usr/include/sys/cdefs.h qpntarry.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/include/qbuffer.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qrgn_x11.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qrgn_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qrgn_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qrgn_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qrgn_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qrgn_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qrgn_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qrgn_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qrgn_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qrgn_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qrgn_x11.o: /usr/include/sys/param.h /usr/include/limits.h
qrgn_x11.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
qrgn_x11.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
qrgn_x11.o: /usr/include/asm/param.h
qsignal.o: qsignal.h qobject.h qobjdefs.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qstring.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qarray.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qshared.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qsignal.o: /usr/include/string.h /usr/include/features.h
qsignal.o: /usr/include/sys/cdefs.h qmetaobj.h qconnect.h
qsignal.o: /usr/include/ctype.h
qsize.o: qsize.h qpoint.h qwindefs.h qobjdefs.h
qsize.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qsize.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qsize.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qtimer.o: qtimer.h qobject.h qobjdefs.h
qtimer.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qtimer.o: /home/popcorn/home/hanord/qt/include/qstring.h
qtimer.o: /home/popcorn/home/hanord/qt/include/qarray.h
qtimer.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qtimer.o: /home/popcorn/home/hanord/qt/include/qshared.h
qtimer.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qtimer.o: /usr/include/string.h /usr/include/features.h
qtimer.o: /usr/include/sys/cdefs.h qevent.h qwindefs.h qrect.h qsize.h
qtimer.o: qpoint.h
qview.o: qview.h qwidget.h qwindefs.h qobjdefs.h
qview.o: /home/popcorn/home/hanord/qt/include/qglobal.h qobject.h
qview.o: /home/popcorn/home/hanord/qt/include/qstring.h
qview.o: /home/popcorn/home/hanord/qt/include/qarray.h
qview.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qview.o: /home/popcorn/home/hanord/qt/include/qshared.h
qview.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qview.o: /usr/include/string.h /usr/include/features.h
qview.o: /usr/include/sys/cdefs.h qpaintd.h qrect.h qsize.h qpoint.h qcolor.h
qview.o: qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h qpixmap.h
qwid_x11.o: qview.h qwidget.h qwindefs.h qobjdefs.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qobject.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qwid_x11.o: /usr/include/string.h /usr/include/features.h
qwid_x11.o: /usr/include/sys/cdefs.h qpaintd.h qrect.h qsize.h qpoint.h
qwid_x11.o: qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h qapp.h
qwid_x11.o: qpaintdc.h qpixmap.h qobjcoll.h qmetaobj.h qconnect.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qlist.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qglist.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qdict.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qwid_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qwid_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qwid_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qwid_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qwid_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qwid_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qwid_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qwid_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qwid_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qwid_x11.o: /usr/include/sys/param.h /usr/include/limits.h
qwid_x11.o: /usr/include/posix1_lim.h /usr/include/linux/limits.h
qwid_x11.o: /usr/include/posix2_lim.h /usr/include/linux/param.h
qwid_x11.o: /usr/include/asm/param.h
qwidget.o: qobjcoll.h qmetaobj.h qconnect.h qobject.h qobjdefs.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qstring.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qarray.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qshared.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qwidget.o: /usr/include/string.h /usr/include/features.h
qwidget.o: /usr/include/sys/cdefs.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qlist.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qglist.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qdict.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qgdict.h qwidget.h qwindefs.h
qwidget.o: qpaintd.h qrect.h qsize.h qpoint.h qcolor.h qfontmet.h qfont.h
qwidget.o: qfontinf.h qcursor.h qevent.h qwidcoll.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qintdict.h qapp.h
qwmatrix.o: qwmatrix.h qwindefs.h qobjdefs.h
qwmatrix.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qwmatrix.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qwmatrix.o: /home/popcorn/home/hanord/qt/include/qiodev.h /usr/include/math.h
qwmatrix.o: /usr/include/features.h /usr/include/sys/cdefs.h
qwmatrix.o: /usr/include/huge_val.h /usr/include/endian.h
qwmatrix.o: /usr/include/bytesex.h /usr/include/nan.h /usr/include/values.h
qimage.o: qimage.h qpixmap.h qpaintd.h qwindefs.h qobjdefs.h
qimage.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qimage.o: qpoint.h qcolor.h /home/popcorn/home/hanord/qt/include/qstring.h
qimage.o: /home/popcorn/home/hanord/qt/include/qarray.h
qimage.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qimage.o: /home/popcorn/home/hanord/qt/include/qshared.h
qimage.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qimage.o: /usr/include/string.h /usr/include/features.h
qimage.o: /usr/include/sys/cdefs.h
qimage.o: /home/popcorn/home/hanord/qt/include/qregexp.h
qimage.o: /home/popcorn/home/hanord/qt/include/qfile.h
qimage.o: /home/popcorn/home/hanord/qt/include/qiodev.h /usr/include/stdio.h
qimage.o: /usr/include/libio.h /usr/include/_G_config.h
qimage.o: /home/popcorn/home/hanord/qt/include/qdstream.h qbitmap.h
qimage.o: /usr/include/stdlib.h /usr/include/errno.h
qimage.o: /usr/include/linux/errno.h /usr/include/alloca.h
qimage.o: /usr/include/ctype.h
mtimer.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qstring.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qarray.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qshared.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
mtimer.o: /usr/include/string.h /usr/include/features.h
mtimer.o: /usr/include/sys/cdefs.h qtimer.h
mview.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mview.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mview.o: /home/popcorn/home/hanord/qt/include/qstring.h
mview.o: /home/popcorn/home/hanord/qt/include/qarray.h
mview.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mview.o: /home/popcorn/home/hanord/qt/include/qshared.h
mview.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
mview.o: /usr/include/string.h /usr/include/features.h
mview.o: /usr/include/sys/cdefs.h qview.h qwidget.h qwindefs.h qpaintd.h
mview.o: qrect.h qsize.h qpoint.h qcolor.h qfontmet.h qfont.h qfontinf.h
mview.o: qcursor.h qevent.h
mwidget.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qstring.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qarray.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qshared.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
mwidget.o: /usr/include/string.h /usr/include/features.h
mwidget.o: /usr/include/sys/cdefs.h qwidget.h qwindefs.h qpaintd.h qrect.h
mwidget.o: qsize.h qpoint.h qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h
mwidget.o: qevent.h
