# Project ID used by some IDEs
GUID 		= {f122a764-796a-4a33-adae-cd9114d54288}
TEMPLATE	= app
TARGET		= process

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		=
SOURCES		= process.cpp
INTERFACES	=
