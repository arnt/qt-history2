# Project ID used by some IDEs
GUID 	 = {b137d679-7729-4265-9648-24b72bdfc115}
TEMPLATE = lib
CONFIG += staticlib
CONFIG += qt x11
CONFIG -= dll
TARGET = arq

SOURCES += qarchive.cpp ../keygen/keyinfo.cpp
HEADERS += qarchive.h
!zlib:unix:LIBS += -lz
