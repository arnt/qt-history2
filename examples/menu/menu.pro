TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= menu.h
SOURCES		= menu.cpp
TARGET		= menu
DEPENDPATH=../../include
QTDIR_build:REQUIRES=large-config
