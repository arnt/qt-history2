TEMPLATE	= app
TARGET		= launcher

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = embedded large-config

HEADERS		=
SOURCES		= launcher.cpp
