# Project ID used by some IDEs
GUID 		= {d86673a5-5b79-4b82-ba48-191771cc687a}
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannon.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t8
QTDIR_build:REQUIRES=large-config
