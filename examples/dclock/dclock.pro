TEMPLATE	= app
TARGET		= dclock

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= dclock.h
SOURCES		= dclock.cpp \
		  main.cpp
