# -*- Mode: makefile -*-
ARTHUR=../../arthur
COMMON_FOLDER = $$ARTHUR/common
include($$ARTHUR/arthurtester.pri)
TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += . $$ARTHUR

QT += xml svg network

contains(QT_CONFIG, opengl):QT += opengl

include($$ARTHUR/datagenerator/datagenerator.pri)

load(qttest_p4)

# Input
HEADERS += atWrapper.h

SOURCES += atWrapperAutotest.cpp atWrapper.cpp

DEFINES+=ARTHURDIR=\"$$ARTHUR\"
#include($$COMMON_FOLDER/common.pri)
