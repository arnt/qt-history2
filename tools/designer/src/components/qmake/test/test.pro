mac:CONFIG -= resource_fork
CONFIG += qt
CONFIG += debug console
QT = core

INCLUDEPATH += ..
LIBS += -L.. -lqmake
SOURCES += main.cpp

