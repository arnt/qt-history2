TEMPLATE	= app
TARGET		= cursor

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= small-config

HEADERS		=
SOURCES		= cursor.cpp

