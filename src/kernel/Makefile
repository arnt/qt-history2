#############################################################################
# Qt Makefile - kernel
#############################################################################

####### Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2 -DCHECK_MEMORY
CC	=	gcc

####### Files

HEADERS	=	q2matrix.h	qaccel.h	qapp.h		qbitmap.h     \
		qbrush.h	qcolor.h	qconnect.h	qcursor.h     \
		qdialog.h	qevent.h	qfont.h		qfontdta.h    \
		qfontinf.h	qfontmet.h	qimage.h	qkeycode.h    \
		qmetaobj.h	qobjcoll.h	qobjdefs.h	qobject.h     \
		qpaintd.h	qpaintdc.h	qpainter.h	qpalette.h    \
		qpdevmet.h	qpen.h		qpicture.h	qpixmap.h     \
		qpmcache.h	qpntarry.h	qpoint.h	qprinter.h    \
		qrect.h		qregion.h	qsignal.h	qsize.h	      \
		qtimer.h	qwidcoll.h	qwidget.h	qwindefs.h    \
		qwindow.h
SOURCES	=	q2matrix.cpp	qaccel.cpp	qapp.cpp	qapp_x11.cpp  \
		qbitmap.cpp	qcol_x11.cpp	qcolor.cpp	qconnect.cpp  \
		qcur_x11.cpp	qcursor.cpp	qdialog.cpp	qevent.cpp    \
		qfnt_x11.cpp	qfont.cpp	qimage.cpp	qmetaobj.cpp  \
		qobject.cpp	qpainter.cpp	qpalette.cpp	qpdevmet.cpp  \
		qpic_x11.cpp	qpicture.cpp	qpixmap.cpp	qpm_x11.cpp   \
		qpmcache.cpp	qpntarry.cpp	qpoint.cpp	qprinter.cpp  \
		qptd_x11.cpp	qptr_x11.cpp	qrect.cpp	qrgn_x11.cpp  \
		qsignal.cpp	qsize.cpp	qtimer.cpp	qwid_x11.cpp  \
		qwidget.cpp	qwindow.cpp	qregion.cpp
OBJECTS	=	q2matrix.o	qaccel.o	qapp.o		qapp_x11.o    \
		qbitmap.o	qcol_x11.o	qcolor.o	qconnect.o    \
		qcur_x11.o	qcursor.o	qdialog.o	qevent.o      \
		qfnt_x11.o	qfont.o		qimage.o	qmetaobj.o    \
		qobject.o	qpainter.o	qpalette.o	qpdevmet.o    \
		qpic_x11.o	qpicture.o	qpixmap.o	qpm_x11.o     \
		qpmcache.o	qpntarry.o	qpoint.o	qprinter.o    \
		qptd_x11.o	qptr_x11.o	qrect.o		qrgn_x11.o    \
		qsignal.o	qsize.o		qtimer.o	qwid_x11.o    \
		qwidget.o	qwindow.o	qregion.o
SRCMETA	=	maccel.cpp	mapp.cpp	mdialog.cpp	mtimer.cpp    \
		mwidget.cpp	mwindow.cpp
OBJMETA	=	maccel.o	mapp.o		mdialog.o	mtimer.o      \
		mwidget.o	mwindow.o
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
	/bin/rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS) $(OBJMETA)
	ranlib $(TARGET)

rcs:: ./RCS/$$@,v

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
	/bin/true	# tell make it worked

####### Meta classes

maccel.cpp: qaccel.h
	$(QTDIR)/bin/moc qaccel.h -o maccel.cpp

mapp.cpp: qapp.h
	$(QTDIR)/bin/moc qapp.h -o mapp.cpp

mdialog.cpp: qdialog.h
	$(QTDIR)/bin/moc qdialog.h -o mdialog.cpp

mtimer.cpp: qtimer.h
	$(QTDIR)/bin/moc qtimer.h -o mtimer.cpp

mwidget.cpp: qwidget.h
	$(QTDIR)/bin/moc qwidget.h -o mwidget.cpp

mwindow.cpp: qwindow.h
	$(QTDIR)/bin/moc qwindow.h -o mwindow.cpp


