TARGET = rcc
CONFIG += console
QT = xml core
DESTDIR = ../../../bin
mac:CONFIG -= resource_fork
build_all:CONFIG += release

unix:!contains(QT_CONFIG, zlib):LIBS        += -lz

SOURCES += main.cpp

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
