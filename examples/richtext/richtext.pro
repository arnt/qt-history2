TEMPLATE	= app
TARGET		= richtext

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, medium-config)"

HEADERS		= richtext.h
SOURCES		= main.cpp \
		  richtext.cpp
