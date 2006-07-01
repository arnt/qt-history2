SOURCES = dbusxml2cpp.cpp
TARGET = ../../../bin/dbusxml2cpp
QT = core xml
CONFIG += qdbus
target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
