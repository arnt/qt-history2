TEMPLATE	= app
TARGET		= splitter

CONFIG		+= qt warn_on release
QT         += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, medium-config)"

HEADERS		=
SOURCES		= splitter.cpp
