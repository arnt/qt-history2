
TEMPLATE = app
QT = xml core
CONFIG += warn_on console no_batch

TARGET = uic4
DESTDIR = ../../../bin

include(uic.pri)

SOURCES += main.cpp
