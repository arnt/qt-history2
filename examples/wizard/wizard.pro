TEMPLATE	= app
TARGET		= wizard

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= wizard.h
SOURCES		= main.cpp \
		  wizard.cpp
