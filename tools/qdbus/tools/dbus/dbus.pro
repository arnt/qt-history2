SOURCES = dbus.cpp
DESTDIR = ../../../bin
TARGET = dbus
QT = core
CONFIG += qdbus
CONFIG -= app_bundle

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
