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
		qwmatrix.h	qbasic.h	qgeom.h
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
		qwmatrix.cpp	qbasic.cpp	qgeom.cpp
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
		qwmatrix.o	qbasic.o	qgeom.o
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

moc: $(SRCMETA)

depend: $(SRCMETA)
	makedepend -I$(INCDIR) $(SOURCES) $(SRCMETA) 2> /dev/null

clean:
	rm -f *.o *.bak *~ *% #*
	rm -f $(SRCMETA)

####### Meta classes

maccel.cpp: qaccel.h
	moc qaccel.h -o maccel.cpp

mapp.cpp: qapp.h
	moc qapp.h -o mapp.cpp

mclipbrd.cpp: qclipbrd.h
	moc qclipbrd.h -o mclipbrd.cpp

mdialog.cpp: qdialog.h
	moc qdialog.h -o mdialog.cpp

msocknot.cpp: qsocknot.h
	moc qsocknot.h -o msocknot.cpp

mtimer.cpp: qtimer.h
	moc qtimer.h -o mtimer.cpp

mwidget.cpp: qwidget.h
	moc qwidget.h -o mwidget.cpp

mwindow.cpp: qwindow.h
	moc qwindow.h -o mwindow.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.
qaccel.o: qaccel.cpp qaccel.h qobject.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h qkeycode.h qapp.h qwidget.h \
 qpaintd.h qpalette.h qcolor.h qcursor.h qfont.h qfontmet.h qfontinf.h \
 /home/hanord/qt/include/qlist.h /home/hanord/qt/include/qglist.h \
 /home/hanord/qt/include/qcollect.h qsignal.h
qapp.o: qapp.cpp qapp.h qwidget.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qobject.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h qrect.h \
 qsize.h qpoint.h qpaintd.h qpalette.h qcolor.h qcursor.h qfont.h \
 qfontmet.h qfontinf.h qobjcoll.h qmetaobj.h qconnect.h \
 /home/hanord/qt/include/qlist.h /home/hanord/qt/include/qglist.h \
 /home/hanord/qt/include/qcollect.h /home/hanord/qt/include/qdict.h \
 /home/hanord/qt/include/qgdict.h qwidcoll.h \
 /home/hanord/qt/include/qintdict.h
qapp_x11.o: qapp_x11.cpp qapp.h qwidget.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qobject.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h qrect.h \
 qsize.h qpoint.h qpaintd.h qpalette.h qcolor.h qcursor.h qfont.h \
 qfontmet.h qfontinf.h qobjcoll.h qmetaobj.h qconnect.h \
 /home/hanord/qt/include/qlist.h /home/hanord/qt/include/qglist.h \
 /home/hanord/qt/include/qcollect.h /home/hanord/qt/include/qdict.h \
 /home/hanord/qt/include/qgdict.h qwidcoll.h \
 /home/hanord/qt/include/qintdict.h qpainter.h qregion.h qpen.h \
 qbrush.h qpntarry.h qwmatrix.h qpmcache.h qpixmap.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/signal.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h \
 /usr/include/asm/types.h /usr/include/sys/bitypes.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/locale.h /usr/include/X11/Xlib.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/X11/Xatom.h /home/hanord/qt/include/qfile.h \
 /home/hanord/qt/include/qiodev.h /usr/include/stdio.h \
 /local/lib/g++-include/libio.h /local/lib/g++-include/_G_config.h \
 /usr/include/X11/keysymdef.h qkeycode.h
qbitmap.o: qbitmap.cpp qbitmap.h qpixmap.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 qcolor.h /home/hanord/qt/include/qshared.h qimage.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
qclb_x11.o: qclb_x11.cpp qclipbrd.h qobject.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h qapp.h qwidget.h qpaintd.h \
 qpalette.h qcolor.h qcursor.h qfont.h qfontmet.h qfontinf.h qpixmap.h \
 /home/hanord/qt/include/qdatetm.h /usr/include/X11/Xlib.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h \
 /usr/include/asm/types.h /usr/include/sys/bitypes.h \
 /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
 /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
 /usr/include/X11/Xos.h /usr/include/fcntl.h /usr/include/gnu/types.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/X11/Xatom.h
qclipbrd.o: qclipbrd.cpp qclipbrd.h qobject.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h qapp.h qwidget.h qpaintd.h \
 qpalette.h qcolor.h qcursor.h qfont.h qfontmet.h qfontinf.h
