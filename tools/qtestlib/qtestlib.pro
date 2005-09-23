TEMPLATE = lib
CONFIG   += qt warn_on dll

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
contains(QT_CONFIG, debug):contains(QT_CONFIG, release):CONFIG += debug_and_release build_all
contains(QT_CONFIG, embedded):CONFIG += embedded

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
DESTDIR         = $$[QT_INSTALL_LIBS]
DLLDESTDIR      = $$[QT_INSTALL_BINS]
VERSION         = 4.1.0

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

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII QTESTLIB_MAKEDLL QT_NO_DATASTREAM QTEST_LIGHT QTEST_NOEXITCODE

embedded:QMAKE_CXXFLAGS+=-fno-rtti
