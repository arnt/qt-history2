TEMPLATE	= app
TARGET		= layout

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		=
SOURCES		= layout.cpp