qcol_x11.o: qcol_x11.cpp qcolor.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/linux/posix_types.h \
 /usr/include/asm/posix_types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /home/hanord/qt/include/qintdict.h /home/hanord/qt/include/qgdict.h \
 /home/hanord/qt/include/qcollect.h /home/hanord/qt/include/qgeneric.h
qcolor.o: qcolor.cpp qcolor.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qdstream.h \
 /home/hanord/qt/include/qiodev.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
qconnect.o: qconnect.cpp qconnect.h qobject.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h
qcur_x11.o: qcur_x11.cpp qcursor.h qpoint.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qshared.h \
 qbitmap.h qpixmap.h qpaintd.h qrect.h qsize.h qcolor.h qapp.h \
 qwidget.h qobject.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qpalette.h qfont.h qfontmet.h qfontinf.h \
 /home/hanord/qt/include/qdstream.h /home/hanord/qt/include/qiodev.h \
 /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/linux/posix_types.h \
 /usr/include/asm/posix_types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/X11/cursorfont.h
qcursor.o: qcursor.cpp qcursor.h qpoint.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qshared.h \
 qbitmap.h qpixmap.h qpaintd.h qrect.h qsize.h qcolor.h \
 /home/hanord/qt/include/qdstream.h /home/hanord/qt/include/qiodev.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
qdialog.o: qdialog.cpp qdialog.h qwindow.h qwidget.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qglobal.h qobject.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h qrect.h \
 qsize.h qpoint.h qpaintd.h qpalette.h qcolor.h qcursor.h qfont.h \
 qfontmet.h qfontinf.h /home/hanord/qt/include/qpushbt.h \
 /home/hanord/qt/include/qbutton.h /home/hanord/qt/include/qwidget.h \
 qapp.h qkeycode.h qobjcoll.h qmetaobj.h qconnect.h \
 /home/hanord/qt/include/qlist.h /home/hanord/qt/include/qglist.h \
 /home/hanord/qt/include/qcollect.h /home/hanord/qt/include/qdict.h \
 /home/hanord/qt/include/qgdict.h
qdrawutl.o: qdrawutl.cpp qdrawutl.h qpainter.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 qcolor.h qfontmet.h qfont.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qfontinf.h \
 qregion.h qpen.h qbrush.h qpntarry.h qwmatrix.h qpalette.h
qevent.o: qevent.cpp qevent.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h
qfnt_x11.o: qfnt_x11.cpp qwidget.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qobject.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h qrect.h \
 qsize.h qpoint.h qpaintd.h qpalette.h qcolor.h qcursor.h qfont.h \
 qfontmet.h qfontinf.h qpainter.h qregion.h qpen.h qbrush.h qpntarry.h \
 qwmatrix.h /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/linux/posix_types.h \
 /usr/include/asm/posix_types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/X11/Xatom.h qfontdta.h /home/hanord/qt/include/qcache.h \
 /home/hanord/qt/include/qgcache.h /home/hanord/qt/include/qcollect.h \
 /home/hanord/qt/include/qglist.h /home/hanord/qt/include/qgdict.h \
 /home/hanord/qt/include/qdict.h /usr/include/ctype.h \
 /usr/include/endian.h /usr/include/bytesex.h /usr/include/stdlib.h \
 /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h
qfont.o: qfont.cpp qfont.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qfontdta.h \
 qfontmet.h qrect.h qsize.h qpoint.h qfontinf.h qwidget.h qobject.h \
 qevent.h qpaintd.h qpalette.h qcolor.h qcursor.h qpainter.h qregion.h \
 qpen.h qbrush.h qpntarry.h qwmatrix.h /home/hanord/qt/include/qdict.h \
 /home/hanord/qt/include/qgdict.h /home/hanord/qt/include/qcollect.h \
 /home/hanord/qt/include/qstrlist.h /home/hanord/qt/include/qlist.h \
 /home/hanord/qt/include/qglist.h /home/hanord/qt/include/qdstream.h \
 /home/hanord/qt/include/qiodev.h
