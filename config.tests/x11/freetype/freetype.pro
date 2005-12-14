SOURCES = freetype.cpp
CONFIG += x11
CONFIG -= qt
LIBS += -lfreetype
include(../fontconfig/fontconfig.pri)
