#CONFIG += staticlib 
CONFIG += qt 
CONFIG -= dll
TARGET = arq
TEMPLATE = lib
INCLUDEPATH += $(QTDIR)\src\3rdparty $(QTDIR)/util/install/keygen

SOURCES += qarchive.cpp $(QTDIR)/util/install/keygen/keyinfo.cpp
HEADERS += qarchive.h
!zlib:unix:LIBS += -lz
