GUID 		= {4298e9ce-4f94-4389-b619-3ea510c21d08}
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
