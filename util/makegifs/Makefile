####### This section was automatically generated from
#######    /home/hanord/qt/util/Makefile for building on
#######     on ...

INCDIR = $(QTDIR)/include
CFLAGS =  -g -Wall -W -I$(QTDIR)/examples/life
LIBCFLAGS = -fPIC
YACCCFLAGS = -Wno-unused -Wno-parentheses
LFLAGS = -L$(QTDIR)/lib -lqt
CC = gcc
MOC = moc
SHELL =	/bin/sh

####### End of automatically generated section
#
# $Source: /tmp/cvs/qt/util/makegifs/Makefile,v $
#

####### Files

SOURCES	=	makeppms.cpp $(QTDIR)/examples/life/life.cpp
OBJECTS	=	makeppms.o life.o
SRCMETA	=	moc_life.cpp
OBJMETA =	moc_life.o
TARGET	=	makeppms

####### Implicit rules

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all:
	@echo "make gifs  - generate widget gifs in $(QTDIR)/gif/"
	@echo "make $(TARGET)  - generate ppm generator"

gifs: $(TARGET)
	./$(TARGET)
	for i in *.ppm; do ppmquant 256 $$i | \
	    ppmtogif >$(QTDIR)/gif/$${i%ppm}gif; rm $$i; done

$(TARGET): $(SRCMETA) $(OBJECTS) $(OBJMETA)
	$(CC) $(OBJECTS) $(OBJMETA) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	-rm -f *.o *.bak *~ *% *-[wm].ppm
	-rm -f $(SRCMETA) $(TARGET)

####### Explicit dependencies

life.o: $(QTDIR)/examples/life/life.cpp
	$(CC) -o life.o -c $(CFLAGS) -I$(INCDIR) $(QTDIR)/examples/life/life.cpp

####### Meta objects

moc_life.cpp: $(QTDIR)/examples/life/life.h
	$(MOC) $(QTDIR)/examples/life/life.h -o moc_life.cpp

# DO NOT DELETE THIS LINE -- make depend depends on it.

