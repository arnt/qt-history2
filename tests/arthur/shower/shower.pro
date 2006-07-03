# -*- Mode: makefile -*-
COMMON_FOLDER = ../common
include(../arthurtester.pri)
TEMPLATE = app
TARGET = shower
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = ../bin

QT += xml opengl svg

# Input
HEADERS += shower.h
SOURCES += main.cpp shower.cpp
