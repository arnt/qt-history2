SOURCES = qdbus.cpp
DESTDIR = ../../../../bin
TARGET = qdbus
QT = core
CONFIG += qdbus
CONFIG -= app_bundle
win32:CONFIG += console

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
