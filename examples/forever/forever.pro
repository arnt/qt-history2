TEMPLATE	= app
TARGET		= forever

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, small-config)"

HEADERS		= forever.h
SOURCES		= forever.cpp
QT	+= compat
