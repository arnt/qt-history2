TEMPLATE = lib
CONFIG -= dll
CONFIG += qt release x11 staticlib
TARGET = qmotif
DESTINCDIR = ../../../include
DESTDIR = ../../../lib
VERSION = 0.0.0
LIBS += -lXm -lXt

contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    HEADERS = qmotifdialog.h qmotifwidget.h qmotif.h qxtwidget.h
    SOURCES = qmotifdialog.cpp qmotifwidget.cpp qmotif.cpp qxtwidget.cpp

    headers.files = $$HEADERS
    INSTALLS += target headers
} else {
    message( "QMotif requires a Qt/Enterprise edition." )
}
