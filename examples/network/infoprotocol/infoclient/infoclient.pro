TEMPLATE	= app
TARGET		= infoclient

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config nocrosscompiler

HEADERS		= client.h
SOURCES		= main.cpp \
		  client.cpp
INTERFACES	= clientbase.ui
