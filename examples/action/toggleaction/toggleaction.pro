TEMPLATE	= app
TARGET		= toggleaction

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= toggleaction.cpp
