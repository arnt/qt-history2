# Project ID used by some IDEs
GUID 		= {9caf9d72-faf9-4673-8830-94d20a373ece}
TEMPLATE	= app
TARGET		= forever

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= small-config

HEADERS		= forever.h
SOURCES		= forever.cpp
