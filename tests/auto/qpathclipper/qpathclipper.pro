load(qttest_p4)
INCLUDEPATH += .
HEADERS += paths.h
SOURCES  += tst_qpathclipper.cpp paths.cpp

unix:!mac:LIBS+=-lm

DEFINES += QT_USE_USING_NAMESPACE

