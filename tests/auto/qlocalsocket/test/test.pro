load(qttest_p4)

include(../src/src.pri)

DEFINES += QLOCALSERVER_DEBUG

SOURCES += ../tst_qlocalsocket.cpp
TARGET = ../tst_qlocalsocket

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_qlocalsocket
} else {
    TARGET = ../../release/tst_qlocalsocket
  }
}



DEFINES += QT_USE_USING_NAMESPACE

