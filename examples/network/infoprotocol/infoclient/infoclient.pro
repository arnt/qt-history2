QTDIR_build:REQUIRES        = network full-config nocrosscompiler
TEMPLATE	= app
CONFIG		+= qt warn_on release

HEADERS		= client.h

SOURCES		= main.cpp \
		  client.cpp

INTERFACES	= clientbase.ui

TARGET		= infoclient
