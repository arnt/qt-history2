#############################################################################
# Qt Makefile - Tutorial 13
#############################################################################

####### Directories

BASEDIR	=	$(QTDIR)
INCDIR	=	$(BASEDIR)/include
LIBDIR	=	$(BASEDIR)/lib

####### Compiler

CFLAGS	=	-O2
LFLAGS	=	-L$(LIBDIR) -lqt -lX11 -lm
CC	=	gcc

####### Files

HEADERS	=	lcdrange.h cannon.h gamebrd.h
SOURCES	=	lcdrange.cpp cannon.cpp gamebrd.cpp
OBJECTS	=	lcdrange.o cannon.o gamebrd.o t14.o
SRCMETA	=       mlcdrang.cpp mcannon.cpp mgamebr.cpp
OBJMETA	=	mlcdrang.o mcannon.o mgamebr.o
TARGET	=	t14

####### Implicit rules

.SUFFIXES:
.SUFFIXES: .cpp $(SUFFIXES)

.cpp.o:
	$(CC) -c $(CFLAGS) -I$(INCDIR) $<

####### Build rules

all: qtdirtest $(TARGET)

qtdirtest:
	@if [ "$(QTDIR)" = "" ]; then			\
	    make "QTDIR=/usr/local/qt";			\
	    exit;					\
	fi

$(TARGET): $(OBJECTS) $(OBJMETA)
	$(CC) $(OBJECTS) $(OBJMETA) -o $(TARGET) $(LFLAGS)

depend:
	@makedepend -I$(INCDIR) $(SOURCES) 2> /dev/null

clean:
	/bin/rm -f *.o *.bak *~ *% #*
	/bin/rm -f $(SRCMETA) $(TARGET)

####### Meta classes

mlcdrang.cpp: lcdrange.h
	moc -o $@ $<

mcannon.cpp: cannon.h
	moc -o $@ $<

mgamebr.cpp: gamebrd.h
	moc -o $@ $<
 

# DO NOT DELETE THIS LINE -- make depend depends on it.
lcdrange.o: lcdrange.cpp lcdrange.h /home/eiriken/qt/include/qscrbar.h \
 /home/eiriken/qt/include/qwidget.h \
 /home/eiriken/qt/include/qwindefs.h \
 /home/eiriken/qt/include/qobjdefs.h \
 /home/eiriken/qt/include/qglobal.h /home/eiriken/qt/include/qobject.h \
 /home/eiriken/qt/include/qstring.h /home/eiriken/qt/include/qarray.h \
 /home/eiriken/qt/include/qgarray.h /home/eiriken/qt/include/qshared.h \
 /home/eiriken/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/eiriken/qt/include/qevent.h /home/eiriken/qt/include/qrect.h \
 /home/eiriken/qt/include/qsize.h /home/eiriken/qt/include/qpoint.h \
 /home/eiriken/qt/include/qpaintd.h \
 /home/eiriken/qt/include/qpalette.h /home/eiriken/qt/include/qcolor.h \
 /home/eiriken/qt/include/qcursor.h /home/eiriken/qt/include/qfont.h \
 /home/eiriken/qt/include/qfontmet.h \
 /home/eiriken/qt/include/qfontinf.h \
 /home/eiriken/qt/include/qrangect.h \
 /home/eiriken/qt/include/qlcdnum.h /home/eiriken/qt/include/qframe.h \
 /home/eiriken/qt/include/qbitarry.h /home/eiriken/qt/include/qlabel.h
cannon.o: cannon.cpp cannon.h /home/eiriken/qt/include/qframe.h \
 /home/eiriken/qt/include/qwidget.h \
 /home/eiriken/qt/include/qwindefs.h \
 /home/eiriken/qt/include/qobjdefs.h \
 /home/eiriken/qt/include/qglobal.h /home/eiriken/qt/include/qobject.h \
 /home/eiriken/qt/include/qstring.h /home/eiriken/qt/include/qarray.h \
 /home/eiriken/qt/include/qgarray.h /home/eiriken/qt/include/qshared.h \
 /home/eiriken/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/eiriken/qt/include/qevent.h /home/eiriken/qt/include/qrect.h \
 /home/eiriken/qt/include/qsize.h /home/eiriken/qt/include/qpoint.h \
 /home/eiriken/qt/include/qpaintd.h \
 /home/eiriken/qt/include/qpalette.h /home/eiriken/qt/include/qcolor.h \
 /home/eiriken/qt/include/qcursor.h /home/eiriken/qt/include/qfont.h \
 /home/eiriken/qt/include/qfontmet.h \
 /home/eiriken/qt/include/qfontinf.h \
 /home/eiriken/qt/include/qscrbar.h \
 /home/eiriken/qt/include/qrangect.h \
 /home/eiriken/qt/include/qlcdnum.h \
 /home/eiriken/qt/include/qbitarry.h \
 /home/eiriken/qt/include/qpainter.h \
 /home/eiriken/qt/include/qregion.h /home/eiriken/qt/include/qpen.h \
 /home/eiriken/qt/include/qbrush.h /home/eiriken/qt/include/qpntarry.h \
 /home/eiriken/qt/include/qwmatrix.h \
 /home/eiriken/qt/include/qpixmap.h /usr/include/math.h \
 /usr/include/huge_val.h /usr/include/endian.h /usr/include/bytesex.h \
 /usr/include/linux/version.h /usr/include/asm/byteorder.h \
 /usr/include/nan.h /usr/lib/gcc-lib/i486-linux/2.7.0/include/float.h \
 /usr/include/values.h /usr/include/stdlib.h /usr/include/errno.h \
 /usr/include/linux/errno.h /usr/include/alloca.h \
 /home/eiriken/qt/include/qtstream.h /home/eiriken/qt/include/qiodev.h \
 /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
gamebrd.o: gamebrd.cpp /home/eiriken/qt/include/qfont.h \
 /home/eiriken/qt/include/qwindefs.h \
 /home/eiriken/qt/include/qobjdefs.h \
 /home/eiriken/qt/include/qglobal.h /home/eiriken/qt/include/qstring.h \
 /home/eiriken/qt/include/qarray.h /home/eiriken/qt/include/qgarray.h \
 /home/eiriken/qt/include/qshared.h \
 /home/eiriken/qt/include/qgeneric.h /usr/include/string.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/lib/gcc-lib/i486-linux/2.7.0/include/stddef.h \
 /home/eiriken/qt/include/qapp.h /home/eiriken/qt/include/qwidget.h \
 /home/eiriken/qt/include/qobject.h /home/eiriken/qt/include/qevent.h \
 /home/eiriken/qt/include/qrect.h /home/eiriken/qt/include/qsize.h \
 /home/eiriken/qt/include/qpoint.h /home/eiriken/qt/include/qpaintd.h \
 /home/eiriken/qt/include/qpalette.h /home/eiriken/qt/include/qcolor.h \
 /home/eiriken/qt/include/qcursor.h \
 /home/eiriken/qt/include/qfontmet.h \
 /home/eiriken/qt/include/qfontinf.h /home/eiriken/qt/include/qlabel.h \
 /home/eiriken/qt/include/qframe.h /home/eiriken/qt/include/qkeycode.h \
 gamebrd.h /home/eiriken/qt/include/qpushbt.h \
 /home/eiriken/qt/include/qbutton.h lcdrange.h \
 /home/eiriken/qt/include/qscrbar.h \
 /home/eiriken/qt/include/qrangect.h \
 /home/eiriken/qt/include/qlcdnum.h \
 /home/eiriken/qt/include/qbitarry.h cannon.h
