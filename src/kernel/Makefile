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
		  qsize.h	qtimer.h	qview.h		qwidcoll.h    \
		  qwidget.h	qwindefs.h	q2matrix.h	qpalette.h    \
		  qaccel.h	qdialog.h	qimage.h	qpmcache.h
SOURCES		= qapp.cpp	qapp_x11.cpp	qbitmap.cpp	qcol_x11.cpp  \
		  qcolor.cpp	qconnect.cpp	qcur_x11.cpp	qcursor.cpp   \
		  qevent.cpp	qfnt_x11.cpp	qfont.cpp		      \
		  qmetaobj.cpp	qobject.cpp	qpainter.cpp	qpdevmet.cpp  \
		  qpic_x11.cpp	qpicture.cpp	qpixmap.cpp	qpm_x11.cpp  \
		  qpntarry.cpp	qpoint.cpp	qprinter.cpp	\
		  qptd_x11.cpp	qptr_x11.cpp	qrect.cpp	qrgn_x11.cpp  \
		  qsignal.cpp	qsize.cpp	qtimer.cpp	qview.cpp     \
		  qwid_x11.cpp	qwidget.cpp	q2matrix.cpp	qpalette.cpp  \
		  qaccel.cpp	qdialog.cpp	qimage.cpp	qpmcache.cpp
OBJECTS		= qapp.o	qapp_x11.o	qbitmap.o	qcol_x11.o    \
		  qcolor.o	qconnect.o	qcur_x11.o	qcursor.o     \
		  qevent.o	qfnt_x11.o	qfont.o			      \
		  qmetaobj.o	qobject.o	qpainter.o	qpdevmet.o    \
		  qpic_x11.o	qpicture.o	qpixmap.o	qpm_x11.o    \
		  qpntarry.o	qpoint.o	qprinter.o	    \
		  qptd_x11.o	qptr_x11.o	qrect.o		qrgn_x11.o    \
		  qsignal.o	qsize.o		qtimer.o	qview.o	      \
		  qwid_x11.o	qwidget.o	q2matrix.o	qpalette.o    \
		  qaccel.o	qdialog.o	qimage.o	qpmcache.o
SRCMETA		= maccel.cpp	mdialog.cpp	mtimer.cpp	mview.cpp     \
		  mwidget.cpp
OBJMETA		= maccel.o	mdialog.o	mtimer.o	mview.o	      \
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

mview.cpp: qview.h
	$(QTDIR)/bin/moc qview.h -o mview.cpp

mwidget.cpp: qwidget.h
	$(QTDIR)/bin/moc qwidget.h -o mwidget.cpp


# DO NOT DELETE
qapp.o: qapp.cpp qapp.h qwidget.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpaintd.h qrect.h \
 qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h \
 qcursor.h qevent.h qobjcoll.h qmetaobj.h qconnect.h \
 /home/agulbra/qt/include/qlist.h /home/agulbra/qt/include/qglist.h \
 /home/agulbra/qt/include/qcollect.h /home/agulbra/qt/include/qdict.h \
 /home/agulbra/qt/include/qgdict.h qwidcoll.h \
 /home/agulbra/qt/include/qintdict.h
qapp_x11.o: qapp_x11.cpp qapp.h qwidget.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpaintd.h qrect.h \
 qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h \
 qcursor.h qevent.h qwidcoll.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qintdict.h /home/agulbra/qt/include/qgdict.h \
 qpainter.h qregion.h qpen.h qbrush.h q2matrix.h qpntarry.h qpmcache.h \
 qpixmap.h /usr/include/stdlib.h /usr/include/errno.h \
 /usr/include/linux/errno.h /usr/include/asm/errno.h \
 /usr/include/alloca.h /usr/include/signal.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h \
 /usr/include/locale.h /usr/include/X11/Xlib.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h \
 /usr/include/X11/Xatom.h /home/agulbra/qt/include/qmemchk.h \
 /usr/include/X11/keysymdef.h qkeycode.h
qbitmap.o: qbitmap.cpp qbitmap.h qpixmap.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/agulbra/qt/include/qglobal.h qrect.h qsize.h \
 qpoint.h qcolor.h /home/agulbra/qt/include/qshared.h
