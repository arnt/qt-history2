load(qttest_p4)
SOURCES  += ../tst_qfile.cpp
QT = core network
RESOURCES      += ../qfile.qrc

TARGET = ../tst_qfile

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_qfile
} else {
    TARGET = ../../release/tst_qfile
  }
  LIBS+=ole32.lib
}
