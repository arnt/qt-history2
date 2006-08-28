SOURCES = dbus.cpp
DESTDIR = ../../../bin
TARGET = dbus
QT = core
CONFIG += qdbus

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
