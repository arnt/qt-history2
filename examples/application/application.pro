TEMPLATE	= app
TARGET		= application

CONFIG		+= qt warn_on release
DEFINES         += QT_COMPAT_WARNINGS
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
