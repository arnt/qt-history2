TEMPLATE	= app
TARGET		= dirview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, full-config)"

HEADERS		= dirview.h
SOURCES		= dirview.cpp \
		  main.cpp
