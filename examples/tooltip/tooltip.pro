TEMPLATE	= app
TARGET		= tooltip

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= tooltip.h
SOURCES		= main.cpp \
		  tooltip.cpp
