TEMPLATE	= app
TARGET		= infoserver

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config nocrosscompiler

HEADERS		= server.h \
		  infodata.h
SOURCES		= main.cpp \
		  server.cpp \
		  infodata.cpp
INTERFACES	= serverbase.ui
