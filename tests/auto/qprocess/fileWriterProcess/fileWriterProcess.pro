SOURCES = main.cpp
CONFIG += console
CONFIG -= app_bundle
QT = core
DESTDIR = ./

# no install rule for application used by test
INSTALLS =

DEFINES += QT_USE_USING_NAMESPACE

