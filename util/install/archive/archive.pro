CONFIG += staticlib 
CONFIG += qt x11
CONFIG -= dll
TARGET = arq
TEMPLATE = lib

SOURCES += qarchive.cpp ../keygen/keyinfo.cpp
HEADERS += qarchive.h
!zlib:unix:LIBS += -lz
