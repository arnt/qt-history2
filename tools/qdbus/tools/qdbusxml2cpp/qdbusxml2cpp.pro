SOURCES = qdbusxml2cpp.cpp
DESTDIR = ../../../../bin
TARGET = qdbusxml2cpp
QT = core xml
CONFIG += qdbus
CONFIG -= app_bundle

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
