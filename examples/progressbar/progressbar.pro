TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= progressbar.h
SOURCES		= main.cpp \
		  progressbar.cpp
TARGET		= progressbar
DEPENDPATH=../../include
QTDIR_build:REQUIRES=large-config
