#############################################################################
# Qt Makefile - widgets
#############################################################################

#######	Directories

BASEDIR		= $(QTDIR)
INCDIR		= $(BASEDIR)/include
LIBDIR		= $(BASEDIR)/lib

#######	Compiler

#CFLAGS		= -O2 -DNO_DEBUG -DNO_CHECK
CFLAGS		= -O2
CC		= gcc

#######	Files

HEADERS		= qbutton.h	qbttngrp.h	qchkbox.h	qlabel.h      \
		  qlcdnum.h     \
		  qlined.h	qlistbox.h	qmenubar.h	qmenudta.h    \
		  qpopmenu.h	qpushbt.h	qradiobt.h	qrangect.h    \
		  qscrbar.h	qtablew.h	qframe.h	qgrpbox.h     \
		  qcombo.h	qmsgbox.h	qiconbt.h
SOURCES		= qbutton.cpp	qbttngrp.cpp	qchkbox.cpp	qlabel.cpp    \
		  qlcdnum.cpp   \
		  qlined.cpp	qlistbox.cpp	qmenubar.cpp	qmenudta.cpp  \
		  qpopmenu.cpp	qpushbt.cpp	qradiobt.cpp	qrangect.cpp  \
		  qscrbar.cpp	qtablew.cpp	qframe.cpp	qgrpbox.cpp   \
		  qcombo.cpp	qmsgbox.cpp	qiconbt.cpp
OBJECTS		= qbutton.o	qbttngrp.o	qchkbox.o	qlabel.o      \
		  qlcdnum.o     \
		  qlined.o	qlistbox.o	qmenubar.o	qmenudta.o    \
		  qpopmenu.o	qpushbt.o	qradiobt.o	qrangect.o    \
		  qscrbar.o	qtablew.o	qframe.o	qgrpbox.o     \
		  qcombo.o	qmsgbox.o	qiconbt.o
SRCMETA		= mbutton.cpp	mbttngrp.cpp	mchkbox.cpp	mlabel.cpp    \
		  mlcdnum.cpp   miconbt.cpp \
		  mlined.cpp	mlistbox.cpp	mmenubar.cpp	mpopmenu.cpp  \
		  mpushbt.cpp	mradiobt.cpp	mscrbar.cpp	mtablew.cpp   \
		  mframe.cpp	mgrpbox.cpp     mcombo.cpp	mmsgbox.cpp
OBJMETA		= mbutton.o	mbttngrp.o	mchkbox.o	mlabel.o      \
		  mlcdnum.o     miconbt.o\
		  mlined.o	mlistbox.o	mmenubar.o	mpopmenu.o    \
		  mpushbt.o	mradiobt.o	mscrbar.o	mtablew.o     \
		  mframe.o	mgrpbox.o	mcombo.o	mmsgbox.o
TARGET		= $(LIBDIR)/libqwidgets.a

#######	Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

m%.cpp: q%.h
	$(QTDIR)/bin/moc $< -o $@

#######	Build rules

all: $(HEADERS) $(SOURCES) $(TARGET)

obj: $(OBJECTS) $(OBJMETA)

$(TARGET): $(OBJECTS) $(OBJMETA)
	/bin/rm -f $(TARGET)
	ar q $(TARGET) $(OBJECTS) $(OBJMETA)
	ranlib $(TARGET)

rcs:: ./RCS/$$@,v

include:
	ln -s `echo $(HEADERS) | fmt -1 | sed 's-^-../src/widgets/-'` $(INCDIR)

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

#######	Meta classes

# DO NOT DELETE
qbutton.o: qbutton.cpp qbutton.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qbttngrp.h qgrpbox.h qframe.h /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h
qbttngrp.o: qbttngrp.cpp qbttngrp.h qgrpbox.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qbutton.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h
qchkbox.o: qchkbox.cpp qchkbox.h qbutton.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h \
 /home/agulbra/qt/include/qpixmap.h \
 /home/agulbra/qt/include/qpmcache.h
qlabel.o: qlabel.cpp qlabel.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h
qlcdnum.o: qlcdnum.cpp qlcdnum.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qbitarry.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h
qlined.o: qlined.cpp qlined.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h \
 /home/agulbra/qt/include/qpixmap.h \
 /home/agulbra/qt/include/qkeycode.h
qlistbox.o: qlistbox.cpp qlistbox.h qtablew.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h \
 /home/agulbra/qt/include/qstrlist.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h \
 /home/agulbra/qt/include/qkeycode.h \
 /home/agulbra/qt/include/qpixmap.h
qmenubar.o: qmenubar.cpp qmenubar.h qpopmenu.h qtablew.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qmenudta.h /home/agulbra/qt/include/qpixmap.h \
 /home/agulbra/qt/include/qsignal.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qaccel.h /home/agulbra/qt/include/qkeycode.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h /home/agulbra/qt/include/qapp.h \
 /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h
qmenudta.o: qmenudta.cpp qmenudta.h /home/agulbra/qt/include/qglobal.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpixmap.h /home/agulbra/qt/include/qpaintd.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qcolor.h /home/agulbra/qt/include/qsignal.h \
 /home/agulbra/qt/include/qobject.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 qpopmenu.h qtablew.h qframe.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qpalette.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qapp.h