# DO NOT DELETE THIS LINE -- make depend depends on it.

q2matrix.o: q2matrix.h qwindefs.h qobjdefs.h
q2matrix.o: /home/popcorn/home/hanord/qt/include/qglobal.h qpntarry.h
q2matrix.o: /home/popcorn/home/hanord/qt/include/qarray.h
q2matrix.o: /home/popcorn/home/hanord/qt/include/qgarray.h
q2matrix.o: /home/popcorn/home/hanord/qt/include/qshared.h
q2matrix.o: /home/popcorn/home/hanord/qt/include/qgeneric.h qpoint.h qrect.h
q2matrix.o: qsize.h /home/popcorn/home/hanord/qt/include/qdstream.h
q2matrix.o: /home/popcorn/home/hanord/qt/include/qiodev.h /usr/include/math.h
q2matrix.o: /usr/include/features.h /usr/include/sys/cdefs.h
q2matrix.o: /usr/include/huge_val.h /usr/include/endian.h
q2matrix.o: /usr/include/bytesex.h /usr/include/linux/version.h
q2matrix.o: /usr/include/asm/byteorder.h /usr/include/nan.h
q2matrix.o: /usr/include/values.h
qaccel.o: qaccel.h qobject.h qobjdefs.h
qaccel.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qaccel.o: /home/popcorn/home/hanord/qt/include/qstring.h
qaccel.o: /home/popcorn/home/hanord/qt/include/qarray.h
qaccel.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qaccel.o: /home/popcorn/home/hanord/qt/include/qshared.h
qaccel.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qaccel.o: /usr/include/string.h /usr/include/features.h
qaccel.o: /usr/include/sys/cdefs.h qkeycode.h qapp.h qwidget.h qwindefs.h
qaccel.o: qpaintd.h qrect.h qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h
qaccel.o: qfont.h qfontinf.h qcursor.h qevent.h
qaccel.o: /home/popcorn/home/hanord/qt/include/qlist.h
qaccel.o: /home/popcorn/home/hanord/qt/include/qglist.h
qaccel.o: /home/popcorn/home/hanord/qt/include/qcollect.h qsignal.h
qapp.o: qapp.h qwidget.h qwindefs.h qobjdefs.h
qapp.o: /home/popcorn/home/hanord/qt/include/qglobal.h qobject.h
qapp.o: /home/popcorn/home/hanord/qt/include/qstring.h
qapp.o: /home/popcorn/home/hanord/qt/include/qarray.h
qapp.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qapp.o: /home/popcorn/home/hanord/qt/include/qshared.h
qapp.o: /home/popcorn/home/hanord/qt/include/qgeneric.h /usr/include/string.h
qapp.o: /usr/include/features.h /usr/include/sys/cdefs.h qpaintd.h qrect.h
qapp.o: qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h
qapp.o: qcursor.h qevent.h qobjcoll.h qmetaobj.h qconnect.h
qapp.o: /home/popcorn/home/hanord/qt/include/qlist.h
qapp.o: /home/popcorn/home/hanord/qt/include/qglist.h
qapp.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qapp.o: /home/popcorn/home/hanord/qt/include/qdict.h
qapp.o: /home/popcorn/home/hanord/qt/include/qgdict.h qwidcoll.h
qapp.o: /home/popcorn/home/hanord/qt/include/qintdict.h
qapp_x11.o: qapp.h qwidget.h qwindefs.h qobjdefs.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qobject.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qapp_x11.o: /usr/include/string.h /usr/include/features.h
qapp_x11.o: /usr/include/sys/cdefs.h qpaintd.h qrect.h qsize.h qpoint.h
qapp_x11.o: qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h
qapp_x11.o: qevent.h qwidcoll.h /home/popcorn/home/hanord/qt/include/qlist.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qglist.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qintdict.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qgdict.h qpainter.h
qapp_x11.o: qregion.h qpen.h qbrush.h q2matrix.h qpntarry.h qpmcache.h
qapp_x11.o: qpixmap.h /usr/include/stdlib.h /usr/include/errno.h
qapp_x11.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
qapp_x11.o: /usr/include/alloca.h /usr/include/signal.h
qapp_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qapp_x11.o: /usr/include/asm/types.h /usr/include/linux/signal.h
qapp_x11.o: /usr/include/asm/signal.h /usr/include/ctype.h
qapp_x11.o: /usr/include/endian.h /usr/include/bytesex.h
qapp_x11.o: /usr/include/linux/version.h /usr/include/asm/byteorder.h
qapp_x11.o: /usr/include/locale.h /usr/include/X11/Xlib.h
qapp_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qapp_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qapp_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qapp_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qapp_x11.o: /usr/include/asm/fcntl.h /usr/include/unistd.h
qapp_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qapp_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qapp_x11.o: /usr/include/sys/time.h /usr/include/X11/Xatom.h
qapp_x11.o: /home/popcorn/home/hanord/qt/include/qmemchk.h
qapp_x11.o: /usr/include/X11/keysymdef.h qkeycode.h
qbitmap.o: qbitmap.h qpixmap.h qpaintd.h qwindefs.h qobjdefs.h
qbitmap.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qbitmap.o: qpoint.h qcolor.h /home/popcorn/home/hanord/qt/include/qshared.h
qbitmap.o: qimage.h /home/popcorn/home/hanord/qt/include/qstring.h
qbitmap.o: /home/popcorn/home/hanord/qt/include/qarray.h
qbitmap.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qbitmap.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qbitmap.o: /usr/include/string.h /usr/include/features.h
qbitmap.o: /usr/include/sys/cdefs.h
qcol_x11.o: qcolor.h qwindefs.h qobjdefs.h
qcol_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qcol_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qcol_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qcol_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qcol_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qcol_x11.o: /usr/include/X11/Xos.h /usr/include/string.h
qcol_x11.o: /usr/include/features.h /usr/include/sys/cdefs.h
qcol_x11.o: /usr/include/fcntl.h /usr/include/gnu/types.h
qcol_x11.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
qcol_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qcol_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qcol_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
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
qcur_x11.o: /usr/include/sys/cdefs.h qpaintd.h qrect.h qsize.h qpalette.h
qcur_x11.o: qcolor.h qfontmet.h qfont.h qfontinf.h qevent.h qbitmap.h
qcur_x11.o: qpixmap.h /home/popcorn/home/hanord/qt/include/qdstream.h
qcur_x11.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qcur_x11.o: /home/popcorn/home/hanord/qt/include/qmemchk.h
qcur_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qcur_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qcur_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qcur_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qcur_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qcur_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qcur_x11.o: /usr/include/asm/fcntl.h /usr/include/unistd.h
qcur_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qcur_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qcur_x11.o: /usr/include/sys/time.h /usr/include/X11/cursorfont.h
qcursor.o: qcursor.h qpoint.h qwindefs.h qobjdefs.h
qcursor.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qcursor.o: /home/popcorn/home/hanord/qt/include/qshared.h qbitmap.h qpixmap.h
qcursor.o: qpaintd.h qrect.h qsize.h qcolor.h
qcursor.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qcursor.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qdialog.o: qdialog.h qwindow.h qwidget.h qwindefs.h qobjdefs.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qglobal.h qobject.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qstring.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qarray.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qshared.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qdialog.o: /usr/include/string.h /usr/include/features.h
qdialog.o: /usr/include/sys/cdefs.h qpaintd.h qrect.h qsize.h qpoint.h
qdialog.o: qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h
qdialog.o: qevent.h /home/popcorn/home/hanord/qt/include/qpushbt.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qbutton.h qapp.h qkeycode.h
qdialog.o: qobjcoll.h qmetaobj.h qconnect.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qlist.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qglist.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qdict.h
qdialog.o: /home/popcorn/home/hanord/qt/include/qgdict.h
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
qfnt_x11.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
qfnt_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qfnt_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qfnt_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qfnt_x11.o: /usr/include/X11/Xatom.h qfont.h qwindefs.h qobjdefs.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h qfontdta.h
qfnt_x11.o: qfontmet.h qrect.h qsize.h qpoint.h qfontinf.h qpainter.h
qfnt_x11.o: qpaintd.h qcolor.h qregion.h qpen.h qbrush.h q2matrix.h
qfnt_x11.o: qpntarry.h /usr/include/ctype.h /usr/include/endian.h
qfnt_x11.o: /usr/include/bytesex.h /usr/include/linux/version.h
qfnt_x11.o: /usr/include/asm/byteorder.h /usr/include/stdlib.h
qfnt_x11.o: /usr/include/errno.h /usr/include/linux/errno.h
qfnt_x11.o: /usr/include/asm/errno.h /usr/include/alloca.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qcache.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qgcache.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qglist.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qfnt_x11.o: /home/popcorn/home/hanord/qt/include/qdict.h
qfont.o: qfont.h qwindefs.h qobjdefs.h
qfont.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qfont.o: /home/popcorn/home/hanord/qt/include/qstring.h
qfont.o: /home/popcorn/home/hanord/qt/include/qarray.h
qfont.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qfont.o: /home/popcorn/home/hanord/qt/include/qshared.h
qfont.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qfont.o: /usr/include/string.h /usr/include/features.h
qfont.o: /usr/include/sys/cdefs.h qfontdta.h qfontmet.h qrect.h qsize.h
qfont.o: qpoint.h qfontinf.h qpainter.h qpaintd.h qcolor.h qregion.h qpen.h
qfont.o: qbrush.h q2matrix.h qpntarry.h qwidcoll.h qwidget.h qobject.h
qfont.o: qpalette.h qcursor.h qevent.h
qfont.o: /home/popcorn/home/hanord/qt/include/qlist.h
qfont.o: /home/popcorn/home/hanord/qt/include/qglist.h
qfont.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qfont.o: /home/popcorn/home/hanord/qt/include/qintdict.h
qfont.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qfont.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qfont.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qimage.o: qimage.h qpixmap.h qpaintd.h qwindefs.h qobjdefs.h
qimage.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qimage.o: qpoint.h qcolor.h /home/popcorn/home/hanord/qt/include/qshared.h
qimage.o: /home/popcorn/home/hanord/qt/include/qstring.h
qimage.o: /home/popcorn/home/hanord/qt/include/qarray.h
qimage.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qimage.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qimage.o: /usr/include/string.h /usr/include/features.h
qimage.o: /usr/include/sys/cdefs.h
qimage.o: /home/popcorn/home/hanord/qt/include/qregexp.h
qimage.o: /home/popcorn/home/hanord/qt/include/qfile.h
qimage.o: /home/popcorn/home/hanord/qt/include/qiodev.h /usr/include/stdio.h
qimage.o: /usr/include/libio.h /usr/include/_G_config.h
qimage.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qimage.o: /home/popcorn/home/hanord/qt/include/qlist.h
qimage.o: /home/popcorn/home/hanord/qt/include/qglist.h
qimage.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qimage.o: /home/popcorn/home/hanord/qt/include/qintdict.h
qimage.o: /home/popcorn/home/hanord/qt/include/qgdict.h /usr/include/stdlib.h
qimage.o: /usr/include/errno.h /usr/include/linux/errno.h
qimage.o: /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/ctype.h
qimage.o: /usr/include/endian.h /usr/include/bytesex.h
qimage.o: /usr/include/linux/version.h /usr/include/asm/byteorder.h
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
qobject.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qobject.o: /home/popcorn/home/hanord/qt/include/qregexp.h
qobject.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
qobject.o: /usr/include/linux/version.h /usr/include/asm/byteorder.h
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
qpainter.o: q2matrix.h qpntarry.h qpaintdc.h qbitmap.h qpixmap.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qstack.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qglist.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qpainter.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qpalette.o: qpalette.h qwindefs.h qobjdefs.h
qpalette.o: /home/popcorn/home/hanord/qt/include/qglobal.h qcolor.h
qpalette.o: /home/popcorn/home/hanord/qt/include/qshared.h
qpalette.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qpalette.o: /home/popcorn/home/hanord/qt/include/qiodev.h
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
qpic_x11.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
qpic_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qpic_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qpic_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
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
qpicture.o: q2matrix.h qpntarry.h qpixmap.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qfile.h /usr/include/stdio.h
qpicture.o: /usr/include/libio.h /usr/include/_G_config.h
qpicture.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qpixmap.o: qpixmap.h qpaintd.h qwindefs.h qobjdefs.h
qpixmap.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qpixmap.o: qpoint.h qcolor.h /home/popcorn/home/hanord/qt/include/qshared.h
qpixmap.o: qimage.h /home/popcorn/home/hanord/qt/include/qstring.h
qpixmap.o: /home/popcorn/home/hanord/qt/include/qarray.h
qpixmap.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qpixmap.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qpixmap.o: /usr/include/string.h /usr/include/features.h
qpixmap.o: /usr/include/sys/cdefs.h
qpixmap.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qpixmap.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qpm_x11.o: qpixmap.h qpaintd.h qwindefs.h qobjdefs.h
qpm_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qpm_x11.o: qpoint.h qcolor.h /home/popcorn/home/hanord/qt/include/qshared.h
qpm_x11.o: qimage.h /home/popcorn/home/hanord/qt/include/qstring.h
qpm_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qpm_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qpm_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qpm_x11.o: /usr/include/string.h /usr/include/features.h
qpm_x11.o: /usr/include/sys/cdefs.h qpaintdc.h q2matrix.h qpntarry.h qapp.h
qpm_x11.o: qwidget.h qobject.h qpalette.h qfontmet.h qfont.h qfontinf.h
qpm_x11.o: qcursor.h qevent.h /usr/include/malloc.h /usr/include/X11/Xlib.h
qpm_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qpm_x11.o: /usr/include/asm/types.h /usr/include/X11/X.h
qpm_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qpm_x11.o: /usr/include/X11/Xutil.h /usr/include/X11/Xos.h
qpm_x11.o: /usr/include/fcntl.h /usr/include/gnu/types.h
qpm_x11.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
qpm_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qpm_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qpm_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
qpmcache.o: qpmcache.h qpixmap.h qpaintd.h qwindefs.h qobjdefs.h
qpmcache.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qpmcache.o: qpoint.h qcolor.h /home/popcorn/home/hanord/qt/include/qshared.h
qpmcache.o: /home/popcorn/home/hanord/qt/include/qcache.h
qpmcache.o: /home/popcorn/home/hanord/qt/include/qgcache.h
qpmcache.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qpmcache.o: /home/popcorn/home/hanord/qt/include/qglist.h
qpmcache.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qpmcache.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
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
qpntarry.o: /usr/include/bytesex.h /usr/include/linux/version.h
qpntarry.o: /usr/include/asm/byteorder.h /usr/include/nan.h
qpntarry.o: /usr/include/values.h
qpoint.o: qpoint.h qwindefs.h qobjdefs.h
qpoint.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qpoint.o: /home/popcorn/home/hanord/qt/include/qdstream.h
qpoint.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qprinter.o: qprinter.h qpaintd.h qwindefs.h qobjdefs.h
qprinter.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qprinter.o: qpoint.h
qptd_x11.o: qpaintd.h qwindefs.h qobjdefs.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qptd_x11.o: qpoint.h qpaintdc.h qwidget.h qobject.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qptd_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qptd_x11.o: /usr/include/string.h /usr/include/features.h
qptd_x11.o: /usr/include/sys/cdefs.h qpalette.h qcolor.h qfontmet.h qfont.h
qptd_x11.o: qfontinf.h qcursor.h qevent.h qpixmap.h qapp.h
qptd_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qptd_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qptd_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qptd_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qptd_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qptd_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qptd_x11.o: /usr/include/asm/fcntl.h /usr/include/unistd.h
qptd_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qptd_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qptd_x11.o: /usr/include/sys/time.h
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
qptr_x11.o: q2matrix.h qpntarry.h qpaintdc.h qwidget.h qobject.h qpalette.h
qptr_x11.o: qcursor.h qevent.h qbitmap.h qpixmap.h qpmcache.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qlist.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qglist.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qintdict.h
qptr_x11.o: /home/popcorn/home/hanord/qt/include/qgdict.h
qptr_x11.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
qptr_x11.o: /usr/include/linux/version.h /usr/include/asm/byteorder.h
qptr_x11.o: /usr/include/malloc.h /usr/include/X11/Xlib.h
qptr_x11.o: /usr/include/sys/types.h /usr/include/linux/types.h
qptr_x11.o: /usr/include/asm/types.h /usr/include/X11/X.h
qptr_x11.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
qptr_x11.o: /usr/include/X11/Xutil.h /usr/include/X11/Xos.h
qptr_x11.o: /usr/include/fcntl.h /usr/include/gnu/types.h
qptr_x11.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
qptr_x11.o: /usr/include/unistd.h /usr/include/posix_opt.h
qptr_x11.o: /usr/include/confname.h /usr/include/sys/time.h
qptr_x11.o: /usr/include/linux/time.h /usr/include/sys/time.h
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
qrgn_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qrgn_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qrgn_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qrgn_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qrgn_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qrgn_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qrgn_x11.o: /usr/include/asm/fcntl.h /usr/include/unistd.h
qrgn_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qrgn_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qrgn_x11.o: /usr/include/sys/time.h
qsignal.o: qsignal.h qobject.h qobjdefs.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qglobal.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qstring.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qarray.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qshared.h
qsignal.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qsignal.o: /usr/include/string.h /usr/include/features.h
qsignal.o: /usr/include/sys/cdefs.h qmetaobj.h qconnect.h
qsignal.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
qsignal.o: /usr/include/linux/version.h /usr/include/asm/byteorder.h
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
qwid_x11.o: qwindow.h qwidget.h qwindefs.h qobjdefs.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qglobal.h qobject.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qstring.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qarray.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qshared.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qwid_x11.o: /usr/include/string.h /usr/include/features.h
qwid_x11.o: /usr/include/sys/cdefs.h qpaintd.h qrect.h qsize.h qpoint.h
qwid_x11.o: qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h
qwid_x11.o: qevent.h qapp.h qpaintdc.h qpainter.h qregion.h qpen.h qbrush.h
qwid_x11.o: q2matrix.h qpntarry.h qpixmap.h qwidcoll.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qlist.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qglist.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qcollect.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qintdict.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qgdict.h qobjcoll.h
qwid_x11.o: qmetaobj.h qconnect.h
qwid_x11.o: /home/popcorn/home/hanord/qt/include/qdict.h
qwid_x11.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
qwid_x11.o: /usr/include/linux/types.h /usr/include/asm/types.h
qwid_x11.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
qwid_x11.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
qwid_x11.o: /usr/include/X11/Xos.h /usr/include/fcntl.h
qwid_x11.o: /usr/include/gnu/types.h /usr/include/linux/fcntl.h
qwid_x11.o: /usr/include/asm/fcntl.h /usr/include/unistd.h
qwid_x11.o: /usr/include/posix_opt.h /usr/include/confname.h
qwid_x11.o: /usr/include/sys/time.h /usr/include/linux/time.h
qwid_x11.o: /usr/include/sys/time.h
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
qwidget.o: qpaintd.h qrect.h qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h
qwidget.o: qfont.h qfontinf.h qcursor.h qevent.h qwidcoll.h
qwidget.o: /home/popcorn/home/hanord/qt/include/qintdict.h qkeycode.h qapp.h
qwindow.o: qwindow.h qwidget.h qwindefs.h qobjdefs.h
qwindow.o: /home/popcorn/home/hanord/qt/include/qglobal.h qobject.h
qwindow.o: /home/popcorn/home/hanord/qt/include/qstring.h
qwindow.o: /home/popcorn/home/hanord/qt/include/qarray.h
qwindow.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qwindow.o: /home/popcorn/home/hanord/qt/include/qshared.h
qwindow.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qwindow.o: /usr/include/string.h /usr/include/features.h
qwindow.o: /usr/include/sys/cdefs.h qpaintd.h qrect.h qsize.h qpoint.h
qwindow.o: qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h
qwindow.o: qevent.h qpixmap.h
qregion.o: qregion.h /home/popcorn/home/hanord/qt/include/qshared.h
qregion.o: /home/popcorn/home/hanord/qt/include/qglobal.h qrect.h qsize.h
qregion.o: qpoint.h qwindefs.h qobjdefs.h
qregion.o: /home/popcorn/home/hanord/qt/include/qstring.h
qregion.o: /home/popcorn/home/hanord/qt/include/qarray.h
qregion.o: /home/popcorn/home/hanord/qt/include/qgarray.h
qregion.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
qregion.o: /usr/include/string.h /usr/include/features.h
qregion.o: /usr/include/sys/cdefs.h qpntarry.h
qregion.o: /home/popcorn/home/hanord/qt/include/qbuffer.h
qregion.o: /home/popcorn/home/hanord/qt/include/qiodev.h
qregion.o: /home/popcorn/home/hanord/qt/include/qdstream.h
maccel.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
maccel.o: /home/popcorn/home/hanord/qt/include/qglobal.h
maccel.o: /home/popcorn/home/hanord/qt/include/qstring.h
maccel.o: /home/popcorn/home/hanord/qt/include/qarray.h
maccel.o: /home/popcorn/home/hanord/qt/include/qgarray.h
maccel.o: /home/popcorn/home/hanord/qt/include/qshared.h
maccel.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
maccel.o: /usr/include/string.h /usr/include/features.h
maccel.o: /usr/include/sys/cdefs.h qaccel.h qkeycode.h
mapp.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mapp.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mapp.o: /home/popcorn/home/hanord/qt/include/qstring.h
mapp.o: /home/popcorn/home/hanord/qt/include/qarray.h
mapp.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mapp.o: /home/popcorn/home/hanord/qt/include/qshared.h
mapp.o: /home/popcorn/home/hanord/qt/include/qgeneric.h /usr/include/string.h
mapp.o: /usr/include/features.h /usr/include/sys/cdefs.h qapp.h qwidget.h
mapp.o: qwindefs.h qpaintd.h qrect.h qsize.h qpoint.h qpalette.h qcolor.h
mapp.o: qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h
mdialog.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mdialog.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mdialog.o: /home/popcorn/home/hanord/qt/include/qstring.h
mdialog.o: /home/popcorn/home/hanord/qt/include/qarray.h
mdialog.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mdialog.o: /home/popcorn/home/hanord/qt/include/qshared.h
mdialog.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
mdialog.o: /usr/include/string.h /usr/include/features.h
mdialog.o: /usr/include/sys/cdefs.h qdialog.h qwindow.h qwidget.h qwindefs.h
mdialog.o: qpaintd.h qrect.h qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h
mdialog.o: qfont.h qfontinf.h qcursor.h qevent.h
mtimer.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qstring.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qarray.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qshared.h
mtimer.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
mtimer.o: /usr/include/string.h /usr/include/features.h
mtimer.o: /usr/include/sys/cdefs.h qtimer.h
mwidget.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qstring.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qarray.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qshared.h
mwidget.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
mwidget.o: /usr/include/string.h /usr/include/features.h
mwidget.o: /usr/include/sys/cdefs.h qwidget.h qwindefs.h qpaintd.h qrect.h
mwidget.o: qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h
mwidget.o: qcursor.h qevent.h
mwindow.o: qmetaobj.h qconnect.h qobject.h qobjdefs.h
mwindow.o: /home/popcorn/home/hanord/qt/include/qglobal.h
mwindow.o: /home/popcorn/home/hanord/qt/include/qstring.h
mwindow.o: /home/popcorn/home/hanord/qt/include/qarray.h
mwindow.o: /home/popcorn/home/hanord/qt/include/qgarray.h
mwindow.o: /home/popcorn/home/hanord/qt/include/qshared.h
mwindow.o: /home/popcorn/home/hanord/qt/include/qgeneric.h
mwindow.o: /usr/include/string.h /usr/include/features.h
mwindow.o: /usr/include/sys/cdefs.h qwindow.h qwidget.h qwindefs.h qpaintd.h
mwindow.o: qrect.h qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h qfont.h
mwindow.o: qfontinf.h qcursor.h qevent.h
