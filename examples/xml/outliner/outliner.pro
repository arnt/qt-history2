TEMPLATE	= app
TARGET		= outliner

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= xml large-config

HEADERS		= outlinetree.h
SOURCES		= main.cpp \
		  outlinetree.cpp
INTERFACES	=
