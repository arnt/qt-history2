SOURCES = main.cpp
CONFIG -= qt app_bundle
CONFIG += console
DESTDIR = ./

TARGET = "two space s"

# no install rule for application used by test
INSTALLS =

DEFINES += QT_USE_USING_NAMESPACE

