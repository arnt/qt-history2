# Project ID used by some IDEs
GUID 		= {4b07c961-5d82-4236-abb7-c2021e0bfdb3}
TEMPLATE	= app

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network large-config

HEADERS		+= archivedialog.ui.h
INTERFACES	+= archivedialog.ui
SOURCES		+= main.cpp
