TEMPLATE	= app
TARGET		= qmag

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= qmag.cpp
