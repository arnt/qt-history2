TEMPLATE	= app
TARGET		= listviews

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= listviews.h
SOURCES		= listviews.cpp \
		  main.cpp
