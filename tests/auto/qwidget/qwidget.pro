load(qttest_p4)
SOURCES  += tst_qwidget.cpp


QT += qt3support

aix-g++*:QMAKE_CXXFLAGS+=-fpermissive

CONFIG += x11inc
win32: LIBS += -luser32 -lgdi32
