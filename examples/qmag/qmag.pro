TEMPLATE	= app
TARGET		= qmag

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= qmag.cpp
QT	+= compat
