# Project ID used by some IDEs
GUID 		= {db3794f0-bb7c-4170-8160-e2b8a296c435}
TEMPLATE	= app
TARGET		= infoclient

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config nocrosscompiler

HEADERS		= client.h
SOURCES		= main.cpp \
		  client.cpp
INTERFACES	= clientbase.ui
