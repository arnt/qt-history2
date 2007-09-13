load(qttest_p4)
SOURCES  += tst_qpointarray.cpp

unix:!mac:LIBS+=-lm

DEFINES += QT_USE_USING_NAMESPACE

