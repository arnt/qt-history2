SOURCES = dbuscpp2xml.cpp
DESTDIR = ../../../bin
TARGET = dbuscpp2xml
QT = core xml
CONFIG += qdbus

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
