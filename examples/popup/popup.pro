TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= popup.h
SOURCES		= popup.cpp
TARGET		= popup
DEPENDPATH=../../include
QTDIR_build:REQUIRES=large-config
