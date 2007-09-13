load(qttest_p4)
SOURCES  += tst_qwmatrix.cpp

unix:!mac:LIBS+=-lm

DEFINES += QT_USE_USING_NAMESPACE

