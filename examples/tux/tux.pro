TEMPLATE	= app
TARGET		= tux

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, small-config)"

HEADERS		=
SOURCES		= tux.cpp
INTERFACES	=
