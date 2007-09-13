load(qttest_p4)
SOURCES  += tst_qpixmap.cpp
DEFINES += SRCDIR=\\\"$$PWD\\\"

win32:LIBS += -lgdi32 -luser32

DEFINES += QT_USE_USING_NAMESPACE

