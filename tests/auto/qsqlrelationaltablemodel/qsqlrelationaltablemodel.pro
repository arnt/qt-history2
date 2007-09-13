load(qttest_p4)
SOURCES  += tst_qsqlrelationaltablemodel.cpp

QT += sql
win32-g++ {
     LIBS += -lws2_32
} else:win32 {
  LIBS += ws2_32.lib
}

DEFINES += QT_USE_USING_NAMESPACE

