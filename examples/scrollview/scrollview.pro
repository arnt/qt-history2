TEMPLATE	= app
TARGET		= scrollview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		=
SOURCES		= scrollview.cpp
