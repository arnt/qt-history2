QTDIR_build:REQUIRES        = network large-config
TEMPLATE	= app
QCONFIG         += network
CONFIG		+= qt warn_on release
HEADERS		+= archivedialog.ui.h
INTERFACES	+= archivedialog.ui
SOURCES		+= main.cpp
QTDIR_build:REQUIRES=full-config
