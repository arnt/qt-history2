TEMPLATE	= app
TARGET		= movies

CONFIG		+= qt warn_on release
QT         += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= main.cpp
QT	+= compat
