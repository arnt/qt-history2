TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= outlinetree.h
SOURCES		= main.cpp \
		  outlinetree.cpp
INTERFACES	=
TARGET		= outliner
QTDIR_build:REQUIRES	= xml large-config
