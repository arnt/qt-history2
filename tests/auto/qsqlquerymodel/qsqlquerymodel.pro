load(qttest_p4)
SOURCES  += tst_qsqlquerymodel.cpp

QT += sql

win32:LIBS += -lws2_32


DEFINES += QT_USE_USING_NAMESPACE