qcol_x11.o: qcol_x11.cpp qcolor.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /usr/include/X11/Xlib.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/asm/types.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h \
 /home/agulbra/qt/include/qintdict.h /home/agulbra/qt/include/qgdict.h \
 /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qgeneric.h
qcolor.o: qcolor.cpp qcolor.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qconnect.o: qconnect.cpp qconnect.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h
qcur_x11.o: qcur_x11.cpp qcursor.h qpoint.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qshared.h \
 qapp.h qwidget.h qobject.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpaintd.h qrect.h \
 qsize.h qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h qevent.h \
 qbitmap.h qpixmap.h /home/agulbra/qt/include/qdstream.h \
 /home/agulbra/qt/include/qiodev.h /home/agulbra/qt/include/qmemchk.h \
 /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
 /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
 /usr/include/X11/Xos.h /usr/include/fcntl.h /usr/include/gnu/types.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h \
 /usr/include/X11/cursorfont.h
qcursor.o: qcursor.cpp qcursor.h qpoint.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qevent.o: qevent.cpp qevent.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h
qfnt_x11.o: qfnt_x11.cpp /usr/include/X11/Xlib.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/asm/types.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h \
 /usr/include/X11/Xatom.h qfont.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h qfontdta.h qfontmet.h qrect.h \
 qsize.h qpoint.h qfontinf.h qpainter.h qpaintd.h qcolor.h qregion.h \
 qpen.h qbrush.h q2matrix.h qpntarry.h /usr/include/ctype.h \
 /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h \
 /home/agulbra/qt/include/qcache.h /home/agulbra/qt/include/qgcache.h \
 /home/agulbra/qt/include/qcollect.h /home/agulbra/qt/include/qglist.h \
 /home/agulbra/qt/include/qgdict.h /home/agulbra/qt/include/qdict.h
qfont.o: qfont.cpp qfont.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qfontdta.h \
 qfontmet.h qrect.h qsize.h qpoint.h qfontinf.h qpainter.h qpaintd.h \
 qcolor.h qregion.h qpen.h qbrush.h q2matrix.h qpntarry.h qwidcoll.h \
 qwidget.h qobject.h qpalette.h qcursor.h qevent.h \
 /home/agulbra/qt/include/qlist.h /home/agulbra/qt/include/qglist.h \
 /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qintdict.h /home/agulbra/qt/include/qgdict.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qmetaobj.o: qmetaobj.cpp qmetaobj.h qconnect.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qobjcoll.h \
 /home/agulbra/qt/include/qlist.h /home/agulbra/qt/include/qglist.h \
 /home/agulbra/qt/include/qcollect.h /home/agulbra/qt/include/qdict.h \
 /home/agulbra/qt/include/qgdict.h /home/agulbra/qt/include/qstrlist.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qobject.o: qobject.cpp qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qobjcoll.h \
 qmetaobj.h qconnect.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qdict.h /home/agulbra/qt/include/qgdict.h \
 /home/agulbra/qt/include/qregexp.h /usr/include/ctype.h \
 /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h
qpainter.o: qpainter.cpp qpainter.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h qcolor.h \
 qfontmet.h qfont.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qfontinf.h \
 qregion.h qpen.h qbrush.h q2matrix.h qpntarry.h qpaintdc.h qbitmap.h \
 qpixmap.h /home/agulbra/qt/include/qstack.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qpdevmet.o: qpdevmet.cpp qpdevmet.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qpaintd.h qrect.h qsize.h qpoint.h \
 qpaintdc.h
qpic_x11.o: qpic_x11.cpp qpicture.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 /home/agulbra/qt/include/qbuffer.h /home/agulbra/qt/include/qiodev.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
 /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
 /usr/include/X11/Xos.h /usr/include/fcntl.h /usr/include/gnu/types.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h
qpicture.o: qpicture.cpp qpicture.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 /home/agulbra/qt/include/qbuffer.h /home/agulbra/qt/include/qiodev.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpaintdc.h \
 qpainter.h qcolor.h qfontmet.h qfont.h qfontinf.h qregion.h qpen.h \
 qbrush.h q2matrix.h qpntarry.h qpixmap.h \
 /home/agulbra/qt/include/qfile.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /home/agulbra/qt/include/qdstream.h
