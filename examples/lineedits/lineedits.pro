TEMPLATE	= app
TARGET		= lineedits

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, medium-config)"

HEADERS		= lineedits.h
SOURCES		= lineedits.cpp \
		  main.cpp
