TEMPLATE	= app
TARGET		= progressbar

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= progressbar.h
SOURCES		= main.cpp \
		  progressbar.cpp
QT	+= compat
