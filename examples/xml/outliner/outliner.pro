GUID 		= {02830f46-1928-4f1d-912d-3a4573ac2c95}
TEMPLATE	= app
TARGET		= outliner

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= xml large-config

HEADERS		= outlinetree.h
SOURCES		= main.cpp \
		  outlinetree.cpp
INTERFACES	=