qpopmenu.o: qpopmenu.cpp qpopmenu.h qtablew.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qmenudta.h /home/agulbra/qt/include/qpixmap.h \
 /home/agulbra/qt/include/qsignal.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 qmenubar.h /home/agulbra/qt/include/qaccel.h \
 /home/agulbra/qt/include/qkeycode.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h qscrbar.h qrangect.h \
 /home/agulbra/qt/include/qapp.h
qpushbt.o: qpushbt.cpp qpushbt.h qbutton.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qdialog.h /home/agulbra/qt/include/qview.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h \
 /home/agulbra/qt/include/qpixmap.h \
 /home/agulbra/qt/include/qpmcache.h
qradiobt.o: qradiobt.cpp qradiobt.h qbutton.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h \
 /home/agulbra/qt/include/qpixmap.h \
 /home/agulbra/qt/include/qpmcache.h
qrangect.o: qrangect.cpp qrangect.h
qscrbar.o: qscrbar.cpp qscrbar.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qrangect.h /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h
qtablew.o: qtablew.cpp qtablew.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qscrbar.h qrangect.h /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h
qframe.o: qframe.cpp qframe.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h
qgrpbox.o: qgrpbox.cpp qgrpbox.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h
qcombo.o: qcombo.cpp qcombo.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qpopmenu.h qtablew.h qframe.h qmenudta.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h \
 /home/agulbra/qt/include/qkeycode.h \
 /home/agulbra/qt/include/qstrlist.h /home/agulbra/qt/include/qlist.h \
 /home/agulbra/qt/include/qglist.h /home/agulbra/qt/include/qcollect.h \
 /home/agulbra/qt/include/qdstream.h /home/agulbra/qt/include/qiodev.h \
 /home/agulbra/qt/include/qpixmap.h
qmsgbox.o: qmsgbox.cpp qmsgbox.h /home/agulbra/qt/include/qdialog.h \
 /home/agulbra/qt/include/qview.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qlabel.h qframe.h qpushbt.h qbutton.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h /home/agulbra/qt/include/qapp.h
qiconbt.o: qiconbt.cpp qiconbt.h qpushbt.h qbutton.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qstring.h /home/agulbra/qt/include/qarray.h \
 /home/agulbra/qt/include/qgarray.h /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qpainter.h \
 /home/agulbra/qt/include/qregion.h /home/agulbra/qt/include/qpen.h \
 /home/agulbra/qt/include/qbrush.h /home/agulbra/qt/include/q2matrix.h \
 /home/agulbra/qt/include/qpntarry.h \
 /home/agulbra/qt/include/qpixmap.h \
 /home/agulbra/qt/include/qpmcache.h
mbutton.o: mbutton.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qbutton.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mbttngrp.o: mbttngrp.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qbttngrp.h \
 qgrpbox.h qframe.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mchkbox.o: mchkbox.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qchkbox.h \
 qbutton.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mlabel.o: mlabel.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qlabel.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mlcdnum.o: mlcdnum.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qlcdnum.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 /home/agulbra/qt/include/qbitarry.h
miconbt.o: miconbt.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qiconbt.h \
 qpushbt.h qbutton.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mlined.o: mlined.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qlined.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mlistbox.o: mlistbox.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qlistbox.h \
 qtablew.h qframe.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mmenubar.o: mmenubar.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qmenubar.h \
 qpopmenu.h qtablew.h qframe.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qmenudta.h
mpopmenu.o: mpopmenu.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpopmenu.h \
 qtablew.h qframe.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qmenudta.h
mpushbt.o: mpushbt.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qpushbt.h \
 qbutton.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mradiobt.o: mradiobt.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qradiobt.h \
 qbutton.h /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mscrbar.o: mscrbar.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qscrbar.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h \
 qrangect.h
mtablew.o: mtablew.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qtablew.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mframe.o: mframe.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mgrpbox.o: mgrpbox.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qgrpbox.h qframe.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mcombo.o: mcombo.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qcombo.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
mmsgbox.o: mmsgbox.cpp /home/agulbra/qt/include/qmetaobj.h \
 /home/agulbra/qt/include/qconnect.h \
 /home/agulbra/qt/include/qobject.h \
 /home/agulbra/qt/include/qobjdefs.h \
 /home/agulbra/qt/include/qglobal.h /home/agulbra/qt/include/qstring.h \
 /home/agulbra/qt/include/qarray.h /home/agulbra/qt/include/qgarray.h \
 /home/agulbra/qt/include/qshared.h \
 /home/agulbra/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h qmsgbox.h \
 /home/agulbra/qt/include/qdialog.h /home/agulbra/qt/include/qview.h \
 /home/agulbra/qt/include/qwidget.h \
 /home/agulbra/qt/include/qwindefs.h \
 /home/agulbra/qt/include/qpaintd.h /home/agulbra/qt/include/qrect.h \
 /home/agulbra/qt/include/qsize.h /home/agulbra/qt/include/qpoint.h \
 /home/agulbra/qt/include/qpalette.h /home/agulbra/qt/include/qcolor.h \
 /home/agulbra/qt/include/qfontmet.h /home/agulbra/qt/include/qfont.h \
 /home/agulbra/qt/include/qfontinf.h \
 /home/agulbra/qt/include/qcursor.h /home/agulbra/qt/include/qevent.h
