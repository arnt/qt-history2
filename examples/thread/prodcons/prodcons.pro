TEMPLATE	= app
TARGET		= prodcons

CONFIG		+= qt warn_on

QTDIR_build:REQUIRES	= thread large-config

SOURCES		= prodcons.cpp
CLEAN_FILES	= prodcons.out
