load(qttest_p4)
SOURCES  += ../tst_qtextstream.cpp

TARGET = ../tst_qtextstream

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_qtextstream
} else {
    TARGET = ../../release/tst_qtextstream
  }
}

RESOURCES += ../qtextstream.qrc

QT = core network qt3support

