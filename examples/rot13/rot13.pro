TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= rot13.h
SOURCES		= rot13.cpp
TARGET		= rot13
DEPENDPATH=../../include
QTDIR_build:REQUIRES=large-config
