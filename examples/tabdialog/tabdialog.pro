TEMPLATE	= app
TARGET		= tabdialog

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= tabdialog.h
SOURCES		= main.cpp \
		  tabdialog.cpp
