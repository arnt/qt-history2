load(qttest_p4)
SOURCES         += ../tst_qlibrary.cpp
TARGET  = ../tst_qlibrary
QT = core

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_qlibrary
} else {
    TARGET = ../../release/tst_qlibrary
  }
}

DEFINES += QT_USE_USING_NAMESPACE

