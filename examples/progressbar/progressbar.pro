TEMPLATE	= app
TARGET		= progressbar

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= progressbar.h
SOURCES		= main.cpp \
		  progressbar.cpp
