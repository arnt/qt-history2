TEMPLATE = lib

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
contains(QT_CONFIG, debug):contains(QT_CONFIG, release):CONFIG += debug_and_release build_all
contains(QT_CONFIG, embedded):CONFIG += embedded
!contains(CONFIG, static) {
	CONFIG += dll
	DEFINES += QT_SHARED
}

CONFIG(debug, debug|release) {
    TARGET = QtTest_debug
    OBJECTS_DIR = tmp/debug
} else {
    TARGET = QtTest
    OBJECTS_DIR = tmp/release
}

QT       = core

INCLUDEPATH += .

MOC_DIR         = tmp
DESTDIR = ../../../lib
DLLDESTDIR = ../../../bin
isEmpty(QT_MAJOR_VERSION) {
   VERSION=4.2.0
} else {
   VERSION=$${QT_MAJOR_VERSION}.$${QT_MINOR_VERSION}.$${QT_PATCH_VERSION}
}
QMAKE_TARGET_COMPANY = Trolltech AS
QMAKE_TARGET_PRODUCT = QTestLib
QMAKE_TARGET_DESCRIPTION = Qt Unit Testing Library
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2003-2005 Trolltech

unix {
   CONFIG     += create_libtool create_pc explicitlib
   QMAKE_PKGCONFIG_LIBDIR = $$[QT_INSTALL_LIBS]
   QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_HEADERS]
   QMAKE_PKGCONFIG_DESCRIPTION = Qt Unit Testing Library
   CONFIG(debug, debug|release):QMAKE_PKGCONFIG_NAME = QtTest_debug
   else:QMAKE_PKGCONFIG_NAME = QtTest
}

# Input
HEADERS = qtest_global.h qtestcase.h qtestdata.h
SOURCES = qtestcase.cpp qtestlog.cpp qtesttable.cpp qtestdata.cpp qtestresult.cpp qasciikey.cpp qplaintestlogger.cpp qxmltestlogger.cpp qsignaldumper.cpp qabstracttestlogger.cpp

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII QTESTLIB_MAKEDLL QT_NO_DATASTREAM

embedded:QMAKE_CXXFLAGS+=-fno-rtti

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

include($$QT_SOURCE_TREE/include/QtTest/headers.pri)
qtestlib_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
qtestlib_headers.path = $$[QT_INSTALL_HEADERS]/QtTest
INSTALLS        += qtestlib_headers