qimage.o: qimage.cpp qimage.h qpixmap.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 qcolor.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qregexp.h /home/hanord/qt/include/qfile.h \
 /home/hanord/qt/include/qiodev.h /usr/include/stdio.h \
 /local/lib/g++-include/libio.h /local/lib/g++-include/_G_config.h \
 /home/hanord/qt/include/qdstream.h /home/hanord/qt/include/qlist.h \
 /home/hanord/qt/include/qglist.h /home/hanord/qt/include/qcollect.h \
 /home/hanord/qt/include/qintdict.h /home/hanord/qt/include/qgdict.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/ctype.h \
 /usr/include/endian.h /usr/include/bytesex.h /usr/include/X11/Xlib.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h \
 /usr/include/asm/types.h /usr/include/sys/bitypes.h \
 /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
 /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
 /usr/include/X11/Xos.h /usr/include/fcntl.h /usr/include/gnu/types.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/time.h
qmetaobj.o: qmetaobj.cpp qmetaobj.h qconnect.h qobject.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h qobjcoll.h \
 /home/hanord/qt/include/qlist.h /home/hanord/qt/include/qglist.h \
 /home/hanord/qt/include/qcollect.h /home/hanord/qt/include/qdict.h \
 /home/hanord/qt/include/qgdict.h /home/hanord/qt/include/qstrlist.h \
 /home/hanord/qt/include/qdstream.h /home/hanord/qt/include/qiodev.h
qobject.o: qobject.cpp qobject.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h qobjcoll.h qmetaobj.h qconnect.h \
 /home/hanord/qt/include/qlist.h /home/hanord/qt/include/qglist.h \
 /home/hanord/qt/include/qcollect.h /home/hanord/qt/include/qdict.h \
 /home/hanord/qt/include/qgdict.h /home/hanord/qt/include/qregexp.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
qpainter.o: qpainter.cpp qpainter.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h qcolor.h \
 qfontmet.h qfont.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qfontinf.h \
 qregion.h qpen.h qbrush.h qpntarry.h qwmatrix.h qpaintdc.h qbitmap.h \
 qpixmap.h /home/hanord/qt/include/qstack.h \
 /home/hanord/qt/include/qglist.h /home/hanord/qt/include/qcollect.h \
 /home/hanord/qt/include/qdstream.h /home/hanord/qt/include/qiodev.h
qpalette.o: qpalette.cpp qpalette.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qcolor.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qdstream.h \
 /home/hanord/qt/include/qiodev.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
qpdevmet.o: qpdevmet.cpp qpdevmet.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qpaintd.h qrect.h qsize.h qpoint.h \
 qpaintdc.h
qpic_x11.o: qpic_x11.cpp qpicture.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 /home/hanord/qt/include/qbuffer.h /home/hanord/qt/include/qiodev.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/linux/posix_types.h \
 /usr/include/asm/posix_types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h
qpicture.o: qpicture.cpp qpicture.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 /home/hanord/qt/include/qbuffer.h /home/hanord/qt/include/qiodev.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qpaintdc.h \
 qpainter.h qcolor.h qfontmet.h qfont.h qfontinf.h qregion.h qpen.h \
 qbrush.h qpntarry.h qwmatrix.h qpixmap.h \
 /home/hanord/qt/include/qfile.h /usr/include/stdio.h \
 /local/lib/g++-include/libio.h /local/lib/g++-include/_G_config.h \
 /home/hanord/qt/include/qdstream.h
qpixmap.o: qpixmap.cpp qbitmap.h qpixmap.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 qcolor.h /home/hanord/qt/include/qshared.h qimage.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qwidget.h \
 qobject.h qevent.h qpalette.h qcursor.h qfont.h qfontmet.h qfontinf.h \
 qpainter.h qregion.h qpen.h qbrush.h qpntarry.h qwmatrix.h \
 /home/hanord/qt/include/qdstream.h /home/hanord/qt/include/qiodev.h \
 /home/hanord/qt/include/qbuffer.h
qpm_x11.o: qpm_x11.cpp qbitmap.h qpixmap.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 qcolor.h /home/hanord/qt/include/qshared.h qimage.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qpaintdc.h \
 qwmatrix.h qpntarry.h qapp.h qwidget.h qobject.h qevent.h qpalette.h \
 qcursor.h qfont.h qfontmet.h qfontinf.h /usr/include/malloc.h \
 /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/linux/posix_types.h \
 /usr/include/asm/posix_types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h
qpmcache.o: qpmcache.cpp qpmcache.h qpixmap.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 qcolor.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qcache.h /home/hanord/qt/include/qgcache.h \
 /home/hanord/qt/include/qcollect.h /home/hanord/qt/include/qglist.h \
 /home/hanord/qt/include/qgdict.h /home/hanord/qt/include/qgeneric.h
