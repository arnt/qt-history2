# Project ID used by some IDEs
GUID 		= {6e73fc5c-4c0c-42f9-8a09-eb7f69e949fd}
TEMPLATE	= app
TARGET		= richtext

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= medium-config

HEADERS		= richtext.h
SOURCES		= main.cpp \
		  richtext.cpp
