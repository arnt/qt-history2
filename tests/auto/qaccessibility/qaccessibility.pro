load(qttest_p4)
SOURCES  += tst_qaccessibility.cpp

unix:!mac:LIBS+=-lm
QT += qt3support
