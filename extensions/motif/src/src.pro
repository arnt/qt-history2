TEMPLATE = lib
TARGET   = QtMotif
CONFIG  += qt x11

dll:contains(QT_PRODUCT, qt-internal) {
	CONFIG -= staticlib
 	CONFIG += dll
} else {
	CONFIG -= dll
	CONFIG += staticlib
}
DESTDIR  = ../../../lib
VERSION  = 4.0.0
LIBS    += -lXm -lXt

DESTINCDIR = ../../../include/Motif

contains( QT_PRODUCT, qt-(enterprise|internal|eval) ) {
    HEADERS = qmotifdialog.h qmotifwidget.h qmotif.h qxtwidget.h
    SOURCES = qmotifdialog.cpp qmotifwidget.cpp qmotif.cpp qxtwidget.cpp

    include(../../../src/qt_install.pri)
    targ_headers.files += $$HEADERS
} else {
    message( "QMotif requires a Qt/Enterprise edition." )
}


