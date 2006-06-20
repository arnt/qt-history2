load(qttest_p4)
SOURCES  += tst_qsqlquery.cpp

QT = core sql


win32:LIBS += -lws2_32
