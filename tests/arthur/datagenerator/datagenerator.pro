# -*- Mode: makefile -*-
COMMON_FOLDER = $$PWD/../common
include(../arthurtester.pri)
CONFIG+=debug
TEMPLATE = app
TARGET = datagenerator
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = ../bin

QT += svg opengl xml

# Input
HEADERS += datagenerator.h \
	xmlgenerator.h
SOURCES += main.cpp datagenerator.cpp \
	xmlgenerator.cpp 
