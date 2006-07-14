# -*- Mode: makefile -*-
COMMON_FOLDER = $$PWD/../common
include(../arthurtester.pri)
TEMPLATE = app
TARGET = performancediff
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = ../bin

CONFIG += console

QT += xml opengl svg

# Input
HEADERS += performancediff.h
SOURCES += main.cpp performancediff.cpp
