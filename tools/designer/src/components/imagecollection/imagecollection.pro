TEMPLATE = lib
QT += xml
CONFIG += qt
CONFIG += staticlib
INCLUDEPATH += \
    ../../lib/uilib \
    ../../lib/sdk

TARGET = imagecollection
DESTDIR = ../../../lib

DEFINES += QT_IMAGECOLLECTION_LIBRARY

SOURCES += imagecollection.cpp
HEADERS += imagecollection.h \
    imagecollection_global.h

include(../../sharedcomponents.pri)