makeppms.o: /usr/include/stdlib.h /usr/include/features.h
makeppms.o: /usr/include/sys/cdefs.h
makeppms.o: /usr/lib/gcc-lib/i386-linux/2.7.2.1/include/stddef.h
makeppms.o: /usr/include/errno.h /usr/include/linux/errno.h
makeppms.o: /usr/include/asm/errno.h /usr/include/alloca.h
makeppms.o: /usr/include/unistd.h /usr/include/posix_opt.h
makeppms.o: /usr/include/gnu/types.h /usr/include/confname.h
makeppms.o: /usr/include/sys/types.h /usr/include/linux/types.h
makeppms.o: /usr/include/linux/posix_types.h /usr/include/asm/posix_types.h
makeppms.o: /usr/include/asm/types.h /usr/include/sys/bitypes.h
makeppms.o: /home/warwick/qt/include/qapp.h
makeppms.o: /home/warwick/qt/include/qwidget.h
makeppms.o: /home/warwick/qt/include/qwindefs.h
makeppms.o: /home/warwick/qt/include/qobjdefs.h
makeppms.o: /home/warwick/qt/include/qglobal.h
makeppms.o: /home/warwick/qt/include/qobject.h
makeppms.o: /home/warwick/qt/include/qstring.h
makeppms.o: /home/warwick/qt/include/qarray.h
makeppms.o: /home/warwick/qt/include/qgarray.h
makeppms.o: /home/warwick/qt/include/qshared.h
makeppms.o: /home/warwick/qt/include/qgeneric.h /usr/include/string.h
makeppms.o: /home/warwick/qt/include/qevent.h
makeppms.o: /home/warwick/qt/include/qrect.h /home/warwick/qt/include/qsize.h
makeppms.o: /home/warwick/qt/include/qpoint.h
makeppms.o: /home/warwick/qt/include/qpaintd.h
makeppms.o: /home/warwick/qt/include/qpalette.h
makeppms.o: /home/warwick/qt/include/qcolor.h
makeppms.o: /home/warwick/qt/include/qcursor.h
makeppms.o: /home/warwick/qt/include/qfont.h
makeppms.o: /home/warwick/qt/include/qfontmet.h
makeppms.o: /home/warwick/qt/include/qfontinf.h
makeppms.o: /home/warwick/qt/include/qmainwindow.h
makeppms.o: /home/warwick/qt/include/qtoolbutton.h
makeppms.o: /home/warwick/qt/include/qbutton.h
makeppms.o: /home/warwick/qt/include/qpixmap.h
makeppms.o: /home/warwick/qt/include/qiconset.h
makeppms.o: /home/warwick/qt/include/qtoolbar.h
makeppms.o: /home/warwick/qt/include/qstatusbar.h
makeppms.o: /home/warwick/qt/include/qchkbox.h
makeppms.o: /home/warwick/qt/include/qpushbt.h
makeppms.o: /home/warwick/qt/include/qradiobt.h
makeppms.o: /home/warwick/qt/include/qcombo.h
makeppms.o: /home/warwick/qt/include/qfiledlg.h
makeppms.o: /home/warwick/qt/include/qdir.h
makeppms.o: /home/warwick/qt/include/qstrlist.h
makeppms.o: /home/warwick/qt/include/qlist.h
makeppms.o: /home/warwick/qt/include/qglist.h
makeppms.o: /home/warwick/qt/include/qcollect.h
makeppms.o: /home/warwick/qt/include/qdstream.h
makeppms.o: /home/warwick/qt/include/qiodev.h
makeppms.o: /home/warwick/qt/include/qfileinf.h
makeppms.o: /home/warwick/qt/include/qfile.h /usr/include/stdio.h
makeppms.o: /usr/include/libio.h /usr/include/_G_config.h
makeppms.o: /home/warwick/qt/include/qdatetm.h
makeppms.o: /home/warwick/qt/include/qdialog.h
makeppms.o: /home/warwick/qt/include/qmsgbox.h
makeppms.o: /home/warwick/qt/include/qprogdlg.h
makeppms.o: /home/warwick/qt/include/qsemimodal.h
makeppms.o: /home/warwick/qt/include/qlabel.h
makeppms.o: /home/warwick/qt/include/qframe.h
makeppms.o: /home/warwick/qt/include/qprogbar.h
makeppms.o: /home/warwick/qt/include/qtabdlg.h
makeppms.o: /home/warwick/qt/include/qgrpbox.h
makeppms.o: /home/warwick/qt/include/qbttngrp.h
makeppms.o: /home/warwick/qt/include/qlcdnum.h
makeppms.o: /home/warwick/qt/include/qbitarry.h
makeppms.o: /home/warwick/qt/include/qmenubar.h
makeppms.o: /home/warwick/qt/include/qpopmenu.h
makeppms.o: /home/warwick/qt/include/qtablevw.h
makeppms.o: /home/warwick/qt/include/qmenudta.h
makeppms.o: /home/warwick/qt/include/qspinbox.h
makeppms.o: /home/warwick/qt/include/qrangect.h
makeppms.o: /home/warwick/qt/include/qlistbox.h
makeppms.o: /home/warwick/qt/include/qmlined.h
makeppms.o: /home/warwick/qt/include/qheader.h
makeppms.o: /home/warwick/qt/include/qlined.h
makeppms.o: /home/warwick/qt/include/qscrbar.h
makeppms.o: /home/warwick/qt/include/qdrawutl.h
makeppms.o: /home/warwick/qt/include/qpainter.h
makeppms.o: /home/warwick/qt/include/qregion.h
makeppms.o: /home/warwick/qt/include/qpen.h /home/warwick/qt/include/qbrush.h
makeppms.o: /home/warwick/qt/include/qpntarry.h
makeppms.o: /home/warwick/qt/include/qwmatrix.h
makeppms.o: /home/warwick/qt/include/qslider.h
makeppms.o: /home/warwick/qt/include/qtabbar.h
makeppms.o: /home/warwick/qt/include/qscrollview.h
makeppms.o: /home/warwick/qt/include/qsplitter.h
makeppms.o: /home/warwick/qt/include/qtstream.h
makeppms.o: /home/warwick/qt/include/qimage.h
makeppms.o: /home/warwick/qt/include/qkeycode.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/examples/life/life.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qframe.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qwidget.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qwindefs.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qobjdefs.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qglobal.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qobject.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qstring.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qarray.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qgarray.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qshared.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qgeneric.h
/home/warwick/qt/examples/life/life.o: /usr/include/string.h
/home/warwick/qt/examples/life/life.o: /usr/include/features.h
/home/warwick/qt/examples/life/life.o: /usr/include/sys/cdefs.h
/home/warwick/qt/examples/life/life.o: /usr/lib/gcc-lib/i386-linux/2.7.2.1/include/stddef.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qevent.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qrect.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qsize.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qpoint.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qpaintd.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qpalette.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qcolor.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qcursor.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qfont.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qfontmet.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qfontinf.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qpainter.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qregion.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qpen.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qbrush.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qpntarry.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qwmatrix.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qdrawutl.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qchkbox.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qbutton.h
/home/warwick/qt/examples/life/life.o: /home/warwick/qt/include/qapp.h