qpntarry.o: qpntarry.cpp qpntarry.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qgeneric.h \
 qpoint.h qwindefs.h qobjdefs.h qrect.h qsize.h \
 /home/hanord/qt/include/qbitarry.h /home/hanord/qt/include/qstring.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qdstream.h /home/hanord/qt/include/qiodev.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h
qpoint.o: qpoint.cpp qpoint.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qdstream.h \
 /home/hanord/qt/include/qiodev.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
qprinter.o: qprinter.cpp qprinter.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
qprn_x11.o: qprn_x11.cpp qprinter.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qpaintdc.h \
 qpsprn.h /home/hanord/qt/include/qtstream.h \
 /home/hanord/qt/include/qiodev.h /usr/include/stdio.h \
 /local/lib/g++-include/libio.h /local/lib/g++-include/_G_config.h \
 /home/hanord/qt/include/qprndlg.h /home/hanord/qt/include/qdialog.h \
 /home/hanord/qt/include/qwindow.h /home/hanord/qt/include/qwidget.h \
 /home/hanord/qt/include/qwindefs.h /home/hanord/qt/include/qobject.h \
 /home/hanord/qt/include/qobjdefs.h /home/hanord/qt/include/qevent.h \
 /home/hanord/qt/include/qrect.h /home/hanord/qt/include/qpaintd.h \
 /home/hanord/qt/include/qpalette.h /home/hanord/qt/include/qcolor.h \
 /home/hanord/qt/include/qcursor.h /home/hanord/qt/include/qpoint.h \
 /home/hanord/qt/include/qfont.h /home/hanord/qt/include/qfontmet.h \
 /home/hanord/qt/include/qfontinf.h /home/hanord/qt/include/qfile.h \
 qapp.h qwidget.h /usr/include/stdlib.h /usr/include/errno.h \
 /usr/include/linux/errno.h /usr/include/asm/errno.h \
 /usr/include/alloca.h /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/gnu/types.h /usr/include/confname.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h \
 /usr/include/asm/types.h /usr/include/sys/bitypes.h
qpsprn.o: qpsprn.cpp qpsprn.h qprinter.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qtstream.h /home/hanord/qt/include/qiodev.h \
 /usr/include/stdio.h /local/lib/g++-include/libio.h \
 /local/lib/g++-include/_G_config.h qpainter.h qcolor.h qfontmet.h \
 qfont.h qfontinf.h qregion.h qpen.h qbrush.h qpntarry.h qwmatrix.h \
 qpaintdc.h qimage.h qpixmap.h /home/hanord/qt/include/qdatetm.h \
 /home/hanord/qt/include/qfile.h /home/hanord/qt/include/qbuffer.h
qptd_x11.o: qptd_x11.cpp qpaintd.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h qpaintdc.h \
 qwidget.h qobject.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qpalette.h qcolor.h qcursor.h qfont.h qfontmet.h qfontinf.h qbitmap.h \
 qpixmap.h qapp.h /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/linux/posix_types.h \
 /usr/include/asm/posix_types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h
qptr_x11.o: qptr_x11.cpp qpainter.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h qcolor.h \
 qfontmet.h qfont.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qfontinf.h \
 qregion.h qpen.h qbrush.h qpntarry.h qwmatrix.h qpaintdc.h qwidget.h \
 qobject.h qevent.h qpalette.h qcursor.h qbitmap.h qpixmap.h \
 qpmcache.h /home/hanord/qt/include/qlist.h \
 /home/hanord/qt/include/qglist.h /home/hanord/qt/include/qcollect.h \
 /home/hanord/qt/include/qintdict.h /home/hanord/qt/include/qgdict.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/malloc.h /usr/include/X11/Xlib.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h \
 /usr/include/asm/types.h /usr/include/sys/bitypes.h \
 /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
 /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
 /usr/include/X11/Xos.h /usr/include/fcntl.h /usr/include/gnu/types.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/time.h
qrect.o: qrect.cpp qrect.h qsize.h qpoint.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qdstream.h \
 /home/hanord/qt/include/qiodev.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
qregion.o: qregion.cpp qregion.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qpntarry.h \
 /home/hanord/qt/include/qbuffer.h /home/hanord/qt/include/qiodev.h \
 /home/hanord/qt/include/qdstream.h
