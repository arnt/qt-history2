TEMPLATE = app
CONFIG += qt
HEADERS = ../environment.h
SOURCES = uninstaller.cpp ../environment.cpp
INTERFACES = uninstall.ui
TARGET  = $(QTDIR)/bin/quninstall
