TEMPLATE = lib
TARGET = QtForm
QT += xml
CONFIG += qt staticlib
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin

CONFIG(debug, debug|release) {
    unix:TARGET = $$member(TARGET, 0)_debug
    else:TARGET = $$member(TARGET, 0)d
}

DEFINES += QFORMINTERNAL_NAMESPACE QT_DESIGNER_STATIC
isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.1.0
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
QMAKE_TARGET_COMPANY = Trolltech AS
QMAKE_TARGET_PRODUCT = Form
QMAKE_TARGET_DESCRIPTION = QForm
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2003-2005 Trolltech

include(../lib/uilib/uilib.pri)

HEADERS += qformloader.h
SOURCES += qformloader.cpp
