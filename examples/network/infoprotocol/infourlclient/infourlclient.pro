TEMPLATE	= app
TARGET		= infourlclient

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config nocrosscompiler

HEADERS		= client.h \
		  qip.h
SOURCES		= main.cpp \
		  client.cpp \
		  qip.cpp
INTERFACES	= clientbase.ui
