TEMPLATE	= app
TARGET		= menu

CONFIG		+= qt warn_on release
QCONFIG         += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= menu.h
SOURCES		= menu.cpp
