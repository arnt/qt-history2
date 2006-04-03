TEMPLATE = lib

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
contains(QT_CONFIG, debug):contains(QT_CONFIG, release):CONFIG += debug_and_release build_all
contains(QT_CONFIG, embedded):CONFIG += embedded
!contains(CONFIG, static) {
	CONFIG += dll
	DEFINES += QT_SHARED
}

CONFIG(debug, debug|release) {
    unix:TARGET = QtTest_debug
    else:TARGET = QtTestd
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
QMAKE_TARGET_COPYRIGHT = Copyright (C) 2003-2006 Trolltech AS

unix {
   CONFIG     += create_libtool create_pc explicitlib
   QMAKE_PKGCONFIG_LIBDIR = $$[QT_INSTALL_LIBS]
   QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_HEADERS]
   QMAKE_PKGCONFIG_DESCRIPTION = Qt Unit Testing Library
   CONFIG(debug, debug|release):QMAKE_PKGCONFIG_NAME = QtTest_debug
   else:QMAKE_PKGCONFIG_NAME = QtTest
}


#load up the headers info
CONFIG += qt_install_headers
HEADERS_PRI = $$QT_BUILD_TREE/include/QtTest/headers.pri
include($$HEADERS_PRI)|clear(HEADERS_PRI)

#mac frameworks
mac:!static:contains(QT_CONFIG, qt_framework) {
   QMAKE_FRAMEWORK_BUNDLE_NAME = $$TARGET
   CONFIG += lib_bundle qt_no_framework_direct_includes qt_framework
   CONFIG(debug, debug|release) {
      !build_pass:CONFIG += build_all
   } else { #release
      !debug_and_release|build_pass {
	  CONFIG -= qt_install_headers #no need to install these as well
	  FRAMEWORK_HEADERS.version = Versions
	  FRAMEWORK_HEADERS.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
      	  FRAMEWORK_HEADERS.path = Headers
      }
      QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
   }
}

# Input
HEADERS = qtest_global.h qtestcase.h qtestdata.h
SOURCES = qtestcase.cpp qtestlog.cpp qtesttable.cpp qtestdata.cpp qtestresult.cpp qasciikey.cpp qplaintestlogger.cpp qxmltestlogger.cpp qsignaldumper.cpp qabstracttestlogger.cpp

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII QTESTLIB_MAKEDLL QT_NO_DATASTREAM

embedded:QMAKE_CXXFLAGS+=-fno-rtti

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

qt_install_headers {
    qtestlib_headers.files = $$SYNCQT.HEADER_FILES $$SYNCQT.HEADER_CLASSES
    qtestlib_headers.path = $$[QT_INSTALL_HEADERS]/QtTest
    INSTALLS        += qtestlib_headers
}
