SOURCES = dbus.cpp
TARGET = ../../../bin/dbus
QT = core
CONFIG += qdbus
target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
