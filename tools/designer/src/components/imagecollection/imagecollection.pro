TEMPLATE = lib
QT += xml
CONFIG += qt
CONFIG += staticlib
INCLUDEPATH += \
    ../../uilib \
    ../../sdk

TARGET = imagecollection
DESTDIR = ../../../lib

LIBS += -L../../../lib -luilib -lQtDesigner

DEFINES += QT_IMAGECOLLECTION_LIBRARY

SOURCES += imagecollection.cpp
HEADERS += imagecollection.h \
    imagecollection_global.h

