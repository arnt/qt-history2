# Project ID used by some IDEs
GUID 		= {b5efdd44-53c5-4d3f-86af-9ad6207d623f}
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= cannon.h \
		  lcdrange.h
SOURCES		= cannon.cpp \
		  lcdrange.cpp \
		  main.cpp
TARGET		= t9
QTDIR_build:REQUIRES=full-config
