TEMPLATE	= app
TARGET		= process

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		=
SOURCES		= process.cpp
INTERFACES	=
