GUID 		= {4612786e-30b6-459d-927b-2978355ff7a3}
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
