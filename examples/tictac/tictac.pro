TEMPLATE	= app
TARGET		= tictac

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

HEADERS		= tictac.h
SOURCES		= main.cpp \
		  tictac.cpp
