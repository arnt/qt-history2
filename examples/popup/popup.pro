# Project ID used by some IDEs
GUID 		= {93616725-aee4-4bbc-b2ab-7eea3ec44528}
TEMPLATE	= app
TARGET		= popup

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= popup.h
SOURCES		= popup.cpp
