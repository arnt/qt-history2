
TEMPLATE = app
QT = xml core
CONFIG += warn_on console no_batch
CONFIG -= resource_fork

TARGET = uic4
DESTDIR = ../../../bin

include(uic.pri)

SOURCES += main.cpp
