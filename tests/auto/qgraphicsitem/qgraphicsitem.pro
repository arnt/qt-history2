load(qttest_p4)
SOURCES  += tst_qgraphicsitem.cpp
DEFINES += QT_NO_CAST_TO_ASCII

win32: LIBS += User32.lib
