REQUIRES        = network large-config
TEMPLATE	= app
CONFIG		+= qt warn_on release

HEADERS		= server.h

SOURCES		= main.cpp \
		  server.cpp	

INTERFACES	= serverbase.ui

TARGET		= infoserver
