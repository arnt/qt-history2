TEMPLATE = lib
CONFIG += qt release x11
TARGET = qmotif
DESTINCDIR = ../../../include
DESTDIR = ../../../lib
VERSION = 0.0.0
LIBS += -lXm

!contains( QT_PRODUCT, qt-enterprise ) {
    message( "QMotif requires a Qt/Enterprise edition." )
}
contains( QT_PRODUCT, qt-enterprise ) {
    HEADERS = qmotifdialog.h qmotifeventloop.h qmotifwidget.h qmotif.h qmotif_p.h
    SOURCES = qmotifdialog.cpp qmotifeventloop.cpp qmotifwidget.cpp qmotif.cpp
}
