SOURCES = main.cpp
CONFIG -= qt app_bundle
CONFIG += console
DESTDIR = ./

# no install rule for application used by test
INSTALLS =

DEFINES += QT_USE_USING_NAMESPACE

