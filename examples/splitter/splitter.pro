TEMPLATE	= app
TARGET		= splitter

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= medium-config

HEADERS		=
SOURCES		= splitter.cpp
