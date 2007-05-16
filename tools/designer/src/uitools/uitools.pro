TEMPLATE = lib
TARGET = $$qtLibraryTarget(QtUiTools)
QT += xml
CONFIG += qt staticlib
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin

win32|mac:!macx-xcode:CONFIG += debug_and_release build_all

DEFINES += QFORMINTERNAL_NAMESPACE QT_DESIGNER_STATIC QT_FORMBUILDER_NO_SCRIPT
isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.3.0
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
QMAKE_TARGET_COMPANY = Trolltech ASA
QMAKE_TARGET_PRODUCT = UiLoader
QMAKE_TARGET_DESCRIPTION = QUiLoader
QMAKE_TARGET_COPYRIGHT = Copyright (C) 2003-2006 Trolltech ASA

include(../lib/uilib/uilib.pri)

HEADERS += quiloader.h
SOURCES += quiloader.cpp

include($$QT_SOURCE_TREE/include/QtUiTools/headers.pri)
quitools_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
quitools_headers.path = $$[QT_INSTALL_HEADERS]/QtUiTools
INSTALLS        += quitools_headers

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

unix {
   CONFIG     += create_pc
   QMAKE_PKGCONFIG_LIBDIR = $$[QT_INSTALL_LIBS]
   QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_HEADERS]/$$TARGET
   QMAKE_PKGCONFIG_CFLAGS = -I$$[QT_INSTALL_HEADERS]
   QMAKE_PKGCONFIG_DESTDIR = pkgconfig
}

