TEMPLATE = app
QT = xml core
CONFIG += warn_on console no_batch
CONFIG -= app_bundle
build_all:CONFIG += release

unix:!contains(QT_CONFIG, zlib):LIBS        += -lz

TARGET = uic
DESTDIR = ../../../bin

include(uic.pri)

SOURCES += main.cpp

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
