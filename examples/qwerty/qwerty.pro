TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= qwerty.h
SOURCES		= main.cpp \
		  qwerty.cpp
TARGET		= qwerty
DEPENDPATH=../../include
QTDIR_build:REQUIRES=large-config
