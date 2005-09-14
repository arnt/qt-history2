TEMPLATE = lib
CONFIG   += qt warn_on dll

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
contains(QT_CONFIG, debug):contains(QT_CONFIG, release):CONFIG += debug_and_release build_all
contains(QT_CONFIG, embedded):CONFIG += embedded

CONFIG(debug, debug|release) {
    TARGET = QTest_debug
    OBJECTS_DIR = tmp/debug
} else {
    TARGET = QTest
    OBJECTS_DIR = tmp/release
}

QT       = core

INCLUDEPATH += .

MOC_DIR         = tmp
DESTDIR         = $$[QT_INSTALL_LIBS]
DLLDESTDIR      = $$[QT_INSTALL_BINS]
VERSION         = 2.0.0

# Input
HEADERS = qtest_global.h qtestcase.h qtestdata.h
SOURCES = qtestcase.cpp qtestlog.cpp qtesttable.cpp qtestdata.cpp qtestresult.cpp qasciikey.cpp qplaintestlogger.cpp qxmltestlogger.cpp qsignaldumper.cpp qabstracttestlogger.cpp

DEFINES += QT_NO_CAST_TO_ASCII QT_NO_CAST_FROM_ASCII QTESTLIB_MAKEDLL QT_NO_DATASTREAM QTEST_LIGHT QTEST_NOEXITCODE

embedded:QMAKE_CXXFLAGS+=-fno-rtti
