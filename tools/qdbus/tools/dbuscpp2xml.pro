SOURCES = dbuscpp2xml.cpp
TARGET = ../../../bin/dbuscpp2xml
QT = core
CONFIG += qdbus
target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
