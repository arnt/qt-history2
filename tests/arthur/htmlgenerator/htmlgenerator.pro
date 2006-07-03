# -*- Mode: makefile -*-
COMMON_FOLDER = ../common
include(../arthurtester.pri)
TEMPLATE = app
TARGET = htmlgenerator
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = ../bin

QT += svg opengl xml

# Input
HEADERS += htmlgenerator.h
SOURCES += main.cpp htmlgenerator.cpp
