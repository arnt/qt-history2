TEMPLATE = lib
TARGET   = qmotif
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

    headers.files = $$HEADERS
    target.path=$$[QT_INSTALL_LIBS]
    INSTALLS += target headers
} else {
    message( "QMotif requires a Qt/Enterprise edition." )
}
