mac:CONFIG -= app_bundle
CONFIG += qt
CONFIG += debug console
QT = core

INCLUDEPATH += ..
LIBS += -L.. -lqmake
SOURCES += main.cpp

