TEMPLATE	= app

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network large-config

HEADERS		+= archivedialog.ui.h
INTERFACES	+= archivedialog.ui
SOURCES		+= main.cpp
