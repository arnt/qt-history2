# -*- Mode: makefile -*-

# Investigating autotest build error
message("QT_SOURCE_TREE" $$QT_SOURCE_TREE)
message("ARTHUR" $$ARTHUR)
message("PWD" $$PWD)

ARTHUR=$$QT_SOURCE_TREE/tests/arthur
COMMON_FOLDER = $$ARTHUR/common
include($$ARTHUR/arthurtester.pri)
TEMPLATE = app
INCLUDEPATH += $$ARTHUR
DEFINES += SRCDIR=\\\"$$PWD\\\"

QT += qt3support xml svg network

contains(QT_CONFIG, opengl):QT += opengl

include($$ARTHUR/datagenerator/datagenerator.pri)

load(qttest_p4)

# Input
HEADERS += atWrapper.h

SOURCES += atWrapperAutotest.cpp atWrapper.cpp

#include($$COMMON_FOLDER/common.pri)
