TEMPLATE = app
QT = xml core
QT += compat # until qprocess is done(tm)
CONFIG += warn_on console no_batch
CONFIG -= resource_fork
build_all:CONFIG += release

unix:!contains(QT_CONFIG, zlib):LIBS        += -lz

TARGET = uic
DESTDIR = ../../../bin

include(uic.pri)

SOURCES += main.cpp

target.path=$$bins.path
INSTALLS += target
