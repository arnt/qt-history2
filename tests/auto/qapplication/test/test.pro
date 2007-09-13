load(qttest_p4)

SOURCES += ../tst_qapplication.cpp
TARGET = ../tst_qapplication

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_qapplication
} else {
    TARGET = ../../release/tst_qapplication
  }
}

DEFINES += QT_USE_USING_NAMESPACE