qrgn_x11.o: qrgn_x11.cpp qregion.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qglobal.h qrect.h qsize.h qpoint.h qwindefs.h \
 qobjdefs.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qpntarry.h \
 /home/hanord/qt/include/qbuffer.h /home/hanord/qt/include/qiodev.h \
 /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/linux/posix_types.h \
 /usr/include/asm/posix_types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h
qsignal.o: qsignal.cpp qsignal.h qobject.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h qmetaobj.h qconnect.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
qsize.o: qsize.cpp qsize.h qpoint.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qdstream.h \
 /home/hanord/qt/include/qiodev.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
qsocknot.o: qsocknot.cpp qsocknot.h qobject.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h
qtimer.o: qtimer.cpp qtimer.h qobject.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h /home/hanord/qt/include/qstring.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h
qwid_x11.o: qwid_x11.cpp qwindow.h qwidget.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qobject.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h qrect.h \
 qsize.h qpoint.h qpaintd.h qpalette.h qcolor.h qcursor.h qfont.h \
 qfontmet.h qfontinf.h qapp.h qpaintdc.h qpainter.h qregion.h qpen.h \
 qbrush.h qpntarry.h qwmatrix.h qpixmap.h qwidcoll.h \
 /home/hanord/qt/include/qlist.h /home/hanord/qt/include/qglist.h \
 /home/hanord/qt/include/qcollect.h /home/hanord/qt/include/qintdict.h \
 /home/hanord/qt/include/qgdict.h qobjcoll.h qmetaobj.h qconnect.h \
 /home/hanord/qt/include/qdict.h /usr/include/X11/Xlib.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h \
 /usr/include/asm/types.h /usr/include/sys/bitypes.h \
 /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
 /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
 /usr/include/X11/Xos.h /usr/include/fcntl.h /usr/include/gnu/types.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/time.h
qwidget.o: qwidget.cpp qobjcoll.h qmetaobj.h qconnect.h qobject.h \
 qobjdefs.h /home/hanord/qt/include/qglobal.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h /home/hanord/qt/include/qlist.h \
 /home/hanord/qt/include/qglist.h /home/hanord/qt/include/qcollect.h \
 /home/hanord/qt/include/qdict.h /home/hanord/qt/include/qgdict.h \
 qwidget.h qpaintd.h qpalette.h qcolor.h qcursor.h qfont.h qfontmet.h \
 qfontinf.h qwidcoll.h /home/hanord/qt/include/qintdict.h qpixmap.h \
 qkeycode.h qapp.h
qwindow.o: qwindow.cpp qwindow.h qwidget.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qobject.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h qevent.h qrect.h \
 qsize.h qpoint.h qpaintd.h qpalette.h qcolor.h qcursor.h qfont.h \
 qfontmet.h qfontinf.h qpixmap.h
qwmatrix.o: qwmatrix.cpp qwmatrix.h qwindefs.h qobjdefs.h \
 /home/hanord/qt/include/qglobal.h qpntarry.h \
 /home/hanord/qt/include/qarray.h /home/hanord/qt/include/qgarray.h \
 /home/hanord/qt/include/qshared.h /home/hanord/qt/include/qgeneric.h \
 qpoint.h qrect.h qsize.h /home/hanord/qt/include/qdstream.h \
 /home/hanord/qt/include/qiodev.h /home/hanord/qt/include/qstring.h \
 /usr/include/string.h /usr/include/features.h \
 /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
maccel.o: maccel.cpp /home/hanord/qt/include/qmetaobj.h \
 /home/hanord/qt/include/qconnect.h /home/hanord/qt/include/qobject.h \
 /home/hanord/qt/include/qobjdefs.h /home/hanord/qt/include/qglobal.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qevent.h /home/hanord/qt/include/qwindefs.h \
 /home/hanord/qt/include/qrect.h /home/hanord/qt/include/qsize.h \
 /home/hanord/qt/include/qpoint.h qaccel.h qobject.h qkeycode.h
