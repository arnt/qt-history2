# -*- Mode: makefile -*-
COMMON_FOLDER = ../common
include(../arthurtester.pri)
TEMPLATE = lib
CONFIG += static
QT += xml opengl svg qt3support

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

TARGET = testcommon

include(common.pri)
