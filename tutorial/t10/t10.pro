# Project ID used by some IDEs
GUID 		= {bcd35c16-b0d6-4ca5-b72f-8f7853b95bf0}
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannon.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t10
QTDIR_build:REQUIRES=full-config
