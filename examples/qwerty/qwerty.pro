TEMPLATE	= app
TARGET		= qwerty

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= qwerty.h
SOURCES		= main.cpp \
		  qwerty.cpp
QT	+= compat
