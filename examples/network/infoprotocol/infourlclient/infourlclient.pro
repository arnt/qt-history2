QTDIR_build:REQUIRES        = network full-config nocrosscompiler
TEMPLATE	= app
QCONFIG         += network
CONFIG		+= qt warn_on release

HEADERS		= client.h \
		  qip.h

SOURCES		= main.cpp \
		  client.cpp \
		  qip.cpp

INTERFACES	= clientbase.ui

TARGET		= infourlclient
