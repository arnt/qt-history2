
CONFIG += staticlib qt 
CONFIG -= dll
TARGET = arq
TEMPLATE = lib
INCLUDEPATH += $(QTDIR)\src\3rdparty

SOURCES += qarchive.cpp
HEADERS += qarchive.h
!zlib:unix:LIBS += -lz
