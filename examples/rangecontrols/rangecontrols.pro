TEMPLATE	= app
TARGET		= rangecontrols

CONFIG		+= qt warn_on release
QT         += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= rangecontrols.h
SOURCES		= main.cpp \
		  rangecontrols.cpp
