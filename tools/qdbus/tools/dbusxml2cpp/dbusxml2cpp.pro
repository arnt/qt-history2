SOURCES = dbusxml2cpp.cpp
DESTDIR = ../../../bin
TARGET = dbusxml2cpp
QT = core xml
CONFIG += qdbus

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