qpixmap.o: qpixmap.cpp qpixmap.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h qcolor.h \
 /home/agulbra/qt/include/qshared.h qimage.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qpm_x11.o: qpm_x11.cpp qpixmap.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h qcolor.h \
 /home/agulbra/qt/include/qshared.h qimage.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpaintdc.h \
 q2matrix.h qpntarry.h qapp.h qwidget.h qobject.h qpalette.h \
 qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h \
 /usr/include/malloc.h /usr/include/X11/Xlib.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/asm/types.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h
qpntarry.o: qpntarry.cpp qpntarry.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qglobal.h \
 /home/agulbra/qt/include/qgeneric.h qpoint.h qwindefs.h qobjdefs.h \
 qrect.h qsize.h /home/agulbra/qt/include/qbitarry.h \
 /home/agulbra/qt/include/qstring.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stdarg.h
qpoint.o: qpoint.cpp qpoint.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qprinter.o: qprinter.cpp qprinter.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h
qptd_x11.o: qptd_x11.cpp qpaintd.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 qpaintdc.h qwidget.h qobject.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpalette.h \
 qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h qpixmap.h \
 qapp.h /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
 /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
 /usr/include/X11/Xos.h /usr/include/fcntl.h /usr/include/gnu/types.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h
qptr_x11.o: qptr_x11.cpp qpainter.h qpaintd.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h qcolor.h \
 qfontmet.h qfont.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qfontinf.h \
 qregion.h qpen.h qbrush.h q2matrix.h qpntarry.h qpaintdc.h qwidget.h \
 qobject.h qpalette.h qcursor.h qevent.h qbitmap.h qpixmap.h \
 qpmcache.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qintdict.h /home/agulbra/qt/include/qgdict.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h \
 /usr/include/malloc.h /usr/include/X11/Xlib.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/asm/types.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h
qrect.o: qrect.cpp qrect.h qsize.h qpoint.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qrgn_x11.o: qrgn_x11.cpp qregion.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qglobal.h qrect.h qsize.h qpoint.h \
 qwindefs.h qobjdefs.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpntarry.h \
 /home/agulbra/qt/include/qbuffer.h /home/agulbra/qt/include/qiodev.h \
 /home/agulbra/qt/include/qdstream.h /usr/include/X11/Xlib.h \
 /usr/include/sys/types.h /usr/include/linux/types.h \
 /usr/include/asm/types.h /usr/include/X11/X.h \
 /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h \
 /usr/include/X11/Xutil.h /usr/include/X11/Xos.h /usr/include/fcntl.h \
 /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/unistd.h \
 /usr/include/posix_opt.h /usr/include/confname.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h
qsignal.o: qsignal.cpp qsignal.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qmetaobj.h \
 qconnect.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h /usr/include/linux/version.h \
 /usr/include/asm/byteorder.h
qsize.o: qsize.cpp qsize.h qpoint.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qtimer.o: qtimer.cpp qtimer.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qevent.h \
 qwindefs.h qrect.h qsize.h qpoint.h
qview.o: qview.cpp qview.h qwidget.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpaintd.h qrect.h \
 qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h \
 qcursor.h qevent.h qpixmap.h
qwid_x11.o: qwid_x11.cpp qview.h qwidget.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpaintd.h qrect.h \
 qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h \
 qcursor.h qevent.h qapp.h qpaintdc.h qpainter.h qregion.h qpen.h \
 qbrush.h q2matrix.h qpntarry.h qpixmap.h qwidcoll.h \
 /home/agulbra/qt/include/qlist.h /home/agulbra/qt/include/qglist.h \
 /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qintdict.h /home/agulbra/qt/include/qgdict.h \
 qobjcoll.h qmetaobj.h qconnect.h /home/agulbra/qt/include/qdict.h \
 /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
 /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
 /usr/include/X11/Xos.h /usr/include/fcntl.h /usr/include/gnu/types.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h
