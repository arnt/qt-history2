TEMPLATE	= app
TARGET		= helpviewer

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= helpwindow.h
SOURCES		= helpwindow.cpp \
		  main.cpp
QT	+= compat
