# Project ID used by some IDEs
GUID 		= {ce3faf37-8160-4c5a-9efb-f1c288c8e85f}
TEMPLATE	= app
TARGET		= lineedits

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= medium-config

HEADERS		= lineedits.h
SOURCES		= lineedits.cpp \
		  main.cpp
