TEMPLATE	= app
TARGET		= desktop

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= desktop.cpp
