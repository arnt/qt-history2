# Project ID used by some IDEs
GUID 		= {e7b392ac-2263-40c5-8ad8-bde9a4d9e40f}
TEMPLATE	= app
TARGET		= dclock

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= dclock.h
SOURCES		= dclock.cpp \
		  main.cpp