qwidget.o: qwidget.cpp qobjcoll.h qmetaobj.h qconnect.h qobject.h \
 qobjdefs.h /home/agulbra/qt/include/qglobal.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qlist.h /home/agulbra/qt/include/qglist.h \
 /home/agulbra/qt/include/qcollect.h /home/agulbra/qt/include/qdict.h \
 /home/agulbra/qt/include/qgdict.h qwidget.h qwindefs.h qpaintd.h \
 qrect.h qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h qfont.h \
 qfontinf.h qcursor.h qevent.h qwidcoll.h \
 /home/agulbra/qt/include/qintdict.h qkeycode.h qapp.h
q2matrix.o: q2matrix.cpp q2matrix.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qpntarry.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h qpoint.h qrect.h qsize.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qpalette.o: qpalette.cpp qpalette.h qwindefs.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h qcolor.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h
qaccel.o: qaccel.cpp qaccel.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qkeycode.h qapp.h \
 qwidget.h qwindefs.h qpaintd.h qrect.h qsize.h qpoint.h qpalette.h \
 qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h \
 /home/agulbra/qt/include/qlist.h /home/agulbra/qt/include/qglist.h \
 /home/agulbra/qt/include/qcollect.h qsignal.h
qdialog.o: qdialog.cpp qdialog.h qview.h qwidget.h qwindefs.h \
 qobjdefs.h /home/agulbra/qt/include/qglobal.h qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpaintd.h qrect.h \
 qsize.h qpoint.h qpalette.h qcolor.h qfontmet.h qfont.h qfontinf.h \
 qcursor.h qevent.h /home/agulbra/qt/include/qpushbt.h \
 /home/agulbra/qt/include/qbutton.h /home/agulbra/qt/include/qwidget.h \
 qapp.h qkeycode.h qobjcoll.h qmetaobj.h qconnect.h \
 /home/agulbra/qt/include/qlist.h /home/agulbra/qt/include/qglist.h \
 /home/agulbra/qt/include/qcollect.h /home/agulbra/qt/include/qdict.h \
 /home/agulbra/qt/include/qgdict.h
qimage.o: qimage.cpp qimage.h qpixmap.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/agulbra/qt/include/qglobal.h qrect.h qsize.h \
 qpoint.h qcolor.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qregexp.h /home/agulbra/qt/include/qfile.h \
 /home/agulbra/qt/include/qiodev.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qintdict.h /home/agulbra/qt/include/qgdict.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/ctype.h \
 /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h \
 /usr/include/X11/Xlib.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h \
 /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h \
 /usr/include/X11/Xos.h /usr/include/fcntl.h /usr/include/gnu/types.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/unistd.h /usr/include/posix_opt.h \
 /usr/include/confname.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/time.h \
 /usr/include/sys/param.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix1_lim.h \
 /usr/include/linux/limits.h /usr/include/posix2_lim.h \
 /usr/include/linux/param.h /usr/include/asm/param.h
qpmcache.o: qpmcache.cpp qpmcache.h qpixmap.h qpaintd.h qwindefs.h \
 qobjdefs.h /home/agulbra/qt/include/qglobal.h qrect.h qsize.h \
 qpoint.h qcolor.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qcache.h /home/agulbra/qt/include/qgcache.h \
 /home/agulbra/qt/include/qcollect.h /home/agulbra/qt/include/qglist.h \
 /home/agulbra/qt/include/qgdict.h /home/agulbra/qt/include/qgeneric.h
maccel.o: maccel.cpp qmetaobj.h qconnect.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qaccel.h \
 qkeycode.h
mdialog.o: mdialog.cpp qmetaobj.h qconnect.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qdialog.h qview.h \
 qwidget.h qwindefs.h qpaintd.h qrect.h qsize.h qpoint.h qpalette.h \
 qcolor.h qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h
mtimer.o: mtimer.cpp qmetaobj.h qconnect.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qtimer.h
mview.o: mview.cpp qmetaobj.h qconnect.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qview.h qwidget.h \
 qwindefs.h qpaintd.h qrect.h qsize.h qpoint.h qpalette.h qcolor.h \
 qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h
mwidget.o: mwidget.cpp qmetaobj.h qconnect.h qobject.h qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qwidget.h \
 qwindefs.h qpaintd.h qrect.h qsize.h qpoint.h qpalette.h qcolor.h \
 qfontmet.h qfont.h qfontinf.h qcursor.h qevent.h