mapp.o: mapp.cpp /home/hanord/qt/include/qmetaobj.h \
 /home/hanord/qt/include/qconnect.h /home/hanord/qt/include/qobject.h \
 /home/hanord/qt/include/qobjdefs.h /home/hanord/qt/include/qglobal.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qevent.h /home/hanord/qt/include/qwindefs.h \
 /home/hanord/qt/include/qrect.h /home/hanord/qt/include/qsize.h \
 /home/hanord/qt/include/qpoint.h qapp.h qwidget.h qwindefs.h \
 qobject.h qpaintd.h qrect.h qpalette.h qcolor.h qcursor.h qpoint.h \
 qfont.h qfontmet.h qfontinf.h
mclipbrd.o: mclipbrd.cpp /home/hanord/qt/include/qmetaobj.h \
 /home/hanord/qt/include/qconnect.h /home/hanord/qt/include/qobject.h \
 /home/hanord/qt/include/qobjdefs.h /home/hanord/qt/include/qglobal.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qevent.h /home/hanord/qt/include/qwindefs.h \
 /home/hanord/qt/include/qrect.h /home/hanord/qt/include/qsize.h \
 /home/hanord/qt/include/qpoint.h qclipbrd.h qobject.h
mdialog.o: mdialog.cpp /home/hanord/qt/include/qmetaobj.h \
 /home/hanord/qt/include/qconnect.h /home/hanord/qt/include/qobject.h \
 /home/hanord/qt/include/qobjdefs.h /home/hanord/qt/include/qglobal.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qevent.h /home/hanord/qt/include/qwindefs.h \
 /home/hanord/qt/include/qrect.h /home/hanord/qt/include/qsize.h \
 /home/hanord/qt/include/qpoint.h qdialog.h qwindow.h qwidget.h \
 qwindefs.h qobject.h qpaintd.h qrect.h qpalette.h qcolor.h qcursor.h \
 qpoint.h qfont.h qfontmet.h qfontinf.h
mtimer.o: mtimer.cpp /home/hanord/qt/include/qmetaobj.h \
 /home/hanord/qt/include/qconnect.h /home/hanord/qt/include/qobject.h \
 /home/hanord/qt/include/qobjdefs.h /home/hanord/qt/include/qglobal.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qevent.h /home/hanord/qt/include/qwindefs.h \
 /home/hanord/qt/include/qrect.h /home/hanord/qt/include/qsize.h \
 /home/hanord/qt/include/qpoint.h qtimer.h qobject.h
mwidget.o: mwidget.cpp /home/hanord/qt/include/qmetaobj.h \
 /home/hanord/qt/include/qconnect.h /home/hanord/qt/include/qobject.h \
 /home/hanord/qt/include/qobjdefs.h /home/hanord/qt/include/qglobal.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qevent.h /home/hanord/qt/include/qwindefs.h \
 /home/hanord/qt/include/qrect.h /home/hanord/qt/include/qsize.h \
 /home/hanord/qt/include/qpoint.h qwidget.h qwindefs.h qobject.h \
 qpaintd.h qrect.h qpalette.h qcolor.h qcursor.h qpoint.h qfont.h \
 qfontmet.h qfontinf.h
mwindow.o: mwindow.cpp /home/hanord/qt/include/qmetaobj.h \
 /home/hanord/qt/include/qconnect.h /home/hanord/qt/include/qobject.h \
 /home/hanord/qt/include/qobjdefs.h /home/hanord/qt/include/qglobal.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qevent.h /home/hanord/qt/include/qwindefs.h \
 /home/hanord/qt/include/qrect.h /home/hanord/qt/include/qsize.h \
 /home/hanord/qt/include/qpoint.h qwindow.h qwidget.h qwindefs.h \
 qobject.h qpaintd.h qrect.h qpalette.h qcolor.h qcursor.h qpoint.h \
 qfont.h qfontmet.h qfontinf.h
msocknot.o: msocknot.cpp /home/hanord/qt/include/qmetaobj.h \
 /home/hanord/qt/include/qconnect.h /home/hanord/qt/include/qobject.h \
 /home/hanord/qt/include/qobjdefs.h /home/hanord/qt/include/qglobal.h \
 /home/hanord/qt/include/qstring.h /home/hanord/qt/include/qarray.h \
 /home/hanord/qt/include/qgarray.h /home/hanord/qt/include/qshared.h \
 /home/hanord/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /home/hanord/qt/include/qevent.h /home/hanord/qt/include/qwindefs.h \
 /home/hanord/qt/include/qrect.h /home/hanord/qt/include/qsize.h \
 /home/hanord/qt/include/qpoint.h qsocknot.h qobject.h
