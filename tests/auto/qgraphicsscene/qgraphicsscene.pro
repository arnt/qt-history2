load(qttest_p4)
SOURCES  += tst_qgraphicsscene.cpp
RESOURCES += images.qrc
win32: LIBS += -lUser32


DEFINES += QT_NO_CAST_TO_ASCII

DEFINES += QT_USE_USING_NAMESPACE

