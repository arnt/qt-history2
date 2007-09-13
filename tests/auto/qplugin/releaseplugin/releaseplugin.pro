TEMPLATE = lib
CONFIG += plugin release
CONFIG -= debug debug_and_release
SOURCES = main.cpp
QT = core
DESTDIR = ../plugins

DEFINES += QT_USE_USING_NAMESPACE

