load(qttest_p4)
SOURCES  += tst_qgraphicsscene.cpp
RESOURCES += images.qrc
win32: LIBS += User32.lib


DEFINES += QT_NO_CAST_TO_ASCII
