TEMPLATE = lib
TARGET = QtUiTools
QT += xml
CONFIG += qt staticlib
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin
CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    unix:TARGET = $$member(TARGET, 0)_debug
    else:TARGET = $$member(TARGET, 0)d
}

DEFINES += QUILOADERINTERNAL_NAMESPACE QT_DESIGNER_STATIC
isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.2.0
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
QMAKE_TARGET_COMPANY = Trolltech AS
QMAKE_TARGET_PRODUCT = UiLoader
QMAKE_TARGET_DESCRIPTION = QUiLoader
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2003-2005 Trolltech

include(../lib/uilib/uilib.pri)

HEADERS += quiloader.h
SOURCES += quiloader.cpp

include($$QT_SOURCE_TREE/include/QtUiTools/headers.pri)
quitools_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
quitools_headers.path = $$[QT_INSTALL_HEADERS]/QtUiTools
INSTALLS        += quitools_headers
