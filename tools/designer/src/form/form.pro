TEMPLATE = lib
TARGET = QtForm_debug
QT += xml
CONFIG += qt debug
CONFIG += static
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin

DEFINES += QFORMINTERNAL_NAMESPACE
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
