TEMPLATE = lib
TARGET 	 = qmotif

CONFIG  -= dll
CONFIG  += qt release x11 staticlib
DESTDIR  = ../../../lib
VERSION  = 0.0.0
LIBS    += -lXm -lXt

DESTINCDIR = ../../../include

contains( QT_PRODUCT, qt-(enterprise|internal|eval) ) {
    HEADERS = qmotifdialog.h qmotifwidget.h qmotif.h qxtwidget.h
    SOURCES = qmotifdialog.cpp qmotifwidget.cpp qmotif.cpp qxtwidget.cpp

    headers.files = $$HEADERS
    target.path=$$libs.path
    INSTALLS += target headers
} else {
    message( "QMotif requires a Qt/Enterprise edition." )
}
