TEMPLATE	= app
TARGET		= application

CONFIG		+= qt warn_on release
QT += compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
