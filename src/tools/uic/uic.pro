TEMPLATE = app
QT = xml core
CONFIG += warn_on console no_batch
CONFIG -= resource_fork

unix:!contains(QT_CONFIG, zlib):LIBS        += -lz

TARGET = uic
DESTDIR = ../../../bin

include(uic.pri)

SOURCES += main.cpp
