TEMPLATE	= app
TARGET		= popup

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= popup.h
SOURCES		= popup.cpp
QT	+= compat
