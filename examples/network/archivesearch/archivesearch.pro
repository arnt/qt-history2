TEMPLATE	= app

QT         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network large-config

INTERFACES	+= archivedialog.ui
SOURCES		+= main.cpp archivedialog.ui.h
