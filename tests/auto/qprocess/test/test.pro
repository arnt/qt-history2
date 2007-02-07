load(qttest_p4)

SOURCES += ../tst_qprocess.cpp
TARGET = ../tst_qprocess

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_qprocess
} else {
    TARGET = ../../release/tst_qprocess
  }
}

QT = core network
embedded: QT += gui
