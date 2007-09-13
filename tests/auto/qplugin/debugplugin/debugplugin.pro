TEMPLATE = lib
CONFIG += plugin debug
CONFIG -= release debug_and_release
SOURCES = main.cpp
QT = core
DESTDIR = ../plugins

DEFINES += QT_USE_USING_NAMESPACE

