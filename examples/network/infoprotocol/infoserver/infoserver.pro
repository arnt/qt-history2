REQUIRES        = network large-config
TEMPLATE	= app
CONFIG		+= qt warn_on release

HEADERS		= server.h \
		  infodata.h

SOURCES		= main.cpp \
		  server.cpp \
		  infodata.cpp	

INTERFACES	= serverbase.ui

TARGET		= infoserver
