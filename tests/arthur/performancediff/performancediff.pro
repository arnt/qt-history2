# -*- Mode: makefile -*-
COMMON_FOLDER = ../common
include(../arthurtester.pri)
TEMPLATE = app
TARGET = performancediff
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = ../bin

QT += xml opengl svg

# Input
HEADERS += performancediff.h
SOURCES += main.cpp performancediff.cpp
