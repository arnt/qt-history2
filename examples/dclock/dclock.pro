TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= dclock.h
SOURCES		= dclock.cpp \
		  main.cpp
TARGET		= dclock
DEPENDPATH=../../include
QTDIR_build:REQUIRES=large-config
