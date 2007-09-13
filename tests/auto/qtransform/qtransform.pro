load(qttest_p4)
HEADERS += 
SOURCES  += tst_qtransform.cpp

unix:!mac:LIBS+=-lm

DEFINES += QT_USE_USING_NAMESPACE

