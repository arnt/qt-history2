# Project ID used by some IDEs
GUID 	 = {ecccd5bb-8305-4430-9715-4b70de90f597}
TEMPLATE = app
CONFIG += qt
HEADERS = ../environment.h uninstallimpl.h
SOURCES = uninstaller.cpp ../environment.cpp uninstallimpl.cpp
INTERFACES = uninstall.ui
TARGET  = quninstall
DESTDIR = ../../../../dist/win/bin
